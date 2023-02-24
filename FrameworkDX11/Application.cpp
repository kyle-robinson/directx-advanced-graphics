#include "Application.h"
#include "M3dLoader.h"

Application::Application()
{
    m_pCB = nullptr;
    m_pLightCB = nullptr;
    m_pPostProcessingCB = nullptr;

    m_pLightController = new LightController();
    m_pImGuiManager = new ImGuiManager();
    m_pInput = new Input();
}

Application::~Application()
{
    Cleanup();
}

HRESULT Application::Initialize( HINSTANCE hInstance, int width, int height )
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

    m_pImGuiManager->Initialize( m_window.GetHWND(), m_gfx.GetDevice(), m_gfx.GetContext() );

    return S_OK;
}

HRESULT Application::InitMesh()
{
    // Create object meshes
    HRESULT hr = m_cube.GetAppearance()->InitMesh_Cube( m_gfx.GetDevice(), m_gfx.GetContext() );
    if ( FAILED( hr ) )
        return hr;

    hr = m_ground.GetAppearance()->InitMesh_Quad( m_gfx.GetDevice(), m_gfx.GetContext() );
    if ( FAILED( hr ) )
        return hr;
    m_ground.GetTransfrom()->SetPosition( -5, -2, 5 );

    // Terrain generation
    m_pTerrain = new Terrain( "Resources/Textures/coastMountain513.raw", XMFLOAT2( 513, 513 ),
        100, TerrainGenType::HightMapLoad, m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController() );

    std::vector<string> texGround;
    texGround.push_back( "Resources/Textures/grass.dds" );
    texGround.push_back( "Resources/Textures/darkdirt.dds" );
    texGround.push_back( "Resources/Textures/lightdirt.dds" );
    texGround.push_back( "Resources/Textures/stone.dds" );
    texGround.push_back( "Resources/Textures/snow.dds" );

    m_pTerrain->SetTex( texGround, m_gfx.GetDevice() );
    m_pTerrain->SetBlendMap( "Resources/Textures/blend.dds", m_gfx.GetDevice() );
    m_pVoxelTerrain = new TerrainVoxel( m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController(), 3, 3 );

    // Create miscellaneous objects
    m_pBillboard = new BillboardObject( "Resources/Textures/bricks_TEX.dds", 2, m_gfx.GetDevice() );
    m_pAnimModel = new AnimatedModel( "Resources/AnimModel/soldier.m3d",
        m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController() );

    // Create the constant buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( ConstantBuffer );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_gfx.GetDevice()->CreateBuffer( &bd, nullptr, &m_pCB );
    if ( FAILED( hr ) )
        return hr;

    // Create the light constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( LightPropertiesConstantBuffer );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_gfx.GetDevice()->CreateBuffer( &bd, nullptr, &m_pLightCB );
    if ( FAILED( hr ) )
        return hr;

    // Create the post processing constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( PostProcessingCB );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = m_gfx.GetDevice()->CreateBuffer( &bd, nullptr, &m_pPostProcessingCB );
    if ( FAILED( hr ) )
        return hr;

    return hr;
}

vector<float> CubicBezierBasis( float u )
{
    float compla = 1 - u; // complement of u
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
    vector<XMFLOAT3> points;
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

        DirectX::XMFLOAT3 point = sum; // point for current u on cubic Bezier curve
        points.push_back( point );
    }
    return points;
}

