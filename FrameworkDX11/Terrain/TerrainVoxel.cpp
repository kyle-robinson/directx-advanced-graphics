#include "stdafx.h"
#include "TerrainVoxel.h"

/* REFERENCE: https://sites.google.com/site/letsmakeavoxelengine/ */

#pragma region FRUSTUM-CULLING
void ExtractFrustumPlanes2( XMFLOAT4 frustumPlane[6], CXMMATRIX m )
{
    XMFLOAT4X4 viewProj;
    XMStoreFloat4x4( &viewProj, m );
    // Left Frustum Plane
    // Add first column of the matrix to the fourth column
    frustumPlane[0].x = viewProj._14 + viewProj._11;
    frustumPlane[0].y = viewProj._24 + viewProj._21;
    frustumPlane[0].z = viewProj._34 + viewProj._31;
    frustumPlane[0].w = viewProj._44 + viewProj._41;

    // Right Frustum Plane
    // Subtract first column of matrix from the fourth column
    frustumPlane[1].x = viewProj._14 - viewProj._11;
    frustumPlane[1].y = viewProj._24 - viewProj._21;
    frustumPlane[1].z = viewProj._34 - viewProj._31;
    frustumPlane[1].w = viewProj._44 - viewProj._41;

    // Top Frustum Plane
    // Subtract second column of matrix from the fourth column
    frustumPlane[2].x = viewProj._14 - viewProj._12;
    frustumPlane[2].y = viewProj._24 - viewProj._22;
    frustumPlane[2].z = viewProj._34 - viewProj._32;
    frustumPlane[2].w = viewProj._44 - viewProj._42;

    // Bottom Frustum Plane
    // Add second column of the matrix to the fourth column
    frustumPlane[3].x = viewProj._14 + viewProj._12;
    frustumPlane[3].y = viewProj._24 + viewProj._22;
    frustumPlane[3].z = viewProj._34 + viewProj._32;
    frustumPlane[3].w = viewProj._44 + viewProj._42;

    // Near Frustum Plane
    // We could add the third column to the fourth column to get the near plane,
    // but we don't have to do this because the third column IS the near plane
    frustumPlane[4].x = viewProj._13;
    frustumPlane[4].y = viewProj._23;
    frustumPlane[4].z = viewProj._33;
    frustumPlane[4].w = viewProj._43;

    // Far Frustum Plane
    // Subtract third column of matrix from the fourth column
    frustumPlane[5].x = viewProj._14 - viewProj._13;
    frustumPlane[5].y = viewProj._24 - viewProj._23;
    frustumPlane[5].z = viewProj._34 - viewProj._33;
    frustumPlane[5].w = viewProj._44 - viewProj._43;

    // Normalize the plane equations.
    for ( int i = 0; i < 6; ++i )
    {
        XMVECTOR v = XMPlaneNormalize( XMLoadFloat4( &frustumPlane[i] ) );
        XMStoreFloat4( &frustumPlane[i], v );
    }
}

bool AabbBehindPlaneTest( XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 plane )
{
    XMFLOAT3 n = XMFLOAT3( abs( plane.x ), abs( plane.y ), abs( plane.z ) );

    // This is always positive.
    XMVECTOR a = XMVector3Dot( XMLoadFloat3( &extents ), XMLoadFloat3( &n ) );
    float r = XMVectorGetX( XMVector3Dot( XMLoadFloat3( &extents ), XMLoadFloat3( &n ) ) );
    XMFLOAT4 cnet = XMFLOAT4( center.x, center.y, center.z, 1.0f );
    // signed distance from center point to plane.
    float s = XMVectorGetX( XMVector4Dot( XMLoadFloat4( &cnet ), XMLoadFloat4( &plane ) ) );;

    // If the center point of the box is a distance of e or more behind the
    // plane (in which case s is negative since it is behind the plane),
    // then the box is completely in the negative half space of the plane.
    return ( s + r ) < 0.0f;
}

