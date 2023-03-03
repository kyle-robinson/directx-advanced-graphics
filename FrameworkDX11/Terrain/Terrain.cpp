#include "stdafx.h"
#include "Terrain.h"
#include <ostream>
#include <fstream>

// Noise Generator : https://github.com/Auburn/FastNoiseLite
#include "fastNoiseLite\Cpp\FastNoiseLite.h"
#include "DirectXPackedVector.h"

#pragma region FRUSTUM
void ExtractFrustumPlanes( XMFLOAT4 frustumPlane[6], CXMMATRIX m )
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
#pragma endregion

Terrain::Terrain( std::string heightMapName, XMFLOAT2 size, double scale, TerrainGenType genType, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ShaderController* shaderControl ) :
    m_sHeightMapName( heightMapName ),
    m_iHeightMapWidth( size.x ),
    m_iHeightMapHeight( size.y ),
    m_fHeightScale( scale ),
    m_eTerrainCreationType( genType )
{
    m_pTransform = new Transform();
    m_pTransform->SetPosition( 0.0f, -50.0f, 0.0f );
    m_pAppearance = new TerrainAppearance( m_iHeightMapHeight, m_iHeightMapWidth, 1.0f, m_vHeightMapData );
    shaderControl->NewTessellationShader( "Terrain", L"Terrain.hlsl", pDevice, pContext );

    CreateHeightData();
    BuildHeightMap( pDevice );

    m_pAppearance->SetHeightData( m_vHeightMapData );
    m_pAppearance->CalcAllPatchBoundsY();
    m_pAppearance->InitMesh_Terrain( pDevice );

    try
    {
        // Create the terrain constant buffer
        HRESULT hr = m_terrainCB.Initialize( pDevice, pContext );
        COM_ERROR_IF_FAILED( hr, "Failed to create TERRAIN constant buffer!" );

        m_terrainCB.data.MaxDist = 500.0f;
        m_terrainCB.data.MinDist = 20.0f;
        m_terrainCB.data.MaxTess = 6.0f;
        m_terrainCB.data.MinTess = 0.0f;
        m_terrainCB.data.Layer1MaxHeight = 20.0f;
        m_terrainCB.data.Layer2MaxHeight = 30.0f;
        m_terrainCB.data.Layer3MaxHeight = 40.0f;
        m_terrainCB.data.Layer4MaxHeight = 50.0f;
        m_terrainCB.data.Layer5MaxHeight = 55.0f;
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }
}

Terrain::~Terrain()
{
    CleanUp();
}

void Terrain::Update() {}

