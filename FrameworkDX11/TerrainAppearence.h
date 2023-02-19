#pragma once
#include "Appearance.h"
#include<vector>
#include<string>

//terrain vetex data
struct TerrainVert
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
    XMFLOAT2 BoundsY;
};

/// <summary>
/// terrain Appearance data
/// allows for grid creation for LOD plane
/// allows for a special cube to be created
/// </summary>
class TerrainAppearence :
    public Appearance
{
public:
    TerrainAppearence();
    TerrainAppearence(UINT Width, UINT Length,float CellSpacing, vector<float> HightMap);
    ~TerrainAppearence();

    HRESULT	InitMeshGround(ID3D11Device* pd3dDevice);
    void CreateCube(bool lXNegative, bool lXPositive, bool lYNegative, bool lYPositive, bool lZNegative, bool lZPositive,ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext);

    vector<ID3D11ShaderResourceView*> GetTextureResourceView() { return _pGroundTextureRV; }
    void SetTex(std::vector<std::string> texName, ID3D11Device* pd3dDevice);

    void setHightdata(vector<float> HightMap) { _HightmapData = HightMap; }

    float GetWidth();
    void SetWidth(float Width);

    float GetDepth();
    void SetDepth(float Depth);

    float GetCellSpaceing() { return _CellSpaceing; }
    void SetCellSpaceing(float CellSpace);


    void CalcAllPatchBoundsY();

    void Draw(ID3D11DeviceContext* pImmediateContext) override;
private:
    void BuildPatchVertex(ID3D11Device* pd3dDevice);
    void BuildPatchIndex(ID3D11Device* pd3dDevice); 
   
    void CalcPatchBoundsY(UINT i, UINT j);
private:
    vector<ID3D11ShaderResourceView*> _pGroundTextureRV;
    bool _ToDraw = true;


    int _CellsPerPatch = 64;
    int _NumPatchVertRows;
    int _NumPatchVertCols;
    int _NumPatchVertices;
    int _NumPatchQuadFaces;

    int _HightMapWidth;
    int _HightMapHight;
    float _CellSpaceing;

    std::vector<XMFLOAT2> _PatchBoundsY;
    vector<float> _HightmapData;
   
};

