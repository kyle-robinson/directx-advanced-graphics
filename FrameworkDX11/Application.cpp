#include "stdafx.h"
#include "Application.h"

bool Application::Initialize( HINSTANCE hInstance, int width, int height )
{
    try
    {
        // Initialize window
        if ( !renderWindow.Initialize( &m_input, hInstance, "DirectX 11 Physics Framework", "TutorialWindowClass", width, height ) )
		    return false;

        // Initialize graphics
        if ( !graphics.Initialize( renderWindow.GetHWND(), width, height ) )
		    return false;

        // Initialize systems
        m_imgui.Initialize( renderWindow.GetHWND(), graphics.GetDevice(), graphics.GetContext() );
        m_postProcessing.Initialize( graphics.GetDevice() );

        // Initialize input
        m_camera.Initialize( XMFLOAT3( 0.0f, 0.0f, -3.0f ), width, height );
        m_input.Initialize( renderWindow, m_camera );

        // Initialize constant buffers
        HRESULT hr = m_cbMatrices.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'Matrices' constant buffer!" );

        // Initialize game objects
	    hr = m_cube.InitializeMesh( graphics.GetDevice(), graphics.GetContext() );
        COM_ERROR_IF_FAILED(hr, "Failed to create 'cube' object!");

        hr = m_light.Initialize( graphics.GetDevice(), graphics.GetContext() );
	    COM_ERROR_IF_FAILED( hr, "Failed to create 'light' object!" );
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

    // Update the cube transform, material etc. 
    m_cube.Update( dt, graphics.GetContext() );
}

void Application::Render()
{
    // Setup graphics
    graphics.BeginFrame();

    // Get the game object world transform
    DirectX::XMMATRIX mGO = XMLoadFloat4x4( m_cube.GetTransform() );

    // Store this and the view / projection in a constant buffer for the vertex shader to use
	m_cbMatrices.data.mWorld = DirectX::XMMatrixTranspose( mGO );
	m_cbMatrices.data.mView = DirectX::XMMatrixTranspose( m_camera.GetViewMatrix() );
	m_cbMatrices.data.mProjection = DirectX::XMMatrixTranspose( m_camera.GetProjectionMatrix() );
	m_cbMatrices.data.vOutputColor = DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
	if ( !m_cbMatrices.ApplyChanges() ) return;
    
    // Update light constant buffer
    m_light.UpdateCB( graphics.GetContext(), m_camera );

    // Render objects
    graphics.GetContext()->VSSetConstantBuffers(0u, 1u, m_cbMatrices.GetAddressOf() );
    graphics.GetContext()->PSSetConstantBuffers( 1u, 1u, m_cube.GetMaterialCB() );
    graphics.GetContext()->PSSetConstantBuffers( 2u, 1u, m_light.GetLightCB() );
    m_cube.Draw( graphics.GetContext() );

    // Render scene to texture
    graphics.RenderSceneToTexture();
    m_postProcessing.Bind( graphics.GetContext(), graphics.GetRenderTarget() );

    // Render imgui windows
    m_imgui.BeginRender();
    m_imgui.SpawnInstructionWindow();
    m_postProcessing.SpawnControlWindow();
    m_imgui.EndRender();

    // Present frame
    graphics.EndFrame();
}