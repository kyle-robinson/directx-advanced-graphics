#include "stdafx.h"
#include "Level1.h"
#include "TextRenderer.h"

void Level1::OnCreate()
{
    // Initialize systems
    m_postProcessing.Initialize( m_gfx->GetDevice() );

    // Initialize constant buffers
    HRESULT hr = m_cbMatrices.Initialize( m_gfx->GetDevice(), m_gfx->GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'Matrices' constant buffer!" );

    hr = m_cbMatricesNormalDepth.Initialize( m_gfx->GetDevice(), m_gfx->GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'Matrices Normal Depth' constant buffer!" );

    // Initialize game objects
    hr = m_cube.InitializeMesh( m_gfx->GetDevice(), m_gfx->GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'cube' object!" );

    hr = m_light.Initialize( m_gfx->GetDevice(), m_gfx->GetContext(), m_cbMatrices );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'light' object!" );

    hr = m_mapping.Initialize( m_gfx->GetDevice(), m_gfx->GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'mapping' system!" );

    hr = m_motionBlur.Initialize( m_gfx->GetDevice(), m_gfx->GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'motion blur' system!" );

    hr = m_fxaa.Initialize( m_gfx->GetDevice(), m_gfx->GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'FXAA' system!" );

    hr = m_ssao.Initialize( m_gfx->GetDevice(), m_gfx->GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'SSAO' system!" );

    hr = m_deferred.Initialize( m_gfx->GetDevice(), m_gfx->GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'deferred' system!" );

#if defined ( _x64 )
    // Initialize models
    if ( !m_objSkysphere.Initialize( "Resources\\Models\\sphere.obj", m_gfx->GetDevice(), m_gfx->GetContext(), m_cbMatrices ) )
        return ;
    m_objSkysphere.SetInitialScale( 50.0f, 50.0f, 50.0f );
#endif
}

void Level1::OnSwitch()
{

}

void Level1::RenderFrame()
{
    std::function<void( bool, bool )> RenderScene = [&]( bool useDeferred, bool useGBuffer ) -> void
    {
#if defined ( _x64 )
        // Render skyphere first
        m_gfx->UpdateRenderStateSkysphere();
        m_objSkysphere.Draw( m_camera->GetViewMatrix(), m_camera->GetProjectionMatrix() );
#endif

        // Update constant buffers
        m_light.UpdateCB( *m_camera );
        m_deferred.UpdateCB();
        m_mapping.UpdateCB();
        m_cube.UpdateCB();

        // Render objects
        m_gfx->UpdateRenderStateCube( useDeferred, useGBuffer );
        m_cube.UpdateBuffers( m_cbMatrices, *m_camera );
        m_gfx->GetContext()->VSSetConstantBuffers( 0u, 1u, m_cbMatrices.GetAddressOf() );
        m_gfx->GetContext()->PSSetConstantBuffers( 0u, 1u, m_cube.GetCB() );
        m_gfx->GetContext()->PSSetConstantBuffers( 1u, 1u, m_light.GetCB_DPtr() );
        m_gfx->GetContext()->PSSetConstantBuffers( 2u, 1u, m_mapping.GetCB() );
        if ( useDeferred && useGBuffer )
        {
            m_gfx->GetContext()->PSSetConstantBuffers( 3u, 1u, m_deferred.GetCB() );
            m_cube.DrawDeferred( m_gfx->GetContext(),
                m_gfx->GetDeferredRenderTarget( Bind::RenderTarget::Type::POSITION )->GetShaderResourceViewPtr(),
                m_gfx->GetDeferredRenderTarget( Bind::RenderTarget::Type::ALBEDO )->GetShaderResourceViewPtr(),
                m_gfx->GetDeferredRenderTarget( Bind::RenderTarget::Type::NORMAL )->GetShaderResourceViewPtr() );
        }
        else
        {
            m_cube.Draw( m_gfx->GetContext() );
        }

#if defined ( _x64 )
        m_gfx->UpdateRenderStateTexture();
        m_light.Draw( m_camera->GetViewMatrix(), m_camera->GetProjectionMatrix() );
#endif
    };

    if ( m_deferred.IsActive() )
    {
        // Normal pass
        m_gfx->BeginFrameDeferred();
        RenderScene( true, false );
    }

    if ( m_ssao.IsActive() )
    {
        // Normal pass
        m_gfx->BeginFrameNormal();
        RenderScene( m_deferred.IsActive(), true );

        // Update normal/depth constant buffer
        MatricesNormalDepth mndData;
        mndData.mWorld = XMMatrixIdentity();
        mndData.mView = XMMatrixTranspose( m_camera->GetViewMatrix() );
        mndData.mProjection = XMMatrixTranspose( m_camera->GetProjectionMatrix() );
        mndData.mWorldInvTransposeView = XMMatrixTranspose( XMMatrixInverse( nullptr, mndData.mWorld ) ) * mndData.mView;

        // Add to constant buffer
        m_cbMatricesNormalDepth.data = mndData;
        if ( !m_cbMatricesNormalDepth.ApplyChanges() ) return;
        m_gfx->RenderSceneToTextureNormal( m_cbMatricesNormalDepth.GetAddressOf() );
    }

    // Standard pass
    m_gfx->BeginFrame();
    RenderScene( m_deferred.IsActive(), true );
}

void Level1::EndFrame_Start()
{
    // Setup motion blur
    XMMATRIX viewProjInv = XMMatrixInverse( nullptr,
        XMMatrixTranspose( m_camera->GetViewMatrix() ) *
        XMMatrixTranspose( m_camera->GetProjectionMatrix() ) );
    m_motionBlur.SetViewProjInv( viewProjInv );
    XMMATRIX prevViewProj = XMLoadFloat4x4( &m_previousViewProjection );
    m_motionBlur.SetPrevViewProj( prevViewProj );
    m_motionBlur.UpdateCB();

    // Setup FXAA
    m_fxaa.UpdateCB( m_gfx->GetWidth(), m_gfx->GetHeight() );

    // Setup SSAO
    m_ssao.UpdateCB( m_gfx->GetWidth(), m_gfx->GetHeight(), *m_camera );

    // Render text
    if ( !m_motionBlur.IsActive() && !m_fxaa.IsActive() && !m_ssao.IsActive() )
    {
        static XMFLOAT2 textPosition = { m_gfx->GetWidth() * 0.5f, m_gfx->GetHeight() * 0.96f };
#if defined( _x64 )
        XMVECTORF32 color = Colors::Green;
        std::string text = "[x64] Assimp lib found! Complex models in use!";
#elif defined( _x86 )
        XMVECTORF32 color = Colors::Red;
        std::string text = "[x86] Assimp lib not found! Complex models removed!";
#endif
		TextRenderer::GetInstance()->RenderString( text, textPosition, color, true );
    }

    // Render scene to texture
    m_gfx->BeginRenderSceneToTexture();
    ( m_motionBlur.IsActive() || m_fxaa.IsActive() || m_ssao.IsActive() ) ?
        m_gfx->RenderSceneToTexture( m_motionBlur.GetCB(), m_fxaa.GetCB(), m_ssao.GetCB(), m_ssao.GetNoiseTexture() ) :
        m_postProcessing.Bind( m_gfx->GetContext(), m_gfx->GetRenderTarget() );

    // Render imgui windows
    m_imgui->BeginRender();
    m_imgui->SpawnInstructionWindow();
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
}

void Level1::EndFrame_End()
{
    m_imgui->EndRender();

    // Present frame
    m_gfx->EndFrame();

    // Store current viewProj for next render pass
    XMStoreFloat4x4( &m_previousViewProjection,
        XMMatrixTranspose( m_camera->GetViewMatrix() ) *
        XMMatrixTranspose( m_camera->GetProjectionMatrix() ) );
}

void Level1::Update( const float dt )
{
 #if defined ( _x64 )
    // Update skysphere position
    m_objSkysphere.SetPosition( m_camera->GetPositionFloat3() );
#endif

    // Update the cube transform, material etc.
    m_cube.Update( dt );
}