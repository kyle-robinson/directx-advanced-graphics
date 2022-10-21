#include "stdafx.h"
#include "Application.h"

// Register class and create window
bool Application::Initialize( HINSTANCE hInstance, int width, int height )
{    
    // Initialize window
    if ( !renderWindow.Initialize( this, hInstance, "DirectX 11 Physics Framework", "TutorialWindowClass", width, height ) )
		return false;

    // Initialize graphics
    if ( !graphics.Initialize( renderWindow.GetHWND(), width, height ) )
		return false;

    // Initialize constant buffers
    if ( !InitializeMesh() )
        return false;

    // Create game object
	HRESULT hr = m_gameObject.InitializeMesh( graphics.GetDevice(), graphics.GetContext() );
	COM_ERROR_IF_FAILED( hr, "Failed to initialize game object!" );

    // Initialize the camera
    m_pCamera = std::make_shared<Camera>( XMFLOAT3( 0.0f, 0.0f, -3.0f ) );
    m_pCamera->SetProjectionValues( 75.0f, static_cast<float>( width ) / static_cast<float>( height ), 0.01f, 100.0f );

    // Update keyboard processing
    keyboard.DisableAutoRepeatKeys();
    keyboard.DisableAutoRepeatChars();

    return true;
}

// Initialize meshes
bool Application::InitializeMesh()
{
    HRESULT hr = m_cbMatrices.Initialize( graphics.GetDevice(), graphics.GetContext() );
	COM_ERROR_IF_FAILED( hr, "Failed to create 'Matrices' constant buffer!" );

    hr = m_cbLight.Initialize( graphics.GetDevice(), graphics.GetContext() );
    COM_ERROR_IF_FAILED( hr, "Failed to create 'Light' constant buffer!" );

	return true;
}

// Cleanup pipeline
void Application::CleanupDevice()
{
    m_gameObject.Cleanup();

    ID3D11Debug* debugDevice = nullptr;
    graphics.GetDevice()->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice));

    // handy for finding dx memory leaks
    debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

    if (debugDevice)
        debugDevice->Release();
}

// Setup light data
void Application::setupLightForRender()
{
    Light light;
    light.Enabled = static_cast<int>(true);
    light.LightType = PointLight;
    light.Color = DirectX::XMFLOAT4( DirectX::Colors::White );
    light.SpotAngle = DirectX::XMConvertToRadians( 45.0f );
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = 1.0f;
    light.QuadraticAttenuation = 1.0f;

    // set up the light
    XMFLOAT4 eyePosition = { m_pCamera->GetPositionFloat3().x, m_pCamera->GetPositionFloat3().y, m_pCamera->GetPositionFloat3().z, 1.0f };
    DirectX::XMFLOAT4 LightPosition( eyePosition );
    light.Position = LightPosition;
    DirectX::XMVECTOR LightDirection = DirectX::XMVectorSet(
        m_pCamera->GetCameraTarget().x - LightPosition.x,
        m_pCamera->GetCameraTarget().y - LightPosition.y,
        m_pCamera->GetCameraTarget().z - LightPosition.z,
        0.0f
    );
    LightDirection = DirectX::XMVector3Normalize( LightDirection );
    DirectX::XMStoreFloat4( &light.Direction, LightDirection );

    m_cbLight.data.EyePosition = LightPosition;
    m_cbLight.data.Lights[0] = light;
    if ( !m_cbLight.ApplyChanges() ) return;
}

// Update program time
float Application::calculateDeltaTime()
{
    // Update our time
    static float deltaTime = 0.0f;
    static ULONGLONG timeStart = 0;
    ULONGLONG timeCur = GetTickCount64();
    if (timeStart == 0)
        timeStart = timeCur;
    deltaTime = (timeCur - timeStart) / 1000.0f;
    timeStart = timeCur;

    float FPS60 = 1.0f / 60.0f;
    static float cummulativeTime = 0;

    // cap the framerate at 60 fps 
    cummulativeTime += deltaTime;
    if (cummulativeTime >= FPS60) {
        cummulativeTime = cummulativeTime - FPS60;
    }
    else {
        return 0;
    }

    return deltaTime;
}

