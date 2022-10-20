#pragma once
#ifndef DRAWABLEGAMEOBJECT_H
#define DRAWABLEGAMEOBJECT_H

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include "structures.h"

struct SimpleVertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 Tangent;
	XMFLOAT3 Binormal;
};

class DrawableGameObject
{
public:
	DrawableGameObject();
	~DrawableGameObject();
	void cleanup();

	HRESULT initMesh( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext );
	void update( float t, ID3D11DeviceContext* pContext );
	void draw( ID3D11DeviceContext* pContext );

	ID3D11Buffer* getVertexBuffer() { return m_pVertexBuffer; }
	ID3D11Buffer* getIndexBuffer() { return m_pIndexBuffer; }

	ID3D11ShaderResourceView** getTextureResourceView_Diff() { return &m_pTextureResourceView_Diff; }
	ID3D11ShaderResourceView** getTextureResourceView_Norm() { return &m_pTextureResourceView_Norm; }
	ID3D11ShaderResourceView** getTextureResourceView_Disp() { return &m_pTextureResourceView_Disp; }

	DirectX::XMFLOAT4X4* getTransform() { return &m_World; }
	void setPosition( DirectX::XMFLOAT3 position );

	ID3D11SamplerState** getTextureSamplerState() { return &m_pSamplerLinear; }
	ID3D11Buffer* getMaterialConstantBuffer() { return m_pMaterialConstantBuffer;}

private:
	DirectX::XMFLOAT4X4 m_World;
	DirectX::XMFLOAT3 m_position;

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ID3D11Buffer* m_pMaterialConstantBuffer;

	ID3D11ShaderResourceView* m_pTextureResourceView_Diff;
	ID3D11ShaderResourceView* m_pTextureResourceView_Norm;
	ID3D11ShaderResourceView* m_pTextureResourceView_Disp;

	ID3D11SamplerState* m_pSamplerLinear;
	MaterialPropertiesConstantBuffer m_material;
};

#endif