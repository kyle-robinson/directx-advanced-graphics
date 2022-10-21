#pragma once
#ifndef WINDOWCONTAINER_H
#define WINDOWCONTAINER_H

#include "Mouse.h"
#include "Keyboard.h"
#include "Graphics.h"
#include "RenderWindow.h"

class WindowContainer
{
public:
	WindowContainer();
	LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	RenderWindow renderWindow;
	Graphics graphics;
	Keyboard keyboard;
	Mouse mouse;
};

#endif