// Returns true if the box is completely outside the frustum.
bool AabbOutsideFrustumTest( XMFLOAT3 center, XMFLOAT3 min, XMFLOAT3 max, std::vector<XMFLOAT4> frustumPlanes )
{

    bool cull = false;
    for ( size_t planeID = 0; planeID < frustumPlanes.size(); planeID++ )
    {
        XMVECTOR planeNormal = XMVectorSet( frustumPlanes[planeID].x, frustumPlanes[planeID].y, frustumPlanes[planeID].z, 0.0f );
        float planeConstant = frustumPlanes[planeID].w;
        XMFLOAT3 axisVert;
        // x-axis
        if ( frustumPlanes[planeID].x < 0.0f ) // Which AABB vertex is furthest down (plane normals direction) the x axis
            axisVert.x = min.x + center.x; // min x plus tree positions x
        else
            axisVert.x = max.x + center.x; // max x plus tree positions x

        // y-axis
        if ( frustumPlanes[planeID].y < 0.0f ) // Which AABB vertex is furthest down (plane normals direction) the y axis
            axisVert.y = min.y + center.y; // min y plus tree positions y
        else
            axisVert.y = max.y + center.y; // max y plus tree positions y

        // z-axis
        if ( frustumPlanes[planeID].z < 0.0f ) // Which AABB vertex is furthest down (plane normals direction) the z axis
            axisVert.z = min.z + center.z; // min z plus tree positions z
        else
            axisVert.z = max.z + center.z; // max z plus tree positions z

        // Now we get the signed distance from the AABB vertex that's furthest down the frustum planes normal,
        // and if the signed distance is negative, then the entire bounding box is behind the frustum plane, which means
        // that it should be culled
        XMFLOAT3 b;
        XMStoreFloat3( &b, XMVector3Dot( planeNormal, XMLoadFloat3( &axisVert ) ) );
        if ( b.x + planeConstant < 0.0f )
        {
            cull = true;
            return cull;
        }
    }
    return cull;
}
#pragma endregion

#pragma region TERRAIN-VOXEL
TerrainVoxel::TerrainVoxel( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ShaderController* shaderControl, int numOfChunks_X, int numOfChunks_Z ) :
    m_fDefaultChunkSize( XMFLOAT3( 16.0f, 0.0f, 16.0f ) ),
    m_iNumOfChunksX( numOfChunks_X ),
    m_iNumOfChunksZ( numOfChunks_Z ),
    m_iNumOfChunks( numOfChunks_X * numOfChunks_Z )
{
    shaderControl->NewShader( "Voxel", L"Voxel.hlsl", pDevice, pContext );
    m_vChunkData.resize( m_iNumOfChunksX );

    int xIdx = 0;
    // Center the terrain and move it in front of the spawn location
    for ( float x = -( m_iNumOfChunksX / 2 ); x < m_iNumOfChunksX / 2; x++ )
    {
        m_vChunkData.push_back( std::vector<Chunk*>() );
        for ( float z = 0.5f; z < m_iNumOfChunksZ + 0.5f; z++ )
        {
            m_vChunkData[xIdx].push_back( new Chunk( pDevice, pContext,
                XMFLOAT3( ( m_fDefaultChunkSize.x * 2 ) * x, 0,
                ( m_fDefaultChunkSize.z * 2 ) * z ), m_fDefaultChunkSize ) );
        }
        xIdx++;
    }

    try
    {
        HRESULT hr = m_cubeInfoCB.Initialize( pDevice, pContext );
        COM_ERROR_IF_FAILED( hr, "Failed to create VOXEL TERRAIN constant buffer!" );
    }
	catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }
}

TerrainVoxel::~TerrainVoxel()
{
    CleanUp();
}

