#include "Application.h"
#include "M3dLoader.h"
#include <dxtk/WICTextureLoader.h>

#define WALL_COUNT 3

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

    if ( !InitializeWorld() )
        return false;

    if ( !m_pImGuiManager->Initialize( m_window.GetHWND(), m_gfx.GetDevice(), m_gfx.GetContext() ) )
        return false;

    return true;
}

bool Application::InitializeWorld()
{
    try
    {
        // Create object meshes
		m_pCube = new DrawableGameObject( "Cube" );
        HRESULT hr = m_pCube->GetAppearance()->InitMesh_Cube( m_gfx.GetDevice(), m_gfx.GetContext() );
        COM_ERROR_IF_FAILED( hr, "Failed to create CUBE mesh!" );

        std::vector<std::string> wallNames = { "Bottom Quad", "Back Quad", "Right Quad" };
        for ( unsigned int i = 0; i < WALL_COUNT; i++ )
        {
            DrawableGameObject* wall = new DrawableGameObject( wallNames[i] );
            hr = wall->GetAppearance()->InitMesh_Quad( m_gfx.GetDevice(), m_gfx.GetContext() );
            COM_ERROR_IF_FAILED( hr, "Failed to create a WALL mesh!" );
            m_pWalls.push_back( wall );
        }
        m_pWalls[0]->GetTransform()->SetPosition( 0.0f, -2.5f, 0.0f );
        m_pWalls[0]->GetTransform()->SetRotation( 90.0f, 0.0f, 0.0f );
        m_pWalls[0]->GetTransform()->SetScale( 5.0f, 5.0f, 5.0f );

        m_pWalls[1]->GetTransform()->SetPosition( 0.0f, 2.5f, 5.0f );
        m_pWalls[1]->GetTransform()->SetScale( 5.0f, 5.0f, 5.0f );

        m_pWalls[2]->GetTransform()->SetPosition( 5.0f, 2.5f, 0.0f );
        m_pWalls[2]->GetTransform()->SetRotation( 0.0f, 90.0f, 0.0f );
        m_pWalls[2]->GetTransform()->SetScale( 5.0f, 5.0f, 5.0f );

        // Create the constant buffers
        hr = m_matrixCB.Initialize( m_gfx.GetDevice(), m_gfx.GetContext() );
        COM_ERROR_IF_FAILED( hr, "Failed to create MATRIX constant buffer!" );

        hr = m_lightCB.Initialize( m_gfx.GetDevice(), m_gfx.GetContext() );
        COM_ERROR_IF_FAILED( hr, "Failed to create LIGHT constant buffer!" );

        hr = m_postProcessingCB.Initialize( m_gfx.GetDevice(), m_gfx.GetContext() );
        COM_ERROR_IF_FAILED( hr, "Failed to create POST PROCESSING constant buffer!" );

        // Create full screen buffer
        ScreenVertex svQuad[4] =
        {
            { XMFLOAT3( -1.0f, 1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) },
            { XMFLOAT3( 1.0f, 1.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) },
            { XMFLOAT3( -1.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) },
            { XMFLOAT3( 1.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }
        };
        hr = m_screenVB.Initialize( m_gfx.GetDevice(), svQuad, ARRAYSIZE( svQuad ) );
        COM_ERROR_IF_FAILED( hr, "Failed to create SCREEN VERTEX BUFFER!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return false;
    }

    // Terrain generation
    m_pTerrain = new Terrain( "Resources/Textures/coastMountain513.raw", XMFLOAT2( 513, 513 ),
        100, TerrainGenType::HeightMapLoad, m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController() );
    std::vector<std::string> texGround = { "grass.dds", "darkdirt.dds", "lightdirt.dds", "stone.dds", "snow.dds" };
    m_pTerrain->SetTextures( texGround, m_gfx.GetDevice() );
    m_pTerrain->SetBlendMap( "Resources/Textures/blend.dds", m_gfx.GetDevice() );
    m_pVoxelTerrain = new TerrainVoxel( m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController(), 5, 5 );

    // Create animated model
    m_pSoldier = new AnimatedModel( "Resources/Models/Soldier/soldier.m3d", m_gfx.GetDevice(), m_gfx.GetContext(), m_gfx.GetShaderController() );

    // Create lights
    m_pDepthLight = new ShadowMap( m_gfx.GetDevice(), m_gfx.GetWidth(), m_gfx.GetHeight() );

    m_pLightController->AddLight( "Point", true, LightType::PointLight,
        XMFLOAT4( -3.0f, 0.0f, -3.0f, 0.0f ), XMFLOAT4( Colors::White ),
        XMConvertToRadians( 45.0f ), 1.0f, 0.0f, 0.0f, m_gfx.GetDevice(), m_gfx.GetContext() );
    m_pLightController->GetLight( "Point" )->GetCamera()->SetRotation( 0.0f, XMConvertToRadians( 45.0f ), 0.0f );

    m_pLightController->AddLight( "Spot", true, LightType::SpotLight,
        XMFLOAT4( 0.0f, 5.0f, 0.0f, 0.0f ), XMFLOAT4( Colors::White ),
        XMConvertToRadians( 25.0f ), 1.0f, 0.0f, 0.0f, m_gfx.GetDevice(), m_gfx.GetContext() );
    m_pLightController->GetLight( "Spot" )->GetCamera()->SetRotation( XMConvertToRadians( 90.0f ), 0.0f, 0.0f );

    // Initialize the cameras
    m_pCamController = new CameraController();
    Camera* pCamera = new Camera( XMFLOAT3( 0.0f, 0.0f, -5.0f ), m_gfx.GetWidth(), m_gfx.GetHeight(), 0.01f, 175.0f );
    pCamera->SetName( "Free Camera" );
    m_pCamController->AddCam( pCamera );
    for ( unsigned int i = 0; i < MAX_LIGHTS; i++ )
        m_pCamController->AddCam( m_pLightController->GetLight( i )->GetCamera() );
    m_pInput->AddCamControl( m_pCamController );

    // Initialize the sky
#if defined ( _x64 )
    if ( !m_pSky.Initialize( "Resources/Models/Skysphere/sphere.obj", m_gfx.GetDevice(), m_gfx.GetContext(), m_matrixCB ) )
        return false;
    m_pSky.SetScale( 100.0f, 100.0f, 100.0f );
#endif

    return true;
}

void Application::Update()
{
    float dt = m_timer.GetDeltaTime(); // capped at 60 fps
    if ( dt == 0.0f )
        return;

    m_pInput->Update( dt );
    m_pTerrain->Update();

    // Can't adjust far plane when drawing voxels
    if ( *m_pVoxelTerrain->GetIsDraw() )
        m_pCamController->GetCurrentCam()->SetFar( 100.0f );

#if defined ( _x64 )
    // Update the skybox based on the camera
    m_pSky.SetPosition( m_pCamController->GetCurrentCam()->GetPosition() );
    float farPlane = m_pCamController->GetCurrentCam()->GetFar();
    static float farPlaneOffset = 0.5f;
    m_pSky.SetScale( farPlane * farPlaneOffset,
        farPlane * farPlaneOffset, farPlane * farPlaneOffset );
#endif

    // Update the cube transform, material etc.
    m_pCube->Update( dt, m_gfx.GetContext() );
    for ( unsigned int i = 0; i < m_pWalls.size(); i++ )
        m_pWalls[i]->Update( dt, m_gfx.GetContext() );
    m_pLightController->Update( dt, m_gfx.GetContext(), m_pCamController->GetCurrentCam()->GetName() );
    m_pSoldier->Update( dt );
}

void Application::Draw()
{
    // Setup the viewport
    m_gfx.GetViewports()[0]->Bind( m_gfx.GetContext() );

    // Move objects that will be shadowed into list
    std::vector<DrawableGameObject*> gameObjects;
    gameObjects.push_back( m_pCube );
    for ( unsigned int i = 0; i < m_pWalls.size(); i++ )
        gameObjects.push_back( m_pWalls[i] );

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
    m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
    m_gfx.GetSamplerController()->SetState( "Clamp", 1u, m_gfx.GetContext() );
    m_gfx.GetSamplerController()->SetState( "Border", 2u, m_gfx.GetContext() );

    // get the game object world transform
    XMFLOAT4X4 WorldAsFloat = m_pCube->GetTransform()->GetWorldMatrix();
    XMMATRIX mGO = XMLoadFloat4x4( &WorldAsFloat );

    XMFLOAT4X4 viewAsFloats = m_pCamController->GetCurrentCam()->GetView();
    XMFLOAT4X4 projectionAsFloats = m_pCamController->GetCurrentCam()->GetProjection();

    XMMATRIX RTTview = XMLoadFloat4x4( &viewAsFloats );
    XMMATRIX RTTprojection = XMLoadFloat4x4( &projectionAsFloats );
    XMMATRIX viewProject = RTTview * RTTprojection;

    // Store values to use in vertex shader
    m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
    m_matrixCB.data.mView = XMMatrixTranspose( RTTview );
    m_matrixCB.data.mProjection = XMMatrixTranspose( RTTprojection );
    m_matrixCB.data.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
    m_matrixCB.data.camPos = XMFLOAT4(
        m_pCamController->GetCurrentCam()->GetPosition().x,
        m_pCamController->GetCurrentCam()->GetPosition().y,
        m_pCamController->GetCurrentCam()->GetPosition().z, 0.0f );
    if ( !m_matrixCB.ApplyChanges() )
        return;
    m_lightCB.data.EyePosition = m_pCamController->GetCurrentCam()->GetPositionF4();
    for ( unsigned int i = 0; i < m_pLightController->GetLightList().size(); ++i )
        m_lightCB.data.Lights[i] = m_pLightController->GetLight( i )->GetLightData();
    if ( !m_lightCB.ApplyChanges() )
        return;

    // Render the cube
    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
    m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderData().m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
    ID3D11ShaderResourceView* shadowMaps[MAX_LIGHTS];
    for ( unsigned int i = 0; i < MAX_LIGHTS; i++ )
        shadowMaps[i] = m_pLightController->GetLight( i )->GetShadow()->DepthMapSRV();
    m_gfx.GetContext()->PSSetShaderResources( 3, MAX_LIGHTS, shadowMaps );
    m_pCube->GetAppearance()->SetTextures( m_gfx.GetContext() );
    m_pCube->Draw( m_gfx.GetContext() );

    // Render the walls
    for ( unsigned int i = 0; i < m_pWalls.size(); i++ )
    {
        WorldAsFloat = m_pWalls[i]->GetTransform()->GetWorldMatrix();
        mGO = XMLoadFloat4x4( &WorldAsFloat );
        m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
        if ( !m_matrixCB.ApplyChanges() )
            return;
        m_pWalls[i]->GetAppearance()->SetTextures( m_gfx.GetContext() );
        m_pWalls[i]->Draw( m_gfx.GetContext() );
    }

    // Draw animated model
    m_gfx.GetRasterizerController()->SetState( "None", m_gfx.GetContext() );
    m_pSoldier->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), m_matrixCB );

    // Set set for additional objects
    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );

    // Render the lights
    m_gfx.GetRasterizerController()->SetOverrideState( m_gfx.GetContext() );
    m_pLightController->Draw( m_gfx.GetContext(), m_matrixCB );

    // Render terrain
    m_pVoxelTerrain->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), m_matrixCB, m_pCamController );
    m_pTerrain->Draw( m_gfx.GetContext(), m_gfx.GetShaderController(), m_matrixCB, m_pCamController );

