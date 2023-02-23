#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <map>
#include <memory>

#include "Sampler.h"
#include "Viewport.h"
#include "SwapChain.h"
#include "BackBuffer.h"
#include "ShaderController.h"
#include "RenderTargetController.h"

class Graphics
{
public:
	Graphics();
	~Graphics();
	void Initialize( HWND hWnd, UINT width, UINT height );

	inline UINT GetWidth() const noexcept { return m_viewWidth; }
	inline UINT GetHeight() const noexcept { return m_viewHeight; }

	inline std::shared_ptr<Bind::Viewport> GetViewport() const noexcept { return m_pViewport; }
	inline std::shared_ptr<Bind::BackBuffer> GetBackBuffer() const noexcept { return m_pBackBuffer; }
	inline std::shared_ptr<Bind::DepthStencil> GetDepthStencil() const noexcept { return m_pDepthStencil; }

	inline ID3D11Device* GetDevice() const noexcept { return m_pDevice.Get(); }
	inline ID3D11DeviceContext* GetContext() const noexcept { return m_pContext.Get(); }
	inline IDXGISwapChain* GetSwapChain() const noexcept { return m_pSwapChain->GetSwapChain(); }

	inline ShaderController* GetShaderController() const noexcept { return m_pShaderController; }
	inline RenderTargetController* GetRenderTargetController() const noexcept { return m_pRenderTargetController; }
	inline std::shared_ptr<Bind::Sampler> GetSampler( std::string type ) const noexcept { return m_pSamplerStates.at( type ); }

private:
	void InitializeDirectX( HWND hWnd );
	void InitializeShaders();
	void InitializeRenderTargets();

	// Window data
	UINT m_viewWidth;
	UINT m_viewHeight;

	// Pipeline components
	ShaderController* m_pShaderController;
	RenderTargetController* m_pRenderTargetController;
	std::map<std::string, std::shared_ptr<Bind::Sampler>> m_pSamplerStates;

	std::shared_ptr<Bind::Viewport> m_pViewport;
	std::shared_ptr<Bind::BackBuffer> m_pBackBuffer;
	std::shared_ptr<Bind::DepthStencil> m_pDepthStencil;

	std::shared_ptr<Bind::SwapChain> m_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;
};

#endif