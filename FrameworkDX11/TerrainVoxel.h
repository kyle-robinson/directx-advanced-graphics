#pragma once
#include"TerrainAppearence.h"
#include"Transform.h"
#include<vector>
#include<DirectXMath.h>
#include"ShaderController.h"
#include"CameraController.h"
#include<DirectXCollision.h>
// noise genrater userd : https://github.com/Auburn/FastNoiseLite
#include"fastNoiseLite\Cpp\FastNoiseLite.h"

/// <summary>
/// Bolck types
/// sets the textuer
/// coud be used for other propities
/// </summary>
enum class BlockType
{
    Air = 0,
    Water,
    Grass,
    Snow,
    Stone
};

/// <summary>
/// cube data cb
/// </summary>
struct VoxelCube
{
    int CubeType=0;
    XMFLOAT3 Pad1;
};
/// <summary>
/// cube data
/// </summary>
class Block
{
public:
    Block(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
    ~Block();


    void CreatCube(bool lXNegative, bool lXPositive, bool lYNegative, bool lYPositive, bool lZNegative, bool lZPositive, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
    void SetIsActive(bool isActive) { _IsActive = isActive; }
    bool GetIsActive() { return _IsActive; }



    Transform* GetTransForm() { return _CubeTransform; }
    TerrainAppearence* GetApparance() { return _CubeApparace; }

    void SetBlockType(BlockType Block);
    BlockType GetBlockType() {
        return _BlockType;
    }
    VoxelCube GetCubeData() { return _CubeData; }


private:
    void CleanUp();
    TerrainAppearence* _CubeApparace=nullptr;
    Transform* _CubeTransform=nullptr;

    bool _IsActive = false;
    BlockType _BlockType = BlockType::Air;
    VoxelCube _CubeData;
};


/// <summary>
/// chunck gneration
/// controll cubes in a area
/// </summary>
class Chunk
{
public:
	Chunk(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, XMFLOAT3 pos, XMFLOAT3 Size);
    Chunk(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, XMFLOAT3 pos, XMFLOAT3 Size,int Seed, float Frequancy, int Octave);
	~Chunk();
    void Draw(ID3D11DeviceContext* pContext, ShaderController* ShaderControll, ConstantBuffer* buffer, ID3D11Buffer* _pConstantBuffer, CameraController* camControll, ID3D11Buffer* VoxelCB);

    Transform* GetTrnasfor() { return _ChunkTransform; }

    int GetMaxHight() { return _MaxHight; }
    void SetSeed(int seed) { _Seed = seed; }
private:
    void GenrateTerrain(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);
    void CleanUp();

    std::vector< std::vector< std::vector<Block*>>> _AllCubeInChuck;
    std::vector<Block*> _CubeToDraw;
    XMFLOAT3 _Pos;
    int _XSize = 16;
    int _ZSize = 16;
    int _MaxHight = 0;

    Transform* _ChunkTransform;

    int _Seed= 1338;
    float _Frequancy=0.01;
    int _Octaves = 3;
};


/// <summary>
/// controlls all chunks
/// </summary>
class TerrainVoxel
{
public:
    TerrainVoxel(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext,ShaderController* shader, int NumberOfChunks_X, int NumberOfChunks_Z);
    ~TerrainVoxel();

    void Draw(ID3D11DeviceContext* pContext, ShaderController* ShaderControll, ConstantBuffer* buffer, ID3D11Buffer* _pConstantBuffer, CameraController* camControll);
    bool* GetIsDraw() { return &_IsDraw; }
    int GetNumberOfChunks() { return _NumberOfChunks; }

    void RebuildMap(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext,int Seed, int NumberOfChunks_X, int NumberOfChunks_Z,float Frequancy, int Octave);
private:
    void CleanUp();


    std::vector< std::vector<Chunk*>> _ChunkData;
    int _NumberOfChunks=0;
    int _NumberOfChunksX = 0;
    int _NumberOfChunksZ = 0;
    XMFLOAT3 _DefaultChunkSize;
    vector<ID3D11ShaderResourceView*> _pGroundTextureRV;
    bool _IsDraw=false;
    ID3D11Buffer* _CubeInfoCB = nullptr;

    int _Seed = 1338;
    float _Frequancy = 0.01;
    int _Octaves = 3;
};

