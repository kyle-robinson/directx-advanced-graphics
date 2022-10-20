#include "DrawableGameObject.h"
#define NUM_VERTICES 36

DrawableGameObject::DrawableGameObject()
{
	m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;
	m_pTextureResourceView_Diff = nullptr;
	m_pTextureResourceView_Norm = nullptr;
	m_pTextureResourceView_Disp = nullptr;
	m_pSamplerLinear = nullptr;
	m_pMaterialConstantBuffer = nullptr;
	DirectX::XMStoreFloat4x4( &m_World, DirectX::XMMatrixIdentity() );
}


DrawableGameObject::~DrawableGameObject()
{
	cleanup();
}

void DrawableGameObject::cleanup()
{
	if (m_pVertexBuffer)
		m_pVertexBuffer->Release();
	m_pVertexBuffer = nullptr;

	if (m_pIndexBuffer)
		m_pIndexBuffer->Release();
	m_pIndexBuffer = nullptr;

	if (m_pTextureResourceView_Diff)
		m_pTextureResourceView_Diff->Release();
	m_pTextureResourceView_Diff = nullptr;

	if (m_pTextureResourceView_Norm)
		m_pTextureResourceView_Norm->Release();
	m_pTextureResourceView_Norm = nullptr;

	if (m_pTextureResourceView_Disp)
		m_pTextureResourceView_Disp->Release();
	m_pTextureResourceView_Disp = nullptr;

	if (m_pSamplerLinear)
		m_pSamplerLinear->Release();
	m_pSamplerLinear = nullptr;

	if (m_pMaterialConstantBuffer)
		m_pMaterialConstantBuffer->Release();
	m_pMaterialConstantBuffer = nullptr;
}

HRESULT DrawableGameObject::initMesh( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext )
{
	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ {  1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ {  1.0f, 1.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ { -1.0f, 1.0f,  1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ {  1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ {  1.0f, -1.0f,  1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
		{ { -1.0f, -1.0f,  1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },

		{ { -1.0f, -1.0f,  1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -1.0f, -1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -1.0f,  1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -1.0f,  1.0f,  1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f } },

		{ { 1.0f, -1.0f,  1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { 1.0f,  1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { 1.0f,  1.0f,  1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },

		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ {  1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },

		{ { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ {  1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ {  1.0f,  1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
		{ { -1.0f,  1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
	};

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 24;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	HRESULT hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Create index buffer
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * NUM_VERTICES;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// load and setup textures
	hr = DirectX::CreateDDSTextureFromFile(pd3dDevice, L"res\\textures\\bricks_TEX.dds", nullptr, &m_pTextureResourceView_Diff);
	if (FAILED(hr))
		return hr;

	hr = DirectX::CreateDDSTextureFromFile(pd3dDevice, L"res\\textures\\bricks_NORM.dds", nullptr, &m_pTextureResourceView_Norm);
	if (FAILED(hr))
		return hr;

	hr = DirectX::CreateDDSTextureFromFile(pd3dDevice, L"res\\textures\\bricks_DISP.dds", nullptr, &m_pTextureResourceView_Disp);
	if (FAILED(hr))
		return hr;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pd3dDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear);

	m_material.Material.Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Material.Specular = DirectX::XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	m_material.Material.SpecularPower = 32.0f;
	m_material.Material.UseTexture = true;

	// Create the material constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MaterialPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = pd3dDevice->CreateBuffer(&bd, nullptr, &m_pMaterialConstantBuffer);
	if (FAILED(hr))
		return hr;

	return hr;
}

void DrawableGameObject::setPosition( DirectX::XMFLOAT3 position )
{
	m_position = position;
}

void DrawableGameObject::update( float t, ID3D11DeviceContext* pContext )
{
	static float cummulativeTime = 0;
	cummulativeTime += t;

	DirectX::XMMATRIX mSpin = DirectX::XMMatrixRotationY( cummulativeTime );
	DirectX::XMMATRIX mTranslate = DirectX::XMMatrixTranslation( 0.0f, 0.0f, 0.0f );
	DirectX::XMMATRIX world = mTranslate * mSpin;
	XMStoreFloat4x4( &m_World, world );

	pContext->UpdateSubresource( m_pMaterialConstantBuffer, 0u, nullptr, &m_material, 0u, 0u );
}

void DrawableGameObject::draw( ID3D11DeviceContext* pContext )
{
	pContext->PSSetShaderResources( 0u, 1u, &m_pTextureResourceView_Diff );
	pContext->PSSetShaderResources( 1u, 1u, &m_pTextureResourceView_Norm );
	pContext->PSSetShaderResources( 2u, 1u, &m_pTextureResourceView_Disp );
	pContext->PSSetSamplers( 0u, 1u, &m_pSamplerLinear );
	pContext->DrawIndexed( NUM_VERTICES, 0, 0 );
}