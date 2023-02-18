#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include "Camera.h"
#include "structures.h"
#include "ConstantBuffer.h"

#if defined ( _x64 )
#include "RenderableGameObject.h"
#endif

class Light
{
public:
	Light();
	Light( std::string name, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer, Camera& camera );
	Light( std::string name, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer, Camera& camera,
		BOOL enabled, LightType type, XMFLOAT4 pos, XMFLOAT4 col, FLOAT angle, FLOAT constAtten, FLOAT linAtten, FLOAT quadAtten, FLOAT intensity );
	~Light();

	void Draw( const XMMATRIX& view, const XMMATRIX& projection );
	void Update( Camera& camera );
	void SpawnControlWindow();

	inline void SetPosition( const XMFLOAT4& pos ) noexcept { m_fPosition = pos;
		m_lightCamera.SetPosition( XMFLOAT3( pos.x, pos.y, pos.z ) ); }
	inline void SetColor( const XMFLOAT4& color ) noexcept { m_fColor = color; }
	inline void SetSpotAngle( FLOAT angle ) noexcept { m_fSpotAngle = angle; }
	inline void SetConstantAttenuation( FLOAT constant ) noexcept { m_fConstantAttenuation = constant; }
	inline void SetLinearAttenuation( FLOAT linear ) noexcept { m_fLinearAttenuation = linear; }
	inline void SetQuadraticAttenuation( FLOAT quadratic ) noexcept { m_fQuadraticAttenuation = quadratic; }
	inline void SetIntensity( FLOAT intensity ) noexcept { m_fIntensity = intensity; }
	inline void SetAttachedToCamera( bool attached ) noexcept { m_bAttachedToCamera = attached; }
	inline void SetName( std::string name ) noexcept { m_sName = name; }
	inline void SetType( LightType type ) noexcept { m_eType = type; }
	inline void SetEnabled( BOOL enabled ) noexcept { m_bEnabled = enabled; }

	inline const XMFLOAT4& GetPosition() const noexcept { return m_fPosition; }
	inline const XMFLOAT4& GetColor() const noexcept { return m_fColor; }
	inline const FLOAT GetSpotAngle() const noexcept { return m_fSpotAngle; }
	inline const FLOAT GetConstantAttenuation() const noexcept { return m_fConstantAttenuation; }
	inline const FLOAT GetLinearAttenuation() const noexcept { return m_fLinearAttenuation; }
	inline const FLOAT GetQuadraticAttenuation() const noexcept { return m_fQuadraticAttenuation; }
	inline const FLOAT GetIntensity() const noexcept { return m_fIntensity; }
	inline const bool GetAttachedToCamera() const noexcept { return m_bAttachedToCamera; }
	inline const std::string GetName() const noexcept { return m_sName; }
	inline const LightType GetType() const noexcept { return m_eType; }
	inline const BOOL GetEnabled() const noexcept { return m_bEnabled; }

	inline const ConstantBuffer<LightData>& GetCB() const noexcept { return m_cbLight; }
	inline ID3D11Buffer* GetCB_Ptr() const noexcept { return m_cbLight.Get(); }
	inline ID3D11Buffer* const* GetCB_DPtr() const noexcept { return m_cbLight.GetAddressOf(); }
private:
	bool SetModel( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ConstantBuffer<Matrices>& cbuffer );
	bool SetCBData( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, Camera& camera );

	XMFLOAT4 m_fPosition = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMFLOAT4 m_fColor = XMFLOAT4( Colors::White );

	FLOAT m_fSpotAngle = 0.0f;
	FLOAT m_fConstantAttenuation = 1.0f;
	FLOAT m_fLinearAttenuation = 1.0f;
	FLOAT m_fQuadraticAttenuation = 1.0f;
	FLOAT m_fIntensity = 4.0f;

	bool m_bAttachedToCamera = false;
	LightType m_eType = PointLight;
	BOOL m_bEnabled = TRUE;
	std::string m_sName;

#if defined ( _x64 )
	RenderableGameObject m_objLight;
#endif

	Camera m_lightCamera;
	ConstantBuffer<LightData> m_cbLight;
};

#endif