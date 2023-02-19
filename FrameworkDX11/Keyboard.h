#pragma once
#include"KeyboardEvent.h"
#include<queue>


using namespace std;
/// <summary>
/// class to check the state of the keyborad
/// </summary>
class Keyboard
{
public:
	Keyboard();
	bool KeyIsPress(const unsigned char key);
	bool KeyBufferIsEmpty();
	bool CharBufferIsEmpty();

	KeyboardEvent ReadKey();
	unsigned char Raedchar();

	void OnKeyPress(const unsigned char key);
	void OnKeyRelace(const unsigned char key);
	void OnChar(const unsigned char key);


	void EnableAutoRepeatKeys();
	void DisableAutoRepeatKeys();

	void EnableAutoRepeatChars();
	void DisableAutoRepeatChars();

	bool IsCharAutoRepat();
	bool IsKeysAutoRepat();
	//vars
public:

private:
	bool autoRepeatKeys = false;
	bool  autoRepeatChars = false;
	bool keystates[256];

	queue<KeyboardEvent> keyBuffer;
	queue<unsigned char>charBuff;
};

