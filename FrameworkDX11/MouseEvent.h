#pragma once

struct MousePoint
{
	int x;
	int y;
};

enum MouseEventType
		{
			LPress,
			LRelease,
			RPress,
			RRelease,
			MPress,
			MRelease,
			WheelUp,
			WheelDown,
			Move,
			RawMove,
			Invalid
		};
/// <summary>
/// class to hold the mouse data
/// created from https://github.com/ThomasMillard123/FGAGC
/// </summary>
class MouseEvent
{
	//Func
public:
	MouseEvent();
	MouseEvent(const MouseEventType type, const int x, const int y);

	bool IsValid();
	MouseEventType GetType();
	MousePoint GetPos();

	int GetMouseX();
	int GetMouseY();

private:


	//vars
public:
	

	MouseEventType type;
	int x;
	int y;

private:

};

