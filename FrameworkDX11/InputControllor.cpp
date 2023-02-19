#include "InputControllor.h"
#include <imgui.h>

InputControllor::InputControllor()
{
	_CamreaControll = nullptr;
}

InputControllor::~InputControllor()
{

}


bool InputControllor::HandleInput(UINT message, WPARAM wParam, LPARAM lParam)
{
	

	//get input from windows
	switch (message)
	{
		//Keyborad Input
	case WM_KEYUP: {
		unsigned char ch = static_cast<unsigned char>(wParam);
		_KeyInput.OnKeyRelace(ch);
		return true;
	}
				 break;
	case WM_KEYDOWN: {
		unsigned char ch = static_cast<unsigned char>(wParam);
		if (_KeyInput.IsKeysAutoRepat()) {

			_KeyInput.OnKeyPress(ch);
		}
		else
		{
			const bool wasPressed = lParam & WasP;
			if (!wasPressed) {
				_KeyInput.OnKeyPress(ch);
			}

		}
		return true;
	}
				   break;
	case WM_CHAR:
	{
		unsigned char ch = static_cast<unsigned char>(wParam);
		if (_KeyInput.IsCharAutoRepat()) {
			_KeyInput.OnChar(ch);
		}
		else
		{
			const bool wasPressed = lParam & WasP;
			if (!wasPressed) {
				_KeyInput.OnChar(ch);
			}

		}
		return true;
	}
	break;
	//mouse input
	case WM_MOUSEMOVE:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		_MouseInput.OnMouseMove(x, y);
		return true;
	}
	break;
	case WM_LBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		_MouseInput.OnLeftPress(x, y);
		return true;
	}
	break;
	case WM_LBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		_MouseInput.OnLeftReleased(x, y);
		return true;
	}
	break;
	case WM_RBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		_MouseInput.OnRightPress(x, y);
		return true;
	}
	break;
	case WM_RBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		_MouseInput.OnRightReleased(x, y);
		return true;
	}
	break;
	case WM_MBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		_MouseInput.OnMiddlePress(x, y);
		return true;
	}
	break;
	case WM_MBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);

		_MouseInput.OnMiddleReleased(x, y);
		return true;
	}
	break;
	case WM_MOUSEHWHEEL:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
			_MouseInput.OnWheelUp(x, y);
		}
		else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
			_MouseInput.OnWheelDown(x, y);
		}



		return true;
	}
	break;

	case WM_INPUT:
	{
		UINT dataSize = 0u;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
		if (dataSize > 0)
		{
			std::unique_ptr<BYTE[]> rawData = std::make_unique<BYTE[]>(dataSize);
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawData.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
			{
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawData.get());
				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					_MouseInput.OnMouseMoveRaw(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				}
			}
		}


		
	}
	break;
	}
	return false;
}

void InputControllor::Update()
{
	CharInputUpdate();
	KeyInputUpdate();
	MouseInputUpdate();
}

void InputControllor::CharInputUpdate()
{
	//keyborad input
	while (!_KeyInput.CharBufferIsEmpty())
	{
		unsigned char ch = _KeyInput.Raedchar();

		_KeyInput.EnableAutoRepeatChars();

	}
	

}

void InputControllor::KeyInputUpdate()
{
	while (!_KeyInput.KeyBufferIsEmpty())
		{
			KeyboardEvent keyEvent = _KeyInput.ReadKey();
			 ch = keyEvent.GetKeyCode();

			_KeyInput.EnableAutoRepeatKeys();

			//---------------inputs------------------

			
			////cam controlls
			if (_KeyInput.KeyIsPress('W')) {
				_CamreaControll->GetCurentCam()->AgustPos({
						_CamreaControll->GetCurentCam()->GetVecFord().x *  _CamreaControll->GetCurentCam()->GetCamSpeed()  ,
						_CamreaControll->GetCurentCam()->GetVecFord().y * _CamreaControll->GetCurentCam()->GetCamSpeed() ,
						_CamreaControll->GetCurentCam()->GetVecFord().z * _CamreaControll->GetCurentCam()->GetCamSpeed()
				});
			}
			if (_KeyInput.KeyIsPress('S')) {
				_CamreaControll->GetCurentCam()->AgustPos({
						_CamreaControll->GetCurentCam()->GetVecBack().x * _CamreaControll->GetCurentCam()->GetCamSpeed(),
						_CamreaControll->GetCurentCam()->GetVecBack().y * _CamreaControll->GetCurentCam()->GetCamSpeed() ,
						_CamreaControll->GetCurentCam()->GetVecBack().z * _CamreaControll->GetCurentCam()->GetCamSpeed()
					});
			}
			if (_KeyInput.KeyIsPress('D')) {
				_CamreaControll->GetCurentCam()->AgustPos({
						_CamreaControll->GetCurentCam()->GetVecRight().x * _CamreaControll->GetCurentCam()->GetCamSpeed(),
						_CamreaControll->GetCurentCam()->GetVecRight().y * _CamreaControll->GetCurentCam()->GetCamSpeed() ,
						_CamreaControll->GetCurentCam()->GetVecRight().z * _CamreaControll->GetCurentCam()->GetCamSpeed()
					});
			}
			if (_KeyInput.KeyIsPress('A')) {
				_CamreaControll->GetCurentCam()->AgustPos({
						_CamreaControll->GetCurentCam()->GetVecLeft().x * _CamreaControll->GetCurentCam()->GetCamSpeed(),
						_CamreaControll->GetCurentCam()->GetVecLeft().y * _CamreaControll->GetCurentCam()->GetCamSpeed() ,
						_CamreaControll->GetCurentCam()->GetVecLeft().z * _CamreaControll->GetCurentCam()->GetCamSpeed()
					});
			}

		


			//----------------------------------------------------------
		}
}

static XMFLOAT2 mousePos;
void InputControllor::MouseInputUpdate()
{
//mouse input
	while (!_MouseInput.EventBufferIsEmpty())
	{
		MouseEvent mEvent = _MouseInput.ReadEvent();
		
		if (_MouseInput.IsLeftDown())
		{
			// update raw camera movement
			if (mEvent.GetType() == MouseEventType::RawMove )
			{
				mousePos = { static_cast<float>(mEvent.GetMouseX()),static_cast<float>(mEvent.GetMouseY()) };
				_CamreaControll->GetCurentCam()->AgustRot(
					XMFLOAT3(
						mousePos.y * 0.005f,
						mousePos.x * 0.005f,
						0.0f
					));
		
			}

		}

		if (_MouseInput.IsRightDown())
		{
			


		}
		
	}
}

void InputControllor::AddCam(CameraController* cam)
{

	_CamreaControll = cam;

}


