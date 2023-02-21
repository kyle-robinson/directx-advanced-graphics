#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include <windows.h>
#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include <iostream>
#include <vector>

#include "WindowContainer.h"
#include "Input.h"

#include "DDSTextureLoader.h"
#include "resource.h"
#include "DrawableGameObject.h"
#include "structures.h"

#include"CameraController.h"


#include"ImGuiManager.h"

#include"ShaderController.h"
#include"LightControll.h"
#include"ShadowMap.h"
#include"RenderTargetTextureClass.h"
#include"BillboradObject.h"
#include"RenderTargetControll.h"
#include"Terrain.h"

#include"RansterStateManager.h"

#include"TerrainVoxel.h"
#include"AnimatedModel.h"

class Application : public WindowContainer
{
private:
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel ;
	ID3D11Device* _pd3dDevice;
	ID3D11Device1* _pd3dDevice1;
	ID3D11DeviceContext* _pImmediateContext ;
	ID3D11DeviceContext1* _pImmediateContext1 ;
	IDXGISwapChain* _pSwapChain ;
	IDXGISwapChain1* _pSwapChain1 ;
	ID3D11RenderTargetView* _pRenderTargetView ;
	ID3D11Texture2D* _pDepthStencil = nullptr;
	ID3D11DepthStencilView* _pDepthStencilView = nullptr;
	RasterStateManager* RSControll;
	ID3D11RasterizerState* RSCullNone;
	ID3D11RasterizerState* Wirer;
	ID3D11Buffer* _pConstantBuffer ;

	ID3D11Buffer* _pLightConstantBuffer;
	ID3D11Buffer* _pPostProcessingConstantBuffer;

	//post processing
	//--------------------------------------------------------
	PostProcessingCB postSettings;

	bool isRTT = false;


	//full screen quad
	struct SCREEN_VERTEX
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex;
	};

	ID3D11Buffer* g_pScreenQuadVB = nullptr;
	ID3D11SamplerState* m_pPointrLinear;
	ID3D11SamplerState* m_pLINEARBORDER;
	ShadowMap* DepthLight;
	RenderTargetControll* RenderTargetControl;
	BillboardObject* BillBoradObject;

	XMMATRIX _View;
	XMMATRIX _Projection;

	int	_viewWidth;
	int	_viewHeight;

	DrawableGameObject		_GameObject;
	DrawableGameObject		_GameObjectFloor;
	Terrain* _Terrain;
	TerrainVoxel* _VoxelTerrain;
	Input* m_pInput;
	ShaderController* _Shader;
	CameraController* _pCamControll;
	ImGuiManager* DimGuiManager;
	LightControll* _pLightContol;
	Camera* _Camrea;
	AnimatedModel* AnimmationObject;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
	void Update();
	void Draw();

private:
	float calculateDeltaTime();
	void setupLightForRender();
	HRESULT InitDevice();
	void Cleanup();

	HRESULT	InitMesh();
	HRESULT	InitWorld(int width, int height);
};

#endif