#pragma once
//https://github.com/OneLoneCoder/videos/blob/master/OneLoneCoder_PerlinNoise.cpp

#include<random>

class PerlinNoise2D
{
public:

	PerlinNoise2D() {

	}
	~PerlinNoise2D() {

	}


	void MakePerlinNoise2D(int nWidth, int nHeight, int nOctaves, float fBias)
	{
		nOutputWidth = nWidth;
		nOutputHeight = nHeight;

		fNoiseSeed2D = new float[nOutputWidth * nOutputHeight];
		fPerlinNoise2D = new float[nOutputWidth * nOutputHeight];

		//fill values
		for (int i = 0; i < nOutputWidth * nOutputHeight; i++) fNoiseSeed2D[i] = (float)rand() / (float)RAND_MAX;

		
		for (int x = 0; x < nWidth; x++)
			for (int y = 0; y < nHeight; y++)
			{
				float fNoise = 0.0f;
				float fScaleAcc = 0.0f;
				float fScale = 1.0f;

				for (int o = 0; o < nOctaves; o++)
				{
					int nPitch = nWidth >> o;
					int nSampleX1 = (x / nPitch) * nPitch;
					int nSampleY1 = (y / nPitch) * nPitch;

					int nSampleX2 = (nSampleX1 + nPitch) % nWidth;
					int nSampleY2 = (nSampleY1 + nPitch) % nWidth;

					float fBlendX = (float)(x - nSampleX1) / (float)nPitch;
					float fBlendY = (float)(y - nSampleY1) / (float)nPitch;

					float fSampleT = (1.0f - fBlendX) * fNoiseSeed2D[nSampleY1 * nWidth + nSampleX1] + fBlendX * fNoiseSeed2D[nSampleY1 * nWidth + nSampleX2];
					float fSampleB = (1.0f - fBlendX) * fNoiseSeed2D[nSampleY2 * nWidth + nSampleX1] + fBlendX * fNoiseSeed2D[nSampleY2 * nWidth + nSampleX2];

					fScaleAcc += fScale;
					fNoise += (fBlendY * (fSampleB - fSampleT) + fSampleT) * fScale;
					fScale = fScale / fBias;
				}

				// Scale to seed range
				fPerlinNoise2D[y * nWidth + x] = fNoise / fScaleAcc;
			}

	}


	float Noise(int x, int y) {
		return fPerlinNoise2D[y * nOutputWidth + x];
	}

	void CleanUp() {
		delete[] fNoiseSeed2D;
		delete[] fPerlinNoise2D;
	}
private:


	// 2D noise variables
	int nOutputWidth = 256;
	int nOutputHeight = 256;
	float* fNoiseSeed2D = nullptr;
	float* fPerlinNoise2D = nullptr;




};

