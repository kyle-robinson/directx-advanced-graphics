#include "stdafx.h"
#include "Application.h"

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
        m_postProcessing.Initialize( graphics.GetDevice() );

        hr = m_mapping.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'mapping' system!" );

        hr = m_motionBlur.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'motion blur' system!" );

        hr = m_fxaa.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'FXAA' system!" );

        hr = m_ssao.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'SSAO' system!" );

        // Initialize models
        if ( !m_objSkysphere.Initialize( "Resources\\Models\\sphere.obj", graphics.GetDevice(), graphics.GetContext(), m_cbMatrices ) )
		    return false;
        m_objSkysphere.SetInitialScale( 50.0f, 50.0f, 50.0f );
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
    // Usefult for finding dx memory leaks
    ID3D11Debug* debugDevice = nullptr;
    graphics.GetDevice()->QueryInterface( __uuidof(ID3D11Debug), reinterpret_cast<void**>( &debugDevice ) );
    debugDevice->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
    if ( debugDevice ) debugDevice->Release();
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

    // Update skysphere position
    m_objSkysphere.SetPosition( m_camera.GetPositionFloat3() );

    // Update the cube transform, material etc. 
    m_cube.Update( dt, graphics.GetContext() );
}

void Application::Render()
{
#pragma region NORMAL_DEPTH_PASS
    // Normal pass
    graphics.BeginFrameNormal();
    // Render skyphere first
    graphics.UpdateRenderStateSkysphere();
    m_objSkysphere.Draw( m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix() );
    
    // Update constant buffers
    m_light.UpdateCB( m_camera );
    m_mapping.UpdateCB();
    m_cube.UpdateCB();

    // Render objects
    graphics.UpdateRenderStateCube();
    m_cube.UpdateBuffers( m_cbMatrices, m_camera );
    graphics.GetContext()->VSSetConstantBuffers( 0u, 1u, m_cbMatrices.GetAddressOf() );
    graphics.GetContext()->PSSetConstantBuffers( 1u, 1u, m_cube.GetCB() );
    graphics.GetContext()->PSSetConstantBuffers( 2u, 1u, m_light.GetCB() );
    graphics.GetContext()->PSSetConstantBuffers( 3u, 1u, m_mapping.GetCB() );
    m_cube.Draw( graphics.GetContext() );

    graphics.UpdateRenderStateTexture();
    m_light.Draw( m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix() );

    // Update normal/depth constant buffer
    MatricesNormalDepth mndData;
    mndData.mWorld = XMMatrixIdentity();
    mndData.mView = XMMatrixTranspose( m_camera.GetViewMatrix() );
    mndData.mProjection = XMMatrixTranspose( m_camera.GetProjectionMatrix() );
    mndData.mWorldInvTransposeView = XMMatrixTranspose( XMMatrixInverse( nullptr, mndData.mWorld ) ) * mndData.mView;

    // Add to constant buffer
    m_cbMatricesNormalDepth.data = mndData;
    if ( !m_cbMatricesNormalDepth.ApplyChanges() ) return;
    graphics.RenderSceneToTextureNormalDepth( m_cbMatricesNormalDepth.GetAddressOf() );
#pragma endregion

#pragma MAIN_RENDER_PASS
    // Standard pass
    graphics.BeginFrame();

    // Render skyphere first
    graphics.UpdateRenderStateSkysphere();
    m_objSkysphere.Draw( m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix() );
    
    // Update constant buffers
    m_light.UpdateCB( m_camera );
    m_mapping.UpdateCB();
    m_cube.UpdateCB();

    // Render objects
    graphics.UpdateRenderStateCube();
    m_cube.UpdateBuffers( m_cbMatrices, m_camera );
    graphics.GetContext()->VSSetConstantBuffers( 0u, 1u, m_cbMatrices.GetAddressOf() );
    graphics.GetContext()->PSSetConstantBuffers( 1u, 1u, m_cube.GetCB() );
    graphics.GetContext()->PSSetConstantBuffers( 2u, 1u, m_light.GetCB() );
    graphics.GetContext()->PSSetConstantBuffers( 3u, 1u, m_mapping.GetCB() );
    m_cube.Draw( graphics.GetContext() );

    graphics.UpdateRenderStateTexture();
    m_light.Draw( m_camera.GetViewMatrix(), m_camera.GetProjectionMatrix() );
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
    m_mapping.SpawnControlWindow();
    m_light.SpawnControlWindow();
    m_cube.SpawnControlWindow();
    m_imgui.EndRender();

    // Present frame
    graphics.EndFrame();

    // Store current viewProj for next render pass
     XMStoreFloat4x4( &m_previousViewProjection,
        XMMatrixTranspose( m_camera.GetViewMatrix() ) *
        XMMatrixTranspose( m_camera.GetProjectionMatrix() ) );
#pragma endregion
}