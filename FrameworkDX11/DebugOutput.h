#pragma once

#include <iostream>
#include <string>
#include <Windows.h>

using namespace std;



class DebugOutput
{
public:
	DebugOutput();
	~DebugOutput();

	void OutputString(string Text);
	void OutputInt(int Number);
	void OutputFloat(float Float);

private:
	char sz[1024] = { 0 };




};

