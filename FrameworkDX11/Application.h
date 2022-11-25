#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include <dxtk/SpriteFont.h>
#include <dxtk/SpriteBatch.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "Resource.h"

#include "Cube.h"
#include "FXAA.h"
#include "SSAO.h"
#include "Input.h"
#include "Timer.h"
#include "Light.h"
#include "Mapping.h"
#include "Shaders.h"
#include "Deferred.h"
#include "MotionBlur.h"
#include "ImGuiManager.h"
#include "PostProcessing.h"
#include "WindowContainer.h"
#include "RenderableGameObject.h"

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
	Cube m_cube;
	Light m_light;
	Camera m_camera;
	ImGuiManager m_imgui;
	RenderableGameObject m_objSkysphere;
	
	// Systems
	FXAA m_fxaa;
	SSAO m_ssao;
	Mapping m_mapping;
	Deferred m_deferred;
	MotionBlur m_motionBlur;
	PostProcessing m_postProcessing;

	// Data
	Timer m_timer;
	Input m_input;
	XMFLOAT4X4 m_previousViewProjection;
	ConstantBuffer<Matrices> m_cbMatrices;
	std::unique_ptr<SpriteFont> m_spriteFont;
	std::unique_ptr<SpriteBatch> m_spriteBatch;
	ConstantBuffer<MatricesNormalDepth> m_cbMatricesNormalDepth;
};

#endif