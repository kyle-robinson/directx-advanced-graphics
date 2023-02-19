#pragma once
#include<Windows.h>
#include<memory>


#include<d3d11.h>
#include <directxmath.h>

#include"Keyboard.h"
#include"MouseInput.h"
#include"Mouse_Picking.h"


#define WasP 0x40000000;

#include"CameraController.h"

using namespace DirectX;


/// <summary>
/// Class to hold all inputs need by application
/// created from https://github.com/ThomasMillard123/FGAGC
/// </summary>
class InputControllor
{

public:
	InputControllor();
	~InputControllor();

	bool HandleInput(UINT message, WPARAM wParam, LPARAM lParam);
	void Update();
	void CharInputUpdate();
	void KeyInputUpdate();
	void MouseInputUpdate();



	void AddCam(CameraController* cam);


private:
	unsigned char ch;
	Keyboard _KeyInput;
	MouseInput _MouseInput;
	
	CameraController* _CamreaControll;
};

