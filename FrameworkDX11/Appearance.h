#pragma once
#include <directxmath.h>
#include <d3d11_1.h>
#include <string>
#include "DDSTextureLoader.h"
#include"Structures.h"
#include"DataStucts.h"
#include<vector>

using namespace DirectX;
using namespace std;

#define NUM_VERTICES 36

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
};

/// <summary>
/// class to hold apperance data of an object
/// </summary>
class Appearance
{
public:

	Appearance();
	~Appearance();
	HRESULT	initMesh(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
	HRESULT	initMeshFloor(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, UINT Width, UINT Hight);

	template <typename VertexType>
	HRESULT SetVertexBuffer(ID3D11Device* pd3dDevice, const VertexType* vertices, UINT count);
	HRESULT SetVertexBuffer(ID3D11Device* pd3dDevice, vector<SkinedVertex> Verts, UINT count);
	HRESULT SetIndices(ID3D11Device* device, const USHORT* indices, UINT count);
	//sets
	void SetTextureRV(ID3D11ShaderResourceView* textureRV) { m_pTextureResourceView = textureRV; }
	void SetNormalRV(ID3D11ShaderResourceView* textureRV) { m_pNormalMapResourceView = textureRV; }
	void SetTextures(ID3D11DeviceContext* pImmediateContext);


	//gets
	ID3D11Buffer* getVertexBuffer() { return m_pVertexBuffer; }
	ID3D11Buffer* getIndexBuffer() { return m_pIndexBuffer; }
	ID3D11ShaderResourceView** getTextureResourceView() { return &m_pTextureResourceView; }
	ID3D11ShaderResourceView** getNormalMapResourceView() { return &m_pNormalMapResourceView; }
	ID3D11SamplerState** getTextureSamplerState() { return &m_pSamplerLinear; }
	ID3D11Buffer* getMaterialConstantBuffer() { return m_pMaterialConstantBuffer; }
	
	MaterialPropertiesConstantBuffer getMaterialPropertiesConstantBuffer() { return m_material; }
	void SetMaterial(MaterialPropertiesConstantBuffer material) { m_material = material; }

	

	bool HasTexture() const { return m_pTextureResourceView ? true : false; }


	 void Update(ID3D11DeviceContext* pContext);
	virtual void Draw(ID3D11DeviceContext* pImmediateContext);
	virtual void Draw(ID3D11DeviceContext* pImmediateContext,int vertToDraw,int Start);

	

protected:
	//data
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	
	ID3D11SamplerState* m_pSamplerLinear;
	MaterialPropertiesConstantBuffer	m_material;
	ID3D11Buffer* m_pMaterialConstantBuffer = nullptr;
	int NumberOfVert =0;
	
	ID3D11ShaderResourceView* m_pNormalMapResourceView;
	ID3D11ShaderResourceView* m_pParralaxMapResourceView;
	void CleanUp();
	UINT mVertexStride=0;
private:
	ID3D11ShaderResourceView* m_pTextureResourceView;
};

template<typename VertexType>
inline HRESULT Appearance::SetVertexBuffer(ID3D11Device* pd3dDevice, const VertexType* vertices, UINT count)
{

	mVertexStride = sizeof(VertexType);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VertexType) * count;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;

	HRESULT hr = pd3dDevice->CreateBuffer(&vbd, &vinitData, &m_pVertexBuffer);
	return hr;
}