void TerrainVoxel::Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl, ConstantBuffer<MatrixBuffer>& buffer, CameraController* camControl )
{
    if ( m_bToDraw )
    {
        pContext->VSSetShader( shaderControl->GetShaderByName( "Voxel" ).m_pVertexShader, nullptr, 0 );
        pContext->VSSetConstantBuffers( 0, 1, buffer.GetAddressOf() );
        pContext->PSSetShader( shaderControl->GetShaderByName( "Voxel" ).m_pPixelShader, nullptr, 0 );
        pContext->PSSetConstantBuffers( 3, 1, m_cubeInfoCB.GetAddressOf() );

        // Setup frustrum planes for culling terrain when not in view
        XMFLOAT4X4 viewAsFloats = camControl->GetCam( 0 )->GetView();
        XMFLOAT4X4 projectionAsFloats = camControl->GetCam( 0 )->GetProjection();

        XMMATRIX RTTview = XMLoadFloat4x4( &viewAsFloats );
        XMMATRIX RTTprojection = XMLoadFloat4x4( &projectionAsFloats );
        XMMATRIX viewProject = RTTview * RTTprojection;

        // Setup values for frustum culling
        XMFLOAT4 worldPlanes[6];
        ExtractFrustumPlanes2( worldPlanes, viewProject );
        std::vector<XMFLOAT4> planes;
        planes.push_back( worldPlanes[0] );
        planes.push_back( worldPlanes[1] );
        planes.push_back( worldPlanes[2] );
        planes.push_back( worldPlanes[3] );
        planes.push_back( worldPlanes[4] );
        planes.push_back( worldPlanes[5] );

        for ( auto x : m_vChunkData )
        {
            for ( auto z : x )
            {
                XMFLOAT3 minVertex = XMFLOAT3( FLT_MAX, FLT_MAX, FLT_MAX );
                XMFLOAT3 maxVertex = XMFLOAT3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

                XMFLOAT3 CenterPoint = XMFLOAT3(
                    ( z->GetTransform()->GetPosition().x + m_fDefaultChunkSize.x ),
                    ( z->GetTransform()->GetPosition().y + z->GetMaxHeight() ),
                    ( z->GetTransform()->GetPosition().z + m_fDefaultChunkSize.z ) );
                minVertex = XMFLOAT3( -m_fDefaultChunkSize.x, -1, -m_fDefaultChunkSize.x );
                maxVertex = XMFLOAT3( m_fDefaultChunkSize.x, z->GetMaxHeight(), m_fDefaultChunkSize.x );
                if ( !AabbOutsideFrustumTest( CenterPoint, minVertex, maxVertex, planes ) )
                {
                    z->Draw( pContext, shaderControl, buffer, m_cubeInfoCB, camControl );
                }
            }
        }
    }
}

void TerrainVoxel::RebuildMap( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, int seed, int numOfChunks_X, int numOfChunks_Z, float frequency, int octave )
{
    m_iSeed = seed;
    m_fFrequency = frequency;
    m_iOctaves = octave;

    for ( auto chunk : m_vChunkData )
    {
        for ( auto chunkData : chunk )
        {
            delete chunkData;
            chunkData = nullptr;
        }
        chunk.clear();
    }
    m_vChunkData.clear();

    m_iNumOfChunksX = numOfChunks_X;
    m_iNumOfChunksZ = numOfChunks_Z;
    m_iNumOfChunks = numOfChunks_X * numOfChunks_Z;

    m_vChunkData.resize( m_iNumOfChunksX );
    for ( float x = 0; x < m_iNumOfChunksX; x++ )
    {
        m_vChunkData.push_back( std::vector<Chunk*>() );
        for ( float z = 0; z < m_iNumOfChunksZ; z++ )
        {
            m_vChunkData[x].push_back( new Chunk( pDevice, pContext,
                XMFLOAT3( ( m_fDefaultChunkSize.x * 2 ) * x, 0,
                    ( m_fDefaultChunkSize.z * 2 ) * z ),
                m_fDefaultChunkSize, m_iSeed, m_fFrequency, m_iOctaves ) );
        }
    }
}

void TerrainVoxel::CleanUp()
{
    for ( auto chunk : m_vChunkData )
    {
        for ( auto chunkData : chunk )
        {
            delete chunkData;
            chunkData = nullptr;
        }
        chunk.clear();
    }
    m_vChunkData.clear();
}
#pragma endregion

#pragma region CHUNK
Chunk::Chunk( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, XMFLOAT3 pos, XMFLOAT3 size ) :
    m_iXSize( size.x ),
    m_iZSize( size.z ),
    m_iMaxHeight( size.y )
{
    m_vAllCubesInChunk.resize( m_iXSize );
    m_pChunkTransform = std::make_unique<Transform>();
    m_pChunkTransform->SetPosition( pos );
    GenerateTerrain( pDevice, pContext );
    SetupTextures( pDevice, pContext );
}

