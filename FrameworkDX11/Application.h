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
#include "ShadowMap.h"
#include "MotionBlur.h"
#include "ImGuiManager.h"
#include "PostProcessing.h"
#include "WindowContainer.h"

#if defined ( _x64 )
#include "RenderableGameObject.h"
#endif

struct BoundingSphereShd
{
	BoundingSphereShd()
		: Center(0.0f, 0.0f, 0.0f),
		Radius(0.0f)
	{}

	XMFLOAT3 Center;
	float Radius;
};

class Application : public WindowContainer
{
public:
	bool Initialize( HINSTANCE hInstance, int width, int height );
	void CleanupDevice();

	bool ProcessMessages() noexcept;
	void Update();
	void Render();

	void BuildShadowTransform();
private:
	// Objects
	Cube m_cube;
	Light m_light;
	Camera m_camera;
	ImGuiManager m_imgui;

#if defined ( _x64 )
	RenderableGameObject m_objSkysphere;
#endif
	
	// Systems
	FXAA m_fxaa;
	SSAO m_ssao;
	Mapping m_mapping;
	Deferred m_deferred;
	ShadowMap m_shadowMap;
	MotionBlur m_motionBlur;
	PostProcessing m_postProcessing;

	// Data
	Timer m_timer;
	Input m_input;
	BoundingSphereShd mSceneBounds;

	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;
	XMFLOAT4X4 m_previousViewProjection;
	
	std::unique_ptr<SpriteFont> m_spriteFont;
	std::unique_ptr<SpriteBatch> m_spriteBatch;

	ConstantBuffer<Matrices> m_cbMatrices;
	ConstantBuffer<MatricesShadow> m_cbMatricesShadow;
	ConstantBuffer<MatricesNormalDepth> m_cbMatricesNormalDepth;
};

#endif