#if defined ( _x64 )
    // Draw skybox
    m_gfx.GetRasterizerController()->SetState( "None", m_gfx.GetContext() );
    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Sky" ).m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Sky" ).m_pPixelShader, nullptr, 0 );

    XMFLOAT4X4 view = m_pCamController->GetCurrentCam()->GetView();
    XMFLOAT4X4 proj = m_pCamController->GetCurrentCam()->GetProjection();
    XMMATRIX viewMat = XMLoadFloat4x4( &view );
    XMMATRIX projMat = XMLoadFloat4x4( &proj );
    m_pSky.Draw( viewMat, projMat );
#endif

    // Post 2d
    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
    m_gfx.GetContext()->VSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Basic" ).m_pPixelShader, nullptr, 0 );
    m_gfx.GetContext()->PSSetConstantBuffers( 2, 1, m_lightCB.GetAddressOf() );
	for ( unsigned int i = 0; i < MAX_LIGHTS; i++ )
        m_pLightController->GetLight( i )->GetLightObject()->GetAppearance()->SetTextures( m_gfx.GetContext() );
    m_pLightController->Draw( m_gfx.GetContext(), m_matrixCB );
    m_gfx.GetRasterizerController()->SetState( "Back", m_gfx.GetContext() );

    // Render 3d objects
    m_gfx.GetRenderTargetController()->GetRenderTarget( "Depth" )->SetRenderTarget( m_gfx.GetContext() );

    WorldAsFloat = m_pCube->GetTransform()->GetWorldMatrix();
    mGO = XMLoadFloat4x4( &WorldAsFloat );

    viewAsFloats = m_pCamController->GetCurrentCam()->GetView();
    projectionAsFloats = m_pCamController->GetCurrentCam()->GetProjection();

    RTTview = XMLoadFloat4x4( &viewAsFloats );
    RTTprojection = XMLoadFloat4x4( &projectionAsFloats );

    // Store matrices in constant buffer
    m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
    m_matrixCB.data.mView = XMMatrixTranspose( RTTview );
    m_matrixCB.data.mProjection = XMMatrixTranspose( RTTprojection );
    m_matrixCB.data.vOutputColor = XMFLOAT4( 0, 0, 0, 0 );
    if ( !m_matrixCB.ApplyChanges() )
        return;

    // Render the cube
    m_gfx.GetContext()->VSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Depth" ).m_pVertexShader, nullptr, 0 );
    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetShaderData().m_pVertexLayout );
    m_gfx.GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    m_gfx.GetContext()->VSSetConstantBuffers( 0, 1, m_matrixCB.GetAddressOf() );
    m_gfx.GetContext()->PSSetShader( m_gfx.GetShaderController()->GetShaderByName( "Depth" ).m_pPixelShader, nullptr, 0 );
    if ( !m_postProcessingCB.ApplyChanges() )
        return;
    m_gfx.GetContext()->PSSetConstantBuffers( 1, 1, m_postProcessingCB.GetAddressOf() );
    m_pCube->Draw( m_gfx.GetContext() );

    // Render the walls
    for ( unsigned int i = 0; i < m_pWalls.size(); i++ )
    {
        WorldAsFloat = m_pWalls[i]->GetTransform()->GetWorldMatrix();
        mGO = XMLoadFloat4x4( &WorldAsFloat );
        m_matrixCB.data.mWorld = XMMatrixTranspose( mGO );
        if ( !m_matrixCB.ApplyChanges() )
            return;
        m_pWalls[i]->GetAppearance()->SetTextures( m_gfx.GetContext() );
        m_pWalls[i]->Draw( m_gfx.GetContext() );
    }

    // Setup the viewport
    m_gfx.GetViewports()[1]->Bind( m_gfx.GetContext() );

    // Bloom alpha
    UINT offset = 0;
    ID3D11ShaderResourceView* ResourceView1;
    if ( &m_postProcessingCB.data.UseBloom )
    {
        m_gfx.GetRenderTargetController()->GetRenderTarget( "Alpha" )->SetRenderTarget( m_gfx.GetContext() );
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Alpha" ).m_pVertexLayout );
        m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, m_screenVB.GetAddressOf(), m_screenVB.StridePtr(), &offset );
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
    if ( m_postProcessingCB.data.UseBlur || m_postProcessingCB.data.UseBloom || m_postProcessingCB.data.UseDepthOfField )
    {
        // Setup the viewport
        m_gfx.GetViewports()[2]->Bind( m_gfx.GetContext() );

        // Down sample
        m_gfx.GetRenderTargetController()->GetRenderTarget( "DownSample" )->SetRenderTarget( m_gfx.GetContext() );
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexLayout );
        m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, m_screenVB.GetAddressOf(), m_screenVB.StridePtr(), &offset );
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
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, m_screenVB.GetAddressOf(), m_screenVB.StridePtr(), &offset );
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
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, m_screenVB.GetAddressOf(), m_screenVB.StridePtr(), &offset );
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
        m_gfx.GetViewports()[3]->Bind( m_gfx.GetContext() );

        // Upsample
        m_gfx.GetRenderTargetController()->GetRenderTarget( "UpSample" )->SetRenderTarget( m_gfx.GetContext() );
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "SolidColour" ).m_pVertexLayout );
        m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, m_screenVB.GetAddressOf(), m_screenVB.StridePtr(), &offset );
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
    if ( m_postProcessingCB.data.UseDepthOfField )
    {
        // Depth of field
        m_gfx.GetRenderTargetController()->GetRenderTarget( "DepthOfField" )->SetRenderTarget( m_gfx.GetContext() );
        m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "DepthOfField" ).m_pVertexLayout );
        m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
        m_gfx.GetContext()->IASetVertexBuffers( 0, 1, m_screenVB.GetAddressOf(), m_screenVB.StridePtr(), &offset );
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
    m_gfx.GetContext()->IASetVertexBuffers( 0, 1, m_screenVB.GetAddressOf(), m_screenVB.StridePtr(), &offset );
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
    else if ( m_postProcessingCB.data.UseDepthOfField )
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
    m_gfx.GetRenderTargetController()->GetRenderTarget( "Final" )->SetRenderTarget( m_gfx.GetContext() );
    m_gfx.GetDepthStencil()->Clear( m_gfx.GetContext() );
    m_gfx.GetContext()->IASetInputLayout( m_gfx.GetShaderController()->GetFullScreenShaderByName( "Final" ).m_pVertexLayout );
    m_gfx.GetSamplerController()->SetState( "Wrap", 0u, m_gfx.GetContext() );
    m_gfx.GetContext()->IASetVertexBuffers( 0, 1, m_screenVB.GetAddressOf(), m_screenVB.StridePtr(), &offset );
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

    m_gfx.GetBackBuffer()->Bind( m_gfx.GetContext(), m_gfx.GetDepthStencil().get(), Colors::DarkBlue );

    // Render ImGui windows
    m_pImGuiManager->BeginRender();
    m_pImGuiManager->SceneWindow( m_gfx.GetWidth(), m_gfx.GetHeight(), m_gfx.GetRenderTargetController()->GetRenderTarget( "Final" )->GetTexture(), m_pInput );
    m_pImGuiManager->CameraMenu( m_pCamController, *m_pVoxelTerrain->GetIsDraw() );
    m_pImGuiManager->ObjectMenu( m_gfx.GetDevice(), m_pCamController->GetCurrentCam(), gameObjects );
    m_pImGuiManager->LightMenu( m_pLightController, m_pCamController->GetCurrentCam() );
    m_pImGuiManager->ShaderMenu( m_gfx.GetShaderController(), &m_postProcessingCB.data, m_gfx.GetRasterizerController() );
    m_pImGuiManager->BezierSplineMenu();
    m_pImGuiManager->TerrainMenu( m_gfx.GetDevice(), m_gfx.GetContext(), m_pTerrain, m_pVoxelTerrain );
    m_pImGuiManager->AnimationMenu( m_pSoldier, m_pCamController->GetCurrentCam() );
    m_pImGuiManager->EndRender();

    m_gfx.GetSwapChain()->Present( 1, 0 );
}