Chunk::Chunk( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, XMFLOAT3 pos, XMFLOAT3 size, int seed, float frequency, int octave ) :
    m_iSeed( seed ),
    m_fFrequency( frequency ),
	m_iOctaves( octave )
{
    m_vAllCubesInChunk.resize( m_iXSize );
    m_pChunkTransform = std::make_unique<Transform>();
    m_pChunkTransform->SetPosition( pos );
    GenerateTerrain( pDevice, pContext );
    SetupTextures( pDevice, pContext );
}

Chunk::~Chunk()
{
    CleanUp();
}

void Chunk::Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl,
    ConstantBuffer<MatrixBuffer>& buffer, ConstantBuffer<VoxelCube>& voxelBuffer, CameraController* camControl )
{
    // Setup frustrum planes for culling terrain when not in view
    XMFLOAT4X4 viewAsFloats = camControl->GetCam( 0 )->GetView();
    XMFLOAT4X4 projectionAsFloats = camControl->GetCam( 0 )->GetProjection();

    XMMATRIX RTTview = XMLoadFloat4x4( &viewAsFloats );
    XMMATRIX RTTprojection = XMLoadFloat4x4( &projectionAsFloats );
    XMMATRIX viewProject = RTTview * RTTprojection;

    XMFLOAT4 worldPlanes[6];
    ExtractFrustumPlanes2( worldPlanes, viewProject );
    std::vector<XMFLOAT4> planes;
    planes.push_back( worldPlanes[0] );
    planes.push_back( worldPlanes[1] );
    planes.push_back( worldPlanes[2] );
    planes.push_back( worldPlanes[3] );
    planes.push_back( worldPlanes[4] );
    planes.push_back( worldPlanes[5] );

    for ( auto x : m_vCubesToDraw )
    {
        if ( x->GetIsActive() )
        {
            x->GetTransform()->SetParent( m_pChunkTransform->GetWorldMatrix() );
            XMFLOAT3 center = x->GetTransform()->GetPosition();
            XMFLOAT4X4 world = x->GetTransform()->GetWorldMatrix();
            XMFLOAT3 pos = XMFLOAT3( 0, 0, 0 );
            XMFLOAT3 pos2;
            XMStoreFloat3( &pos2, XMVector3Transform( XMLoadFloat3( &pos ), XMLoadFloat4x4( &world ) ) );
            XMFLOAT3 minVertex = XMFLOAT3( -1, -1, -1 );
            XMFLOAT3 maxVertex = XMFLOAT3( 1, 1, 1 );

            if ( !AabbOutsideFrustumTest( pos2, minVertex, maxVertex, planes ) )
            {
                XMFLOAT4X4 WorldAsFloat = x->GetTransform()->GetWorldMatrix();
                XMMATRIX mGO = XMLoadFloat4x4( &WorldAsFloat );
                buffer.data.mWorld = XMMatrixTranspose( mGO );
                if ( !buffer.ApplyChanges() )
                    return;
				voxelBuffer.data = x->GetCubeData();
                if ( !voxelBuffer.ApplyChanges() )
                    return;
                ID3D11ShaderResourceView* cubeTextures[5u];
                for ( unsigned int i = 0; i < 5u; i++ )
                    cubeTextures[i] = m_vTextures.at( i );
                pContext->PSSetShaderResources( 0u, 5u, cubeTextures );
                x->GetAppearance()->Draw( pContext );
            }
        }
    }
}

