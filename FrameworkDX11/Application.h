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

#if defined ( _x64 )
#include "RenderableGameObject.h"
#endif

class Application : public WindowContainer
{
public:
	Application();
	~Application();

	bool Initialize( HINSTANCE hInstance, int width, int height );
	void Update();
	void Draw();

private:
	bool InitializeWorld();
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
	AnimatedModel* m_pSoldier;
	DrawableGameObject* m_pCube;
	std::vector<DrawableGameObject*> m_pWalls;

#if defined ( _x64 )
	RenderableGameObject m_pSky;
#endif

	// Lights
	LightController* m_pLightController;
	ShadowMap* m_pDepthLight;

	// Input
	CameraController* m_pCamController;
	ImGuiManager* m_pImGuiManager;
	Input* m_pInput;
	Timer m_timer;

	// Terrain
	TerrainVoxel* m_pVoxelTerrain;
	Terrain* m_pTerrain;
};

#endif