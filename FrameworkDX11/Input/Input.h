#pragma once
#ifndef INPUT_H
#define INPUT_H

#include "WindowContainer.h"
#include "CameraController.h"

class Input : public WindowContainer
{
public:
	Input();
	~Input();

	void AddCamControl( CameraController* cam );
	void Update( float dt );

private:
	void UpdateMouse( float dt );
	void UpdateKeyboard( float dt );

	CameraController* m_pCameraControl;
};

#endif