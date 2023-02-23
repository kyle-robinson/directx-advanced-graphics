#include "Application.h"
#include "M3dLoader.h"

Application::Application()
{
    _pRenderTargetView = nullptr;
    _pConstantBuffer = nullptr;

    _pConstantBuffer = nullptr;
    _pLightConstantBuffer = nullptr;

    DimGuiManager = new ImGuiManager();
    _pLightContol = new LightControll();
    m_pInput = new Input();
}

Application::~Application()
{
    Cleanup();
}

HRESULT Application::Initialise( HINSTANCE hInstance, int width, int height )
{
    if ( !m_window.Initialize( m_pInput, hInstance,
        "DirectX 11 Advanced Graphics & Rendering", "TutorialWindowClass",
        width, height ) )
        return 0;

    m_gfx.Initialize( m_window.GetHWND(), width, height );

    if ( FAILED( InitDevice() ) )
    {
        Cleanup();
        return 0;
    }

    DimGuiManager->Initialize( m_window.GetHWND(), m_gfx.GetDevice(), m_gfx.GetContext() );

    return S_OK;
}

HRESULT Application::InitMesh()
{
    // Create object meshes
    HRESULT hr = _GameObject.GetAppearance()->initMesh( m_gfx.GetDevice(), m_gfx.GetContext() );
    if ( FAILED( hr ) )
        return hr;

    hr = _GameObjectFloor.GetAppearance()->initMeshFloor( m_gfx.GetDevice(), m_gfx.GetContext(), 10, 10 );
    if ( FAILED( hr ) )
        return hr;

    _GameObjectFloor.GetTransfrom()->SetPosition( -5, -2, 5 );

    // Terrain generation
    _Terrain = new Terrain( "Resources/Textures/coastMountain513.raw", XMFLOAT2( 513, 513 ),
        100, TerrainGenType::HightMapLoad, m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController() );

    vector<string> texGround;
    texGround.push_back( "Resources/Textures/grass.dds" );
    texGround.push_back( "Resources/Textures/darkdirt.dds" );
    texGround.push_back( "Resources/Textures/lightdirt.dds" );
    texGround.push_back( "Resources/Textures/stone.dds" );
    texGround.push_back( "Resources/Textures/snow.dds" );

    _Terrain->SetTex( texGround, m_gfx.GetDevice() );
    _Terrain->SetBlendMap( "Resources/Textures/blend.dds", m_gfx.GetDevice() );

    _VoxelTerrain = new TerrainVoxel( m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController(), 3, 3 );

    AnimmationObject = new AnimatedModel( "Resources/AnimModel/soldier.m3d",
        m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController() );

    // Create the constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;        // 36 vertices needed for 12 triangles in a triangle list
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( ConstantBuffer );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_gfx.GetDevice()->CreateBuffer( &bd, nullptr, &_pConstantBuffer );
    if ( FAILED( hr ) )
        return hr;

    // Create the light constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( LightPropertiesConstantBuffer );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_gfx.GetDevice()->CreateBuffer( &bd, nullptr, &_pLightConstantBuffer );
    if ( FAILED( hr ) )
        return hr;

    // Create the PostProcessing constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( PostProcessingCB );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_gfx.GetDevice()->CreateBuffer( &bd, nullptr, &_pPostProcessingConstantBuffer );
    if ( FAILED( hr ) )
        return hr;

    BillBoradObject = new BillboardObject( "Resources/Textures/bricks_TEX.dds", 2, m_gfx.GetDevice() );

    return hr;
}

vector<float> CubicBezierBasis( float u )
{
    float compla = 1 - u;	// complement of u
    // compute value of basis functions for given value of u
    float BF0 = compla * compla * compla;
    float BF1 = 3.0 * u * compla * compla;
    float BF2 = 3.0 * u * u * compla;
    float BF3 = u * u * u;

    vector<float> bfArray = { BF0, BF1, BF2, BF3 };
    return bfArray;
}