bool Application::ProcessMessages() noexcept
{
	return renderWindow.ProcessMessages();
}

void Application::UpdateInput( float dt )
{
    // update camera orientation
    while ( !mouse.EventBufferIsEmpty() )
    {
        Mouse::MouseEvent me = mouse.ReadEvent();
        if ( mouse.IsRightDown() )
        {
            if ( me.GetType() == Mouse::MouseEvent::EventType::RawMove )
            {
                m_pCamera->AdjustRotation(
                    static_cast<float>( me.GetPosY() ) * 0.005f,
                    static_cast<float>( me.GetPosX() ) * 0.005f,
                    0.0f
                );
            }
        }
    }

    // camera speed
    m_pCamera->SetCameraSpeed( 2.5f );
    if ( keyboard.KeyIsPressed( VK_SHIFT ) ) m_pCamera->UpdateCameraSpeed( 4.0f );

    // camera movement
    if ( keyboard.KeyIsPressed( 'W' ) ) m_pCamera->MoveForward( dt );
    if ( keyboard.KeyIsPressed( 'A' ) ) m_pCamera->MoveLeft( dt );
    if ( keyboard.KeyIsPressed( 'S' ) ) m_pCamera->MoveBackward( dt );
    if ( keyboard.KeyIsPressed( 'D' ) ) m_pCamera->MoveRight( dt );

    // x world collisions
    if ( m_pCamera->GetPositionFloat3().x <= -5.0f )
        m_pCamera->SetPosition( -5.0f, m_pCamera->GetPositionFloat3().y, m_pCamera->GetPositionFloat3().z );
    if ( m_pCamera->GetPositionFloat3().x >= 5.0f )
        m_pCamera->SetPosition( 5.0f, m_pCamera->GetPositionFloat3().y, m_pCamera->GetPositionFloat3().z );

    // y world collisions
    if ( m_pCamera->GetPositionFloat3().y <= -5.0f )
        m_pCamera->SetPosition( m_pCamera->GetPositionFloat3().x, -5.0f, m_pCamera->GetPositionFloat3().z );
    if ( m_pCamera->GetPositionFloat3().y >= 5.0f )
        m_pCamera->SetPosition( m_pCamera->GetPositionFloat3().x, 5.0f, m_pCamera->GetPositionFloat3().z );

    // z world collisions
    if ( m_pCamera->GetPositionFloat3().z <= -5.0f )
        m_pCamera->SetPosition( m_pCamera->GetPositionFloat3().x, m_pCamera->GetPositionFloat3().y, -5.0f );
    if ( m_pCamera->GetPositionFloat3().z >= 5.0f )
        m_pCamera->SetPosition( m_pCamera->GetPositionFloat3().x, m_pCamera->GetPositionFloat3().y, 5.0f );
}

void Application::Update()
{
    // Update
    float dt = calculateDeltaTime(); // capped at 60 fps
    if ( dt == 0.0f )
        return;

    // Update camera input
    UpdateInput( dt );

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
	m_cbMatrices.data.mView = DirectX::XMMatrixTranspose( m_pCamera->GetViewMatrix() );
	m_cbMatrices.data.mProjection = DirectX::XMMatrixTranspose( m_pCamera->GetProjectionMatrix() );
	m_cbMatrices.data.vOutputColor = DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
	if ( !m_cbMatrices.ApplyChanges() ) return;
    
    setupLightForRender();

    // Render objects
    graphics.GetContext()->VSSetConstantBuffers(0u, 1u, m_cbMatrices.GetAddressOf() );
    graphics.GetContext()->PSSetConstantBuffers( 1u, 1u, m_gameObject.GetMaterialCB() );
    graphics.GetContext()->PSSetConstantBuffers( 2u, 1u, m_cbLight.GetAddressOf() );
    m_gameObject.Draw( graphics.GetContext() );

    // Present frame
    graphics.EndFrame();
}