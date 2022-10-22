#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "Resource.h"

#include "Shaders.h"
#include "structures.h"
#include "DrawableGameObject.h"

#include "Input.h"
#include "ImGuiManager.h"
#include "WindowContainer.h"

typedef std::vector<DrawableGameObject*> vecDrawables;

class Application : public WindowContainer
{
public:
	bool Initialize( HINSTANCE hInstance, int width, int height );
	void CleanupDevice();

	void setupLightForRender();
	float calculateDeltaTime();

	bool ProcessMessages() noexcept;
	void Update();
	void Render();
private:
	// Constant buffers
	ConstantBuffer<Matrices> m_cbMatrices;
	ConstantBuffer<Light_CB> m_cbLight;

	// Objects
	Input m_input;
	Camera m_camera;
	ImGuiManager m_imgui;
	DrawableGameObject m_gameObject;
};

#endif