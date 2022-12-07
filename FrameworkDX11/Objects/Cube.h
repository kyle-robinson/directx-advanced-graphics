#pragma once
#ifndef CUBE_H
#define CUBE_H

class Camera;
#include "Resource.h"
#include "DDSTextureLoader.h"

#include "structures.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"

struct Vertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	XMFLOAT3 Tangent;
	XMFLOAT3 Binormal;
};

class Cube
{
public:
	bool InitializeMesh( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	void Update( float dt );
	void UpdateCB();
	void UpdateBuffers( ConstantBuffer<Matrices>& cb_vs_matrices, const Camera& pCamera );
	void Draw( ID3D11DeviceContext* pContext );
	void DrawDeferred(
		ID3D11DeviceContext* pContext,
		ID3D11ShaderResourceView** position,
		ID3D11ShaderResourceView** albedo,
		ID3D11ShaderResourceView** normal
	);
	void SpawnControlWindows();

	inline ID3D11Buffer* const* GetCB() const noexcept { return m_cbMaterial.GetAddressOf(); }
	inline XMFLOAT4X4* GetTransform() noexcept { return &m_World; }
	
	inline void SetPosition( XMFLOAT3 position ) { m_position = position; UpdateMatrix(); }
	inline XMFLOAT3 GetPosition() const noexcept { return m_position; }

	inline void SetRotation( XMFLOAT3 rotation ) { m_rotation = rotation; UpdateMatrix(); }
	inline XMFLOAT3 GetRotation() const noexcept { return m_rotation; }
	
	inline void SetScale( XMFLOAT3 scale ) { m_scale = scale; UpdateMatrix(); };
	inline XMFLOAT3 GetScale() const noexcept { return m_scale; }

private:
	void UpdateMatrix();

	XMFLOAT4X4 m_World;
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMFLOAT3 m_scale;

	XMFLOAT4 m_fEmissive = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 m_fAmbient = { 0.1f, 0.1f, 0.1f, 1.0f };
	XMFLOAT4 m_fDiffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT4 m_fSpecular = { 1.0f, 1.0f, 1.0f, 1.0f };
	FLOAT m_fSpecularPower = 128.0f;
	BOOL m_bUseTexture = TRUE;
	
	bool m_bResetSpin = false;
	bool m_bEnableSpin = false;
	bool m_bReverseSpin = false;

	IndexBuffer m_indexBuffer;
	VertexBuffer<Vertex> m_vertexBuffer;
	ConstantBuffer<Material_CB> m_cbMaterial;

	int m_textureIndex = 0;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pTexturesDiffuse;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pTexturesNormal;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_pTexturesDisplacement;
};

#endif