#pragma once
#ifndef APPEARANCE_H
#define APPEARANCE_H

#include "DDSTextureLoader.h"
#include "Structures.h"
#include "DataStucts.h"
#include <vector>

#define NUM_VERTICES 36

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 Tangent;
	XMFLOAT3 BiTangent;
};

class Appearance
{
public:
	Appearance();
	~Appearance();

	HRESULT	InitMesh_Cube( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	HRESULT	InitMesh_Quad( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

	template <typename VertexType>
	HRESULT SetVertexBuffer( ID3D11Device* pDevice, const VertexType* vertices, UINT count );
	HRESULT SetVertexBuffer( ID3D11Device* pDevice, std::vector<SkinedVertex> vertices, UINT count );
	HRESULT SetIndices( ID3D11Device* pDevice, const USHORT* indices, UINT count );

	inline void SetTextureRV( ID3D11ShaderResourceView* textureRV ) noexcept { m_pTextureResourceView = textureRV; }
	inline void SetNormalRV( ID3D11ShaderResourceView* textureRV ) noexcept { m_pNormalMapResourceView = textureRV; }
	void SetTextures( ID3D11DeviceContext* pContext );

	inline ID3D11Buffer* GetVertexBuffer() const noexcept { return m_pVertexBuffer; }
	inline ID3D11Buffer* GetIndexBuffer() const noexcept { return m_pIndexBuffer; }

	inline ID3D11ShaderResourceView** GetTextureResourceView() noexcept { return &m_pTextureResourceView; }
	inline ID3D11ShaderResourceView** GetNormalMapResourceView() noexcept { return &m_pNormalMapResourceView; }
	inline ID3D11Buffer* GetMaterialConstantBuffer() const noexcept { return m_pMaterialConstantBuffer; }
	inline ID3D11SamplerState** GetTextureSamplerState() noexcept { return &m_pSamplerLinear; }

	inline MaterialPropertiesConstantBuffer getMaterialPropertiesConstantBuffer() const noexcept { return m_material; }
	inline void SetMaterial( MaterialPropertiesConstantBuffer material ) noexcept { m_material = material; }
	inline bool HasTexture() const { return m_pTextureResourceView ? true : false; }

	void Update( ID3D11DeviceContext* pContext );
	virtual void Draw( ID3D11DeviceContext* pContext );
	virtual void Draw( ID3D11DeviceContext* pContext, int vertToDraw, int start );

protected:
	void CalculateTangentBinormalLH( SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& Tangent, XMFLOAT3& binormal );
	void CalculateTangentBinormalRH( SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& Tangent, XMFLOAT3& binormal );
	void CalculateModelVectors( SimpleVertex* vertices, int vertexCount );
	void CleanUp();

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	ID3D11SamplerState* m_pSamplerLinear;
	MaterialPropertiesConstantBuffer m_material;
	ID3D11Buffer* m_pMaterialConstantBuffer = nullptr;
	int m_iNumberOfVert = 0;

	ID3D11ShaderResourceView* m_pNormalMapResourceView;
	ID3D11ShaderResourceView* m_pParallaxMapResourceView;
	UINT m_uVertexStride = 0;
private:
	ID3D11ShaderResourceView* m_pTextureResourceView;
};

template<typename VertexType>
inline HRESULT Appearance::SetVertexBuffer( ID3D11Device* pd3dDevice, const VertexType* vertices, UINT count )
{
	m_uVertexStride = sizeof( VertexType );

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof( VertexType ) * count;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;

	HRESULT hr = pd3dDevice->CreateBuffer( &vbd, &vinitData, &m_pVertexBuffer );
	return hr;
}

#endif