HRESULT Application::InitWorld()
{
    // Initialize the camrea
    m_pCamController = new CameraController();

    m_pCamera = new Camera( XMFLOAT3( 0.0f, 0, -5 ), XMFLOAT3( 0.0f, 0.0f, 0.0f ),
        XMFLOAT3( 0.0f, 1.0f, 0.0f ), m_gfx.GetWidth(), m_gfx.GetHeight(), 0.01f, 100.0f );
    m_pCamera->SetCamName( "Light eye" );
    m_pCamController->AddCam( m_pCamera );

    m_pCamera = new Camera( XMFLOAT3( 0.0f, 0, -5 ), XMFLOAT3( 0.0f, 0.0f, 0.0f ),
        XMFLOAT3( 0.0f, 1.0f, 0.0f ), m_gfx.GetWidth(), m_gfx.GetHeight(), 0.01f, 100.0f );
    m_pCamera->SetCamName( "Free Cam" );
    m_pCamController->AddCam( m_pCamera );

    m_pCamera = new Camera( XMFLOAT3( 0.0f, 0, -5 ), XMFLOAT3( 0.0f, 0.0f, 0.0f ),
        XMFLOAT3( 0.0f, 1.0f, 0.0f ), m_gfx.GetWidth(), m_gfx.GetHeight(), 0.01f, 50.0f );
    m_pCamera->SetCamName( "Diss Cam" );
    m_pCamController->AddCam( m_pCamera );

    m_pInput->AddCamControl( m_pCamController );

    // Post settings
    m_ppSettings.UseColour = false;
    m_ppSettings.Color = XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f };
    m_ppSettings.UseBloom = false;
    m_ppSettings.UseDepthOfF = false;
    m_ppSettings.UseHDR = false;
    m_ppSettings.UseBlur = false;
    m_ppSettings.FadeAmount = 1.0f;
    m_ppSettings.FarPlane = 100.0f;
    m_ppSettings.FocalDistance = 100.0f;
    m_ppSettings.FocalWidth = 100.0f;
    m_ppSettings.BlurAttenuation = 0.5f;

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
    HRESULT hr = m_gfx.GetDevice()->CreateBuffer( &bd, &InitData, &m_pScreenQuadVB );
    if ( FAILED( hr ) )
        return hr;
    vector<XMFLOAT3> a = {
        XMFLOAT3{ 0.0f,0.0f,0.0f },
        XMFLOAT3{ 2.0f,1.0f,0.0f },
        XMFLOAT3{ 5.0f,0.6f,0.0f },
        XMFLOAT3{ 6.0f,0.0f,0.0f } };
    m_pImGuiManager->points = CubicBezierCurve( a );

    return S_OK;
}

HRESULT Application::InitDevice()
{
    m_pDepthLight = new ShadowMap( m_gfx.GetDevice(), m_gfx.GetWidth(), m_gfx.GetHeight() );

    HRESULT hr = InitMesh();
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

    // Create lights
    m_pLightController->AddLight( "MainPoint", true, LightType::PointLight,
        XMFLOAT4( 0.0f, 0.0f, -4.0f, 0.0f ), XMFLOAT4( Colors::White ),
        XMConvertToRadians( 45.0f ), 1.0f, 0.0f, 0.0f, m_gfx.GetDevice(), m_gfx.GetContext() );

    m_pLightController->AddLight( "Point", true, LightType::SpotLight,
        XMFLOAT4( 0.0f, 5.0f, 0.0f, 0.0f ), XMFLOAT4( Colors::White ),
        XMConvertToRadians( 10.0f ), 1.0f, 0.0f, 0.0f, m_gfx.GetDevice(), m_gfx.GetContext() );

    return S_OK;
}
void Application::Update()
{
    float t = CalculateDeltaTime(); // capped at 60 fps
    if ( t == 0.0f )
        return;

    m_pInput->Update( t );
    m_pCamController->Update();
    m_pTerrain->Update();

    // Update the cube transform, material etc.
    m_cube.Update( t, m_gfx.GetContext() );
    m_ground.Update( t, m_gfx.GetContext() );
    m_pLightController->Update( t, m_gfx.GetContext() );
    m_pBillboard->UpdatePositions( m_gfx.GetContext() );
    m_pAnimModel->Update( t );
}

