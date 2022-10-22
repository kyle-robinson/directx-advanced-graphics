#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "Resource.h"

#include "Cube.h"
#include "Timer.h"
#include "Light.h"
#include "Shaders.h"

#include "Input.h"
#include "ImGuiManager.h"
#include "WindowContainer.h"

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
	Light m_light;
	Camera m_camera;
	Cube m_gameObject;
	ImGuiManager m_imgui;

	// Program data
	Timer m_timer;
	Input m_input;
	ConstantBuffer<Matrices> m_cbMatrices;
};

#endif