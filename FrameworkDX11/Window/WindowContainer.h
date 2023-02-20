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

	void EnableCursor() noexcept;
	void DisableCursor() noexcept;
	inline bool IsCursorEnabled() const noexcept { return cursorEnabled; }

protected:
	void ConfineCursor() noexcept;
	void FreeCursor() noexcept;
	void ShowCursor() noexcept;
	void HideCursor() noexcept;
	void EnableImGuiMouse() noexcept;
	void DisableImGuiMouse() noexcept;

	DirectX::XMFLOAT2 windowSize;
	bool windowResized = false;
	bool cursorEnabled = true;

	RenderWindow renderWindow;
	Keyboard keyboard;
	Mouse mouse;
};

#endif