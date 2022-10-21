#pragma once
#ifndef DRAWABLEGAMEOBJECT_H
#define DRAWABLEGAMEOBJECT_H

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "Resource.h"
#include "structures.h"
#include "ConstantBuffer.h"

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
	void Cleanup();

	HRESULT InitializeMesh( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	void Update( float dt, ID3D11DeviceContext* pContext );
	void Draw( ID3D11DeviceContext* pContext );

	inline ID3D11Buffer* GetVertexBuffer() const noexcept { return m_pVertexBuffer; }
	inline ID3D11Buffer* GetIndexBuffer() const noexcept { return m_pIndexBuffer; }
	inline ID3D11Buffer* const* GetMaterialCB() const noexcept { return m_cbMaterial.GetAddressOf(); }

	//inline ID3D11ShaderResourceView** getTextureResourceView_Diff() noexcept { return m_pTextureDiffuse.GetAddressOf(); }
	//inline ID3D11ShaderResourceView** getTextureResourceView_Norm() noexcept { return m_pTextureNormal.GetAddressOf(); }
	//inline ID3D11ShaderResourceView** getTextureResourceView_Disp() noexcept { return m_pTextureDisplacement.GetAddressOf(); }

	inline XMFLOAT4X4* GetTransform() noexcept { return &m_World; }
	inline void SetPosition( DirectX::XMFLOAT3 position ) noexcept { m_position = position; }

private:
	XMFLOAT4X4 m_World;
	XMFLOAT3 m_position;

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	ConstantBuffer<Material> m_cbMaterial;

	ID3D11ShaderResourceView* m_pTextureDiffuse;
	ID3D11ShaderResourceView* m_pTextureNormal;
	ID3D11ShaderResourceView* m_pTextureDisplacement;
};

#endif