void Terrain::Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl, ConstantBuffer<MatrixBuffer>& buffer, CameraController* camControl )
{
    if ( m_bToDraw )
    {
        pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST );
        pContext->IASetInputLayout( shaderControl->GetShaderByName( "Terrain" ).m_pVertexLayout );

        XMFLOAT4X4 worldAsFloat = m_pTransform->GetWorldMatrix();
        XMMATRIX mGO = XMLoadFloat4x4( &worldAsFloat );
        buffer.data.mWorld = XMMatrixTranspose( mGO );
		if ( !buffer.ApplyChanges() )
			return;

        // Setup frustrum planes for culling terrain when not in view
        XMFLOAT4X4 viewAsFloats = camControl->GetCam( 0 )->GetView();
        XMFLOAT4X4 projectionAsFloats = camControl->GetCam( 0 )->GetProjection();

        XMMATRIX RTTview = XMLoadFloat4x4( &viewAsFloats );
        XMMATRIX RTTprojection = XMLoadFloat4x4( &projectionAsFloats );
        XMMATRIX viewProject = RTTview * RTTprojection;
        XMFLOAT4 worldPlanes[6];
        ExtractFrustumPlanes( worldPlanes, viewProject );
        m_terrainCB.data.WorldFrustumPlanes[0] = worldPlanes[0];
        m_terrainCB.data.WorldFrustumPlanes[1] = worldPlanes[1];
        m_terrainCB.data.WorldFrustumPlanes[2] = worldPlanes[2];
        m_terrainCB.data.WorldFrustumPlanes[3] = worldPlanes[3];
        m_terrainCB.data.WorldFrustumPlanes[4] = worldPlanes[4];
        m_terrainCB.data.WorldFrustumPlanes[5] = worldPlanes[5];
        m_terrainCB.data.gEyePosition = camControl->GetCam( 0 )->GetPositionFloat4();
        if ( !m_terrainCB.ApplyChanges() )
            return;

        // Setup shaders
        pContext->VSSetShader( shaderControl->GetShaderByName( "Terrain" ).m_pVertexShader, nullptr, 0 );
        pContext->VSSetConstantBuffers( 0, 1, buffer.GetAddressOf() );
        pContext->HSSetConstantBuffers( 4, 1, m_terrainCB.GetAddressOf() );
        pContext->VSSetShaderResources( 1, 1, &m_pHeightMapSRV );

        pContext->HSSetShader( shaderControl->GetShaderByName( "Terrain" ).m_pHullShader, nullptr, 0 );
        pContext->HSSetConstantBuffers( 0, 1, buffer.GetAddressOf() );
        pContext->HSSetConstantBuffers( 4, 1, m_terrainCB.GetAddressOf() );
        pContext->HSSetShaderResources( 1, 1, &m_pHeightMapSRV );

        pContext->DSSetShader( shaderControl->GetShaderByName( "Terrain" ).m_pDomainShader, nullptr, 0 );
        pContext->DSSetConstantBuffers( 0, 1, buffer.GetAddressOf() );
        pContext->DSSetConstantBuffers( 4, 1, m_terrainCB.GetAddressOf() );
        pContext->DSSetShaderResources( 1, 1, &m_pHeightMapSRV );

        pContext->PSSetShader( shaderControl->GetShaderByName( "Terrain" ).m_pPixelShader, nullptr, 0 );
        pContext->PSSetShaderResources( 0, 1, m_pAppearance->GetTextureResourceView().data() );
        pContext->PSSetConstantBuffers( 4, 1, m_terrainCB.GetAddressOf() );
        pContext->PSSetShaderResources( 0, 1, &m_pBlendMap );
        pContext->PSSetShaderResources( 1, 1, &m_pHeightMapSRV );
        pContext->PSSetShaderResources( 2, 5, m_pAppearance->GetTextureResourceView().data() );
        m_pAppearance->DrawTerrain( pContext );

        // As HLSL sets tessellation stages, but does not disable them, do that here
        pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        pContext->HSSetShader( 0, 0, 0 );
        pContext->DSSetShader( 0, 0, 0 );
    }
}

void Terrain::FaultHeightFormation()
{
    m_vHeightMapData.clear();
    m_vHeightMapData.resize( m_iHeightMapHeight * m_iHeightMapWidth, 0 );

    for ( size_t i = 0; i < m_iNumOfIterations; i++ )
    {
        // Line formula : ax + bz = c
        float v = RandomHelper::RandomFloat<float>( 0, RAND_MAX, m_iSeed );
        float a = sin( v );
        float b = cos( v );
        float d = std::sqrt( m_iHeightMapHeight * m_iHeightMapHeight + m_iHeightMapWidth * m_iHeightMapWidth );
        float c = RandomHelper::RandomFloat<float>( -d / 2, d / 2, m_iSeed );

        for ( UINT z = 0; z < m_iHeightMapHeight; ++z )
        {
            for ( UINT x = 0; x < m_iHeightMapWidth; ++x )
            {
                if ( a * x + b * z - c > 0 )
                {
                    // One side of the line goes up
                    m_vHeightMapData[z * m_iHeightMapWidth + x] += m_fDisplacement;
                }
                else
                {
                    // The other side of the line goes down
                    m_vHeightMapData[z * m_iHeightMapWidth + x] -= m_fDisplacement;
                }
            }
        }
    }
}

