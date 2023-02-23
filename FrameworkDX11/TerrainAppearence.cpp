#include "stdafx.h"
#include "TerrainAppearence.h"
#include "DDSTextureLoader.h"

static const float Infinity = FLT_MAX;

template<typename T>
static T Min( const T& a, const T& b )
{
	return a < b ? a : b;
}

template<typename T>
static T Max( const T& a, const T& b )
{
	return a > b ? a : b;
}

TerrainAppearence::TerrainAppearence() {}

TerrainAppearence::TerrainAppearence( UINT width, UINT length, float cellSpacing, std::vector<float> heightMap ) :
	m_fHeightMapWidth( width ),
	m_fHeightMapHeight( length ),
	m_fCellSpacing( cellSpacing ),
	m_vHeightMapData( heightMap )
{
	// Divide heightmap into patches such that each patch has CellsPerPatch.
	m_iNumPatchVertRows = ( ( m_fHeightMapHeight - 1 ) / m_iCellsPerPatch ) + 1;
	m_iNumPatchVertCols = ( ( m_fHeightMapWidth - 1 ) / m_iCellsPerPatch ) + 1;

	m_iNumPatchVertices = m_iNumPatchVertRows * m_iNumPatchVertCols;
	m_iNumPatchQuadFaces = ( m_iNumPatchVertRows - 1 ) * ( m_iNumPatchVertCols - 1 );
}

TerrainAppearence::~TerrainAppearence() {}

void TerrainAppearence::InitMesh_Terrain( ID3D11Device* pDevice )
{
	BuildPatchVertex( pDevice );
	BuildPatchIndex( pDevice );

	m_material.Material.Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_material.Material.Specular = XMFLOAT4( 1.0f, 0.2f, 0.2f, 1.0f );
	m_material.Material.SpecularPower = 32.0f;
	m_material.Material.UseTexture = true;
	m_material.Material.Emissive = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_material.Material.Ambient = XMFLOAT4( 0.1f, 0.1f, 0.1f, 1.0f );
	m_material.Material.HeightScale = 0.1f;
	m_material.Material.MaxLayers = 15.0f;
	m_material.Material.MinLayers = 10.0f;
}

void TerrainAppearence::InitMesh_Cube(
	bool lXNegative, bool lXPositive,
	bool lYNegative, bool lYPositive,
	bool lZNegative, bool lZPositive,
	ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	if ( lXNegative && lXPositive && lYNegative && lYPositive && lZNegative && lZPositive )
	{
		m_bToDraw = false;
		return;
	}

	// Store vertices
	SimpleVertex vertices[] =
	{
		// Top
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 3
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 1
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) }, // 0

		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) }, // 2
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 1
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 3

		// Bottom
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 6
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 4
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) }, // 5

		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) }, // 7
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 4
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, -1.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 6

		// Left
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 11
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 9
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) }, // 8

		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) }, // 10
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 9
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( -1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 11

		// Right
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 14
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 12
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 1.0f ) }, // 13

		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 0.0f ) }, // 15
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 12
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 1.0f, 0.0f, 0.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 14

		// Front
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 19
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ) , XMFLOAT2( 1.0f, 1.0f ) }, // 17
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) }, // 16

		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) }, // 18
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ) , XMFLOAT2( 1.0f, 1.0f ) }, // 17
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3( 0.0f, 0.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 19

		// Back
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 22
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 20
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) }, // 21

		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) }, // 23
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) }, // 20
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT3( 0.0f, 0.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) }, // 22
	};

	// Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( SimpleVertex ) * NUM_VERTICES;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	HRESULT hr = pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
	if ( FAILED( hr ) )
		return;

	// Create index buffer
	std::vector<WORD> ind;
	if ( !lYPositive )
	{
		ind.push_back( 0 );
		ind.push_back( 1 );
		ind.push_back( 2 );

		ind.push_back( 3 );
		ind.push_back( 4 );
		ind.push_back( 5 );
	}
	if ( !lYNegative )
	{
		ind.push_back( 6 );
		ind.push_back( 7 );
		ind.push_back( 8 );

		ind.push_back( 9 );
		ind.push_back( 10 );
		ind.push_back( 11 );
	}
	if ( !lXNegative )
	{
		ind.push_back( 12 );
		ind.push_back( 13 );
		ind.push_back( 14 );

		ind.push_back( 15 );
		ind.push_back( 16 );
		ind.push_back( 17 );
	}
	if ( !lXPositive )
	{
		ind.push_back( 18 );
		ind.push_back( 19 );
		ind.push_back( 20 );

		ind.push_back( 21 );
		ind.push_back( 22 );
		ind.push_back( 23 );
	}
	if ( !lZNegative )
	{
		ind.push_back( 24 );
		ind.push_back( 25 );
		ind.push_back( 26 );

		ind.push_back( 27 );
		ind.push_back( 28 );
		ind.push_back( 29 );
	}
	if ( !lZPositive )
	{
		ind.push_back( 30 );
		ind.push_back( 31 );
		ind.push_back( 32 );

		ind.push_back( 33 );
		ind.push_back( 34 );
		ind.push_back( 35 );
	}

	if ( ind.size() > 0 )
	{
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof( WORD ) * ind.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = &ind[0];
		hr = pDevice->CreateBuffer( &bd, &InitData, &m_pIndexBuffer );
		if ( FAILED( hr ) )
			return ;

		pContext->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	}
	else
	{
		m_bToDraw = false;
	}

	pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_iNumberOfVert = ind.size();
}

