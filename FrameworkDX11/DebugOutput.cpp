#include "DebugOutput.h"

DebugOutput::DebugOutput()
{
}

DebugOutput::~DebugOutput()
{
}

void DebugOutput::OutputString(string Text)
{
	sprintf_s(sz, "Text: %s \n", Text.c_str());
	OutputDebugStringA(sz);
}

void DebugOutput::OutputInt(int Number)
{
	sprintf_s(sz, "Number: %i \n", Number);
	OutputDebugStringA(sz);
}

void DebugOutput::OutputFloat(float Float)
{
	sprintf_s(sz, "Number:  %f \n", Float);
	OutputDebugStringA(sz);
}
