#pragma once
#ifndef LIGHT_H
#define LIGHT_H

class Camera;
#include "structures.h"
#include "ConstantBuffer.h"

class Light
{
public:
	bool Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	void UpdateCB( Camera& camera );
	inline ID3D11Buffer* const* GetLightCB() const noexcept { return m_cbLight.GetAddressOf(); }
private:
	ConstantBuffer<Light_CB> m_cbLight;
};

#endif