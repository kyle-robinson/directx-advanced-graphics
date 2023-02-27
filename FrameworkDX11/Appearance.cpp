#include "stdafx.h"
#include "Appearance.h"

Appearance::Appearance()
{
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_pTextureResourceView = nullptr;
	m_pSamplerLinear = nullptr;
}

Appearance::~Appearance()
{
	CleanUp();
}

void Appearance::Update( ID3D11DeviceContext* pContext )
{
	if ( !m_materialCB.ApplyChanges() )
		return;
}

void Appearance::Draw( ID3D11DeviceContext* pContext )
{
	pContext->PSSetConstantBuffers( 1, 1, m_materialCB.GetAddressOf() );

	// Set vertex buffer
	UINT stride = sizeof( SimpleVertex );
	UINT offset = 0;
	pContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	pContext->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	pContext->PSSetSamplers( 0, 1, &m_pSamplerLinear );
	pContext->DrawIndexed( m_iNumberOfVert, 0, 0 );
}

void Appearance::Draw( ID3D11DeviceContext* pContext, int vertToDraw, int start )
{
	UINT stride = m_uVertexStride;
	UINT offset = 0;
	pContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	pContext->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	pContext->PSSetSamplers( 0, 1, &m_pSamplerLinear );
	pContext->DrawIndexed( vertToDraw, start, 0 );
}

void Appearance::SetTextures( ID3D11DeviceContext* pContext )
{
	pContext->PSSetShaderResources( 0, 1, &m_pTextureResourceView );
	pContext->PSSetShaderResources( 1, 1, &m_pNormalMapResourceView );
	pContext->PSSetShaderResources( 2, 1, &m_pParallaxMapResourceView );
}

HRESULT Appearance::InitMesh_Cube( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	// Create vertex buffer
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

	// Calculate tangent and bitangents
	CalculateModelVectors( vertices, NUM_VERTICES );

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( SimpleVertex ) * NUM_VERTICES;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	HRESULT hr = pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
	if ( FAILED( hr ) )
		return hr;

	// Set vertex buffer
	UINT stride = sizeof( SimpleVertex );
	UINT offset = 0;
	pContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );

	// Create index buffer
	WORD indices[] =
	{
		0,1,2,
		3,4,5,

		6,7,8,
		9,10,11,

		12,13,14,
		15,16,17,

		18,19,20,
		21,22,23,

		24,25,26,
		27,28,29,

		30,31,32,
		33,34,35
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( WORD ) * NUM_VERTICES;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pDevice->CreateBuffer( &bd, &InitData, &m_pIndexBuffer );
	if ( FAILED( hr ) )
		return hr;

	// Set index buffer
	pContext->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	pContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	m_iNumberOfVert = NUM_VERTICES;

	// Setup textures
	hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/bricks_TEX.dds", nullptr, &m_pTextureResourceView );
	if ( FAILED( hr ) )
		return hr;

	hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/bricks_NORM.dds", nullptr, &m_pNormalMapResourceView );
	if ( FAILED( hr ) )
		return hr;

	hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/bricks_DISP.dds", nullptr, &m_pParallaxMapResourceView );
	if ( FAILED( hr ) )
		return hr;

	// Create sampler
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof( sampDesc ) );
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pDevice->CreateSamplerState( &sampDesc, &m_pSamplerLinear );

	// Create material constant buffer
	hr = m_materialCB.Initialize( pDevice, pContext );
	COM_ERROR_IF_FAILED( hr, "Failed to create MATERIAL constant buffer!" );

	// Setup material properties
	m_materialCB.data.Material.Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_materialCB.data.Material.Specular = XMFLOAT4( 1.0f, 0.2f, 0.2f, 1.0f );
	m_materialCB.data.Material.SpecularPower = 32.0f;
	m_materialCB.data.Material.UseTexture = true;
	m_materialCB.data.Material.Emissive = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_materialCB.data.Material.Ambient = XMFLOAT4( 0.1f, 0.1f, 0.1f, 1.0f );
	m_materialCB.data.Material.HeightScale = 0.1f;
	m_materialCB.data.Material.MaxLayers = 15.0f;
	m_materialCB.data.Material.MinLayers = 10.0f;

	return hr;
}

