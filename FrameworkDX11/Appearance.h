#pragma once
#ifndef APPEARANCE_H
#define APPEARANCE_H

#include "ConstantBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
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

	bool InitMesh_Cube( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	bool InitMesh_Quad( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

	bool SetVertexBuffer( ID3D11Device* pDevice, std::vector<SkinnedVertex>& vertices );
	bool SetIndices( ID3D11Device* pDevice, std::vector<WORD>& indices );

	inline void SetTextureRV( ID3D11ShaderResourceView* textureRV ) noexcept { m_pTextureResourceView = textureRV; }
	inline void SetNormalRV( ID3D11ShaderResourceView* textureRV ) noexcept { m_pNormalMapResourceView = textureRV; }
	void SetTextures( ID3D11DeviceContext* pContext );

	inline ID3D11ShaderResourceView** GetTextureResourceView() noexcept { return &m_pTextureResourceView; }
	inline ID3D11ShaderResourceView** GetNormalMapResourceView() noexcept { return &m_pNormalMapResourceView; }
	inline ID3D11SamplerState** GetTextureSamplerState() noexcept { return &m_pSamplerLinear; }

	inline void SetMaterialData( MaterialPropertiesCB material ) noexcept { m_materialCB.data = material; }
	inline MaterialPropertiesCB GetMaterialData() noexcept { return m_materialCB.data; }
	inline bool HasTexture() const { return m_pTextureResourceView ? true : false; }

	void Update( ID3D11DeviceContext* pContext );
	virtual void Draw( ID3D11DeviceContext* pContext );
	virtual void Draw( ID3D11DeviceContext* pContext, int vertToDraw, int start );

protected:
	void CalculateTangentBinormalLH( SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& Tangent, XMFLOAT3& binormal );
	void CalculateTangentBinormalRH( SimpleVertex v0, SimpleVertex v1, SimpleVertex v2, XMFLOAT3& normal, XMFLOAT3& Tangent, XMFLOAT3& binormal );
	void CalculateModelVectors( SimpleVertex* vertices, int vertexCount );
	void CleanUp();

	VertexBuffer<SkinnedVertex> m_skinnedVB;
	IndexBuffer m_skinnedIB;

	VertexBuffer<SimpleVertex> m_simpleVB;
	IndexBuffer m_simpleIB;

	ID3D11SamplerState* m_pSamplerLinear;
	ConstantBuffer<MaterialPropertiesCB> m_materialCB;
	ID3D11ShaderResourceView* m_pNormalMapResourceView;
	ID3D11ShaderResourceView* m_pParallaxMapResourceView;
private:
	ID3D11ShaderResourceView* m_pTextureResourceView;
};

#endif