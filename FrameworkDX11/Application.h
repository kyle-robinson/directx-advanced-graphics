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
	ConstantBuffer<Matrices> m_cbMatrices;
	ConstantBuffer<Light_CB> m_cbLight;

	// Objects
	DrawableGameObject m_gameObject;
	std::shared_ptr<Camera> m_pCamera;
};

#endif