void TerrainAppearence::SetTexture( std::vector<std::string> texName, ID3D11Device* pDevice )
{
	ID3D11ShaderResourceView* res;
	for ( auto texName : texName )
	{
		std::wstring wide_string = std::wstring( texName.begin(), texName.end() );
		const wchar_t* result = wide_string.c_str();
		CreateDDSTextureFromFile( pDevice, result, nullptr, &res );
		m_vGroundTextureRV.push_back( res );
	}
}

float TerrainAppearence::GetWidth()
{
	// Total terrain width.
	return ( m_fHeightMapWidth - 1 ) * m_fCellSpacing;
}

void TerrainAppearence::SetWidth( float width )
{
	m_fHeightMapWidth = width;
	// Divide heightmap into patches such that each patch has CellsPerPatch.
	m_iNumPatchVertCols = ( ( m_fHeightMapWidth - 1 ) / m_iCellsPerPatch ) + 1;
	m_iNumPatchVertices = m_iNumPatchVertRows * m_iNumPatchVertCols;
	m_iNumPatchQuadFaces = ( m_iNumPatchVertRows - 1 ) * ( m_iNumPatchVertCols - 1 );
}

float TerrainAppearence::GetDepth()
{
	// Total terrain depth.
	return ( m_fHeightMapHeight - 1 ) * m_fCellSpacing;
}

void TerrainAppearence::SetDepth( float depth )
{
	m_fHeightMapHeight = depth;
	// Divide heightmap into patches such that each patch has CellsPerPatch.
	m_iNumPatchVertRows = ( ( m_fHeightMapHeight - 1 ) / m_iCellsPerPatch ) + 1;
	m_iNumPatchVertices = m_iNumPatchVertRows * m_iNumPatchVertCols;
	m_iNumPatchQuadFaces = ( m_iNumPatchVertRows - 1 ) * ( m_iNumPatchVertCols - 1 );
}

