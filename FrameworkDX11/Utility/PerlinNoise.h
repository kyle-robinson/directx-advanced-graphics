#pragma once
#ifndef PERLINNOISE_H
#define PERLINNOISE_H

/* REFERENCE : https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_PerlinNoise.cpp */

#include <random>

class PerlinNoise2D
{
public:
	PerlinNoise2D();
	~PerlinNoise2D();

	void MakePerlinNoise2D( int nWidth, int nHeight, int nOctaves, float fBias );
	float Noise( int x, int y );
	void CleanUp();

private:
	// 2D noise variables
	int m_iOutputWidth = 256;
	int m_iOutputHeight = 256;
	float* m_fNoiseSeed2D = nullptr;
	float* m_fPerlinNoise2D = nullptr;
};

#endif