HRESULT Appearance::InitMesh_Quad( ID3D11Device* pDevice, ID3D11DeviceContext* pContext )
{
	// Create quad with height data
	int cols = 2;
	int rows = 2;
	int numVertices = rows * cols;
	int numFaces = ( rows - 1 ) * ( cols - 1 ) * 2;
	float fWidth = 10.0f;
	float fHeight = 10.0f;

	// Define a vertex
	std::vector<SimpleVertex> v;
	float dx = fWidth / ( cols - 1 );
	float dz = fHeight / ( rows - 1 );
	float textCordY = 1.0f;
	float textCordX = 0.0f;
	float increasX = 1.0f / cols;
	float increasZ = 1.0f / rows;

	// Create vertex data
	for ( UINT i = 0; i < rows; i++ )
	{
		for ( UINT j = 0; j < cols; j++ )
		{
			v.push_back( {
				XMFLOAT3{ (float)( -0 * fWidth + j * dx ),
				0, (float)( 0 * fHeight - i * dz ) },
				XMFLOAT3( 0.0f, 1.0f, 0.0f ),
				XMFLOAT2( textCordX,textCordY ),
				XMFLOAT3( 0.0f,0.0f,0.0f ),
				XMFLOAT3( 0.0f,0.0f,0.0f )
			} );
			textCordX += increasX;
		}
		textCordY -= increasZ;
		textCordX = 0;
	}

	// Create indices
	std::vector<WORD>indices;
	for ( UINT i = 0; i < rows - 1; i++ )
	{
		for ( UINT j = 0; j < cols - 1; j++ )
		{
			indices.push_back( i * cols + j );
			indices.push_back( i * cols + ( j + 1 ) );
			indices.push_back( ( i + 1 ) * cols + j );
			indices.push_back( ( i + 1 ) * cols + j );
			indices.push_back( i * cols + ( j + 1 ) );
			indices.push_back( ( i + 1 ) * cols + ( j + 1 ) );
		}
	}

	// Create vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof( bd ) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( SimpleVertex ) * v.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof( InitData ) );
	InitData.pSysMem = &v[0];
	HRESULT hr = pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );

	// Create index buffer
	ZeroMemory( &bd, sizeof( bd ) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( WORD ) * indices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory( &InitData, sizeof( InitData ) );
	InitData.pSysMem = &indices[0];
	hr = pDevice->CreateBuffer( &bd, &InitData, &m_pIndexBuffer );

	m_iNumberOfVert = indices.size();

	hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/bricks_TEX.dds", nullptr, &m_pTextureResourceView );
	if ( FAILED( hr ) )
		return hr;

	hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/bricks_NORM.dds", nullptr, &m_pNormalMapResourceView );
	if ( FAILED( hr ) )
		return hr;

	hr = CreateDDSTextureFromFile( pDevice, L"Resources/Textures/bricks_DISP.dds", nullptr, &m_pParallaxMapResourceView );
	if ( FAILED( hr ) )
		return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof( sampDesc ) );
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pDevice->CreateSamplerState( &sampDesc, &m_pSamplerLinear );

	// Create material constant buffer
	hr = m_materialCB.Initialize( pDevice, pContext );
	COM_ERROR_IF_FAILED( hr, "Failed to create MATERIAL constant buffer!" );

	m_materialCB.data.Material.Diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	m_materialCB.data.Material.Specular = XMFLOAT4( 1.0f, 0.2f, 0.2f, 1.0f );
	m_materialCB.data.Material.SpecularPower = 32.0f;
	m_materialCB.data.Material.UseTexture = true;
	m_materialCB.data.Material.Emissive = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_materialCB.data.Material.Ambient = XMFLOAT4( 0.1f, 0.1f, 0.1f, 1.0f );
	m_materialCB.data.Material.HeightScale = 0.1f;
	m_materialCB.data.Material.MaxLayers = 15.0f;
	m_materialCB.data.Material.MinLayers = 10.0f;

	return hr;
}

HRESULT Appearance::SetVertexBuffer( ID3D11Device* pDevice, std::vector<SkinedVertex> vertices, UINT count )
{
	m_uVertexStride = sizeof( SkinedVertex );
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( SkinedVertex ) * count;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = &vertices[0];
	HRESULT hr = pDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
	if ( FAILED( hr ) )
		return hr;
}

HRESULT Appearance::SetIndices( ID3D11Device* device, const USHORT* indices, UINT count )
{
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof( USHORT ) * count;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;

	HRESULT HR = device->CreateBuffer( &ibd, &iinitData, &m_pIndexBuffer );
	return HR;
}

