#include "PerlinNoise.h"

PerlinNoise2D::PerlinNoise2D() {}

PerlinNoise2D::~PerlinNoise2D() {}

void PerlinNoise2D::MakePerlinNoise2D( int nWidth, int nHeight, int nOctaves, float fBias )
{
	m_iOutputWidth = nWidth;
	m_iOutputHeight = nHeight;
	m_fNoiseSeed2D = new float[m_iOutputWidth * m_iOutputHeight];
	m_fPerlinNoise2D = new float[m_iOutputWidth * m_iOutputHeight];

	for ( int i = 0; i < m_iOutputWidth * m_iOutputHeight; i++ )
	{
		m_fNoiseSeed2D[i] = (float)rand() / (float)RAND_MAX;
	}

	for ( int x = 0; x < nWidth; x++ )
	{
		for ( int y = 0; y < nHeight; y++ )
		{
			float fNoise = 0.0f;
			float fScaleAcc = 0.0f;
			float fScale = 1.0f;

			for ( int o = 0; o < nOctaves; o++ )
			{
				int nPitch = nWidth >> o;
				int nSampleX1 = ( x / nPitch ) * nPitch;
				int nSampleY1 = ( y / nPitch ) * nPitch;

				int nSampleX2 = ( nSampleX1 + nPitch ) % nWidth;
				int nSampleY2 = ( nSampleY1 + nPitch ) % nWidth;

				float fBlendX = (float)( x - nSampleX1 ) / (float)nPitch;
				float fBlendY = (float)( y - nSampleY1 ) / (float)nPitch;

				float fSampleT = ( 1.0f - fBlendX ) * m_fNoiseSeed2D[nSampleY1 * nWidth + nSampleX1] + fBlendX * m_fNoiseSeed2D[nSampleY1 * nWidth + nSampleX2];
				float fSampleB = ( 1.0f - fBlendX ) * m_fNoiseSeed2D[nSampleY2 * nWidth + nSampleX1] + fBlendX * m_fNoiseSeed2D[nSampleY2 * nWidth + nSampleX2];

				fScaleAcc += fScale;
				fNoise += ( fBlendY * ( fSampleB - fSampleT ) + fSampleT ) * fScale;
				fScale = fScale / fBias;
			}

			// Scale according to the seed range
			m_fPerlinNoise2D[y * nWidth + x] = fNoise / fScaleAcc;
		}
	}
}

float PerlinNoise2D::Noise( int x, int y )
{
	return m_fPerlinNoise2D[y * m_iOutputWidth + x];
}

void PerlinNoise2D::CleanUp()
{
	delete[] m_fNoiseSeed2D;
	delete[] m_fPerlinNoise2D;
}