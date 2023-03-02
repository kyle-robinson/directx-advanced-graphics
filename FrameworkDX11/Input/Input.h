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

	inline bool GetIsMovingCursor() const noexcept { return m_bMovingCursor; }
	void AddCamControl( CameraController* cam );
	void Update( float dt );

	inline void BlockMouseInputs() noexcept { m_bAllowMouseInputs = false; }
	inline void UnblockMouseInputs() noexcept { m_bAllowMouseInputs = true; m_mouse.Clear(); }

	inline void BlockKeyInputs() noexcept { m_bAllowKeyboardInputs = false; }
	inline void UnblockKeyInputs() noexcept { m_bAllowKeyboardInputs = true; m_keyboard.Clear(); }

private:
	void UpdateMouse( float dt );
	void UpdateKeyboard( float dt );

	bool m_bMovingCursor = false;
	bool m_bAllowMouseInputs = true;
	bool m_bAllowKeyboardInputs = true;
	CameraController* m_pCameraControl;
};

#endif