void Application::Draw()
{
    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)m_gfx.GetWidth();
    vp.Height = (FLOAT)m_gfx.GetHeight();
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_gfx.GetContext()->RSSetViewports( 1, &vp );

    // Move objects that will be shadowed into list
    vector<DrawableGameObject*> gameObjects;
    gameObjects.push_back( &m_cube );
    gameObjects.push_back( &m_ground );

    // Create shadow depth stencils
    for ( UINT i = 0; i < MAX_LIGHTS; i++ )
    {
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "m_pDepthLight" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &m_pCB );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "m_pDepthLight" ).m_pPixelShader, nullptr, 0 );
        m_pLightController->GetLight( i )->CreateShadowMap( m_gfx.GetContext(), gameObjects, &m_pCB );
    }

    // Render 3d objects
    m_gfx.GetRasterizerController()->SetOverrideState( m_gfx.GetContext() );
    m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->SetRenderTarget( m_gfx.GetContext() );

    // Set shadow samplers
    m_gfx.GetSamplerController()->SetState( "Border", 1u, m_gfx.GetContext() );
    m_gfx.GetSamplerController()->SetState( "Border", 2u, m_gfx.GetContext() );

    // get the game object world transform
    XMFLOAT4X4 WorldAsFloat = m_cube.GetTransfrom()->GetWorldMatrix();
    XMMATRIX mGO = XMLoadFloat4x4( &WorldAsFloat );

    XMFLOAT4X4 viewAsFloats = m_pCamController->GetCurentCam()->GetView();
    XMFLOAT4X4 projectionAsFloats = m_pCamController->GetCurentCam()->GetProjection();

    XMMATRIX RTTview = XMLoadFloat4x4( &viewAsFloats );
    XMMATRIX RTTprojection = XMLoadFloat4x4( &projectionAsFloats );
    XMMATRIX viewProject = RTTview * RTTprojection;
    // store this and the view / projection in a constant buffer for the vertex shader to use
    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose( mGO );
    cb1.mView = XMMatrixTranspose( RTTview );
    cb1.mProjection = XMMatrixTranspose( RTTprojection );
    cb1.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
    cb1.camPos = XMFLOAT4(
        m_pCamController->GetCurentCam()->GetPosition().x,
        m_pCamController->GetCurentCam()->GetPosition().y,
        m_pCamController->GetCurentCam()->GetPosition().z, 0.0f );
    m_gfx.GetContext()->UpdateSubresource( m_pCB, 0, nullptr, &cb1, 0, 0 );
    SetupLightForRender();

    // Render the cube
    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
    m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &m_pCB );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, &m_pLightCB );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, &m_pLightCB );

    ID3D11ShaderResourceView* ShadowMaps[2];
    ShadowMaps[0] = m_pLightController->GetLight( 0 )->GetShadow()->DepthMapSRV();
    ShadowMaps[1] = m_pLightController->GetLight( 1 )->GetShadow()->DepthMapSRV();
    m_gfx.GetContext()->PSSetShaderResources( 3, 2, ShadowMaps );

    // Set textures to buffer
    m_cube.GetAppearance()->SetTextures( m_gfx.GetContext() );
    m_cube.Draw( m_gfx.GetContext() );

    m_gfx.GetRasterizerController()->SetState( "None", m_gfx.GetContext() );
    m_pAnimModel->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), &cb1, m_pCB );
    m_gfx.GetRasterizerController()->SetOverrideState( m_gfx.GetContext() );

    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &m_pCB );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, &m_pLightCB );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, &m_pLightCB );

    WorldAsFloat = m_ground.GetTransfrom()->GetWorldMatrix();
    mGO = XMLoadFloat4x4( &WorldAsFloat );
    cb1.mWorld = XMMatrixTranspose( mGO );
    m_gfx.GetContext()->UpdateSubresource( m_pCB, 0, nullptr, &cb1, 0, 0 );
    m_ground.GetAppearance()->SetTextures( m_gfx.GetContext() );
    m_ground.Draw( m_gfx.GetContext() );

    // Render terrain
    m_pVoxelTerrain->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), &cb1, m_pCB, m_pCamController );
    m_pTerrain->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), &cb1, m_pCB, m_pCamController );
    m_gfx.GetContext()->HSSetConstantBuffers( 2, 1, &m_pLightCB );
    m_pBillboard->Draw( m_gfx.GetContext(), m_gfx.GetShaderController()->GetGeometryData(), &cb1, m_pCB );
    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );

    // Post 2d
    ID3D11ShaderResourceView* ResourceView1;
    if ( m_bIsRTT )
    {
        // RTT to cube or screen like a tv
        D3D11_VIEWPORT vp2;
        vp2.Width = (FLOAT)m_gfx.GetWidth();
        vp2.Height = (FLOAT)m_gfx.GetHeight();
        vp2.MinDepth = 0.0f;
        vp2.MaxDepth = 1.0f;
        vp2.TopLeftX = 0;
        vp2.TopLeftY = 0;
        m_gfx.GetContext()->RSSetViewports( 1, &vp2 );

        m_gfx.GetContext()->OMSetRenderTargets( 1, m_gfx.GetBackBuffer()->GetPtr(), m_gfx.GetDepthStencil()->GetDSV() );
        // Clear the back buffer
        m_gfx.GetContext()->ClearRenderTargetView( m_gfx.GetBackBuffer()->Get(), Colors::LightBlue );
        // Clear the depth buffer to 1.0 (max depth)
        m_gfx.GetContext()->ClearDepthStencilView( m_gfx.GetDepthStencil()->GetDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

        // get the game object world transform
        WorldAsFloat = m_cube.GetTransfrom()->GetWorldMatrix();
        mGO = XMLoadFloat4x4( &WorldAsFloat );

        viewAsFloats = m_pCamController->GetCurentCam()->GetView();
        projectionAsFloats = m_pCamController->GetCurentCam()->GetProjection();

        RTTview = XMLoadFloat4x4( &viewAsFloats );
        RTTprojection = XMLoadFloat4x4( &projectionAsFloats );

        // store this and the view / projection in a constant buffer for the vertex shader to use
        //ConstantBuffer cb1;
        cb1.mWorld = XMMatrixTranspose( mGO );
        cb1.mView = XMMatrixTranspose( RTTview );
        cb1.mProjection = XMMatrixTranspose( RTTprojection );
        cb1.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
        m_gfx.GetContext()->UpdateSubresource( m_pCB, 0, nullptr, &cb1, 0, 0 );

        // Render the cube
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &m_pCB );
        m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, &m_pLightCB );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, &m_pLightCB );

        // Set textures to buffer
        ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
        m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
        m_cube.Draw( m_gfx.GetContext() );

        // Lights
        m_pLightController->Draw( m_gfx.GetContext(), m_pCB, &cb1 );
    }
    else
    {
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &m_pCB );
        m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, &m_pLightCB );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, &m_pLightCB );
        m_pLightController->Draw( m_gfx.GetContext(), m_pCB, &cb1 );
        m_gfx.GetRasterizerController()->SetState( "Back", m_gfx.GetContext() );

        // Render 3d objects
        m_gfx.GetRenderTargetController()->GetRenderTarget( "Depth" )->SetRenderTarget( m_gfx.GetContext() );

        // get the game object world transform
        XMFLOAT4X4 WorldAsFloat = m_cube.GetTransfrom()->GetWorldMatrix();
        XMMATRIX mGO = XMLoadFloat4x4( &WorldAsFloat );

        XMFLOAT4X4 viewAsFloats = m_pCamController->GetCurentCam()->GetView();
        XMFLOAT4X4 projectionAsFloats = m_pCamController->GetCurentCam()->GetProjection();

        XMMATRIX RTTview = XMLoadFloat4x4( &viewAsFloats );
        XMMATRIX RTTprojection = XMLoadFloat4x4( &projectionAsFloats );

        // store this and the view / projection in a constant buffer for the vertex shader to use
        ConstantBuffer cb1;
        cb1.mWorld = XMMatrixTranspose( mGO );
        cb1.mView = XMMatrixTranspose( RTTview );
        cb1.mProjection = XMMatrixTranspose( RTTprojection );
        cb1.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
        m_gfx.GetContext()->UpdateSubresource( m_pCB, 0, nullptr, &cb1, 0, 0 );

        // Render the cube
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Depth" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, &m_pCB );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Depth" ).m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 1, 1, &m_pPostProcessingCB );
        m_cube.Draw( m_gfx.GetContext() );

        WorldAsFloat = m_ground.GetTransfrom()->GetWorldMatrix();
        mGO = XMLoadFloat4x4( &WorldAsFloat );
        cb1.mWorld = XMMatrixTranspose( mGO );
        m_gfx.GetContext()->UpdateSubresource( m_pCB, 0, nullptr, &cb1, 0, 0 );
        m_ground.Draw( m_gfx.GetContext() );
        m_pLightController->Draw( m_gfx.GetContext(), m_pCB, &cb1 );

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
        ID3D11Buffer* pBuffers[1] = { m_pScreenQuadVB };

        // Bloom alpha
        if ( &m_ppSettings.UseBloom )
        {
            m_gfx.GetRenderTargetController()->GetRenderTarget( "Alpha" )->SetRenderTarget( m_gfx.GetContext() );
            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pVertexLayout );
            m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pPixelShader, nullptr, 0 );
            m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &m_pPostProcessingCB );
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
            m_gfx.GetContext()->Draw( 4, 0 );
        }

        // Blur passes
        if ( m_ppSettings.UseBlur || m_ppSettings.UseBloom || m_ppSettings.UseDepthOfF )
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

            // Down sample
            m_gfx.GetRenderTargetController()->GetRenderTarget( "DownSample" )->SetRenderTarget( m_gfx.GetContext() );
            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexLayout );
            m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pPixelShader, nullptr, 0 );
            m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &m_pPostProcessingCB );

            if ( m_ppSettings.UseBloom )
            {
                ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Alpha" )->GetTexture();
            }
            else
            {
                ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
            }
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
            m_gfx.GetContext()->Draw( 4, 0 );

            // Blur 1
            m_gfx.GetRenderTargetController()->GetRenderTarget( "Gaussian1" )->SetRenderTarget( m_gfx.GetContext() );
            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian1" ).m_pVertexLayout );
            m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian1" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian1" ).m_pPixelShader, nullptr, 0 );
            m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &m_pPostProcessingCB );
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "DownSample" )->GetTexture();
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
            m_gfx.GetContext()->Draw( 4, 0 );

            // Blur 2
            m_gfx.GetRenderTargetController()->GetRenderTarget( "Gaussian2" )->SetRenderTarget( m_gfx.GetContext() );
            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian2" ).m_pVertexLayout );
            m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian2" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Gaussian2" ).m_pPixelShader, nullptr, 0 );
            m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &m_pPostProcessingCB );
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

            // Upsample
            m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->SetRenderTarget( m_gfx.GetContext() );
            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexLayout );
            m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pPixelShader, nullptr, 0 );
            m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &m_pPostProcessingCB );
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Gaussian2" )->GetTexture();
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
            m_gfx.GetContext()->Draw( 4, 0 );
        }

        ID3D11ShaderResourceView* ResourceView2;
        ID3D11ShaderResourceView* ResourceView3;
        if ( m_ppSettings.UseDepthOfF )
        {
            // Depth of field
            m_gfx.GetRenderTargetController()->GetRenderTarget( "DepthOfField" )->SetRenderTarget( m_gfx.GetContext() );
            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pVertexLayout );
            m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pPixelShader, nullptr, 0 );
            m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &m_pPostProcessingCB );
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
            ResourceView2 = m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->GetTexture();
            ResourceView3 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Depth" )->GetTexture();
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
            m_gfx.GetContext()->PSSetShaderResources( 1, 1, &ResourceView2 );
            m_gfx.GetContext()->PSSetShaderResources( 2, 1, &ResourceView3 );
            m_gfx.GetContext()->Draw( 4, 0 );
        }

        // Fade implementation
        m_gfx.GetRenderTargetController()->GetRenderTarget( "Fade" )->SetRenderTarget( m_gfx.GetContext() );
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Fade" ).m_pVertexLayout );
        m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Fade" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Fade" ).m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &m_pPostProcessingCB );

        if ( m_ppSettings.UseBlur )
        {
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->GetTexture();
        }
        else if ( m_ppSettings.UseDepthOfF )
        {
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "DepthOfField" )->GetTexture();
        }
        else
        {
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
        }

        m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
        m_gfx.GetContext()->Draw( 4, 0 );

        // Final post processing
        m_gfx.GetContext()->OMSetRenderTargets( 1, m_gfx.GetBackBuffer()->GetPtr(), m_gfx.GetDepthStencil()->GetDSV() );
        m_gfx.GetContext()->ClearRenderTargetView( m_gfx.GetBackBuffer()->Get(), Colors::DarkBlue );
        m_gfx.GetContext()->ClearDepthStencilView( m_gfx.GetDepthStencil()->GetDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0 );
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Final" ).m_pVertexLayout );
        m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Final" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Final" ).m_pPixelShader, nullptr, 0 );
        ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Fade" )->GetTexture();
        ResourceView2 = m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->GetTexture();
        m_gfx.GetContext()->UpdateSubresource( m_pPostProcessingCB, 0, nullptr, &m_ppSettings, 0, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, &m_pPostProcessingCB );
        m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
        m_gfx.GetContext()->PSSetShaderResources( 1, 1, &ResourceView2 );
        m_gfx.GetContext()->Draw( 4, 0 );
    }

    m_pImGuiManager->BeginRender();
    m_pImGuiManager->DrawCamMenu( m_pCamController );
    m_pImGuiManager->ObjectControl( &m_cube );
    m_pImGuiManager->LightControl( m_pLightController );
    m_pImGuiManager->ShaderMenu( m_gfx.GetShaderController(), &m_ppSettings, m_gfx.GetRasterizerController(), m_bIsRTT );
    m_pImGuiManager->BillBoradControl( m_pBillboard );
    m_pImGuiManager->BezierCurveSpline();
    m_pImGuiManager->TerrainControll( m_pTerrain, m_pVoxelTerrain, m_gfx.GetDevice(), m_gfx.GetContext() );
    m_pImGuiManager->AnimationControll( m_pAnimModel );
    m_pImGuiManager->EndRender();

    m_gfx.GetSwapChain()->Present( 1, 0 );
}