void Terrain::HeightFromNoise()
{
    FastNoiseLite noise;
    noise.SetSeed( m_iSeed );
    noise.SetNoiseType( FastNoiseLite::NoiseType_Perlin );
    noise.SetFrequency( m_fFrequency );
    noise.SetFractalOctaves( m_iNumOfOctaves );

    // Gather noise data
    int index = 0;
    m_vHeightMapData.resize( m_iHeightMapHeight * m_iHeightMapWidth, 0 );

    for ( int y = 0; y < m_iHeightMapHeight; y++ )
    {
        for ( int x = 0; x < m_iHeightMapWidth; x++ )
        {
            // Get value between 0 and 1 * by height scale
            m_vHeightMapData[y * m_iHeightMapWidth + x] = ( ( noise.GetNoise( (float)x, (float)y ) + 1 ) / 2 ) * m_fHeightScale;
        }
    }
}

void Terrain::DiamondSquareHeightMap()
{
    m_v2DHeightMap.clear();
    m_v2DHeightMap.resize( m_iHeightMapWidth );

    for ( size_t i = 0; i < m_v2DHeightMap.size(); i++ )
    {
        m_v2DHeightMap[i].resize( m_iHeightMapWidth );
    }

    // Generate a seed for the map
    m_v2DHeightMap[0][0] = RandomHelper::Random<int>( 0, 255, m_iSeed );
    m_v2DHeightMap[0][m_iHeightMapWidth - 1] = RandomHelper::Random<int>( 0, 255, m_iSeed );
    m_v2DHeightMap[m_iHeightMapWidth - 1][0] = RandomHelper::Random<int>( 0, 255, m_iSeed );
    m_v2DHeightMap[m_iHeightMapWidth - 1][m_iHeightMapWidth - 1] = RandomHelper::Random<int>( 0, 255, m_iSeed );

    int sideLength = m_iHeightMapWidth / 2;
    Diamond( m_iHeightMapWidth );
    Square( m_iHeightMapWidth );
    m_iRange /= 2;

    while ( sideLength >= 2 )
    {
        Diamond( sideLength + 1 );
        Square( sideLength + 1 );
        sideLength /= 2;
        m_iRange /= 2;
    }

    // Convert to 1D height map
    m_vHeightMapData.resize( m_iHeightMapWidth * m_iHeightMapWidth );
    for ( int z = 0; z < m_iHeightMapWidth; ++z )
    {
        for ( int x = 0; x < m_iHeightMapWidth; ++x )
        {
            m_vHeightMapData[z * m_iHeightMapWidth + x] = ( m_v2DHeightMap[x][z] / 255 ) * m_fHeightScale;
        }
    }
}

void Terrain::CreateHeightData()
{
    switch ( m_eTerrainCreationType )
    {
    case TerrainGenType::HeightMapLoad:
        LoadHeightMap();
        break;
    case TerrainGenType::FaultLine:
        FaultHeightFormation();
        break;
    case TerrainGenType::Noise:
        HeightFromNoise();
        break;
    case TerrainGenType::DiamondSquare:
        DiamondSquareHeightMap();
        break;
    default:
        LoadHeightMap();
        break;
    }
    Smooth();
}

void Terrain::SetMaxTess( float maxTess )
{
    m_terrainCB.data.MaxTess = maxTess;
}

void Terrain::SetMinTess( float minTess )
{
    m_terrainCB.data.MinTess = minTess;
}

void Terrain::SetMaxTessDist( float maxTessDist )
{
    m_terrainCB.data.MaxDist = maxTessDist;
}

void Terrain::SetMinTessDist( float minTessDist )
{
    m_terrainCB.data.MinDist = minTessDist;
}

bool Terrain::InBounds( int i, int j )
{
    // True if ij are valid indices; false otherwise.
    return
        i >= 0 && i < (int)m_iHeightMapHeight &&
        j >= 0 && j < (int)m_iHeightMapWidth;
}

float Terrain::Average( int i, int j )
{
    float avg = 0.0f;
    float num = 0.0f;

    for ( int m = i - 1; m <= i + 1; ++m )
    {
        for ( int n = j - 1; n <= j + 1; ++n )
        {
            if ( InBounds( m, n ) )
            {
                avg += m_vHeightMapData[m * m_iHeightMapWidth + n];
                num += 1.0f;
            }
        }
    }

    return avg / num;
}

