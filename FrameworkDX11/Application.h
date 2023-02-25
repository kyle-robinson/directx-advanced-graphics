#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include "Input.h"
#include "ImGuiManager.h"
#include "WindowContainer.h"

#include "resource.h"
#include "Structures.h"
#include "ConstantBuffer.h"
#include "DDSTextureLoader.h"

#include "ShadowMap.h"
#include "LightController.h"
#include "CameraController.h"

#include "Terrain.h"
#include "TerrainVoxel.h"

#include "AnimatedModel.h"
#include "BillboradObject.h"
#include "DrawableGameObject.h"

class Application : public WindowContainer
{
private:
	// Constant buffers
	ConstantBuffer<MatrixBuffer> m_matrixCB;
	ConstantBuffer<LightPropertiesCB> m_lightCB;
	ConstantBuffer<PostProcessingCB> m_postProcessingCB;

	// Fullscreen quad
	struct SCREEN_VERTEX
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex;
	};
	bool m_bIsRTT = false;
	ID3D11Buffer* m_pScreenQuadVB = nullptr;
	XMMATRIX m_mProjection;
	XMMATRIX m_mView;

	// Objects
	DrawableGameObject m_cube;
	DrawableGameObject m_ground;
	BillboardObject* m_pBillboard;
	AnimatedModel* m_pAnimModel;

	// Lights
	LightController* m_pLightController;
	ShadowMap* m_pDepthLight;

	// Input
	CameraController* m_pCamController;
	ImGuiManager* m_pImGuiManager;
	Camera* m_pCamera;
	Input* m_pInput;

	// Terrain
	TerrainVoxel* m_pVoxelTerrain;
	Terrain* m_pTerrain;

public:
	Application();
	~Application();

	HRESULT Initialize( HINSTANCE hInstance, int width, int height );
	void Update();
	void Draw();

private:
	float CalculateDeltaTime();
	void SetupLightForRender();
	HRESULT InitDevice();
	HRESULT	InitMesh();
	HRESULT	InitWorld();
	void Cleanup();
};

#endif