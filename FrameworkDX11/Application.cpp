#include "stdafx.h"
#include "Application.h"

// Register class and create window
bool Application::InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    if ( !renderWindow.Initialize( this, hInstance, "DirectX 11 Physics Framework", "TutorialWindowClass", 1280, 720 ) )
		return false;

    g_viewWidth = 1280;
    g_viewHeight = 720;

    return S_OK;
}

// Create device and swap chain
HRESULT Application::InitDevice()
{
    g_pSwapChain = std::make_shared<Bind::SwapChain>( g_pContext.GetAddressOf(), g_pDevice.GetAddressOf(), renderWindow.GetHWND(), g_viewWidth, g_viewHeight);
    g_pRenderTarget = std::make_shared<Bind::RenderTarget>( g_pDevice.Get(), g_pSwapChain->GetSwapChain() );
    g_pDepthStencil = std::make_shared<Bind::DepthStencil>( g_pDevice.Get(), g_viewWidth, g_viewHeight );
	g_pViewport = std::make_shared<Bind::Viewport>( g_pContext.Get(), g_viewWidth, g_viewHeight );
    
    g_pRasterizerStates.emplace( Bind::Rasterizer::Type::SOLID, std::make_shared<Bind::Rasterizer>( g_pDevice.Get(), Bind::Rasterizer::Type::SOLID ) );
    g_pRasterizerStates.emplace( Bind::Rasterizer::Type::WIREFRAME, std::make_shared<Bind::Rasterizer>( g_pDevice.Get(), Bind::Rasterizer::Type::WIREFRAME ) );

    g_pSamplerStates.emplace( Bind::Sampler::Type::ANISOTROPIC, std::make_shared<Bind::Sampler>( g_pDevice.Get(), Bind::Sampler::Type::ANISOTROPIC ) );
	g_pSamplerStates.emplace( Bind::Sampler::Type::BILINEAR, std::make_shared<Bind::Sampler>( g_pDevice.Get(), Bind::Sampler::Type::BILINEAR ) );
	g_pSamplerStates.emplace( Bind::Sampler::Type::POINT, std::make_shared<Bind::Sampler>( g_pDevice.Get(), Bind::Sampler::Type::POINT ) );

    g_pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	HRESULT hr = InitMesh();
    COM_ERROR_IF_FAILED( hr, "Failed to initialize mesh!" );

	hr = InitWorld( g_viewWidth, g_viewHeight);
    COM_ERROR_IF_FAILED( hr, "Failed to initialize world!" );

	hr = g_GameObject.initMesh(g_pDevice.Get(), g_pContext.Get() );
	COM_ERROR_IF_FAILED( hr, "Failed to initialize game object!" );

    return hr;
}

// Initialize meshes
HRESULT Application::InitMesh()
{
    // Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // Create the shaders
    HRESULT hr = vertexShader.Initialize( g_pDevice, L"Resources\\Shaders\\shader.fx", layout, ARRAYSIZE(layout));
	COM_ERROR_IF_FAILED( hr, "Failed to create vertex shader!" );
	hr = pixelShader.Initialize( g_pDevice, L"Resources\\Shaders\\shader.fx" );
	COM_ERROR_IF_FAILED( hr, "Failed to create pixel shader!" );
    
    // Bind shaders to the pipeline
    Shaders::BindShaders( g_pContext.Get(), vertexShader, pixelShader);

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
	hr = g_pDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
		return hr;

	// Create the light constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pDevice->CreateBuffer(&bd, nullptr, &g_pLightConstantBuffer);
	if (FAILED(hr))
		return hr;

	return hr;
}

// Initialize world matrices
HRESULT Application::InitWorld( int width, int height )
{
    // Initialize the camera
    camera = std::make_shared<Camera>( XMFLOAT3( 0.0f, 0.0f, -3.0f ) );
    camera->SetProjectionValues( 75.0f, static_cast<float>( width ) / static_cast<float>( height ), 0.01f, 100.0f );

    // Update keyboard processing
    keyboard.DisableAutoRepeatKeys();
    keyboard.DisableAutoRepeatChars();

    // Initialize imgui
    imgui.Initialize( renderWindow.GetHWND(), g_pDevice.Get(), g_pContext.Get());

	return S_OK;
}

// Cleanup pipeline
void Application::CleanupDevice()
{
    g_GameObject.cleanup();

    if (g_pLightConstantBuffer) g_pLightConstantBuffer->Release();
    if( g_pConstantBuffer ) g_pConstantBuffer->Release();

    ID3D11Debug* debugDevice = nullptr;
    g_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice));

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
    g_pContext->UpdateSubresource( g_pLightConstantBuffer, 0u, nullptr, &lightProperties, 0u, 0u );
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
    g_GameObject.update( t, g_pContext.Get() );
}

// Render a frame
void Application::Render()
{
    // clear render target/depth stencil
    static float clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    g_pRenderTarget->BindAsBuffer( g_pContext.Get(), g_pDepthStencil.get(), clearColor );
    g_pDepthStencil->ClearDepthStencil( g_pContext.Get() );

    // set render state
    g_pRasterizerStates[Bind::Rasterizer::Type::SOLID]->Bind( g_pContext.Get() );
    g_pSamplerStates[Bind::Sampler::Type::ANISOTROPIC]->Bind( g_pContext.Get() );

    // bind shaders
    Shaders::BindShaders( g_pContext.Get(), vertexShader, pixelShader );

    // get the game object world transform
    DirectX::XMMATRIX mGO = XMLoadFloat4x4( g_GameObject.getTransform() );

    // store this and the view / projection in a constant buffer for the vertex shader to use
    ConstantBuffer cb1;
	cb1.mWorld = DirectX::XMMatrixTranspose( mGO );
	cb1.mView = DirectX::XMMatrixTranspose( camera->GetViewMatrix() );
	cb1.mProjection = DirectX::XMMatrixTranspose( camera->GetProjectionMatrix() );
	cb1.vOutputColor = DirectX::XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
	g_pContext->UpdateSubresource( g_pConstantBuffer, 0u, nullptr, &cb1, 0u, 0u );
    
    setupLightForRender();

    // Render objects
	g_pContext->VSSetConstantBuffers( 0u, 1u, &g_pConstantBuffer );
	g_pContext->PSSetConstantBuffers( 2u, 1u, &g_pLightConstantBuffer );
    ID3D11Buffer* materialCB = g_GameObject.getMaterialConstantBuffer();
    g_pContext->PSSetConstantBuffers( 1u, 1u, &materialCB );
    g_GameObject.draw( g_pContext.Get() );

    // Render imgui windows
    imgui.BeginRender();
    imgui.SpawnInstructionWindow();
    imgui.EndRender();

    // Present frame
	HRESULT hr = g_pSwapChain->GetSwapChain()->Present( 1u, NULL );
	if ( FAILED( hr ) )
	{
		hr == DXGI_ERROR_DEVICE_REMOVED ?
			ErrorLogger::Log( g_pDevice->GetDeviceRemovedReason(), "Swap Chain. Graphics device removed!" ) :
			ErrorLogger::Log( hr, "Swap Chain failed to render frame!" );
		exit( -1 );
	}
}