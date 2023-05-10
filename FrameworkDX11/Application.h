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
	DrawableGameObject* m_pCube;
	std::vector<DrawableGameObject*> m_pWalls;
	std::unique_ptr<AnimatedModel> m_pSoldier;

#if defined ( _x64 )
	RenderableGameObject m_pSky;
#endif

	// Lights
	std::unique_ptr<LightController> m_pLightController;
	std::unique_ptr<ShadowMap> m_pDepthLight;

	// Input
	std::unique_ptr<CameraController> m_pCamController;
	std::unique_ptr<ImGuiManager> m_pImGuiManager;
	std::unique_ptr<Input> m_pInput;
	Timer m_timer;

	// Terrain
	std::unique_ptr<TerrainVoxel> m_pVoxelTerrain;
	std::unique_ptr<Terrain> m_pTerrain;
};

#endif