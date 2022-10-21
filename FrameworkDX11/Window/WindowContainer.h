#pragma once
#ifndef WINDOWCONTAINER_H
#define WINDOWCONTAINER_H

#include "Mouse.h"
#include "Keyboard.h"
#include "RenderWindow.h"

class WindowContainer
{
public:
	WindowContainer();
	LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	RenderWindow renderWindow;
	Keyboard keyboard;
	Mouse mouse;
};

#endif