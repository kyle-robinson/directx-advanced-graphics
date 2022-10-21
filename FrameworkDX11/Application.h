#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "Resource.h"

#include "Shaders.h"
#include "structures.h"
#include "DrawableGameObject.h"

#include "Camera.h"
#include "WindowContainer.h"

typedef std::vector<DrawableGameObject*> vecDrawables;

class Application : public WindowContainer
{
public:
	bool Initialize( HINSTANCE hInstance, int width, int height );
	bool InitializeDevice();
	bool InitializeMesh();
	void CleanupDevice();

	void setupLightForRender();
	float calculateDeltaTime();

	bool ProcessMessages() noexcept;
	void UpdateInput( float dt );
	void Update();
	void Render();
private:
	// Constant buffers
	ID3D11Buffer* g_pConstantBuffer = nullptr;
	ID3D11Buffer* g_pLightConstantBuffer = nullptr;

	// Objects
	std::shared_ptr<Camera> camera;
	DrawableGameObject g_GameObject;
};

#endif