void Chunk::GenerateTerrain( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    int maxHeight = 100;
    FastNoiseLite noise;
    noise.SetSeed( m_iSeed );
    noise.SetNoiseType( FastNoiseLite::NoiseType_Perlin );
    noise.SetFrequency( m_fFrequency );
    noise.SetFractalType( FastNoiseLite::FractalType::FractalType_None );
    noise.SetFractalOctaves( m_iOctaves );

    for ( float x = 0; x < m_iXSize; x++ )
    {
        for ( float z = 0; z < m_iZSize; z++ )
        {
            int active = ( ( noise.GetNoise( x + m_pChunkTransform->GetPosition().x, z + m_pChunkTransform->GetPosition().z ) + 1 ) / 2 ) * 20;
            if ( active > maxHeight )
                active = maxHeight;
            if ( m_iMaxHeight < active )
                m_iMaxHeight = active;

            m_vAllCubesInChunk[x].resize( m_iZSize );
            for ( float y = 0; y < maxHeight; y++ )
                m_vAllCubesInChunk[x][z].push_back( new Block( pDevice, pContext ) );
            for ( float y = 0; y < active; y++ )
                m_vAllCubesInChunk[x][z][y]->SetIsActive( true );
        }
    }

    // Create cave systems
    noise.SetNoiseType( FastNoiseLite::NoiseType_Perlin );
    noise.SetFrequency( 0.09f );
    noise.SetFractalType( FastNoiseLite::FractalType::FractalType_None );
    noise.SetFractalOctaves( 6 );
    for ( float x = 0; x < m_iXSize; x++ )
    {
        for ( float z = 0; z < m_iZSize; z++ )
        {
            for ( float y = 0; y < maxHeight; y++ )
            {
                float active = ( ( noise.GetNoise( x + m_pChunkTransform->GetPosition().x, y, z + m_pChunkTransform->GetPosition().z ) + 1 ) / 2 );
                if ( active < 0.5f )
                    m_vAllCubesInChunk[x][z][y]->SetIsActive( false );
            }
        }
    }

    // Set the noise for the cube types
    FastNoiseLite noise2;
    noise.SetNoiseType( FastNoiseLite::NoiseType_Cellular );
    noise.SetFractalType( FastNoiseLite::FractalType::FractalType_None );
    noise.SetFractalOctaves( 3 );

    // Face cube creation
    bool bDefault = false;
    for ( int x = 0; x < m_iXSize; x++ )
    {
        for ( int y = 0; y < m_iXSize; y++ )
        {
            // Remove faces from hidden cubes
            int faceCount = 0;
            for ( int z = 0; z < m_vAllCubesInChunk[x][y].size(); z++ )
            {
                if ( m_vAllCubesInChunk[x][y][z]->GetIsActive() == false )
                    continue;

                bool XNegative = bDefault;
                if ( x > 0 )
                {
                    XNegative = m_vAllCubesInChunk[x - 1][y][z]->GetIsActive();
                    if ( XNegative )
                        faceCount++;
                }

                bool XPositive = bDefault;
                if ( x < m_iXSize - 1 )
                {
                    XPositive = m_vAllCubesInChunk[x + 1][y][z]->GetIsActive();
                    if ( XPositive )
                        faceCount++;
                }

                bool YNegative = bDefault;
                if ( z > 0 )
                {
                    YNegative = m_vAllCubesInChunk[x][y][z - 1]->GetIsActive();
                    if ( YNegative )
                        faceCount++;
                }

                bool YPositive = bDefault;
                if ( z < m_vAllCubesInChunk[x][y].size() )
                {
                    YPositive = m_vAllCubesInChunk[x][y][z + 1]->GetIsActive();
                    if ( YPositive )
                        faceCount++;
                }

                bool ZNegative = bDefault;
                if ( y > 0 )
                {
                    ZNegative = m_vAllCubesInChunk[x][y - 1][z]->GetIsActive();
                    if ( ZNegative )
                        faceCount++;
                }

                bool ZPositive = bDefault;
                if ( y < m_iXSize - 1 )
                {
                    ZPositive = m_vAllCubesInChunk[x][y + 1][z]->GetIsActive();
                    if ( ZPositive )
                        faceCount++;
                }

                if ( !XNegative || !XPositive || !YNegative || !YPositive || !ZNegative || !ZPositive )
                {
                    m_vAllCubesInChunk[x][y][z]->InitMesh_Cube( XNegative, XPositive, YNegative, YPositive, ZNegative, ZPositive, pDevice, pContext );
                    m_vAllCubesInChunk[x][y][z]->GetTransform()->SetPosition( x / 0.5f, z / 0.5f, y / 0.5f );

                    // Set the type for each cube
                    float a = noise2.GetNoise( (float)x, (float)y, (float)z );
                    if ( a < 0 )
                        m_vAllCubesInChunk[x][y][z]->SetBlockType( BlockType::Snow );
                    else if ( a < 0.25f )
                        m_vAllCubesInChunk[x][y][z]->SetBlockType( BlockType::Stone );
                    else if ( a < 0.5f )
                        m_vAllCubesInChunk[x][y][z]->SetBlockType( BlockType::Grass );
                    else if ( a < 0.75f )
                        m_vAllCubesInChunk[x][y][z]->SetBlockType( BlockType::Grass );
                    else if ( a < 1.0f )
                        m_vAllCubesInChunk[x][y][z]->SetBlockType( BlockType::Water );
                }

            }
        }
    }

    // Optimize to limit the number of cubes that are drawn
    for ( int x = 0; x < m_iXSize; x++ )
    {
        for ( int y = 0; y < m_iXSize; y++ )
        {
            for ( int z = 0; z < m_vAllCubesInChunk[x][y].size(); z++ )
            {
                if ( m_vAllCubesInChunk[x][y][z]->GetIsActive() == false )
                    continue;

                if ( m_vAllCubesInChunk[x][y][z]->GetAppearance() == nullptr )
                    m_vAllCubesInChunk[x][y][z]->SetIsActive( false );

                if ( m_vAllCubesInChunk[x][y][z]->GetIsActive() )
                    m_vCubesToDraw.push_back( m_vAllCubesInChunk[x][y][z] );
            }
        }
    }
}

