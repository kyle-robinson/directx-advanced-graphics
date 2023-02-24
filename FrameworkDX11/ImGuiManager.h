#pragma once
#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include <Windows.h>
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "misc/cpp/imgui_stdlib.h"

#include "CameraController.h";
#include "TerrainJsonLoad.h"
#include "Structures.h"

class DrawableGameObject;
class LightController;
class ShaderController;
class BillboardObject;
class Terrain;
class TerrainVoxel;
class RasterizerController;
class AnimatedModel;

class ImGuiManager
{
public:
	ImGuiManager();
	~ImGuiManager();

	void BeginRender();
	void EndRender();
	void Initialize( HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

	void DrawCamMenu( CameraController* cameraControl );
	void ShaderMenu( ShaderController* shaderControl, PostProcessingCB* postSettings, RasterizerController* rasterControl, bool& rtt );
	void ObjectControl( DrawableGameObject* gameObject );
	void LightControl( LightController* lightControl );
	void BillBoradControl( BillboardObject* billboardObject );
	void BezierCurveSpline();
	void TerrainControll( Terrain* terrain, TerrainVoxel* voxelTerrain, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	void AnimationControll( AnimatedModel* animModel );

	inline std::vector<XMFLOAT3> GetPoints() const noexcept { return m_vPoints; }

private:
	void SetBlackGoldStyle();
	std::vector<XMFLOAT3> m_vPoints;
};

#endif