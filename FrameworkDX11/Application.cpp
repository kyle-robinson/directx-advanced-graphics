#include "stdafx.h"
#include "Application.h"
#include <imgui/imgui.h>

bool Application::Initialize( HINSTANCE hInstance, int width, int height )
{
    try
    {
        // Initialize window
        if ( !renderWindow.Initialize( &m_input, hInstance, "DirectX 11 Advanced Graphics & Rendering", "TutorialWindowClass", width, height ) )
		    return false;

        // Initialize graphics
        if ( !graphics.Initialize( renderWindow.GetHWND(), width, height ) )
		    return false;

        // Initialize input
        m_camera.Initialize( XMFLOAT3( 0.0f, 0.0f, -3.0f ), width, height );
        m_input.Initialize( renderWindow, m_camera );
        m_imgui.Initialize( renderWindow.GetHWND(), graphics.GetDevice(), graphics.GetContext() );

        // Initialize constant buffers
        HRESULT hr = m_cbMatrices.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'Matrices' constant buffer!" );

        hr = m_cbMatricesNormalDepth.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'Matrices Normal Depth' constant buffer!" );

        // Initialize game objects
	    hr = m_cube.InitializeMesh( graphics.GetDevice(), graphics.GetContext() );
        COM_ERROR_IF_FAILED(hr, "Failed to create 'cube' object!");

        hr = m_light.Initialize( graphics.GetDevice(), graphics.GetContext(), m_cbMatrices );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'light' object!" );

        // Initialize systems
        m_spriteFont = std::make_unique<SpriteFont>( graphics.GetDevice(), L"Resources\\Fonts\\open_sans_ms_16_bold.spritefont" );
        m_spriteBatch = std::make_unique<SpriteBatch>( graphics.GetContext() );
        m_postProcessing.Initialize( graphics.GetDevice() );

        hr = m_mapping.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'mapping' system!" );

        hr = m_motionBlur.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'motion blur' system!" );

        hr = m_fxaa.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'FXAA' system!" );

        hr = m_ssao.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'SSAO' system!" );

        hr = m_deferred.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'deferred' system!" );

#if defined ( _x64 )
        // Initialize models
        if ( !m_objSkysphere.Initialize( "Resources\\Models\\sphere.obj", graphics.GetDevice(), graphics.GetContext(), m_cbMatrices ) )
		    return false;
        m_objSkysphere.SetInitialScale( 50.0f, 50.0f, 50.0f );
#endif
    }
    catch ( COMException& exception )
	{
		ErrorLogger::Log( exception );
		return false;
	}

    return true;
}

void Application::CleanupDevice()
{
#ifdef _DEBUG
    // Useful for finding dx memory leaks
    ID3D11Debug* debugDevice = nullptr;
    graphics.GetDevice()->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &debugDevice ) );
    debugDevice->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
    if ( debugDevice ) debugDevice->Release();
#endif
}

bool Application::ProcessMessages() noexcept
{
    // Process messages sent to the window
	return renderWindow.ProcessMessages();
}

void Application::Update()
{
    // Update delta time
    float dt = m_timer.GetDeltaTime(); // capped at 60 fps
    if ( dt == 0.0f ) return;

    // Update input
    m_input.Update( dt );
    if ( windowResized )
    {
        m_imgui.Initialize( renderWindow.GetHWND(), graphics.GetDevice(), graphics.GetContext() );
        m_camera.SetProjectionValues( 75.0f,
            static_cast<float>( graphics.GetWidth() ) /
            static_cast<float>( graphics.GetHeight() ),
            0.01f, 100.0f );
    }

#if defined ( _x64 )
    // Update skysphere position
    m_objSkysphere.SetPosition( m_camera.GetPositionFloat3() );
#endif

    // Update the cube transform, material etc. 
    m_cube.Update( dt );
}

