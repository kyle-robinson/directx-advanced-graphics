#pragma once
#ifndef LEVEL1_H
#define LEVEL1_H

#include "LevelContainer.h"

#include "Resource.h"
#include "DDSTextureLoader.h"

#include "Cube.h"
#include "FXAA.h"
#include "SSAO.h"
#include "Mapping.h"
#include "Shaders.h"
#include "Deferred.h"
#include "MotionBlur.h"
#include "LightControl.h"
#include "PostProcessing.h"

#if defined ( _x64 )
#include "RenderableGameObject.h"
#endif

class Level1 : public LevelContainer
{
public:
	Level1( const std::string& name ) { m_sLevelName = name; }

	virtual void OnCreate();
	virtual void OnSwitch();

	virtual void RenderFrame();
	virtual void EndFrame_Start();
	virtual void EndFrame_End();

	virtual void Update( const float dt );
	virtual void CleanUp() {}

private:
	// Objects
	Cube m_cube;
	LightControl* m_pLightControl;

#if defined ( _x64 )
	RenderableGameObject m_objSkysphere;
#endif

	// Systems
	FXAA m_fxaa;
	SSAO m_ssao;
	Mapping m_mapping;
	Deferred m_deferred;
	MotionBlur m_motionBlur;
	PostProcessing m_postProcessing;

	XMFLOAT4X4 m_previousViewProjection;
	ConstantBuffer<Matrices> m_cbMatrices;
	ConstantBuffer<MatricesNormalDepth> m_cbMatricesNormalDepth;
};

#endif