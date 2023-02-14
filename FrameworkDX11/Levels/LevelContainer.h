#pragma once
#ifndef LEVELCONTAINER_H
#define LEVELCONTAINER_H

#include "Camera.h"
#include "Graphics.h"
#include "ImGuiManager.h"

/// <summary>
/// Loads and renders/updates all the components and models for the current scene/level.
/// Sets up any constant buffers that are specific to this particular scene/level.
/// Used to initialize objects native to every level.
/// </summary>
class LevelContainer
{
	friend class Application;
public:
	virtual ~LevelContainer( void ) = default;
	inline void Initialize( Graphics* gfx, Camera* camera, ImGuiManager* imgui )
	{
		m_gfx = gfx;
		m_imgui = imgui;
		m_camera = camera;
	}

	inline bool GetIsCreated() const noexcept { return m_bIsCreated; }
	inline void SetIsCreated( bool created ) noexcept { m_bIsCreated = created; }

	inline void SetLevelName( std::string name ) noexcept { m_sLevelName = name; }
	inline std::string GetLevelName() const noexcept { return m_sLevelName; }

	inline Camera* GetCamera() const noexcept { return m_camera; }
	inline Graphics* GetGraphics() const noexcept { return m_gfx; }
	inline ImGuiManager* GetImguiManager() const noexcept { return m_imgui; }

	// Render/Update Scene Functions
	virtual void OnCreate() {}
	virtual void OnSwitch() {}

	virtual void RenderFrame() {}
	virtual void EndFrame_Start() {}
	virtual void EndFrame_End() {}

	virtual void Update( const float dt ) {}
	virtual void CleanUp() {}

protected:
	// Objects
	Graphics* m_gfx;
	Camera* m_camera;
	ImGuiManager* m_imgui;

	// Next Level
	bool m_bIsCreated = false;
	std::string m_sLevelName;
};

#endif