void Appearance::CalculateTangentBinormalLH( SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& Tangent, XMFLOAT3& binormal )
{
	XMFLOAT3 edge1( v1.Pos.x - v0.Pos.x, v1.Pos.y - v0.Pos.y, v1.Pos.z - v0.Pos.z );
	XMFLOAT3 edge2( v2.Pos.x - v0.Pos.x, v2.Pos.y - v0.Pos.y, v2.Pos.z - v0.Pos.z );

	XMFLOAT2 deltaUV1( v1.TexCoord.x - v0.TexCoord.x, v1.TexCoord.y - v0.TexCoord.y );
	XMFLOAT2 deltaUV2( v2.TexCoord.x - v0.TexCoord.x, v2.TexCoord.y - v0.TexCoord.y );

	float f = 1.0f / ( deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y );

	Tangent.x = f * ( deltaUV2.y * edge1.x - deltaUV1.y * edge2.x );
	Tangent.y = f * ( deltaUV2.y * edge1.y - deltaUV1.y * edge2.y );
	Tangent.z = f * ( deltaUV2.y * edge1.z - deltaUV1.y * edge2.z );
	XMVECTOR tn = XMLoadFloat3( &Tangent );
	tn = XMVector3Normalize( tn );
	XMStoreFloat3( &Tangent, tn );

	binormal.x = f * ( deltaUV1.x * edge2.x - deltaUV2.x * edge1.x );
	binormal.y = f * ( deltaUV1.x * edge2.y - deltaUV2.x * edge1.y );
	binormal.z = f * ( deltaUV1.x * edge2.z - deltaUV2.x * edge1.z );

	tn = XMLoadFloat3( &binormal );
	tn = XMVector3Normalize( tn );
	XMStoreFloat3( &binormal, tn );

	XMVECTOR vv0 = XMLoadFloat3( &v0.Pos );
	XMVECTOR vv1 = XMLoadFloat3( &v1.Pos );
	XMVECTOR vv2 = XMLoadFloat3( &v2.Pos );

	XMVECTOR e0 = vv1 - vv0;
	XMVECTOR e1 = vv2 - vv0;

	XMVECTOR e01cross = XMVector3Cross( e0, e1 );
	e01cross = XMVector3Normalize( e01cross );
	XMFLOAT3 normalOut;
	XMStoreFloat3( &normalOut, e01cross );
	normal = normalOut;
}

void Appearance::CalculateTangentBinormalRH( SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& Tangent, XMFLOAT3& binormal )
{
	// http://softimage.wiki.softimage.com/xsidocs/tex_Tangents_binormals_AboutTangentsandBinormals.html

	// 1. CALCULATE THE NORMAL
	XMVECTOR vv0 = XMLoadFloat3( &v0.Pos );
	XMVECTOR vv1 = XMLoadFloat3( &v1.Pos );
	XMVECTOR vv2 = XMLoadFloat3( &v2.Pos );

	XMVECTOR P = vv1 - vv0;
	XMVECTOR Q = vv2 - vv0;

	XMVECTOR e01cross = XMVector3Cross( P, Q );
	XMFLOAT3 normalOut;
	XMStoreFloat3( &normalOut, e01cross );
	normal = normalOut;

	// 2. CALCULATE THE TANGENT from texture space

	float s1 = v1.TexCoord.x - v0.TexCoord.x;
	float t1 = v1.TexCoord.y - v0.TexCoord.y;
	float s2 = v2.TexCoord.x - v0.TexCoord.x;
	float t2 = v2.TexCoord.y - v0.TexCoord.y;

	float tmp = 0.0f;
	if ( fabsf( s1 * t2 - s2 * t1 ) <= 0.0001f )
	{
		tmp = 1.0f;
	}
	else
	{
		tmp = 1.0f / ( s1 * t2 - s2 * t1 );
	}

	XMFLOAT3 PF3, QF3;
	XMStoreFloat3( &PF3, P );
	XMStoreFloat3( &QF3, Q );

	Tangent.x = ( t2 * PF3.x - t1 * QF3.x );
	Tangent.y = ( t2 * PF3.y - t1 * QF3.y );
	Tangent.z = ( t2 * PF3.z - t1 * QF3.z );

	Tangent.x = Tangent.x * tmp;
	Tangent.y = Tangent.y * tmp;
	Tangent.z = Tangent.z * tmp;

	XMVECTOR vn = XMLoadFloat3( &normal );
	XMVECTOR vt = XMLoadFloat3( &Tangent );

	// 3. CALCULATE THE BINORMAL
	// left hand system b = t cross n (rh would be b = n cross t)
	XMVECTOR vb = XMVector3Cross( vt, vn );

	vn = XMVector3Normalize( vn );
	vt = XMVector3Normalize( vt );
	vb = XMVector3Normalize( vb );

	XMStoreFloat3( &normal, vn );
	XMStoreFloat3( &Tangent, vt );
	XMStoreFloat3( &binormal, vb );

	return;
}

