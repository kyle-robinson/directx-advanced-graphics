#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "SwapChain.h"
#include "ShaderController.h"

class Graphics
{
public:
	Graphics();
	~Graphics();
	void Initialize( HWND hWnd, UINT width, UINT height );

	inline UINT GetWidth() const noexcept { return m_viewWidth; }
	inline UINT GetHeight() const noexcept { return m_viewHeight; }

	inline ID3D11Device* GetDevice() const noexcept { return m_pDevice.Get(); }
	inline ID3D11DeviceContext* GetContext() const noexcept { return m_pContext.Get(); }
	inline IDXGISwapChain* GetSwapChain() const noexcept { return m_pSwapChain->GetSwapChain(); }
	inline ShaderController* GetShaderController() const noexcept { return m_pShaderController; }

private:
	void InitializeDirectX( HWND hWnd );
	void InitializeShaders();

	// Window data
	UINT m_viewWidth;
	UINT m_viewHeight;

	// Pipeline components
	ShaderController* m_pShaderController;
	std::shared_ptr<Bind::SwapChain> m_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
};

#endif