#pragma once
#ifndef LEVEL3_H
#define LEVEL3_H

#include "LevelContainer.h"

class Level3 : public LevelContainer
{
public:
	Level3( const std::string& name ) { m_sLevelName = name; }

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