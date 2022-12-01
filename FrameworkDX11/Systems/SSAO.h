#pragma once
#ifndef SSAO_H
#define SSAO_H

class Camera;
#include "structures.h"
#include "ConstantBuffer.h"

class SSAO
{
public:
	bool Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	void UpdateCB( int width, int height, const Camera& pCamera );
	void SpawnControlWindow( bool usingMotionBlur, bool usingFXAA );

	inline bool IsActive() const noexcept { return m_bUseSSAO; }
	inline ID3D11Buffer* const* GetCB() const noexcept { return m_cbSSAO.GetAddressOf(); }
	inline ID3D11ShaderResourceView* const* GetNoiseTexture() const noexcept { return m_pTextureNoise.GetAddressOf(); }
private:
	XMFLOAT4X4 m_ProjectionMatrix;
	XMFLOAT2 m_fScreenSize;
	XMFLOAT2 m_fNoiseScale;
	FLOAT m_fRadius = 0.5f;
	FLOAT m_fPower = 2.0f;
	int m_kernelSize = 16;
	BOOL m_bUseSSAO = FALSE;
	FLOAT m_fNoiseSize = 2.0f;
	XMFLOAT3 m_fKernelOffsets[64];
	ConstantBuffer<SSAO_CB> m_cbSSAO;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pTextureNoise;
};

#endif