float Application::CalculateDeltaTime()
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

void Application::SetupLightForRender()
{
    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = m_pCamController->GetCam( 0 )->GetPositionFloat4();
    lightProperties.Lights[0] = m_pLightController->GetLight( 0 )->GetLightData();
    lightProperties.Lights[1] = m_pLightController->GetLight( 1 )->GetLightData();
    m_gfx.GetContext()->UpdateSubresource( m_pLightCB, 0, nullptr, &lightProperties, 0, 0 );
}

void Application::Cleanup()
{
    m_cube.CleanUp();
    m_ground.CleanUp();

    delete m_pInput;
    m_pInput = nullptr;

    delete m_pCamController;
    m_pCB = nullptr;

    delete m_pLightController;
    m_pLightController = nullptr;

    delete m_pImGuiManager;
    m_pImGuiManager = nullptr;

    delete m_pBillboard;
    m_pBillboard = nullptr;
    delete m_pDepthLight;
    m_pDepthLight = nullptr;

    delete m_pTerrain;
    m_pTerrain = nullptr;

    delete m_pVoxelTerrain;
    m_pVoxelTerrain = nullptr;

    delete m_pAnimModel;
    m_pAnimModel = nullptr;

    // Remove any bound render target or depth/stencil buffer
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_gfx.GetContext()->OMSetRenderTargets( _countof( nullViews ), nullViews, nullptr );

    if ( m_pLightCB )
        m_pLightCB->Release();

    if ( m_pCB ) m_pCB->Release();
    if ( m_pPostProcessingCB )m_pPostProcessingCB->Release();

    if ( m_pScreenQuadVB ) m_pScreenQuadVB->Release();

    ID3D11Debug* debugDevice = nullptr;
    m_gfx.GetDevice()->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &debugDevice ) );

    // handy for finding dx memory leaks
    debugDevice->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );

    if ( debugDevice )
        debugDevice->Release();
}