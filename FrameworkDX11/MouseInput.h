#pragma once
#include<queue>

#include"MouseEvent.h"

using namespace std;
/// <summary>
/// class to check teh state of the mouse
/// created from https://github.com/ThomasMillard123/FGAGC
/// </summary>
class MouseInput
{

	//Func
public:
	void OnLeftPress(int x, int y);
	void OnLeftReleased(int x, int y);

	void OnRightPress(int x, int y);
	void OnRightReleased(int x, int y);

	void OnMiddlePress(int x, int y);
	void OnMiddleReleased(int x, int y);

	void OnWheelUp(int x, int y);
	void OnWheelDown(int x, int y);


	void OnMouseMove(int x, int y);
	void OnMouseMoveRaw(int x, int y);


	bool IsLeftDown();
	bool IsRightDown();
	bool IsMiddleDown();

	int GetMousePosX();
	int GetMousePosY();
	MousePoint GetMousePos();


	bool EventBufferIsEmpty();
	MouseEvent ReadEvent();



	//vars
private:
	queue<MouseEvent> _MouseEventBuffer;

	bool _IsLeftDown = false;
	bool _IsRightDown = false;
	bool _IsMiddleDown = false;
	int _MousePosX = 0;
	int _MousePosY = 0;
};

