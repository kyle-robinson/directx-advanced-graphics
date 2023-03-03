#pragma once
#ifndef BILLBOARDOBJECT_H
#define BILLBOARDOBJECT_H

#include <vector>
#include "Structures.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "ShaderController.h"

struct SimpleVertexBillboard
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 Tangent;
	XMFLOAT3 BiTangent;
};

class BillboardObject
{
public:
	BillboardObject( std::string texName, int numToCreate, ID3D11Device* pDevice );
	~BillboardObject();

	void CreateBillboard( int num, ID3D11Device* pDevice );
	void Draw( ID3D11DeviceContext* pContext, ShaderController::ShaderData shaderData, ConstantBuffer<MatrixBuffer>& buffer );
	void UpdatePositions( ID3D11DeviceContext* pContext );

	void SetTexture( std::string texName, ID3D11Device* pDevice );
	inline void SetPositions( std::vector<SimpleVertexBillboard> pos ) noexcept { m_vPositions = pos; }
	inline std::vector<SimpleVertexBillboard> GetPositions() const noexcept { return m_vPositions; }

private:
	void CleanUp();

	int m_iNumberOfBillBoards = 0;
	std::vector<SimpleVertexBillboard> m_vPositions;
	ID3D11ShaderResourceView* m_pDiffuseResourceView;
	VertexBuffer<SimpleVertexBillboard> m_billboardVB;
	VertexBuffer<SimpleVertexBillboard> m_billboardInstanceVB;
};

#endif