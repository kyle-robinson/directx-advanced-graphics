#pragma once
#ifndef TERRAIN_H
#define TERRAIN_H

#include "Transform.h"
#include "Structures.h"
#include "CameraController.h"
#include "ShaderController.h"
#include "TerrainAppearance.h"
#include "ConstantBuffer.h"
#include "RandomHelper.h"

/* REFERENCE: Frank Luna, Introduction to 3D Game Programming with DirectX 11, Chapter 19 Terrain Rendering */

enum class TerrainGenType
{
	HeightMapLoad = 0,
	FaultLine,
	Noise,
	DiamondSquare
};

class Terrain
{
public:
	Terrain( std::string heightMapName, XMFLOAT2 size, double scale, TerrainGenType genType,
		ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ShaderController* shaderControl );
	~Terrain();

	void Update();
	void Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl,
		ConstantBuffer<MatrixBuffer>& buffer, CameraController* camControl );

	inline TerrainGenType GetGenType() const noexcept { return m_eTerrainCreationType; }
	inline TerrainCB GetTerrainData() const noexcept { return m_terrainCB.data; }
	void CreateHeightData();

	void SetMaxTess( float maxTess );
	void SetMinTess( float minTess );
	void SetMaxTessDist( float maxTessDist );
	void SetMinTessDist( float minTessDist );

	bool InBounds( int i, int j );
	float Average( int i, int j );
	void Smooth();

	inline TerrainAppearance* GetAppearance() const noexcept { return m_pAppearance.get(); }
	inline Transform* GetTransform() const noexcept { return m_pTransform.get(); }
	void SetBlendMap( std::string name, ID3D11Device* pDevice );

	inline int GetHeightMapWidth() const noexcept { return m_iHeightMapWidth; }
	inline int GetHeightMapHeight() const noexcept { return m_iHeightMapHeight; }
	inline std::string GetHeightMapName() const noexcept { return m_sHeightMapName; }

	inline void SetHeightScale( float heightScale ) noexcept { m_fHeightScale = heightScale; }
	inline int GetHeightScale() const noexcept { return m_fHeightScale; }

	inline void SetCellSpacing( float cellSpacing ) noexcept { m_fCellSpacing = cellSpacing; }
	inline float GetCellSpacing() const noexcept { return m_fCellSpacing; }

	float GetHeightWorld( float x, float z );
	float GetHeight( float x, float z );

	inline bool* GetIsDraw() noexcept { return &m_bToDraw; }
	void ReBuildTerrain( XMFLOAT2 size, double scale, float cellSpacing, TerrainGenType genType, ID3D11Device* pDevice );

	void SetTexHeights( float height1, float height2, float height3, float height4, float height5 );
	inline std::vector<std::string> GetTexNames() const noexcept { return m_vTexGround; }
	void SetTextures( std::vector<std::string> texGroundName, ID3D11Device* pDevice );
	void SetTexture( int index, std::string texName, ID3D11ShaderResourceView* texture );

	inline void SetDiamondSquareData( int seed, int range )
	{
		m_iSeed = seed;
		m_iRange = range;
		m_iRangeStore = range;
		RandomHelper::Random<int>( 0, 255, 0 );
	}

	inline void SetNoiseData( int seed, float frequency, int numOfOctaves )
	{
		m_iSeed = seed;
		m_fFrequency = frequency;
		m_iNumOfOctaves = numOfOctaves;
	}

	inline void SetFaultLineData( int seed, int numOfIteration, float displacement )
	{
		m_iSeed = seed;
		m_fDisplacement = displacement;
		m_iNumOfIterations = numOfIteration;
		RandomHelper::Random<int>( 0, 255, 0 );
	}

	inline int GetSeed() const noexcept { return m_iSeed; }
	inline int GetRange() const noexcept { return m_iRangeStore; }
	inline int GetNumOfOctaves() const noexcept { return m_iNumOfOctaves; }
	inline int GetNumOfIterations() const noexcept { return m_iNumOfIterations; }
	inline float GetDisplacement() const noexcept { return m_fDisplacement; }
	inline float GetFrequency() const noexcept { return m_fFrequency; }

private:
	void BuildHeightMap( ID3D11Device* pDevice );
	void LoadHeightMap();
	void FaultHeightFormation();
	void HeightFromNoise();

	void DiamondSquareHeightMap();
	void Diamond( int sideLength );
	void Square( int sideLength );
	void Average( int x, int y, int sideLength );

	void CleanUp();

	int m_iSeed;
	bool m_bToDraw = false;
	std::string m_sHeightMapName;
	TerrainGenType m_eTerrainCreationType;

	// Diamond square data
	int m_iRange = 196;
	int m_iRangeStore = 196;
	std::vector<std::vector<float>> m_v2DHeightMap;

	// Fault line
	float m_fDisplacement = 0.5f;
	int m_iNumOfIterations = 800;

	// Noise
	int m_iNumOfOctaves = 3;
	float m_fFrequency = 0.1f;

	// Height map data
	float m_fHeightScale;
	int m_iHeightMapWidth;
	int m_iHeightMapHeight;
	std::vector<float> m_vHeightMapData;
	ID3D11ShaderResourceView* m_pHeightMapSRV;

	// Grid data
	XMFLOAT2 m_fGridSize;
	std::unique_ptr<Transform> m_pTransform;
	std::unique_ptr<TerrainAppearance> m_pAppearance;

	// Texture data
	float m_fCellSpacing = 1.0f;
	std::vector<std::string> m_vTexGround;
	ID3D11ShaderResourceView* m_pBlendMap;

	// Shader data
	ConstantBuffer<TerrainCB> m_terrainCB;
};

#endif