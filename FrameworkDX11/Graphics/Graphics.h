#pragma once
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "Quad.h"
#include "Shaders.h"
#include "Sampler.h"
#include "Viewport.h"
#include "SwapChain.h"
#include "Rasterizer.h"
#include "BackBuffer.h"
#include "DepthStencil.h"
#include "RenderTarget.h"

static UINT BUFFER_COUNT = 3u;

class Graphics
{
public:
	bool Initialize( HWND hWnd, UINT width, UINT height );
	void ResizeWindow( HWND hWnd, XMFLOAT2 windowSize );
	void BeginFrame();
	void BeginFrameNormal();
	void BeginFrameDeferred();
	
	void UpdateRenderStateSkysphere();
	void UpdateRenderStateCube(
		bool useDeferred = false,
		bool useGBuffer = false
	);
	void UpdateRenderStateObject();
	void UpdateRenderStateTexture();
	
	void BeginRenderSceneToTexture();
	void RenderSceneToTexture(
		ID3D11Buffer* const* cbMotionBlur,
		ID3D11Buffer* const* cbFXAA,
		ID3D11Buffer* const* cbSSAO,
		ID3D11ShaderResourceView* const* pNoiseTexture );
	void RenderSceneToTextureNormal( ID3D11Buffer* const* cbMatrices );
	void EndFrame();

	inline UINT GetWidth() const noexcept { return m_viewWidth; }
	inline UINT GetHeight() const noexcept { return m_viewHeight; }
	inline ID3D11Device* GetDevice() const noexcept { return m_pDevice.Get(); }
	inline ID3D11DeviceContext* GetContext() const noexcept { return m_pContext.Get(); }
	inline Bind::RenderTarget* GetRenderTarget() const noexcept { return &*m_pRenderTarget; }
	inline Bind::RenderTarget* GetDeferredRenderTarget( Bind::RenderTarget::Type type ) const noexcept { return &*m_pRenderTargetsDeferred.at( type ); }

private:
	void InitializeDirectX( HWND hWnd, bool resizingWindow );
	bool InitializeShaders();
	bool InitializeRTT();

	// Window data
	Quad m_quad;
	UINT m_viewWidth;
	UINT m_viewHeight;
	float m_clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };

	// Shaders
	VertexShader m_vertexShader;
	PixelShader m_pixelShader;

	VertexShader m_vertexShaderPP;
	PixelShader m_pixelShaderPP;

	VertexShader m_vertexShaderDR;
	PixelShader m_pixelShaderDR;

	VertexShader m_vertexShaderGB;
	PixelShader m_pixelShaderGB;

	VertexShader m_vertexShaderNRM;
	PixelShader m_pixelShaderNRM;

	VertexShader m_vertexShaderSDW;
	PixelShader m_pixelShaderSDW;

	VertexShader m_vertexShaderTEX;
	PixelShader m_pixelShaderTEX;

	VertexShader m_vertexShaderOBJ;
	PixelShader m_pixelShaderOBJ;

	// Pipeline components
	std::shared_ptr<Bind::SwapChain> m_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_pContext;

	std::shared_ptr<Bind::RenderTarget> m_pRenderTarget;
	std::shared_ptr<Bind::DepthStencil> m_pDepthStencil;
	std::shared_ptr<Bind::RenderTarget> m_pRenderTargetNormal;
	std::unordered_map<Bind::RenderTarget::Type, std::shared_ptr<Bind::RenderTarget>> m_pRenderTargetsDeferred;

	std::shared_ptr<Bind::Viewport> m_pViewport;
	std::shared_ptr<Bind::BackBuffer> m_pBackBuffer;
	std::unordered_map<Bind::Sampler::Type, std::shared_ptr<Bind::Sampler>> m_pSamplerStates;
	std::unordered_map<Bind::Rasterizer::Type, std::shared_ptr<Bind::Rasterizer>> m_pRasterizerStates;
};

#endif