void Terrain::Smooth()
{
    std::vector<float> dest( m_vHeightMapData.size() );
    for ( UINT i = 0; i < m_iHeightMapHeight; ++i )
    {
        for ( UINT j = 0; j < m_iHeightMapWidth; ++j )
        {
            dest[i * m_iHeightMapWidth + j] = Average( i, j );
        }
    }

    m_vHeightMapData = dest;
}

void Terrain::SetBlendMap( std::string name, ID3D11Device* pd3dDevice )
{
    std::wstring wide_string = std::wstring( name.begin(), name.end() );
    const wchar_t* result = wide_string.c_str();
    CreateDDSTextureFromFile( pd3dDevice, result, nullptr, &m_pBlendMap );
}

float Terrain::GetHeightWorld( float x, float z )
{
    return 0.0f;
}

float Terrain::GetHeight( float x, float z )
{
    return m_vHeightMapData[x * m_iHeightMapWidth + z];
}

void Terrain::ReBuildTerrain( XMFLOAT2 size, double scale, float cellSpacing, TerrainGenType genType, ID3D11Device* pDevice )
{
    m_fHeightScale = scale;
    m_iHeightMapWidth = size.x;
    m_iHeightMapHeight = size.y;
    m_eTerrainCreationType = genType;
    m_fCellSpacing = cellSpacing;

    CreateHeightData();
    BuildHeightMap( pDevice );

    m_pAppearance->SetWidth( size.x );
    m_pAppearance->SetDepth( size.y );
    m_pAppearance->SetCellSpacing( m_fCellSpacing );

    m_pAppearance->SetHeightData( m_vHeightMapData );
    m_pAppearance->CalcAllPatchBoundsY();
    m_pAppearance->InitMesh_Terrain( pDevice );
}

void Terrain::SetTexHeights( float height1, float height2, float height3, float height4, float height5 )
{
    m_terrainCB.data.Layer1MaxHeight = height1;
    m_terrainCB.data.Layer2MaxHeight = height2;
    m_terrainCB.data.Layer3MaxHeight = height3;
    m_terrainCB.data.Layer4MaxHeight = height4;
    m_terrainCB.data.Layer5MaxHeight = height5;
}

void Terrain::SetTextures( std::vector<std::string> texGroundName, ID3D11Device* pDevice )
{
    m_pAppearance->SetTextures( texGroundName, pDevice );
    m_vTexGround = texGroundName;
}

void Terrain::SetTexture( int index, std::string texName, ID3D11ShaderResourceView* texture )
{
    m_pAppearance->SetTexture( index, texture );
    m_vTexGround[index] = texName;
}

void Terrain::BuildHeightMap( ID3D11Device* pDevice )
{
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = m_iHeightMapWidth;
    texDesc.Height = m_iHeightMapHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    std::vector<PackedVector::HALF> hmap( m_vHeightMapData.size() );
    std::transform( m_vHeightMapData.begin(), m_vHeightMapData.end(),
        hmap.begin(), PackedVector::XMConvertFloatToHalf );

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &hmap[0];
    data.SysMemPitch = m_iHeightMapWidth * sizeof( PackedVector::HALF );
    data.SysMemSlicePitch = 0;
    ID3D11Texture2D* hmapTex = 0;
    HRESULT hr = pDevice->CreateTexture2D( &texDesc, &data, &hmapTex );
    if ( FAILED( hr ) )
        return;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;
    hr = pDevice->CreateShaderResourceView( hmapTex, &srvDesc, &m_pHeightMapSRV );
    if ( FAILED( hr ) )
        return;

    hmapTex->Release();
}

