#pragma once
#ifndef SHADOWMAP_H
#define SHADOWMAP_H

#include "structures.h"
#include "ConstantBuffer.h"

class ShadowMap
{
public:
	bool Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, UINT width, UINT height );
	void BeginFrame( ID3D11DeviceContext* pDevice );
	void UpdateCB();
	void SpawnControlWindow();

	inline bool IsActive() const noexcept { return m_bUseShadows; }
	inline ID3D11DepthStencilView* GetDSV() const noexcept { return m_depthDSV.Get(); }
	inline ID3D11ShaderResourceView* GetSRV() const noexcept { return m_depthSRV.Get(); }
	inline ID3D11Buffer* const* GetCB() const noexcept { return m_cbShadows.GetAddressOf(); }
private:
	BOOL m_bUseShadows = FALSE;
	ConstantBuffer<Shadow_CB> m_cbShadows;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_depthSRV;
};

#endif