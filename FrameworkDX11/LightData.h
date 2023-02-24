#pragma once
#ifndef LIGHTDATA_H
#define LIGHTDATA_H

#include "Camera.h"
#include "ShadowMap.h"
#include "DrawableGameObject.h"

class LightData
{
public:
	LightData();
	LightData( std::string name, bool enabled, LightType lightType, XMFLOAT4 pos, XMFLOAT4 colour, float angle, float constantAttenuation, float linearAttenuation, float quadraticAttenuation );
	LightData( std::string name, bool enabled, LightType lightType, XMFLOAT4 pos, XMFLOAT4 colour, float angle, float constantAttenuation, float linearAttenuation, float quadraticAttenuation, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	LightData( Light lightData );
	~LightData();

	void Update( float dt, ID3D11DeviceContext* pContext );
	void Draw( ID3D11DeviceContext* pContext );

	Light GetLightData();
	void SetLightData( Light lightData );
	DrawableGameObject* GetLightObject();

	std::string GetName();
	void SetName( std::string name );

	void CreateView();
	void CreateProjectionMatrix( float screenDepth, float screenNear );

	void SetColour( XMFLOAT4 colour );
	void SetPos( XMFLOAT4 pos );
	void SetDirection( XMFLOAT4 dir );
	void SetEnabled( bool enabled );
	void SetAttenuation( float constantAttenuation = 0.0f, float linearAttenuation = 0.0f, float quadraticAttenuation = 0.0f );
	void SetAngle( float angle );

	inline ShadowMap* GetShadow() const noexcept { return m_pShadow; }
	void CreateShadowMap( ID3D11DeviceContext* pContext, std::vector<DrawableGameObject*> objects, ID3D11Buffer** buffer );

private:
	void CleanUp();

	XMFLOAT3 m_fAt;
	XMFLOAT4X4 m_mView;
	XMFLOAT4X4 m_mProjection;
	std::string m_sName;

	Light m_lightData;
	Camera* m_pCamLight;
	ShadowMap* m_pShadow;
	DrawableGameObject* m_pLightObject;
};

#endif