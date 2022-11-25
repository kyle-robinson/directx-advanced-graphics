#pragma once
#ifndef DEFERRED_H
#define DEFERRED_H

#include "structures.h"
#include "ConstantBuffer.h"

class Deferred
{
public:
	bool Initialize( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	void UpdateCB();
	void SpawnControlWindow();
	inline bool IsActive() const noexcept { return m_bUseDeferredShading; }
	inline ID3D11Buffer* const* GetCB() const noexcept { return m_cbDeferred.GetAddressOf(); }
private:
	BOOL m_bUseDeferredShading = TRUE;
	BOOL m_bOnlyPositions = FALSE;
	BOOL m_bOnlyAlbedo = FALSE;
	BOOL m_bOnlyNormals = FALSE;
	ConstantBuffer<Deferred_CB> m_cbDeferred;
};

#endif