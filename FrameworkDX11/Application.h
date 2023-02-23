#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include "WindowContainer.h"
#include "Input.h"

#include "DDSTextureLoader.h"
#include "resource.h"
#include "DrawableGameObject.h"
#include "structures.h"

#include"CameraController.h"
#include"ImGuiManager.h"

#include"LightControll.h"
#include"ShadowMap.h"
#include"BillboradObject.h"
#include"Terrain.h"

#include"RansterStateManager.h"

#include"TerrainVoxel.h"
#include"AnimatedModel.h"

class Application : public WindowContainer
{
private:
	RasterStateManager* RSControll;
	ID3D11RasterizerState* RSCullNone;
	ID3D11RasterizerState* Wirer;
	ID3D11Buffer* _pConstantBuffer;

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
	ShadowMap* DepthLight;
	BillboardObject* BillBoradObject;

	XMMATRIX _View;
	XMMATRIX _Projection;

	DrawableGameObject _GameObject;
	DrawableGameObject _GameObjectFloor;
	Terrain* _Terrain;
	TerrainVoxel* _VoxelTerrain;
	Input* m_pInput;
	CameraController* _pCamControll;
	ImGuiManager* DimGuiManager;
	LightControll* _pLightContol;
	Camera* _Camrea;
	AnimatedModel* AnimmationObject;

public:
	Application();
	~Application();

	HRESULT Initialise( HINSTANCE hInstance, int width, int height );
	void Update();
	void Draw();

private:
	float calculateDeltaTime();
	void setupLightForRender();
	HRESULT InitDevice();
	void Cleanup();

	HRESULT	InitMesh();
	HRESULT	InitWorld();
};

#endif