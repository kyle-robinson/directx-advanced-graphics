#pragma once
#ifndef LIGHTCONTROL_H
#define LIGHTCONTROL_H

class Camera;
#include "Light.h"

class LightControl
{
public:
	LightControl( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	~LightControl();

	std::vector<Light*>GetLightList();
	Light* GetLight( std::string lightName );
	Light* GetLight( int lightNum );

	void AddLight( std::string name, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer, Camera& camera );
	void AddLight( std::string name, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer, Camera& camera,
		BOOL enabled, LightType type, XMFLOAT4 pos, XMFLOAT4 col, FLOAT angle, FLOAT constAtten, FLOAT linAtten, FLOAT quadAtten, FLOAT intensity );

	void SetupLightsForRender( XMFLOAT4 eyePosition );
	void Draw( const XMMATRIX& view, const XMMATRIX& projection );
	void Update( Camera& camera );
	void SpawnControlWindows();

	inline void SetGlobalAmbient( const XMFLOAT4& globalAmbient ) noexcept { m_fGlobalAmbient = globalAmbient; }
	inline const XMFLOAT4& GetGlobalAmbient() const noexcept { return m_fGlobalAmbient; }

	inline const ConstantBuffer<Light_CB>& GetCB() const noexcept { return m_cbLights; }
	inline ID3D11Buffer* GetCB_Ptr() const noexcept { return m_cbLights.Get(); }
	inline ID3D11Buffer* const* GetCB_DPtr() const noexcept { return m_cbLights.GetAddressOf(); }
private:
	std::vector<Light*> m_pLights;
	ConstantBuffer<Light_CB> m_cbLights;
	XMFLOAT4 m_fGlobalAmbient = { 0.2f, 0.2f, 0.8f, 1.0f };
};

#endif