void Chunk::SetupTextures( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    try
    {
        ID3D11ShaderResourceView* pTexture = nullptr;
        HRESULT hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/darkdirt.dds", nullptr, &pTexture );
        COM_ERROR_IF_FAILED( hr, "Failed to create DARK DIRT texture for VOXEL TERRAIN!" );
        m_vTextures.push_back( pTexture );

        hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/lightdirt.dds", nullptr, &pTexture );
        COM_ERROR_IF_FAILED( hr, "Failed to create LIGHT DIRT texture for VOXEL TERRAIN!" );
        m_vTextures.push_back( pTexture );

        hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/grass.dds", nullptr, &pTexture );
        COM_ERROR_IF_FAILED( hr, "Failed to create GRASS texture for VOXEL TERRAIN!" );
        m_vTextures.push_back( pTexture );

        hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/stone.dds", nullptr, &pTexture );
        COM_ERROR_IF_FAILED( hr, "Failed to create STONE texture for VOXEL TERRAIN!" );
        m_vTextures.push_back( pTexture );

        hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/snow.dds", nullptr, &pTexture );
        COM_ERROR_IF_FAILED( hr, "Failed to create SNOW texture for VOXEL TERRAIN!" );
        m_vTextures.push_back( pTexture );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }
}

void Chunk::CleanUp()
{
    for ( auto cubeX : m_vAllCubesInChunk )
    {
        for ( auto cubeY : cubeX )
        {
            for ( auto cubeZ : cubeY )
            {
                delete cubeZ;
                cubeZ = nullptr;
            }
        }
    }
}
#pragma endregion

#pragma region BLOCK
Block::Block( ID3D11Device* pDevice, ID3D11DeviceContext* pContext ) {}

Block::~Block()
{
    CleanUp();
}

void Block::InitMesh_Cube(
    bool XNegative, bool XPositive,
    bool YNegative, bool YPositive,
    bool ZNegative, bool ZPositive,
    ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
    m_pCubeTransform = std::make_unique<Transform>();
    m_pCubeAppearance = std::make_unique<TerrainAppearance>();
    m_pCubeAppearance->InitMesh_Cube( XNegative, XPositive, YNegative, YPositive, ZNegative, ZPositive, pDevice, pContext );
}

void Block::SetBlockType( BlockType block )
{
    m_eBlockType = block;
    m_cubeData.CubeType = (int)block;
}

void Block::CleanUp()
{
}
#pragma endregion