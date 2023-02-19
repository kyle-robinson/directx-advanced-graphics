#include "Keyboard.h"

Keyboard::Keyboard()
{
    for (int i = 0; i < 256; i++)
    {
        keystates[i] = false;
    }

}

bool Keyboard::KeyIsPress(const unsigned char key)
{
    return keystates[key];
}


bool Keyboard::KeyBufferIsEmpty()
{
    return keyBuffer.empty();
}

bool Keyboard::CharBufferIsEmpty()
{
    return charBuff.empty();
}

KeyboardEvent Keyboard::ReadKey()
{
    if (keyBuffer.empty()) {
        return KeyboardEvent();
    }
    else
    {
        KeyboardEvent event = keyBuffer.front();
        keyBuffer.pop();
        return event;
    }
    

    
}

unsigned char Keyboard::Raedchar()
{
    if (charBuff.empty()) {
        return 0u;
    }
    else
    {
        unsigned char event = charBuff.front();
        charBuff.pop();
        return event;
    }
    
}

void Keyboard::OnKeyPress(const unsigned char key)
{
    keystates[key] = true;
    keyBuffer.push(KeyboardEvent(KeyboardEvent::KeyEvent::press, key));
}

void Keyboard::OnKeyRelace(const unsigned char key)
{
    keystates[key] = false;
    keyBuffer.push(KeyboardEvent(KeyboardEvent::KeyEvent::release, key));
}

void Keyboard::OnChar(const unsigned char key)
{
    charBuff.push(key);
}

void Keyboard::EnableAutoRepeatKeys()
{
    autoRepeatKeys = true;
}

void Keyboard::DisableAutoRepeatKeys()
{
    autoRepeatKeys = false;
}

void Keyboard::EnableAutoRepeatChars()
{
    autoRepeatChars = true;
}

void Keyboard::DisableAutoRepeatChars()
{
    autoRepeatChars = true;
}

bool Keyboard::IsCharAutoRepat()
{
    return autoRepeatChars;
}

bool Keyboard::IsKeysAutoRepat()
{
    return autoRepeatKeys;
}