void Application::Cleanup()
{
    m_pCube->CleanUp();
    for ( unsigned int i = 0; i < m_pWalls.size(); i++ )
        m_pWalls[i]->CleanUp();

    delete m_pInput;
    m_pInput = nullptr;

    delete m_pCamController;
    m_pCamController = nullptr;

    delete m_pLightController;
    m_pLightController = nullptr;

    delete m_pImGuiManager;
    m_pImGuiManager = nullptr;

    delete m_pDepthLight;
    m_pDepthLight = nullptr;

    delete m_pTerrain;
    m_pTerrain = nullptr;

    delete m_pVoxelTerrain;
    m_pVoxelTerrain = nullptr;

    delete m_pSoldier;
    m_pSoldier = nullptr;

    // Remove any bound render target or depth/stencil buffer
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_gfx.GetContext()->OMSetRenderTargets( _countof( nullViews ), nullViews, nullptr );

    ID3D11Debug* debugDevice = nullptr;
    m_gfx.GetDevice()->QueryInterface( __uuidof( ID3D11Debug ), reinterpret_cast<void**>( &debugDevice ) );

    // handy for finding dx memory leaks
    debugDevice->ReportLiveDeviceObjects( D3D11_RLDO_DETAIL );

    if ( debugDevice )
        debugDevice->Release();
}