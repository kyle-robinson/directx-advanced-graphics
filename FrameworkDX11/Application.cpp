#include "stdafx.h"
#include "Application.h"

// Register class and create window
bool Application::Initialize( HINSTANCE hInstance, int width, int height )
{
    // Initialize window
    if ( !renderWindow.Initialize( &m_input, hInstance, "DirectX 11 Physics Framework", "TutorialWindowClass", width, height ) )
		return false;

    // Initialize graphics
    if ( !graphics.Initialize( renderWindow.GetHWND(), width, height ) )
		return false;

    // Initialize imgui
    m_imgui.Initialize( renderWindow.GetHWND(), graphics.GetDevice(), graphics.GetContext() );

    // Initialize input
    m_camera.Initialize( XMFLOAT3( 0.0f, 0.0f, -3.0f ), width, height );
    m_input.Initialize( renderWindow, m_camera );

    // Initialize constant buffers
    HRESULT hr = m_cbMatrices.Initialize( graphics.GetDevice(), graphics.GetContext() );
	COM_ERROR_IF_FAILED( hr, "Failed to create 'Matrices' constant buffer!" );

    hr = m_cbLight.Initialize( graphics.GetDevice(), graphics.GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'Light' constant buffer!" );

    // Initialize game object
	hr = m_gameObject.InitializeMesh( graphics.GetDevice(), graphics.GetContext() );
	COM_ERROR_IF_FAILED( hr, "Failed to create game object!" );

    return true;
}

void Application::CleanupDevice()
{
    m_gameObject.Cleanup();

    // Usefult for finding dx memory leaks
    ID3D11Debug* debugDevice = nullptr;
    graphics.GetDevice()->QueryInterface( __uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice) );
    debugDevice->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );
    if ( debugDevice ) debugDevice->Release();
}

void Application::setupLightForRender()
{
    // Setup light data
    Light light;
    light.Enabled = static_cast<int>(true);
    light.LightType = PointLight;
    light.Color = DirectX::XMFLOAT4( DirectX::Colors::White );
    light.SpotAngle = DirectX::XMConvertToRadians( 45.0f );
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = 1.0f;
    light.QuadraticAttenuation = 1.0f;

    // Setup light
    XMFLOAT4 eyePosition = {
        m_camera.GetPositionFloat3().x,
        m_camera.GetPositionFloat3().y,
        m_camera.GetPositionFloat3().z,
        1.0f
    };
    DirectX::XMFLOAT4 LightPosition( eyePosition );
    light.Position = LightPosition;
    DirectX::XMVECTOR LightDirection = DirectX::XMVectorSet(
        m_camera.GetCameraTarget().x - LightPosition.x,
        m_camera.GetCameraTarget().y - LightPosition.y,
        m_camera.GetCameraTarget().z - LightPosition.z,
        0.0f
    );
    LightDirection = DirectX::XMVector3Normalize( LightDirection );
    DirectX::XMStoreFloat4( &light.Direction, LightDirection );

    // Add to constant buffer
    m_cbLight.data.EyePosition = LightPosition;
    m_cbLight.data.Lights[0] = light;
    if ( !m_cbLight.ApplyChanges() ) return;
}

bool Application::ProcessMessages() noexcept
{
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
    m_gameObject.Update( dt, graphics.GetContext() );
}

// Render a frame
void Application::Render()
{
    // Setup graphics
    graphics.BeginFrame();

    // get the game object world transform
    DirectX::XMMATRIX mGO = XMLoadFloat4x4( m_gameObject.GetTransform() );

    // store this and the view / projection in a constant buffer for the vertex shader to use
	m_cbMatrices.data.mWorld = DirectX::XMMatrixTranspose( mGO );
	m_cbMatrices.data.mView = DirectX::XMMatrixTranspose( m_camera.GetViewMatrix() );
	m_cbMatrices.data.mProjection = DirectX::XMMatrixTranspose( m_camera.GetProjectionMatrix() );
	m_cbMatrices.data.vOutputColor = DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
	if ( !m_cbMatrices.ApplyChanges() ) return;
    
    setupLightForRender();

    // Render objects
    graphics.GetContext()->VSSetConstantBuffers(0u, 1u, m_cbMatrices.GetAddressOf() );
    graphics.GetContext()->PSSetConstantBuffers( 1u, 1u, m_gameObject.GetMaterialCB() );
    graphics.GetContext()->PSSetConstantBuffers( 2u, 1u, m_cbLight.GetAddressOf() );
    m_gameObject.Draw( graphics.GetContext() );

    // Render imgui windows
    m_imgui.BeginRender();
    m_imgui.SpawnInstructionWindow();
    m_imgui.EndRender();

    // Present frame
    graphics.EndFrame();
}