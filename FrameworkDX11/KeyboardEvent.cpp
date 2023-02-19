#include "KeyboardEvent.h"

KeyboardEvent::KeyboardEvent():type(KeyEvent::Invalid),key(0u)
{
	

}

KeyboardEvent::KeyboardEvent(const KeyEvent type, const unsigned char key): type(type),key(key)
{
}

bool KeyboardEvent::isPressed() const
{
	return type==KeyEvent::press;
}

bool KeyboardEvent::isRelaced() const
{
	return type == KeyEvent::release;;
}

bool KeyboardEvent::isValid() const
{
	return type != KeyEvent::Invalid;;
}

unsigned char KeyboardEvent::GetKeyCode() const
{
	return key;
}
