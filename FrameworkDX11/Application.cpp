#include "Application.h"
#include "M3dLoader.h"

Application::Application()
{
    m_pLightController = new LightController();
    m_pImGuiManager = new ImGuiManager();
    m_pInput = new Input();
}

Application::~Application()
{
    Cleanup();
}

bool Application::Initialize( HINSTANCE hInstance, int width, int height )
{
    if ( !m_window.Initialize( m_pInput, hInstance, "DirectX 11 Advanced Graphics & Rendering", "TutorialWindowClass", width, height ) )
        return false;

    m_gfx.Initialize( m_window.GetHWND(), width, height );

    if ( !InitDevice() )
        return false;

    if ( !m_pImGuiManager->Initialize( m_window.GetHWND(), m_gfx.GetDevice(), m_gfx.GetContext() ) )
        return false;

    return true;
}

bool Application::InitMesh()
{
    try
    {
        // Create object meshes
        HRESULT hr = m_cube.GetAppearance()->InitMesh_Cube( m_gfx.GetDevice(), m_gfx.GetContext() );
        COM_ERROR_IF_FAILED( hr, "Failed to create CUBE mesh!" );

        hr = m_ground.GetAppearance()->InitMesh_Quad( m_gfx.GetDevice() );
        COM_ERROR_IF_FAILED( hr, "Failed to create GROUND mesh!" );
        m_ground.GetTransfrom()->SetPosition( -5, -2, 5 );

        // Create the constant buffers
        hr = m_matrixCB.Initialize( m_gfx.GetDevice(), m_gfx.GetContext() );
        COM_ERROR_IF_FAILED( hr, "Failed to create MATRIX constant buffer!" );

        hr = m_lightCB.Initialize( m_gfx.GetDevice(), m_gfx.GetContext() );
        COM_ERROR_IF_FAILED( hr, "Failed to create LIGHT constant buffer!" );

        hr = m_postProcessingCB.Initialize( m_gfx.GetDevice(), m_gfx.GetContext() );
        COM_ERROR_IF_FAILED( hr, "Failed to create POST PROCESSING constant buffer!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    // Terrain generation
    m_pTerrain = new Terrain( "Resources/Textures/coastMountain513.raw", XMFLOAT2( 513, 513 ),
        100, TerrainGenType::HeightMapLoad, m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController() );
    std::vector<std::string> texGround;
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
    m_pAnimModel = new AnimatedModel( "Resources/AnimModel/soldier.m3d", m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController() );

    return true;
}

std::vector<float> CubicBezierBasis( float u )
{
    float compla = 1 - u; // complement of u
    // compute value of basis functions for given value of u
    float BF0 = compla * compla * compla;
    float BF1 = 3.0 * u * compla * compla;
    float BF2 = 3.0 * u * u * compla;
    float BF3 = u * u * u;

    std::vector<float> bfArray = { BF0, BF1, BF2, BF3 };
    return bfArray;
}

std::vector<XMFLOAT3> CubicBezierCurve( std::vector<XMFLOAT3> controlPoints )
{
    std::vector<XMFLOAT3> points;
    for ( float i = 0.0f; i < 1.0f; i += 0.1f )
    {
        // Calculate value of each basis function for current u
        std::vector<float> basisFnValues = CubicBezierBasis( i );

        XMFLOAT3 sum = XMFLOAT3{ 0.0f,0.0f,0.0f };
        for ( int cPointIndex = 0; cPointIndex <= 3; cPointIndex++ )
        {
            // Calculate weighted sum (weightx * CPx)
            sum.x += controlPoints[cPointIndex].x * basisFnValues[cPointIndex];
            sum.y += controlPoints[cPointIndex].y * basisFnValues[cPointIndex];
            sum.z += controlPoints[cPointIndex].z * basisFnValues[cPointIndex];
        }

        XMFLOAT3 point = sum; // point for current u on cubic Bezier curve
        points.push_back( point );
    }
    return points;
}

bool Application::InitWorld()
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

    // Setup bezier spline
    std::vector<XMFLOAT3> a = {
        XMFLOAT3{ 0.0f,0.0f,0.0f },
        XMFLOAT3{ 2.0f,1.0f,0.0f },
        XMFLOAT3{ 5.0f,0.6f,0.0f },
        XMFLOAT3{ 6.0f,0.0f,0.0f } };
    m_pImGuiManager->SetPoints( CubicBezierCurve( a ) );

    // Post settings
    m_postProcessingCB.data.UseColour = false;
    m_postProcessingCB.data.Color = XMFLOAT4{ 1.0f,1.0f,1.0f,0.0f };
    m_postProcessingCB.data.UseBloom = false;
    m_postProcessingCB.data.UseDepthOfF = false;
    m_postProcessingCB.data.UseHDR = false;
    m_postProcessingCB.data.UseBlur = false;
    m_postProcessingCB.data.FadeAmount = 1.0f;
    m_postProcessingCB.data.FarPlane = 100.0f;
    m_postProcessingCB.data.FocalDistance = 100.0f;
    m_postProcessingCB.data.FocalWidth = 100.0f;
    m_postProcessingCB.data.BlurAttenuation = 0.5f;

    try
    {
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
        COM_ERROR_IF_FAILED( hr, "Failed to create SCREEN vertex buffer!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    return true;
}

bool Application::InitDevice()
{
    // Create shadow map light
    m_pDepthLight = new ShadowMap( m_gfx.GetDevice(), m_gfx.GetWidth(), m_gfx.GetHeight() );

    try
    {
	    // Initialize scene objects and data
        HRESULT hr = InitMesh();
        COM_ERROR_IF_FAILED( hr, "Failed to initialize OBJECT DATA!" );

        hr = InitWorld();
        COM_ERROR_IF_FAILED( hr, "Failed to initialize WORLD DATA!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    // Create lights
    m_pLightController->AddLight( "MainPoint", true, LightType::PointLight,
        XMFLOAT4( 0.0f, 0.0f, -4.0f, 0.0f ), XMFLOAT4( Colors::White ),
        XMConvertToRadians( 45.0f ), 1.0f, 0.0f, 0.0f, m_gfx.GetDevice(), m_gfx.GetContext() );

    m_pLightController->AddLight( "Point", true, LightType::SpotLight,
        XMFLOAT4( 0.0f, 5.0f, 0.0f, 0.0f ), XMFLOAT4( Colors::White ),
        XMConvertToRadians( 10.0f ), 1.0f, 0.0f, 0.0f, m_gfx.GetDevice(), m_gfx.GetContext() );

    return true;
}

void Application::Update()
{
    float dt = m_timer.GetDeltaTime(); // capped at 60 fps
    if ( dt == 0.0f )
        return;

    m_pInput->Update( dt );
    m_pCamController->Update();
    m_pTerrain->Update();

    // Update the cube transform, material etc.
    m_cube.Update( dt, m_gfx.GetContext() );
    m_ground.Update( dt, m_gfx.GetContext() );
    m_pLightController->Update( dt, m_gfx.GetContext() );
    m_pBillboard->UpdatePositions( m_gfx.GetContext() );
    m_pAnimModel->Update( dt );
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
    std::vector<DrawableGameObject*> gameObjects;
    gameObjects.push_back( &m_cube );
    gameObjects.push_back( &m_ground );

    // Create shadow depth stencils
    for ( UINT i = 0; i < MAX_LIGHTS; i++ )
    {
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "DepthLight" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "DepthLight" ).m_pPixelShader, nullptr, 0 );
        m_pLightController->GetLight( i )->CreateShadowMap( m_gfx.GetContext(), gameObjects, m_matrixCB );
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

    // Store values to use in vertex shader
    m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
    m_matrixCB.data.mView = XMMatrixTranspose( RTTview );
    m_matrixCB.data.mProjection = XMMatrixTranspose( RTTprojection );
    m_matrixCB.data.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
    m_matrixCB.data.camPos = XMFLOAT4(
        m_pCamController->GetCurentCam()->GetPosition().x,
        m_pCamController->GetCurentCam()->GetPosition().y,
        m_pCamController->GetCurentCam()->GetPosition().z, 0.0f );
    if ( !m_matrixCB.ApplyChanges() )
        return;
    SetupLightForRender();

    // Render the cube
    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
    m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );

    ID3D11ShaderResourceView* ShadowMaps[2];
    ShadowMaps[0] = m_pLightController->GetLight( 0 )->GetShadow()->DepthMapSRV();
    ShadowMaps[1] = m_pLightController->GetLight( 1 )->GetShadow()->DepthMapSRV();
    m_gfx.GetContext()->PSSetShaderResources( 3, 2, ShadowMaps );

    // Set textures to buffer
    m_cube.GetAppearance()->SetTextures( m_gfx.GetContext() );
    m_cube.Draw( m_gfx.GetContext() );

    m_gfx.GetRasterizerController()->SetState( "None", m_gfx.GetContext() );
    m_pAnimModel->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), m_matrixCB );
    m_gfx.GetRasterizerController()->SetOverrideState( m_gfx.GetContext() );

    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );

    WorldAsFloat = m_ground.GetTransfrom()->GetWorldMatrix();
    mGO = XMLoadFloat4x4( &WorldAsFloat );
    m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
    if ( !m_matrixCB.ApplyChanges() )
        return;
    m_ground.GetAppearance()->SetTextures( m_gfx.GetContext() );
    m_ground.Draw( m_gfx.GetContext() );

    // Render terrain
    m_pVoxelTerrain->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), m_matrixCB, m_pCamController );
    m_pTerrain->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), m_matrixCB, m_pCamController );
    m_gfx.GetContext()->HSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
    m_pBillboard->Draw( m_gfx.GetContext(), m_gfx.GetShaderController()->GetGeometryData(), m_matrixCB );
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

		// Clear the back buffer and depth stencil view
        m_gfx.GetContext()->OMSetRenderTargets( 1, m_gfx.GetBackBuffer()->GetPtr(), m_gfx.GetDepthStencil()->GetDSV() );
        m_gfx.GetContext()->ClearRenderTargetView( m_gfx.GetBackBuffer()->Get(), Colors::LightBlue );
        m_gfx.GetContext()->ClearDepthStencilView( m_gfx.GetDepthStencil()->GetDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

        // get the game object world transform
        WorldAsFloat = m_cube.GetTransfrom()->GetWorldMatrix();
        mGO = XMLoadFloat4x4( &WorldAsFloat );

        viewAsFloats = m_pCamController->GetCurentCam()->GetView();
        projectionAsFloats = m_pCamController->GetCurentCam()->GetProjection();

        RTTview = XMLoadFloat4x4( &viewAsFloats );
        RTTprojection = XMLoadFloat4x4( &projectionAsFloats );

        // store this and the view / projection in a constant buffer for the vertex shader to use
        m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
        m_matrixCB.data.mView = XMMatrixTranspose( RTTview );
        m_matrixCB.data.mProjection = XMMatrixTranspose( RTTprojection );
        m_matrixCB.data.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
		if ( !m_matrixCB.ApplyChanges() )
			return;

        // Render the cube
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
        m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );

        // Set textures to buffer
        ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
        m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
        m_cube.Draw( m_gfx.GetContext() );

        // Lights
        m_pLightController->Draw( m_gfx.GetContext(), m_matrixCB );
    }
    else
    {
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
        m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pPixelShader, nullptr, 0 );
        m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
        m_pLightController->Draw( m_gfx.GetContext(), m_matrixCB );
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
        m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
        m_matrixCB.data.mView = XMMatrixTranspose( RTTview );
        m_matrixCB.data.mProjection = XMMatrixTranspose( RTTprojection );
        m_matrixCB.data.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
        if ( !m_matrixCB.ApplyChanges() )
            return;

        // Render the cube
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
        m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Depth" ).m_pVertexShader, nullptr, 0 );
        m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
        m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Depth" ).m_pPixelShader, nullptr, 0 );
        if ( !m_postProcessingCB.ApplyChanges() )
            return;
        m_gfx.GetContext()->PSSetConstantBuffers( 1, 1, m_postProcessingCB.GetAddressOf() );
        m_cube.Draw( m_gfx.GetContext() );

        WorldAsFloat = m_ground.GetTransfrom()->GetWorldMatrix();
        mGO = XMLoadFloat4x4( &WorldAsFloat );
        m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
        if ( !m_matrixCB.ApplyChanges() )
            return;
        m_ground.Draw( m_gfx.GetContext() );
        m_pLightController->Draw( m_gfx.GetContext(), m_matrixCB );

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
        if ( &m_postProcessingCB.data.UseBloom )
        {
            m_gfx.GetRenderTargetController()->GetRenderTarget( "Alpha" )->SetRenderTarget( m_gfx.GetContext() );
            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pVertexLayout );
            m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pPixelShader, nullptr, 0 );
            if ( !m_postProcessingCB.ApplyChanges() )
                return;
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, m_postProcessingCB.GetAddressOf() );
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "RTT" )->GetTexture();
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
            m_gfx.GetContext()->Draw( 4, 0 );
        }

        // Blur passes
        if ( m_postProcessingCB.data.UseBlur || m_postProcessingCB.data.UseBloom || m_postProcessingCB.data.UseDepthOfF )
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
            if ( !m_postProcessingCB.ApplyChanges() )
                return;
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, m_postProcessingCB.GetAddressOf() );

            if ( m_postProcessingCB.data.UseBloom )
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
            if ( !m_postProcessingCB.ApplyChanges() )
                return;
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, m_postProcessingCB.GetAddressOf() );
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
            if ( !m_postProcessingCB.ApplyChanges() )
                return;
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, m_postProcessingCB.GetAddressOf() );
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
            if ( !m_postProcessingCB.ApplyChanges() )
                return;
			m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, m_postProcessingCB.GetAddressOf() );
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "Gaussian2" )->GetTexture();
            m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
            m_gfx.GetContext()->Draw( 4, 0 );
        }

        ID3D11ShaderResourceView* ResourceView2;
        ID3D11ShaderResourceView* ResourceView3;
        if ( m_postProcessingCB.data.UseDepthOfF )
        {
            // Depth of field
            m_gfx.GetRenderTargetController()->GetRenderTarget( "DepthOfField" )->SetRenderTarget( m_gfx.GetContext() );
            m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pVertexLayout );
            m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
            m_gfx.GetContext()->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
            m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
            m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pVertexShader, nullptr, 0 );
            m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pPixelShader, nullptr, 0 );
            if ( !m_postProcessingCB.ApplyChanges() )
                return;
            m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, m_postProcessingCB.GetAddressOf() );
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
        if ( !m_postProcessingCB.ApplyChanges() )
            return;
		m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, m_postProcessingCB.GetAddressOf() );

        if ( m_postProcessingCB.data.UseBlur )
        {
            ResourceView1 = m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->GetTexture();
        }
        else if ( m_postProcessingCB.data.UseDepthOfF )
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
        if ( !m_postProcessingCB.ApplyChanges() )
            return;
        m_gfx.GetContext()->PSSetConstantBuffers( 0, 1, m_postProcessingCB.GetAddressOf() );
        m_gfx.GetContext()->PSSetShaderResources( 0, 1, &ResourceView1 );
        m_gfx.GetContext()->PSSetShaderResources( 1, 1, &ResourceView2 );
        m_gfx.GetContext()->Draw( 4, 0 );
    }

    m_pImGuiManager->BeginRender();
    m_pImGuiManager->CameraMenu( m_pCamController );
    m_pImGuiManager->ObjectMenu( &m_cube );
    m_pImGuiManager->LightMenu( m_pLightController );
    m_pImGuiManager->ShaderMenu( m_gfx.GetShaderController(), &m_postProcessingCB.data, m_gfx.GetRasterizerController(), m_bIsRTT );
    m_pImGuiManager->BillboardMenu( m_pBillboard );
    m_pImGuiManager->BezierSplineMenu();
    m_pImGuiManager->TerrainMenu( m_pTerrain, m_pVoxelTerrain, m_gfx.GetDevice(), m_gfx.GetContext() );
    m_pImGuiManager->AnimationMenu( m_pAnimModel );
    m_pImGuiManager->EndRender();

    m_gfx.GetSwapChain()->Present( 1, 0 );
}

void Application::SetupLightForRender()
{
    m_lightCB.data.EyePosition = m_pCamController->GetCam( 0 )->GetPositionFloat4();
    m_lightCB.data.Lights[0] = m_pLightController->GetLight( 0 )->GetLightData();
    m_lightCB.data.Lights[1] = m_pLightController->GetLight( 1 )->GetLightData();
    if ( !m_lightCB.ApplyChanges() )
        return;
}

void Application::Cleanup()
{
    m_cube.CleanUp();
    m_ground.CleanUp();

    delete m_pInput;
    m_pInput = nullptr;

    delete m_pCamController;
    m_pCamController = nullptr;

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

    if ( m_pScreenQuadVB ) m_pScreenQuadVB->Release();

    ID3D11Debug* debugDevice = nullptr;
    m_gfx.GetDevice()->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &debugDevice ) );

    // handy for finding dx memory leaks
    debugDevice->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );

    if ( debugDevice )
        debugDevice->Release();
}