vector<XMFLOAT3> CubicBezierCurve( vector<XMFLOAT3> controlPoints )
{
    vector<XMFLOAT3> Points;
    for ( float i = 0.0f; i < 1.0f; i += 0.1f )
    {
        // Calculate value of each basis function for current u
        vector<float> basisFnValues = CubicBezierBasis( i );

        XMFLOAT3 sum = XMFLOAT3{ 0.0f,0.0f,0.0f };
        for ( int cPointIndex = 0; cPointIndex <= 3; cPointIndex++ )
        {
            // Calculate weighted sum (weightx * CPx)
            sum.x += controlPoints[cPointIndex].x * basisFnValues[cPointIndex];
            sum.y += controlPoints[cPointIndex].y * basisFnValues[cPointIndex];
            sum.z += controlPoints[cPointIndex].z * basisFnValues[cPointIndex];
        }

        DirectX::XMFLOAT3 point = sum;	// point for current u on cubic Bezier curve
        Points.push_back( point );
    }
    return Points;
}

HRESULT Application::InitWorld()
{
    // Initialize the camrea
    _pCamControll = new CameraController();

    _Camrea = new Camera( XMFLOAT3( 0.0f, 0, -5 ), XMFLOAT3( 0.0f, 0.0f, 0.0f ),
        XMFLOAT3( 0.0f, 1.0f, 0.0f ), m_gfx.GetWidth(), m_gfx.GetHeight(), 0.01f, 100.0f );
    _Camrea->SetCamName( "Light eye" );
    _pCamControll->AddCam( _Camrea );

    _Camrea = new Camera( XMFLOAT3( 0.0f, 0, -5 ), XMFLOAT3( 0.0f, 0.0f, 0.0f ),
        XMFLOAT3( 0.0f, 1.0f, 0.0f ), m_gfx.GetWidth(), m_gfx.GetHeight(), 0.01f, 100.0f );
    _Camrea->SetCamName( "Free Cam" );
    _pCamControll->AddCam( _Camrea );

    _Camrea = new Camera( XMFLOAT3( 0.0f, 0, -5 ), XMFLOAT3( 0.0f, 0.0f, 0.0f ),
        XMFLOAT3( 0.0f, 1.0f, 0.0f ), m_gfx.GetWidth(), m_gfx.GetHeight(), 0.01f, 50.0f );
    _Camrea->SetCamName( "Diss Cam" );
    _pCamControll->AddCam( _Camrea );

    m_pInput->AddCamControl( _pCamControll );

    //postSettings
    postSettings.UseColour = false;
    postSettings.Color = XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f };
    postSettings.UseBloom = false;
    postSettings.UseDepthOfF = false;
    postSettings.UseHDR = false;
    postSettings.UseBlur = false;
    postSettings.fadeAmount = 1.0f;
    postSettings.FarPlane = 100.0f;
    postSettings.focalDistance = 100.0f;
    postSettings.focalwidth = 100.0f;
    postSettings.blerAttenuation = 0.5f;



    SCREEN_VERTEX svQuad[4];

    svQuad[0].pos = XMFLOAT3( -1.0f, 1.0f, 0.0f );
    svQuad[0].tex = XMFLOAT2( 0.0f, 0.0f );

    svQuad[1].pos = XMFLOAT3( 1.0f, 1.0f, 0.0f );
    svQuad[1].tex = XMFLOAT2( 1.0f, 0.0f );

    svQuad[2].pos = XMFLOAT3( -1.0f, -1.0f, 0.0f );
    svQuad[2].tex = XMFLOAT2( 0.0f, 1.0f );

    svQuad[3].pos = XMFLOAT3( 1.0f, -1.0f, 0.0f );
    svQuad[3].tex = XMFLOAT2( 1.0f, 1.0f );


    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SCREEN_VERTEX ) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = svQuad;
    HRESULT hr = m_gfx.GetDevice()->CreateBuffer( &bd, &InitData, &g_pScreenQuadVB );
    if ( FAILED( hr ) )
        return hr;
    vector<XMFLOAT3> a = { XMFLOAT3{0.0f,0.0f,0.0f},XMFLOAT3{2.0f,1.0f,0.0f},XMFLOAT3{5.0f,0.6f,0.0f},XMFLOAT3{6.0f,0.0f,0.0f} };
    DimGuiManager->points = CubicBezierCurve( a );





    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = m_gfx.GetSwapChain()->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &pBackBuffer ) );
    if ( FAILED( hr ) )
        return hr;

    hr = m_gfx.GetDevice()->CreateRenderTargetView( pBackBuffer, nullptr, &_pRenderTargetView );
    pBackBuffer->Release();
    if ( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = m_gfx.GetWidth();
    descDepth.Height = m_gfx.GetHeight();
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = m_gfx.GetDevice()->CreateTexture2D( &descDepth, nullptr, &_pDepthStencil );
    if ( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = m_gfx.GetDevice()->CreateDepthStencilView( _pDepthStencil, &descDSV, &_pDepthStencilView );
    if ( FAILED( hr ) )
        return hr;



    //smaplers
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof( sampDesc ) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = m_gfx.GetDevice()->CreateSamplerState( &sampDesc, &m_pPointrLinear );
    if ( FAILED( hr ) )
        return hr;

    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sampDesc.BorderColor[0] = 1.0f;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = m_gfx.GetDevice()->CreateSamplerState( &sampDesc, &m_pLINEARBORDER );
    if ( FAILED( hr ) )
        return hr;

    DepthLight = new ShadowMap( m_gfx.GetDevice(), m_gfx.GetWidth(), m_gfx.GetHeight() );
    RSControll = new RasterStateManager();

    D3D11_RASTERIZER_DESC cmdesc;
    ZeroMemory( &cmdesc, sizeof( D3D11_RASTERIZER_DESC ) );
    cmdesc.FillMode = D3D11_FILL_SOLID;
    cmdesc.CullMode = D3D11_CULL_BACK;
    RSControll->CreatRasterizerState( m_gfx.GetDevice(), cmdesc, "RSCullBack" );

    ZeroMemory( &cmdesc, sizeof( D3D11_RASTERIZER_DESC ) );
    cmdesc.FillMode = D3D11_FILL_WIREFRAME;
    cmdesc.CullMode = D3D11_CULL_BACK;
    RSControll->CreatRasterizerState( m_gfx.GetDevice(), cmdesc, "RWireframe" );



    ZeroMemory( &cmdesc, sizeof( D3D11_RASTERIZER_DESC ) );
    cmdesc.FillMode = D3D11_FILL_SOLID;
    cmdesc.CullMode = D3D11_CULL_NONE;
    RSControll->CreatRasterizerState( m_gfx.GetDevice(), cmdesc, "RSCullNone" );

    hr = InitMesh();
    if ( FAILED( hr ) )
    {
        MessageBox( nullptr,
            L"Failed to initialise mesh.", L"Error", MB_OK );
        return hr;
    }

    hr = InitWorld();
    if ( FAILED( hr ) )
    {
        MessageBox( nullptr,
            L"Failed to initialise world.", L"Error", MB_OK );
        return hr;
    }



    //creat Lights
    _pLightContol->AddLight( "MainPoint", true, LightType::PointLight, XMFLOAT4( 0.0f, 0.0f, -4.0f, 0.0f ), XMFLOAT4( Colors::White ), XMConvertToRadians( 45.0f ), 1.0f, 0.0f, 0.0f, m_gfx.GetDevice(), m_gfx.GetContext() );
    _pLightContol->AddLight( "Point", true, LightType::SpotLight, XMFLOAT4( 0.0f, 5.0f, 0.0f, 0.0f ), XMFLOAT4( Colors::White ), XMConvertToRadians( 10.0f ), 1.0f, 0.0f, 0.0f, m_gfx.GetDevice(), m_gfx.GetContext() );

    return S_OK;
}
void Application::Update()
{
    float t = calculateDeltaTime(); // capped at 60 fps
    if ( t == 0.0f )
        return;

    m_pInput->Update( t );
    _pCamControll->Update();
    _Terrain->Update();
    // Update the cube transform, material etc.
    _GameObject.update( t, m_gfx.GetContext() );
    _GameObjectFloor.update( t, m_gfx.GetContext() );
    _pLightContol->update( t, m_gfx.GetContext() );
    BillBoradObject->UpdatePositions( m_gfx.GetContext() );

    AnimmationObject->Update( t );
}

void Application::Draw()
{
    ID3D11ShaderResourceView* ResourceView1;
    ID3D11ShaderResourceView* ResourceView2;
    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)m_gfx.GetWidth();
    vp.Height = (FLOAT)m_gfx.GetHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_gfx.GetContext()->RSSetViewports( 1, &vp );


    XMFLOAT4X4 WorldAsFloat;
    XMFLOAT4X4 projectionAsFloats;
    XMFLOAT4X4 viewAsFloats;


    XMMATRIX mGO;
    XMMATRIX RTTview;
    XMMATRIX RTTprojection;

    ConstantBuffer cb1;

    //move objects that will be shadowed into list
    vector<DrawableGameObject*> GameObjects;
    GameObjects.push_back( &_GameObject );
    GameObjects.push_back( &_GameObjectFloor );

    //creat Shadow depth senciles
    for ( UINT i = 0; i < MAX_LIGHTS; i++ )
    {
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
        // Set primitive topology
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "DepthLight" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &_pConstantBuffer );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "DepthLight" ).m_pPixelShader, nullptr, 0 );

        _pLightContol->GetLight( i )->CreateShdowMap( m_gfx.GetContext(), GameObjects, &_pConstantBuffer );


    }


    //render 3d objects
    RSControll->SetRasterizerState( m_gfx.GetContext() );
    m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->SetRenderTarget( m_gfx.GetContext() );

    //set shadow samplers
    m_gfx.GetContext()->PSSetSamplers( 1, 1, &m_pLINEARBORDER );
    m_gfx.GetContext()->PSSetSamplers( 2, 1, &m_pLINEARBORDER );

    // get the game object world transform
    WorldAsFloat = _GameObject.GetTransfrom()->GetWorldMatrix();
    mGO = XMLoadFloat4x4( &WorldAsFloat );

    viewAsFloats = _pCamControll->GetCurentCam()->GetView();
    projectionAsFloats = _pCamControll->GetCurentCam()->GetProjection();

    RTTview = XMLoadFloat4x4( &viewAsFloats );
    RTTprojection = XMLoadFloat4x4( &projectionAsFloats );
    XMMATRIX viewProject = RTTview * RTTprojection;
    // store this and the view / projection in a constant buffer for the vertex shader to use
    //ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose( mGO );
    cb1.mView = XMMatrixTranspose( RTTview );
    cb1.mProjection = XMMatrixTranspose( RTTprojection );
    cb1.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
    cb1.camPos = XMFLOAT4( _pCamControll->GetCurentCam()->GetPosition().x, _pCamControll->GetCurentCam()->GetPosition().y, _pCamControll->GetCurentCam()->GetPosition().z, 0.0f );
    m_gfx.GetContext()->UpdateSubresource( _pConstantBuffer, 0, nullptr, &cb1, 0, 0 );


    setupLightForRender();

    // Render the cube

    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );

    // Set primitive topology
    m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &_pConstantBuffer );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );


    ID3D11ShaderResourceView* ShadowMaps[2];
    ShadowMaps[0] = _pLightContol->GetLight( 0 )->GetShadow()->DepthMapSRV();
    ShadowMaps[1] = _pLightContol->GetLight( 1 )->GetShadow()->DepthMapSRV();


    m_gfx.GetContext()->PSSetShaderResources( 3, 2, ShadowMaps );


    //setTextures to buffer
    _GameObject.GetAppearance()->SetTextures( m_gfx.GetContext() );
    _GameObject.draw( m_gfx.GetContext() );

    RSControll->SetRasterizerState( "RSCullNone", m_gfx.GetContext() );
    AnimmationObject->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), &cb1, _pConstantBuffer );
    RSControll->SetRasterizerState( m_gfx.GetContext() );


    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &_pConstantBuffer );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );

    WorldAsFloat = _GameObjectFloor.GetTransfrom()->GetWorldMatrix();
    mGO = XMLoadFloat4x4( &WorldAsFloat );
    cb1.mWorld = XMMatrixTranspose( mGO );
    m_gfx.GetContext()->UpdateSubresource( _pConstantBuffer, 0, nullptr, &cb1, 0, 0 );

    _GameObjectFloor.GetAppearance()->SetTextures( m_gfx.GetContext() );
    _GameObjectFloor.draw( m_gfx.GetContext() );

    //terrain draw
    _VoxelTerrain->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), &cb1, _pConstantBuffer, _pCamControll );
    _Terrain->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), &cb1, _pConstantBuffer, _pCamControll );


    m_gfx.GetContext()->HSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );


    BillBoradObject->Draw( m_gfx.GetContext(), m_gfx.GetShaderController()->GetGeometryData(), &cb1, _pConstantBuffer );


    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );


    //post 2d
    if ( isRTT )
    {
        //RTT to cube or screen like a tv
        // Setup the viewport
        D3D11_VIEWPORT vp2;
        vp2.Width = (FLOAT)m_gfx.GetWidth();
        vp2.Height = (FLOAT)m_gfx.GetHeight();
        vp2.MinDepth = 0.0f;
        vp2.MaxDepth = 1.0f;
        vp2.TopLeftX = 0;
        vp2.TopLeftY = 0;
        m_gfx.GetContext()->RSSetViewports( 1, &vp2 );

        m_gfx.GetContext()->OMSetRenderTargets( 1, &_pRenderTargetView, _pDepthStencilView );
        // Clear the back buffer
        m_gfx.GetContext()->ClearRenderTargetView( _pRenderTargetView, Colors::LightBlue );
        // Clear the depth buffer to 1.0 (max depth)
        m_gfx.GetContext()->ClearDepthStencilView( _pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );



        // get the game object world transform
        WorldAsFloat = _GameObject.GetTransfrom()->GetWorldMatrix();
        mGO = XMLoadFloat4x4( &WorldAsFloat );

        viewAsFloats = _pCamControll->GetCurentCam()->GetView();
        projectionAsFloats = _pCamControll->GetCurentCam()->GetProjection();

        RTTview = XMLoadFloat4x4( &viewAsFloats );
        RTTprojection = XMLoadFloat4x4( &projectionAsFloats );

        // store this and the view / projection in a constant buffer for the vertex shader to use
        //ConstantBuffer cb1;
        cb1.mWorld = XMMatrixTranspose( mGO );
        cb1.mView = XMMatrixTranspose( RTTview );
        cb1.mProjection = XMMatrixTranspose( RTTprojection );
        cb1.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
        m_gfx.GetContext()->UpdateSubresource( _pConstantBuffer, 0, nullptr, &cb1, 0, 0 );

        // Render the cube
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &_pConstantBuffer );
        m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );

        //m_gfx.GetContext()->PSSetConstantBuffers(1, 1, &materialCB);

        //setTextures to buffer
        ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
        m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );





        _GameObject.draw( m_gfx.GetContext() );

        //lights

        _pLightContol->draw( m_gfx.GetContext(), _pConstantBuffer, &cb1 );
    }
    else
    {

        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &_pConstantBuffer );
        m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, &_pLightConstantBuffer );

        _pLightContol->draw( m_gfx.GetContext(), _pConstantBuffer, &cb1 );

        RSControll->SetRasterizerState( "RSCullBack", m_gfx.GetContext() );

        //deapth find
