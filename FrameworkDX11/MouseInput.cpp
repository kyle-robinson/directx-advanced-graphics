#include "MouseInput.h"

void MouseInput::OnLeftPress(int x, int y)
{

    _IsLeftDown = true;
    
    _MouseEventBuffer.push(MouseEvent(LPress, x, y));

}

void MouseInput::OnLeftReleased(int x, int y)
{
    _IsLeftDown = false;
    _MouseEventBuffer.push(MouseEvent(LRelease, x, y));
}

void MouseInput::OnRightPress(int x, int y)
{
    _IsRightDown = true;
    _MouseEventBuffer.push(MouseEvent(RPress, x, y));
}

void MouseInput::OnRightReleased(int x, int y)
{
    _IsRightDown = false;
    _MouseEventBuffer.push(MouseEvent(RRelease, x, y));
}

void MouseInput::OnMiddlePress(int x, int y)
{
    _IsMiddleDown = true;
    _MouseEventBuffer.push(MouseEvent(MPress, x, y));
}

void MouseInput::OnMiddleReleased(int x, int y)
{
    _IsMiddleDown = false;
    _MouseEventBuffer.push(MouseEvent(MRelease, x, y));
}

void MouseInput::OnWheelUp(int x, int y)
{
    _MouseEventBuffer.push(MouseEvent(WheelUp, x, y));
}

void MouseInput::OnWheelDown(int x, int y)
{
    _MouseEventBuffer.push(MouseEvent(WheelDown, x, y));
}

void MouseInput::OnMouseMove(int x, int y)
{
    _MousePosX = x;
    _MousePosY = y;

    _MouseEventBuffer.push(MouseEvent(Move, x, y));

}

void MouseInput::OnMouseMoveRaw(int x, int y)
{
    _MouseEventBuffer.push(MouseEvent(RawMove, x, y));
}

bool MouseInput::IsLeftDown()
{
    return _IsLeftDown;
}

bool MouseInput::IsRightDown()
{
    return _IsRightDown;
}

bool MouseInput::IsMiddleDown()
{
    return _IsMiddleDown;
}

int MouseInput::GetMousePosX()
{
    return _MousePosX;
}

int MouseInput::GetMousePosY()
{
    return _MousePosY;
}

MousePoint MouseInput::GetMousePos()
{
    return { _MousePosX,_MousePosY };
}

bool MouseInput::EventBufferIsEmpty()
{
    return _MouseEventBuffer.empty();
}

MouseEvent MouseInput::ReadEvent()
{
    if (_MouseEventBuffer.empty()) {
        return MouseEvent();
    }
    else
    {
        MouseEvent Event = _MouseEventBuffer.front();
        _MouseEventBuffer.pop();
        return Event;
    }

}