void Terrain::LoadHeightMap()
{
    // Set a height for each vertex
    std::vector<unsigned char> in( m_iHeightMapWidth * m_iHeightMapHeight );

    // Open the file
    std::ifstream inFile;
    inFile.open( m_sHeightMapName, std::ios_base::binary );

    if ( inFile )
    {
        // Read the RAW bytes
        inFile.read( (char*)&in[0], (std::streamsize)in.size() );
        inFile.close();
    }

    // Copy the array data into a float array and scale it
    m_vHeightMapData.resize( m_iHeightMapHeight * m_iHeightMapWidth, 0 );
    for ( UINT i = 0; i < m_iHeightMapHeight * m_iHeightMapWidth; ++i )
    {
        m_vHeightMapData[i] = ( in[i] / 255.0f ) * m_fHeightScale;
    }
}

void Terrain::Diamond( int sideLength )
{
    int halfSide = sideLength / 2;

    for ( int y = 0; y < m_iHeightMapWidth / ( sideLength - 1 ); y++ )
    {
        for ( int x = 0; x < m_iHeightMapWidth / ( sideLength - 1 ); x++ )
        {
            int center_x = x * ( sideLength - 1 ) + halfSide;
            int center_y = y * ( sideLength - 1 ) + halfSide;

            // Get average of points on vertex
            int avg = ( m_v2DHeightMap[x * ( sideLength - 1 )][y * ( sideLength - 1 )] +
                m_v2DHeightMap[x * ( sideLength - 1 )][( y + 1 ) * ( sideLength - 1 )] +
                m_v2DHeightMap[( x + 1 ) * ( sideLength - 1 )][y * ( sideLength - 1 )] +
                m_v2DHeightMap[( x + 1 ) * ( sideLength - 1 )][( y + 1 ) * ( sideLength - 1 )] ) / 4.0f;

            // Add a random number
            m_v2DHeightMap[center_x][center_y] = avg + RandomHelper::Random<int>( -m_iRange, m_iRange, m_iSeed );
        }
    }
}

void Terrain::Average( int x, int y, int sideLength )
{
    float counter = 0.0f;
    float accumulator = 0.0f;
    int halfSide = sideLength / 2;

    // Get points of data
    if ( x != 0 )
    {
        counter += 1.0f;
        accumulator += m_v2DHeightMap[y][x - halfSide];
    }
    if ( y != 0 )
    {
        counter += 1.0f;
        accumulator += m_v2DHeightMap[y - halfSide][x];
    }
    if ( x != m_iHeightMapWidth - 1 )
    {
        counter += 1.0f;
        accumulator += m_v2DHeightMap[y][x + halfSide];
    }
    if ( y != m_iHeightMapWidth - 1 )
    {
        counter += 1.0f;
        accumulator += m_v2DHeightMap[y + halfSide][x];
    }

    // Get the average, then add a random value to it
    m_v2DHeightMap[y][x] = ( accumulator / counter ) + RandomHelper::Random<int>( -m_iRange, m_iRange, m_iSeed );
}

void Terrain::Square( int sideLength )
{
    int halfLength = sideLength / 2;

    for ( int y = 0; y < m_iHeightMapWidth / ( sideLength - 1 ); y++ )
    {
        for ( int x = 0; x < m_iHeightMapWidth / ( sideLength - 1 ); x++ )
        {
            // Top
            Average( x * ( sideLength - 1 ) + halfLength, y * ( sideLength - 1 ), sideLength );

            // Right
            Average( ( x + 1 ) * ( sideLength - 1 ), y * ( sideLength - 1 ) + halfLength, sideLength );

            // Bottom
            Average( x * ( sideLength - 1 ) + halfLength, ( y + 1 ) * ( sideLength - 1 ), sideLength );

            // Left
            Average( x * ( sideLength - 1 ), y * ( sideLength - 1 ) + halfLength, sideLength );
        }
    }
}

void Terrain::CleanUp()
{
    if ( m_pTransform )
    {
        delete m_pTransform;
        m_pTransform = nullptr;
    }
    if ( m_pAppearance )
    {
        delete m_pAppearance;
        m_pAppearance = nullptr;
    }
    if ( m_pHeightMapSRV )
    {
        m_pHeightMapSRV->Release();
        m_pHeightMapSRV = nullptr;
    }
    if ( m_pBlendMap )
    {
        m_pBlendMap->Release();
        m_pBlendMap = nullptr;
    }
}