// REFERENCE : "Mathematics for 3D Game Programmming and Computer Graphics" by Eric Lengyel
void Appearance::CalculateModelVectors( SimpleVertex* vertices, int vertexCount )
{
	SimpleVertex vertex1, vertex2, vertex3;
	XMFLOAT3 Tangent, binormal, normal;

	// Calculate the number of faces in the model.
	int faceCount = vertexCount / 3;

	// Initialize the index to the model data.
	int index = 0;

	// Go through all the faces and calculate the the Tangent, binormal, and normal vectors.
	for ( int i = 0; i < faceCount; i++ )
	{
		// Get the three vertices for this face from the model.
		vertex1.Pos.x = vertices[index].Pos.x;
		vertex1.Pos.y = vertices[index].Pos.y;
		vertex1.Pos.z = vertices[index].Pos.z;
		vertex1.TexCoord.x = vertices[index].TexCoord.x;
		vertex1.TexCoord.y = vertices[index].TexCoord.y;
		vertex1.Normal.x = vertices[index].Normal.x;
		vertex1.Normal.y = vertices[index].Normal.y;
		vertex1.Normal.z = vertices[index].Normal.z;
		index++;

		vertex2.Pos.x = vertices[index].Pos.x;
		vertex2.Pos.y = vertices[index].Pos.y;
		vertex2.Pos.z = vertices[index].Pos.z;
		vertex2.TexCoord.x = vertices[index].TexCoord.x;
		vertex2.TexCoord.y = vertices[index].TexCoord.y;
		vertex2.Normal.x = vertices[index].Normal.x;
		vertex2.Normal.y = vertices[index].Normal.y;
		vertex2.Normal.z = vertices[index].Normal.z;
		index++;

		vertex3.Pos.x = vertices[index].Pos.x;
		vertex3.Pos.y = vertices[index].Pos.y;
		vertex3.Pos.z = vertices[index].Pos.z;
		vertex3.TexCoord.x = vertices[index].TexCoord.x;
		vertex3.TexCoord.y = vertices[index].TexCoord.y;
		vertex3.Normal.x = vertices[index].Normal.x;
		vertex3.Normal.y = vertices[index].Normal.y;
		vertex3.Normal.z = vertices[index].Normal.z;
		index++;

		// Calculate the Tangent and binormal of that face.
		CalculateTangentBinormalLH( vertex1, vertex2, vertex3, normal, Tangent, binormal );

		// Store the normal, Tangent, and binormal for this face back in the model structure.
		vertices[index - 1].Normal.x = normal.x;
		vertices[index - 1].Normal.y = normal.y;
		vertices[index - 1].Normal.z = normal.z;
		vertices[index - 1].Tangent.x = Tangent.x;
		vertices[index - 1].Tangent.y = Tangent.y;
		vertices[index - 1].Tangent.z = Tangent.z;
		vertices[index - 1].BiTangent.x = binormal.x;
		vertices[index - 1].BiTangent.y = binormal.y;
		vertices[index - 1].BiTangent.z = binormal.z;

		vertices[index - 2].Normal.x = normal.x;
		vertices[index - 2].Normal.y = normal.y;
		vertices[index - 2].Normal.z = normal.z;
		vertices[index - 2].Tangent.x = Tangent.x;
		vertices[index - 2].Tangent.y = Tangent.y;
		vertices[index - 2].Tangent.z = Tangent.z;
		vertices[index - 2].BiTangent.x = binormal.x;
		vertices[index - 2].BiTangent.y = binormal.y;
		vertices[index - 2].BiTangent.z = binormal.z;

		vertices[index - 3].Normal.x = normal.x;
		vertices[index - 3].Normal.y = normal.y;
		vertices[index - 3].Normal.z = normal.z;
		vertices[index - 3].Tangent.x = Tangent.x;
		vertices[index - 3].Tangent.y = Tangent.y;
		vertices[index - 3].Tangent.z = Tangent.z;
		vertices[index - 3].BiTangent.x = binormal.x;
		vertices[index - 3].BiTangent.y = binormal.y;
		vertices[index - 3].BiTangent.z = binormal.z;
	}
}

void Appearance::CleanUp()
{
	if ( m_pVertexBuffer )
		m_pVertexBuffer->Release();
	m_pVertexBuffer = nullptr;

	if ( m_pIndexBuffer )
		m_pIndexBuffer->Release();
	m_pIndexBuffer = nullptr;

	if ( m_pTextureResourceView )
		m_pTextureResourceView->Release();
	m_pTextureResourceView = nullptr;

	if ( m_pSamplerLinear )
		m_pSamplerLinear->Release();
	m_pSamplerLinear = nullptr;
}