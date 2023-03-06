#pragma once
#ifndef TERRAINVOXEL_H
#define TERRAINVOXEL_H

#include "Transform.h"
#include "ShaderController.h"
#include "CameraController.h"
#include "TerrainAppearance.h"
#include "ConstantBuffer.h"

// Noise generator : https://github.com/Auburn/FastNoiseLite
#include "fastNoiseLite\Cpp\FastNoiseLite.h"

enum class BlockType
{
    Air = 0,
    Water,
    Grass,
    Snow,
    Stone
};

struct VoxelCube
{
    int CubeType = 0;
    XMFLOAT3 Padding;
};

class Block
{
public:
    Block( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
    ~Block();

    void InitMesh_Cube(
        bool lXNegative, bool lXPositive,
        bool lYNegative, bool lYPositive,
        bool lZNegative, bool lZPositive,
        ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

    inline bool GetIsActive() const noexcept { return m_bIsActive; }
    inline void SetIsActive( bool isActive ) noexcept { m_bIsActive = isActive; }

    inline Transform* GetTransform() const noexcept { return m_pCubeTransform; }
    inline TerrainAppearance* GetAppearance() noexcept { return m_pCubeAppearance; }

    void SetBlockType( BlockType block );
    inline VoxelCube GetCubeData() const noexcept { return m_cubeData; }
    inline BlockType GetBlockType() const noexcept { return m_eBlockType; }

private:
    void CleanUp();
    Transform* m_pCubeTransform = nullptr;
    TerrainAppearance* m_pCubeAppearance = nullptr;

    VoxelCube m_cubeData;
    bool m_bIsActive = false;
    BlockType m_eBlockType = BlockType::Air;
};

class Chunk
{
public:
    Chunk( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, XMFLOAT3 pos, XMFLOAT3 size );
    Chunk( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, XMFLOAT3 pos, XMFLOAT3 size, int seed, float frequency, int octave );
    ~Chunk();
    void Draw(
        ID3D11DeviceContext* pContext, ShaderController* shaderControl,
        ConstantBuffer<MatrixBuffer>& cbuffer, ConstantBuffer<VoxelCube>& voxelBuffer, CameraController* camControl );

    inline Transform* GetTransform() const noexcept { return m_pChunkTransform; }
    inline int GetMaxHeight() const noexcept { return m_iMaxHeight; }
    inline void SetSeed( int seed ) noexcept { m_iSeed = seed; }

private:
    void GenerateTerrain( ID3D11Device* pDevice, ID3D11DeviceContext* pContext );
    void CleanUp();

    int m_iXSize = 16;
    int m_iZSize = 16;
    int m_iSeed = 1338;
    int m_iOctaves = 3;
    int m_iMaxHeight = 0;
    float m_fFrequency = 0.01f;

    XMFLOAT3 m_fPos;
    Transform* m_pChunkTransform;
    std::vector<Block*> m_vCubesToDraw;
    std::vector<std::vector< std::vector<Block*>>> m_vAllCubesInChunk;
};

class TerrainVoxel
{
public:
    TerrainVoxel( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ShaderController* shaderControl, int numOfChunks_X, int numOfChunks_Z );
    ~TerrainVoxel();

    void Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl, ConstantBuffer<MatrixBuffer>& buffer, CameraController* camControl );
    void RebuildMap( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, int seed, int numOfChunks_X, int numOfChunks_Z, float frequency, int octave );

    inline int GetSeed() const noexcept { return m_iSeed; }
    inline bool* GetIsDraw() noexcept { return &m_bToDraw; }
    inline int GetOctaves() const noexcept { return m_iOctaves; }
    inline float GetFrequency() const noexcept { return m_fFrequency; }
    inline int GetNumberOfChunks() const noexcept { return m_iNumOfChunks; }

private:
    void CleanUp();

    int m_iNumOfChunks = 0;
    int m_iNumOfChunksX = 0;
    int m_iNumOfChunksZ = 0;

    int m_iSeed = 1338;
    int m_iOctaves = 3;
    bool m_bToDraw = false;
    float m_fFrequency = 0.01f;

    XMFLOAT3 m_fDefaultChunkSize;
    ConstantBuffer<VoxelCube> m_cubeInfoCB;
    std::vector<std::vector<Chunk*>> m_vChunkData;
};

#endif