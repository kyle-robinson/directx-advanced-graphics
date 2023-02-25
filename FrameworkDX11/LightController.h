#pragma once
#ifndef LIGHTCONTROLLER_H
#define LIGHTCONTROLLER_H

#include "LightData.h"
#include <vector>

class LightController
{
public:
	LightController();
	~LightController();

	inline std::vector<LightData*> GetLightList() const noexcept { return m_vLightData; }
	LightData* GetLight( std::string name );
	LightData* GetLight( int num );

	void AddLight( std::string name, bool enabled, LightType lightType, XMFLOAT4 pos, XMFLOAT4 colour, float angle, float constantAttenuation, float linearAttenuation, float quadraticAttenuation );
	void AddLight( std::string name, bool enabled, LightType lightType, XMFLOAT4 pos, XMFLOAT4 colour, float angle, float constantAttenuation, float linearAttenuation, float quadraticAttenuation, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	void AddLight( Light light );

	void Update( float dt, ID3D11DeviceContext* pContext );
	void Draw( ID3D11DeviceContext* pContext, ConstantBuffer<MatrixBuffer>& buffer );
	void RemoveAllLights();

private:
	void CleanUp();
	std::vector<LightData*> m_vLightData;
};

#endif