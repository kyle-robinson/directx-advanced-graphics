#include "MouseEvent.h"

MouseEvent::MouseEvent():type(Invalid),x(0),y(0)
{
}

MouseEvent::MouseEvent(const MouseEventType type, const int x, const int y): type(type), x(x), y(y)
{

}

bool MouseEvent::IsValid()
{
    return type!=Invalid;
}

MouseEventType MouseEvent::GetType()
{
    return type;
}

MousePoint MouseEvent::GetPos()
{
    return{ x,y };
}

int MouseEvent::GetMouseX()
{
    return x;
}

int MouseEvent::GetMouseY()
{
    return y;
}
