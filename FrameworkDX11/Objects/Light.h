#pragma once
#ifndef LIGHT_H
#define LIGHT_H

class Camera;
#include "structures.h"
#include "ConstantBuffer.h"

#if defined ( _x64 )
#include "RenderableGameObject.h"
#endif

class Light
{
public:
	bool Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cb_vs_vertexshader );
	void Draw( const XMMATRIX& view, const XMMATRIX& projection );
	
	void UpdateCB( Camera& camera );
	void SpawnControlWindow();

	inline const ConstantBuffer<Light_CB>& GetCB() const noexcept { return m_cbLight; }
	inline ID3D11Buffer* GetCB_Ptr() const noexcept { return m_cbLight.Get(); }
	inline ID3D11Buffer* const* GetCB_DPtr() const noexcept { return m_cbLight.GetAddressOf(); }
private:
	XMFLOAT4 m_fPosition = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 m_fColor = XMFLOAT4( DirectX::Colors::White );

	FLOAT m_fSpotAngle = 45.0f;
	FLOAT m_fConstantAttenuation = 1.0f;
	FLOAT m_fLinearAttenuation = 1.0f;
	FLOAT m_fQuadraticAttenuation = 1.0f;
	FLOAT m_fIntensity = 4.0f;

#if defined ( _x64 )
	RenderableGameObject m_objLight;
#endif

	bool m_bAttachedToCamera = true;
	ConstantBuffer<Light_CB> m_cbLight;
};

#endif