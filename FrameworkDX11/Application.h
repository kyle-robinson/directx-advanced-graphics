#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "Resource.h"

#include "Shaders.h"
#include "structures.h"
#include "ImGuiManager.h"
#include "DrawableGameObject.h"

#include "Sampler.h"
#include "Viewport.h"
#include "SwapChain.h"
#include "Rasterizer.h"
#include "RenderTarget.h"
#include "DepthStencil.h"

#include "Camera.h"
#include "WindowContainer.h"

typedef std::vector<DrawableGameObject*> vecDrawables;

class Application : public WindowContainer
{
public:
	bool InitWindow( HINSTANCE hInstance, int nCmdShow );
	HRESULT InitDevice();
	HRESULT InitMesh();
	HRESULT InitWorld(int width, int height);
	void CleanupDevice();
	LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
	void setupLightForRender();
	float calculateDeltaTime();

	bool ProcessMessages() noexcept;
	void UpdateInput( float dt );
	void Update();
	void Render();
private:
	// Window data
	int g_viewWidth;
	int g_viewHeight;
	HWND g_hWnd = nullptr;
	HINSTANCE g_hInst = nullptr;

	// Pipeline components
	std::shared_ptr<Bind::SwapChain> g_pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> g_pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> g_pContext;

	std::shared_ptr<Bind::Viewport> g_pViewport;
	std::shared_ptr<Bind::DepthStencil> g_pDepthStencil;
	std::shared_ptr<Bind::RenderTarget> g_pRenderTarget;
	std::unordered_map<Bind::Sampler::Type, std::shared_ptr<Bind::Sampler>> g_pSamplerStates;
	std::unordered_map<Bind::Rasterizer::Type, std::shared_ptr<Bind::Rasterizer>> g_pRasterizerStates;

	// Shaders
	VertexShader vertexShader;
	PixelShader pixelShader;

	// Constant buffers
	ID3D11Buffer* g_pConstantBuffer = nullptr;
	ID3D11Buffer* g_pLightConstantBuffer = nullptr;

	// Objects
	ImGuiManager imgui;
	std::shared_ptr<Camera> camera;
	DrawableGameObject g_GameObject;
};

#endif