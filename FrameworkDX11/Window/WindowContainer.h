#pragma once
#ifndef WINDOWCONTAINER_H
#define WINDOWCONTAINER_H

#include <windows.h>
#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include <iostream>
#include <vector>

#include "Mouse.h"
#include "Graphics.h"
#include "Keyboard.h"
#include "RenderWindow.h"

class WindowContainer
{
public:
	WindowContainer();
	LRESULT CALLBACK WindowProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void EnableCursor() noexcept;
	void DisableCursor() noexcept;
	inline bool IsCursorEnabled() const noexcept { return m_bCursorEnabled; }

protected:
	void ConfineCursor() noexcept;
	void FreeCursor() noexcept;
	void ShowCursor() noexcept;
	void HideCursor() noexcept;
	void EnableImGuiMouse() noexcept;
	void DisableImGuiMouse() noexcept;

	DirectX::XMFLOAT2 windowSize;
	bool m_bCursorEnabled = true;

	RenderWindow m_window;
	Keyboard m_keyboard;
	Graphics m_gfx;
	Mouse m_mouse;
};

#endif