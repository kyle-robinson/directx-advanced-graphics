#pragma once

#include<Windows.h>
#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include"misc/cpp/imgui_stdlib.h"
#include"CameraController.h";
//#include"C++HelperFunctions.h"

#include"TerrainJsonLoad.h"
#include"structures.h"

class DrawableGameObject;
class LightControll;
class ShaderController;
class BillboardObject;
class Terrain;
class TerrainVoxel;
class RasterStateManager;
class AnimatedModel;
/// <summary>
/// controlls the DImGui that controll parts of the application
/// </summary>
class ImGuiManager
{
public:
	ImGuiManager();
	~ImGuiManager();

	void BeginRender();
	void EndRender();
	void Initialize(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context);


	//menus
	void DrawCamMenu(CameraController* Cams);
	void ShaderMenu(ShaderController* Shader, PostProcessingCB* postSettings, RasterStateManager* RasterState,bool &rtt);
	void ObjectControl(DrawableGameObject* GameObject);
	void LightControl(LightControll* LightControl);
	void BillBoradControl(BillboardObject* BillControl);
	void BezierCurveSpline();
	void TerrainControll(Terrain* terrain,TerrainVoxel* VoxelTerrain, ID3D11Device* device, ID3D11DeviceContext* _pImmediateContext);
	void AnimationControll(AnimatedModel* AnnimationModel);



	vector<XMFLOAT3> points;
private:
	void SetBlackGoldStyle();


};

