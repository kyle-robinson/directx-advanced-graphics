#pragma once
/// <summary>
/// class to hold the state of the keyborad
/// </summary>
class KeyboardEvent
{
	//func
public:
	enum KeyEvent
	{
		press,
		release,
		Invalid

	};
	KeyboardEvent();
	KeyboardEvent(const KeyEvent type, const unsigned char key );
	
	bool isPressed() const;
	bool isRelaced() const;
	bool isValid() const;
	unsigned char GetKeyCode() const;

private:

	//vars
public:

private:
	KeyEvent type;
	unsigned char key;

};

