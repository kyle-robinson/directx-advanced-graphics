#pragma once
#ifndef IMGUIMANAGER_H
#define IMGUIMANAGER_H

#include "imgui.h"
#include "gizmo/ImGuizmo.h"

#include "CameraController.h";
#include "TerrainJsonLoad.h"
#include "Structures.h"

class Transform;
class AnimatedModel;
class LightController;
class DrawableGameObject;
class RasterizerController;
class ShaderController;
class BillboardObject;
class TerrainVoxel;
class Terrain;
class Input;

class ImGuiManager
{
private:
	struct ImGuizmoData
	{
		ImGuizmoData()
			: ID( 0 )
			, Enable( true )
			, UseSnap( false )
			, IsVisible( false )
			, SnapAmount( { 1.0f, 1.0f, 1.0f } )
			, CurrentGizmoMode( ImGuizmo::WORLD )
			, CurrentGizmoOperation( ImGuizmo::TRANSLATE )
		{}

		int ID;
		bool Enable;
		bool UseSnap;
		bool IsVisible;
		XMFLOAT3 SnapAmount;
		ImGuizmo::MODE CurrentGizmoMode;
		ImGuizmo::OPERATION CurrentGizmoOperation;
	};

public:
	ImGuiManager();
	~ImGuiManager();

	void BeginRender();
	void EndRender();
	bool Initialize( HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

	void SceneWindow( UINT width, UINT height, ID3D11ShaderResourceView* pTexture, Input* pInput );
	void CameraMenu( CameraController* cameraControl, bool usingVoxels );
	void ShaderMenu( ShaderController* shaderControl, PostProcessingCB* postSettings, RasterizerController* rasterControl );
	void ObjectMenu( ID3D11Device* pDevice, Camera* pCamera, std::vector<DrawableGameObject*>& gameObjects );
	void LightMenu( LightController* lightControl, Camera* pCamera );
	void BillboardMenu( BillboardObject* billboardObject );
	void TerrainMenu( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, Terrain* terrain, TerrainVoxel* voxelTerrain );
	void AnimationMenu( AnimatedModel* animModel, Camera* pCamera );
	void BezierSplineMenu();

private:
	void SpawnImGuizmo( Transform* pTransform, Camera* pCamera, XMFLOAT4X4& worldMat, ImGuizmoData& imguizmoData );
	void SetWhiteTheme();
	void SetUbuntuTheme();
	void SetBlackGoldTheme();

	int m_iGizmoID = 0;
	int m_iActiveGizmoID = -1;
	bool m_bUsingTranslation = true;
	bool m_bUsingManipulator = false;

	std::vector<XMFLOAT2> m_vPoints;
	XMFLOAT2 m_vSceneWindowPos = { 0.0f, 0.0f };
	XMFLOAT2 m_vSceneWindowSize = { 0.0f, 0.0f };
};

#endif