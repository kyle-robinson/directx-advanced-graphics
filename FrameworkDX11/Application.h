#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include "Input.h"
#include "ImGuiManager.h"
#include "WindowContainer.h"

#include "Timer.h"
#include "Terrain.h"
#include "TerrainVoxel.h"
#include "AnimatedModel.h"
#include "LightController.h"
#include "DrawableGameObject.h"

class Application : public WindowContainer
{
public:
	Application();
	~Application();

	bool Initialize( HINSTANCE hInstance, int width, int height );
	void Update();
	void Draw();

private:
	void SetupLightForRender();
	bool InitDevice();
	bool InitMesh();
	bool InitWorld();
	void Cleanup();

	// Constant buffers
	ConstantBuffer<MatrixBuffer> m_matrixCB;
	ConstantBuffer<LightPropertiesCB> m_lightCB;
	ConstantBuffer<PostProcessingCB> m_postProcessingCB;

	// Fullscreen quad
	struct ScreenVertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex;
	};
	VertexBuffer<ScreenVertex> m_screenVB;
	XMMATRIX m_mProjection;
	XMMATRIX m_mView;

	// Objects
	AnimatedModel* m_pAnimModel;
	DrawableGameObject* m_pSky;
	DrawableGameObject* m_pCube;
	std::vector<DrawableGameObject*> m_pWalls;

	// Lights
	LightController* m_pLightController;
	ShadowMap* m_pDepthLight;

	// Input
	CameraController* m_pCamController;
	ImGuiManager* m_pImGuiManager;
	Camera* m_pCamera;
	Input* m_pInput;
	Timer m_timer;

	// Terrain
	TerrainVoxel* m_pVoxelTerrain;
	Terrain* m_pTerrain;
};

#endif