//----------------------------------------------------------------------------------------------------------------------------



        //render 3d objects
        m_gfx.GetRenderTargetController()->GetRenderTarget( "Depth" )->SetRenderTarget( m_gfx.GetContext() );




        // get the game object world transform
        XMFLOAT4X4 WorldAsFloat = _GameObject.GetTransfrom()->GetWorldMatrix();
        XMMATRIX mGO = XMLoadFloat4x4( &WorldAsFloat );

        XMFLOAT4X4 viewAsFloats = _pCamControll->GetCurentCam()->GetView();
        XMFLOAT4X4 projectionAsFloats = _pCamControll->GetCurentCam()->GetProjection();

        XMMATRIX RTTview = XMLoadFloat4x4( &viewAsFloats );
        XMMATRIX RTTprojection = XMLoadFloat4x4( &projectionAsFloats );

        // store this and the view / projection in a constant buffer for the vertex shader to use
        ConstantBuffer cb1;
        cb1.mWorld = XMMatrixTranspose( mGO );
        cb1.mView = XMMatrixTranspose( RTTview );
        cb1.mProjection = XMMatrixTranspose( RTTprojection );
        cb1.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
        m_gfx.GetContext()->UpdateSubresource( _pConstantBuffer, 0, nullptr, &cb1, 0, 0 );




        // Render the cube
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
        // Set primitive topology
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Depth" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &_pConstantBuffer );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Depth" ).m_pPixelShader, nullptr, 0 );

        m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 1, 1, &_pPostProcessingConstantBuffer );



        //setTextures to buffer
        _GameObject.draw( m_gfx.GetContext() );

        WorldAsFloat = _GameObjectFloor.GetTransfrom()->GetWorldMatrix();
        mGO = XMLoadFloat4x4( &WorldAsFloat );
        cb1.mWorld = XMMatrixTranspose( mGO );
        m_gfx.GetContext()->UpdateSubresource( _pConstantBuffer, 0, nullptr, &cb1, 0, 0 );

        _GameObjectFloor.draw( m_gfx.GetContext() );


        _pLightContol->draw( m_gfx.GetContext(), _pConstantBuffer, &cb1 );



        //----------------------------------------------------------------------------------------------------




                // Setup the viewport
        D3D11_VIEWPORT vp2;
        vp2.Width = (FLOAT)m_gfx.GetWidth();
        vp2.Height = (FLOAT)m_gfx.GetHeight();
        vp2.MinDepth = 0.0f;
        vp2.MaxDepth = 1.0f;
        vp2.TopLeftX = 0;
        vp2.TopLeftY = 0;
        m_gfx.GetContext()->RSSetViewports( 1, &vp2 );




        UINT strides = sizeof( SCREEN_VERTEX );
        UINT offsets = 0;
        ID3D11Buffer* pBuffers[1] = { g_pScreenQuadVB };

        //bloom alpha get
        if ( &postSettings.UseBloom )
        {
            //bloom
            m_gfx.GetRenderTargetController()->GetRenderTarget( "Alpha" )->SetRenderTarget( m_gfx.GetContext() );


            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pVertexLayout );
            m_gfx.GetContext()->PSSetSamplers( 0, 1, &m_pPointrLinear );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pPixelShader, nullptr, 0 );


            m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &_pPostProcessingConstantBuffer );

            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );

            m_gfx.GetContext()->Draw( 4, 0 );

        }

        //blur passes
        if ( postSettings.UseBlur || postSettings.UseBloom || postSettings.UseDepthOfF )
        {
            // Setup the viewport
            D3D11_VIEWPORT vp2;
            vp2.Width = (FLOAT)m_gfx.GetWidth() / 2;
            vp2.Height = (FLOAT)m_gfx.GetHeight() / 2;
            vp2.MinDepth = 0.0f;
            vp2.MaxDepth = 1.0f;
            vp2.TopLeftX = 0;
            vp2.TopLeftY = 0;
            m_gfx.GetContext()->RSSetViewports( 1, &vp2 );

            //down sample
            m_gfx.GetRenderTargetController()->GetRenderTarget( "DownSample" )->SetRenderTarget( m_gfx.GetContext() );

            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexLayout );
            m_gfx.GetContext()->PSSetSamplers( 0, 1, &m_pPointrLinear );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pPixelShader, nullptr, 0 );


            m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &_pPostProcessingConstantBuffer );

            if ( postSettings.UseBloom )
            {
                ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Alpha" )->GetTexture();
            }
            else
            {
                ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
            }
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );

            m_gfx.GetContext()->Draw( 4, 0 );

            //blur 1
            m_gfx.GetRenderTargetController()->GetRenderTarget( "Gaussian1" )->SetRenderTarget( m_gfx.GetContext() );


            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian1" ).m_pVertexLayout );
            m_gfx.GetContext()->PSSetSamplers( 0, 1, &m_pPointrLinear );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian1" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian1" ).m_pPixelShader, nullptr, 0 );


            m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &_pPostProcessingConstantBuffer );

            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "DownSample" )->GetTexture();

            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );

            m_gfx.GetContext()->Draw( 4, 0 );

            //blur 2
            m_gfx.GetRenderTargetController()->GetRenderTarget( "Gaussian2" )->SetRenderTarget( m_gfx.GetContext() );


            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian2" ).m_pVertexLayout );
            m_gfx.GetContext()->PSSetSamplers( 0, 1, &m_pPointrLinear );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian2" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian2" ).m_pPixelShader, nullptr, 0 );


            m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &_pPostProcessingConstantBuffer );

            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Gaussian1" )->GetTexture();
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );

            m_gfx.GetContext()->Draw( 4, 0 );

            // Setup the viewport
            D3D11_VIEWPORT vp3;
            vp3.Width = (FLOAT)m_gfx.GetWidth();
            vp3.Height = (FLOAT)m_gfx.GetHeight();
            vp3.MinDepth = 0.0f;
            vp3.MaxDepth = 1.0f;
            vp3.TopLeftX = 0;
            vp3.TopLeftY = 0;
            m_gfx.GetContext()->RSSetViewports( 1, &vp );

            //upsample
            m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->SetRenderTarget( m_gfx.GetContext() );

            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexLayout );
            m_gfx.GetContext()->PSSetSamplers( 0, 1, &m_pPointrLinear );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pPixelShader, nullptr, 0 );


            m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &_pPostProcessingConstantBuffer );


            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Gaussian2" )->GetTexture();


            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );

            m_gfx.GetContext()->Draw( 4, 0 );
        }



        ID3D11ShaderResourceView* ResourceView3;

        if ( postSettings.UseDepthOfF )
        {
            //DOF implmentation
            m_gfx.GetRenderTargetController()->GetRenderTarget( "DepthOfField" )->SetRenderTarget( m_gfx.GetContext() );

            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pVertexLayout );
            m_gfx.GetContext()->PSSetSamplers( 0, 1, &m_pPointrLinear );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pPixelShader, nullptr, 0 );
            m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &_pPostProcessingConstantBuffer );

            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
            ResourceView2 = m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->GetTexture();
            ResourceView3 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Depth" )->GetTexture();

            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
            m_gfx.GetContext()->PSSetShaderResources( 1, 1, &ResourceView2 );
            m_gfx.GetContext()->PSSetShaderResources( 2, 1, &ResourceView3 );
            m_gfx.GetContext()->Draw( 4, 0 );

        }



        //fade implmentation
        m_gfx.GetRenderTargetController()->GetRenderTarget( "Fade" )->SetRenderTarget( m_gfx.GetContext() );

        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Fade" ).m_pVertexLayout );
        m_gfx.GetContext()->PSSetSamplers( 0, 1, &m_pPointrLinear );
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Fade" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Fade" ).m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &_pPostProcessingConstantBuffer );

        if ( postSettings.UseBlur )
        {
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->GetTexture();
        }
        else if ( postSettings.UseDepthOfF )
        {
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "DepthOfField" )->GetTexture();
        }
        else
        {
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
        }

        m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
        m_gfx.GetContext()->Draw( 4, 0 );


        //final post processing
        m_gfx.GetContext()->OMSetRenderTargets( 1, &_pRenderTargetView, _pDepthStencilView );
        m_gfx.GetContext()->ClearRenderTargetView( _pRenderTargetView, Colors::DarkBlue );
        m_gfx.GetContext()->ClearDepthStencilView( _pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Final" ).m_pVertexLayout );
        m_gfx.GetContext()->PSSetSamplers( 0, 1, &m_pPointrLinear );
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Final" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Final" ).m_pPixelShader, nullptr, 0 );



        ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Fade" )->GetTexture();
        ResourceView2 = m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->GetTexture();




        m_gfx.GetContext()->UpdateSubresource( _pPostProcessingConstantBuffer, 0, nullptr, &postSettings, 0, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &_pPostProcessingConstantBuffer );
        m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
        m_gfx.GetContext()->PSSetShaderResources( 1, 1, &ResourceView2 );
        m_gfx.GetContext()->Draw( 4, 0 );
    }



    DimGuiManager->BeginRender();
    DimGuiManager->DrawCamMenu( _pCamControll );
    DimGuiManager->ObjectControl( &_GameObject );
    DimGuiManager->LightControl( _pLightContol );
    DimGuiManager->ShaderMenu( m_gfx.GetShaderController(), &postSettings, RSControll, isRTT );
    DimGuiManager->BillBoradControl( BillBoradObject );
    DimGuiManager->BezierCurveSpline();
    DimGuiManager->TerrainControll( _Terrain, _VoxelTerrain, m_gfx.GetDevice(), m_gfx.GetContext() );
    DimGuiManager->AnimationControll( AnimmationObject );
    DimGuiManager->EndRender();

    // Present our back buffer to our front buffer
    m_gfx.GetSwapChain()->Present( 1, 0 );
}