void TerrainAppearence::BuildPatchVertex( ID3D11Device* pDevice )
{
	// Build the terrain grid
	std::vector<TerrainVert> patchVertices( m_iNumPatchVertRows * m_iNumPatchVertCols );
	float halfWidth = 0.5f * GetWidth();
	float halfDepth = 0.5f * GetDepth();
	float patchWidth = GetWidth() / ( m_iNumPatchVertCols - 1 );
	float patchDepth = GetDepth() / ( m_iNumPatchVertRows - 1 );
	float du = 1.0f / ( m_iNumPatchVertCols - 1 );
	float dv = 1.0f / ( m_iNumPatchVertRows - 1 );
	for ( UINT i = 0; i < m_iNumPatchVertRows; ++i )
	{
		float z = halfDepth - i * patchDepth;
		for ( UINT j = 0; j < m_iNumPatchVertCols; ++j )
		{
			float x = -halfWidth + j * patchWidth;
			patchVertices[i * m_iNumPatchVertCols + j].Pos = XMFLOAT3( x, 0.0f, z );

			// Stretch the texture over the grid.
			patchVertices[i * m_iNumPatchVertCols + j].Tex.x = j * du;
			patchVertices[i * m_iNumPatchVertCols + j].Tex.y = i * dv;
		}
	}

	// Store axis-aligned bounding box y-bounds in upper-left patch corner.
	for ( UINT i = 0; i < m_iNumPatchVertRows - 1; ++i )
	{
		for ( UINT j = 0; j < m_iNumPatchVertCols - 1; ++j )
		{
			UINT patchID = i * ( m_iNumPatchVertCols - 1 ) + j;
			patchVertices[i * m_iNumPatchVertCols + j].BoundsY = m_vPatchBoundsY[patchID];
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof( TerrainVert ) * patchVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &patchVertices[0];
	HRESULT hr = pDevice->CreateBuffer( &vbd, &vinitData, &m_pVertexBuffer );
	if ( FAILED( hr ) )
		return;
}

void TerrainAppearence::BuildPatchIndex( ID3D11Device* pDevice )
{
	std::vector<USHORT> indices( m_iNumPatchQuadFaces * 4 );

	// Iterate over each quad and compute indices.
	int k = 0;
	for ( UINT i = 0; i < m_iNumPatchVertRows - 1; ++i )
	{
		for ( UINT j = 0; j < m_iNumPatchVertCols - 1; ++j )
		{
			// Top row of 2x2 quad patch
			indices[k] = i * m_iNumPatchVertCols + j;
			indices[k + 1] = i * m_iNumPatchVertCols + j + 1;

			// Bottom row of 2x2 quad patch
			indices[k + 2] = ( i + 1 ) * m_iNumPatchVertCols + j;
			indices[k + 3] = ( i + 1 ) * m_iNumPatchVertCols + j + 1;
			k += 4; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof( USHORT ) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HRESULT hr = pDevice->CreateBuffer( &ibd, &iinitData, &m_pIndexBuffer );
	if ( FAILED( hr ) )
		return;

	m_iNumberOfVert = m_iNumPatchQuadFaces * 4;
}

void TerrainAppearence::CalcAllPatchBoundsY()
{
	m_vPatchBoundsY.resize( m_iNumPatchQuadFaces );

	// For each patch
	for ( UINT i = 0; i < m_iNumPatchVertRows - 1; ++i )
	{
		for ( UINT j = 0; j < m_iNumPatchVertCols - 1; ++j )
		{
			CalcPatchBoundsY( i, j );
		}
	}
}
void TerrainAppearence::Draw( ID3D11DeviceContext* pContext )
{
	if ( m_bToDraw )
	{
		ID3D11Buffer* materialCB = GetMaterialConstantBuffer();
		pContext->PSSetConstantBuffers( 1, 1, &materialCB );

		// Set vertex buffer
		UINT stride = sizeof( SimpleVertex );
		UINT offset = 0;

		pContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
		pContext->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
		pContext->DrawIndexed( m_iNumberOfVert, 0, 0 );
	}
}

void TerrainAppearence::CalcPatchBoundsY( UINT i, UINT j )
{
	// Scan the heightmap values this patch covers and compute the min/max height.
	UINT x0 = j * m_iCellsPerPatch;
	UINT x1 = ( j + 1 ) * m_iCellsPerPatch;

	UINT y0 = i * m_iCellsPerPatch;
	UINT y1 = ( i + 1 ) * m_iCellsPerPatch;

	float minY = +Infinity;
	float maxY = -Infinity;
	for ( UINT y = y0; y <= y1; ++y )
	{
		for ( UINT x = x0; x <= x1; ++x )
		{
			UINT k = y * m_fHeightMapWidth + x;
			minY = Min<float>( minY, m_vHeightMapData[k] );
			maxY = Max<float>( maxY, m_vHeightMapData[k] );
		}
	}

	UINT patchID = i * ( m_iNumPatchVertCols - 1 ) + j;
	m_vPatchBoundsY[patchID] = XMFLOAT2( minY, maxY );
}