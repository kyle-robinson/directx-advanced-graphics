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

class AnimatedModel;
class LightController;
class DrawableGameObject;
class RasterizerController;
class ShaderController;
class BillboardObject;
class TerrainVoxel;
class Terrain;

class ImGuiManager
{
public:
	ImGuiManager();
	~ImGuiManager();

	void BeginRender();
	void EndRender();
	void Initialize( HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

	void CameraMenu( CameraController* cameraControl );
	void ShaderMenu( ShaderController* shaderControl, PostProcessingCB* postSettings, RasterizerController* rasterControl, bool& rtt );
	void ObjectMenu( DrawableGameObject* gameObject );
	void LightMenu( LightController* lightControl );
	void BillboardMenu( BillboardObject* billboardObject );
	void TerrainMenu( Terrain* terrain, TerrainVoxel* voxelTerrain, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
	void AnimationMenu( AnimatedModel* animModel );
	void BezierSplineMenu();

	inline std::vector<XMFLOAT3> GetPoints() const noexcept { return m_vPoints; }
	inline void SetPoints( std::vector<XMFLOAT3> points ) noexcept { m_vPoints = points; }

private:
	void SetBlackGoldStyle();
	std::vector<XMFLOAT3> m_vPoints;
};

#endif