float Application::calculateDeltaTime()
{
    //Update our time
    static float deltaTime = 0.0f;
    static ULONGLONG timeStart = 0;
    ULONGLONG timeCur = GetTickCount64();
    if ( timeStart == 0 )
        timeStart = timeCur;
    deltaTime = ( timeCur - timeStart ) / 1000.0f;
    timeStart = timeCur;

    float FPS60 = 1.0f / 60.0f;
    static float cummulativeTime = 0;

    // cap the framerate at 60 fps
    cummulativeTime += deltaTime;
    if ( cummulativeTime >= FPS60 )
    {
        cummulativeTime = cummulativeTime - FPS60;
    }
    else
    {
        return 0;
    }

    return deltaTime;
}

void Application::setupLightForRender()
{

    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = _pCamControll->GetCam( 0 )->GetPositionFloat4();
    lightProperties.Lights[0] = _pLightContol->GetLight( 0 )->GetLightData();
    lightProperties.Lights[1] = _pLightContol->GetLight( 1 )->GetLightData();
    m_gfx.GetContext()->UpdateSubresource( _pLightConstantBuffer, 0, nullptr, &lightProperties, 0, 0 );
}


void Application::Cleanup()
{
    _GameObject.cleanup();
    _GameObjectFloor.cleanup();

    delete m_pInput;
    m_pInput = nullptr;

    delete _pCamControll;
    _pConstantBuffer = nullptr;

    delete _pLightContol;
    _pLightContol = nullptr;

    delete DimGuiManager;
    DimGuiManager = nullptr;

    delete BillBoradObject;
    BillBoradObject = nullptr;
    delete DepthLight;
    DepthLight = nullptr;

    delete _Terrain;
    _Terrain = nullptr;

    delete _VoxelTerrain;
    _VoxelTerrain = nullptr;

    delete AnimmationObject;
    AnimmationObject = nullptr;

    // Remove any bound render target or depth/stencil buffer
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_gfx.GetContext()->OMSetRenderTargets( _countof( nullViews ), nullViews, nullptr );

    if ( _pLightConstantBuffer )
        _pLightConstantBuffer->Release();

    if ( _pConstantBuffer ) _pConstantBuffer->Release();
    if ( _pPostProcessingConstantBuffer )_pPostProcessingConstantBuffer->Release();

    if ( _pDepthStencil ) _pDepthStencil->Release();
    if ( _pDepthStencilView ) _pDepthStencilView->Release();
    if ( _pRenderTargetView ) _pRenderTargetView->Release();

    if ( g_pScreenQuadVB ) g_pScreenQuadVB->Release();
    if ( m_pPointrLinear ) m_pPointrLinear->Release();
    if ( m_pPointrLinear ) m_pLINEARBORDER->Release();


    ID3D11Debug* debugDevice = nullptr;
    m_gfx.GetDevice()->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &debugDevice ) );

    // handy for finding dx memory leaks
    debugDevice->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );

    if ( debugDevice )
        debugDevice->Release();
}