void Application::Render()
{
#pragma RENDER_PASSES
    std::function<void( bool, bool, bool )> RenderScene = [&]( bool useDeferred, bool useGBuffer ) -> void
    {
#if defined ( _x64 )
        // Render skyphere first
        graphics.UpdateRenderStateSkysphere();
        m_objSkysphere.Draw( m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix() );
#endif
    
        // Update constant buffers
        m_light.UpdateCB( m_camera );
        m_deferred.UpdateCB();
        m_mapping.UpdateCB();
        m_cube.UpdateCB();

        // Render objects
        graphics.UpdateRenderStateCube( useDeferred, useGBuffer );
        m_cube.UpdateBuffers( m_cbMatrices, m_camera );
        graphics.GetContext()->VSSetConstantBuffers( 0u, 1u, m_cbMatrices.GetAddressOf() );
        graphics.GetContext()->PSSetConstantBuffers( 0u, 1u, m_cube.GetCB() );
        graphics.GetContext()->PSSetConstantBuffers( 1u, 1u, m_light.GetCB_DPtr() );
        graphics.GetContext()->PSSetConstantBuffers( 2u, 1u, m_mapping.GetCB() );
        if ( useDeferred && useGBuffer )
        {
            graphics.GetContext()->PSSetConstantBuffers( 3u, 1u, m_deferred.GetCB() );
            m_cube.DrawDeferred( graphics.GetContext(),
                graphics.GetDeferredRenderTarget( Bind::RenderTarget::Type::POSITION )->GetShaderResourceViewPtr(),
                graphics.GetDeferredRenderTarget( Bind::RenderTarget::Type::ALBEDO )->GetShaderResourceViewPtr(),
                graphics.GetDeferredRenderTarget( Bind::RenderTarget::Type::NORMAL )->GetShaderResourceViewPtr() );
        }
        else
        {
            m_cube.Draw( graphics.GetContext() );
        }

#if defined ( _x64 )
        graphics.UpdateRenderStateTexture();
        m_light.Draw( m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix() );
#endif
    };

    if ( m_deferred.IsActive() )
    {
        // Normal pass
        graphics.BeginFrameDeferred();
        RenderScene( false, true, false );
    }

    if ( m_ssao.IsActive() )
    {
        // Normal pass
        graphics.BeginFrameNormal();
        RenderScene( false, m_deferred.IsActive(), true );

        // Update normal/depth constant buffer
        MatricesNormalDepth mndData;
        mndData.mWorld = XMMatrixIdentity();
        mndData.mView = XMMatrixTranspose( m_camera.GetViewMatrix() );
        mndData.mProjection = XMMatrixTranspose( m_camera.GetProjectionMatrix() );
        mndData.mWorldInvTransposeView = XMMatrixTranspose( XMMatrixInverse( nullptr, mndData.mWorld ) ) * mndData.mView;

        // Add to constant buffer
        m_cbMatricesNormalDepth.data = mndData;
        if ( !m_cbMatricesNormalDepth.ApplyChanges() ) return;
        graphics.RenderSceneToTextureNormal( m_cbMatricesNormalDepth.GetAddressOf() );
    }

    // Standard pass
    graphics.BeginFrame();
    RenderScene( false, m_deferred.IsActive(), true );
#pragma endregion

#pragma region POST_PROCESSING
    // Setup motion blur
    XMMATRIX viewProjInv = XMMatrixInverse( nullptr, XMMatrixTranspose( m_camera.GetViewMatrix() ) * XMMatrixTranspose( m_camera.GetProjectionMatrix() ) );
    m_motionBlur.SetViewProjInv( viewProjInv );
    XMMATRIX prevViewProj = XMLoadFloat4x4( &m_previousViewProjection );
    m_motionBlur.SetPrevViewProj( prevViewProj );
    m_motionBlur.UpdateCB();

    // Setup FXAA
    m_fxaa.UpdateCB( graphics.GetWidth(), graphics.GetHeight() );

    // Setup SSAO
    m_ssao.UpdateCB( graphics.GetWidth(), graphics.GetHeight(), m_camera );

    // Render text
    if ( !m_motionBlur.IsActive() && !m_fxaa.IsActive() && !m_ssao.IsActive() )
    {
        m_spriteBatch->Begin();
        static XMFLOAT2 textPosition = { graphics.GetWidth() * 0.5f, graphics.GetHeight() * 0.96f };
        std::function<XMFLOAT2( const wchar_t* )> DrawOutline = [&]( const wchar_t* text ) mutable -> XMFLOAT2
        {
            XMFLOAT2 originF = XMFLOAT2( 1.0f, 1.0f );
            XMVECTOR origin = m_spriteFont->MeasureString( text ) / 2.0f;
            XMStoreFloat2( &originF, origin );

            // Draw outline
            m_spriteFont->DrawString( m_spriteBatch.get(), text,
                XMFLOAT2( textPosition.x + 1.0f, textPosition.y + 1.0f ), Colors::Black, 0.0f, originF );
            m_spriteFont->DrawString( m_spriteBatch.get(), text,
                XMFLOAT2( textPosition.x - 1.0f, textPosition.y + 1.0f ), Colors::Black, 0.0f, originF );
            m_spriteFont->DrawString( m_spriteBatch.get(), text,
                XMFLOAT2( textPosition.x - 1.0f, textPosition.y - 1.0f ), Colors::Black, 0.0f, originF );
            m_spriteFont->DrawString( m_spriteBatch.get(), text,
                XMFLOAT2( textPosition.x + 1.0f, textPosition.y - 1.0f ), Colors::Black, 0.0f, originF );

            return originF;
        };
#if defined( _x64 )
        const wchar_t* text = L"[x64] Assimp lib found! Complex models in use!";
#elif defined( _x86 )
        const wchar_t* text = L"[x86] Assimp lib not found! Complex models removed!";
#endif
        XMFLOAT2 originF = DrawOutline( text );
        m_spriteFont->DrawString( m_spriteBatch.get(), text, textPosition,
#if defined( _x64 )
            Colors::Green,
#elif defined( _x86 )
            Colors::Red,
#endif
            0.0f, originF, XMFLOAT2( 1.0f, 1.0f ) );
        m_spriteBatch->End();
    }

    // Render scene to texture
    graphics.BeginRenderSceneToTexture();
    ( m_motionBlur.IsActive() || m_fxaa.IsActive() || m_ssao.IsActive() ) ?
        graphics.RenderSceneToTexture( m_motionBlur.GetCB(), m_fxaa.GetCB(), m_ssao.GetCB(), m_ssao.GetNoiseTexture() ) :
        m_postProcessing.Bind( graphics.GetContext(), graphics.GetRenderTarget() );

    // Render imgui windows
    m_imgui.BeginRender();
    m_imgui.SpawnInstructionWindow();
    m_motionBlur.SpawnControlWindow( m_fxaa.IsActive(), m_ssao.IsActive() );
    m_fxaa.SpawnControlWindow( m_motionBlur.IsActive(), m_ssao.IsActive() );
    m_ssao.SpawnControlWindow( m_motionBlur.IsActive(), m_fxaa.IsActive() );
    m_postProcessing.SpawnControlWindow(
        m_motionBlur.IsActive(),
        m_fxaa.IsActive(),
        m_ssao.IsActive() );
    m_deferred.SpawnControlWindow();
    m_mapping.SpawnControlWindow( m_deferred.IsActive() );
    m_light.SpawnControlWindow();
    m_cube.SpawnControlWindows();
    m_imgui.EndRender();

    // Present frame
    graphics.EndFrame();

    // Store current viewProj for next render pass
     XMStoreFloat4x4( &m_previousViewProjection,
        XMMatrixTranspose( m_camera.GetViewMatrix() ) *
        XMMatrixTranspose( m_camera.GetProjectionMatrix() ) );
#pragma endregion
}