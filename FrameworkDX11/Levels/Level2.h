#pragma once
#ifndef LEVEL2_H
#define LEVEL2_H

#include "LevelContainer.h"

class Level2 : public LevelContainer
{
public:
	Level2( const std::string& name ) { m_sLevelName = name; }

	virtual void OnCreate();
	virtual void OnSwitch();

	virtual void RenderFrame();
	virtual void EndFrame_Start();
	virtual void EndFrame_End();

	virtual void Update( const float dt );
	virtual void CleanUp() {}

private:

};

#endif