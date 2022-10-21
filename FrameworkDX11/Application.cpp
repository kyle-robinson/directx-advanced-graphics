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

    if ( !InitializeDevice() )
		return false;

    // Initialize the camera
    camera = std::make_shared<Camera>( XMFLOAT3( 0.0f, 0.0f, -3.0f ) );
    camera->SetProjectionValues( 75.0f, static_cast<float>( width ) / static_cast<float>( height ), 0.01f, 100.0f );

    // Update keyboard processing
    keyboard.DisableAutoRepeatKeys();
    keyboard.DisableAutoRepeatChars();

    return true;
}

// Create device and swap chain
bool Application::InitializeDevice()
{
	if ( !InitializeMesh() )
        return false;

	HRESULT hr = g_GameObject.initMesh( graphics.GetDevice(), graphics.GetContext() );
	COM_ERROR_IF_FAILED( hr, "Failed to initialize game object!" );

    return true;
}

// Initialize meshes
bool Application::InitializeMesh()
{
	// Create the constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	HRESULT hr = graphics.GetDevice()->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
		return false;

	// Create the light constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = graphics.GetDevice()->CreateBuffer(&bd, nullptr, &g_pLightConstantBuffer);
	if (FAILED(hr))
		return false;

	return true;
}

// Cleanup pipeline
void Application::CleanupDevice()
{
    g_GameObject.cleanup();

    if (g_pLightConstantBuffer) g_pLightConstantBuffer->Release();
    if( g_pConstantBuffer ) g_pConstantBuffer->Release();

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
    XMFLOAT4 eyePosition = { camera->GetPositionFloat3().x, camera->GetPositionFloat3().y, camera->GetPositionFloat3().z, 1.0f };
    DirectX::XMFLOAT4 LightPosition( eyePosition );
    light.Position = LightPosition;
    DirectX::XMVECTOR LightDirection = DirectX::XMVectorSet(
        camera->GetCameraTarget().x - LightPosition.x,
        camera->GetCameraTarget().y - LightPosition.y,
        camera->GetCameraTarget().z - LightPosition.z,
        0.0f
    );
    LightDirection = DirectX::XMVector3Normalize( LightDirection );
    DirectX::XMStoreFloat4( &light.Direction, LightDirection );

    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = LightPosition;
    lightProperties.Lights[0] = light;
    graphics.GetContext()->UpdateSubresource(g_pLightConstantBuffer, 0u, nullptr, &lightProperties, 0u, 0u);
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
                camera->AdjustRotation(
                    static_cast<float>( me.GetPosY() ) * 0.005f,
                    static_cast<float>( me.GetPosX() ) * 0.005f,
                    0.0f
                );
            }
        }
    }

    // camera speed
    camera->SetCameraSpeed( 2.5f );
    if ( keyboard.KeyIsPressed( VK_SHIFT ) )camera->UpdateCameraSpeed( 4.0f );

    // camera movement
    if ( keyboard.KeyIsPressed( 'W' ) ) camera->MoveForward( dt );
    if ( keyboard.KeyIsPressed( 'A' ) ) camera->MoveLeft( dt );
    if ( keyboard.KeyIsPressed( 'S' ) ) camera->MoveBackward( dt );
    if ( keyboard.KeyIsPressed( 'D' ) ) camera->MoveRight( dt );

    // x world collisions
    if ( camera->GetPositionFloat3().x <= -5.0f )
        camera->SetPosition( -5.0f, camera->GetPositionFloat3().y, camera->GetPositionFloat3().z );
    if ( camera->GetPositionFloat3().x >= 5.0f )
        camera->SetPosition( 5.0f, camera->GetPositionFloat3().y, camera->GetPositionFloat3().z );

    // y world collisions
    if ( camera->GetPositionFloat3().y <= -5.0f )
        camera->SetPosition( camera->GetPositionFloat3().x, -5.0f, camera->GetPositionFloat3().z );
    if ( camera->GetPositionFloat3().y >= 5.0f )
        camera->SetPosition( camera->GetPositionFloat3().x, 5.0f, camera->GetPositionFloat3().z );

    // z world collisions
    if ( camera->GetPositionFloat3().z <= -5.0f )
        camera->SetPosition( camera->GetPositionFloat3().x, camera->GetPositionFloat3().y, -5.0f );
    if ( camera->GetPositionFloat3().z >= 5.0f )
        camera->SetPosition( camera->GetPositionFloat3().x, camera->GetPositionFloat3().y, 5.0f );
}

void Application::Update()
{
    // Update
    float t = calculateDeltaTime(); // capped at 60 fps
    if ( t == 0.0f )
        return;

    // Update camera input
    UpdateInput( t );

    // Update the cube transform, material etc. 
    g_GameObject.update( t, graphics.GetContext() );
}

// Render a frame
void Application::Render()
{
    // Setup graphics
    graphics.BeginFrame();

    // get the game object world transform
    DirectX::XMMATRIX mGO = XMLoadFloat4x4( g_GameObject.getTransform() );

    // store this and the view / projection in a constant buffer for the vertex shader to use
    ConstantBuffer cb1;
	cb1.mWorld = DirectX::XMMatrixTranspose( mGO );
	cb1.mView = DirectX::XMMatrixTranspose( camera->GetViewMatrix() );
	cb1.mProjection = DirectX::XMMatrixTranspose( camera->GetProjectionMatrix() );
	cb1.vOutputColor = DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
	graphics.GetContext()->UpdateSubresource(g_pConstantBuffer, 0u, nullptr, &cb1, 0u, 0u);
    
    setupLightForRender();

    // Render objects
    graphics.GetContext()->VSSetConstantBuffers(0u, 1u, &g_pConstantBuffer);
    graphics.GetContext()->PSSetConstantBuffers( 2u, 1u, &g_pLightConstantBuffer );
    ID3D11Buffer* materialCB = g_GameObject.getMaterialConstantBuffer();
    graphics.GetContext()->PSSetConstantBuffers( 1u, 1u, &materialCB );
    g_GameObject.draw( graphics.GetContext() );

    // Present frame
    graphics.EndFrame();
}