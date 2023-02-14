#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include "Input.h"
#include "Timer.h"
#include "ImGuiManager.h"
#include "WindowContainer.h"
#include "LevelStateMachine.h"

class Application : public WindowContainer
{
public:
	bool Initialize( HINSTANCE hInstance, int width, int height );
	void CleanupDevice();

	bool ProcessMessages() noexcept;
	void Update();
	void Render();
private:
	// Objects
	Timer m_timer;
	Input m_input;
	Camera m_camera;
	ImGuiManager m_imgui;

	// Levels
	std::string m_sCurrentLevelName;
	LevelStateMachine m_stateMachine;
	std::vector<std::string> m_sLevelNames;
	std::vector<std::shared_ptr<LevelContainer>> m_pLevels;
};

#endif