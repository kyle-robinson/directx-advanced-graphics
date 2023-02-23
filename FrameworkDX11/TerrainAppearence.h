#pragma once
#ifndef TERRAINAPPEARENCE_H
#define TERRAINAPPEARENCE_H

#include "Appearance.h"

struct TerrainVert
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
    XMFLOAT2 BoundsY;
};

class TerrainAppearence : public Appearance
{
public:
    TerrainAppearence();
    TerrainAppearence( UINT width, UINT length, float cellSpacing, std::vector<float> heightMap );
    ~TerrainAppearence();

    void InitMesh_Terrain( ID3D11Device* pd3dDevice );
    void InitMesh_Cube(
        bool lXNegative, bool lXPositive,
        bool lYNegative, bool lYPositive,
        bool lZNegative, bool lZPositive,
        ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

    inline std::vector<ID3D11ShaderResourceView*> GetTextureResourceView() const noexcept { return m_vGroundTextureRV; }
    void SetTexture( std::vector<std::string> texName, ID3D11Device* pDevice );

    inline void SetHeightData( std::vector<float> heightMap ) noexcept { m_vHeightMapData = heightMap; }

    float GetWidth();
    void SetWidth( float width );

    float GetDepth();
    void SetDepth( float depth );

    inline float GetCellSpacing() const noexcept { return m_fCellSpacing; }
    inline void SetCellSpacing( float cellSpace ) noexcept { m_fCellSpacing = cellSpace; }

    void CalcAllPatchBoundsY();
    void Draw( ID3D11DeviceContext* pContext ) override;

private:
    void BuildPatchVertex( ID3D11Device* pd3dDevice );
    void BuildPatchIndex( ID3D11Device* pd3dDevice );
    void CalcPatchBoundsY( UINT i, UINT j );

    int m_iCellsPerPatch = 64;
    int m_iNumPatchVertRows;
    int m_iNumPatchVertCols;
    int m_iNumPatchVertices;
    int m_iNumPatchQuadFaces;

    bool m_bToDraw = true;
    int m_fHeightMapWidth;
    int m_fHeightMapHeight;
    float m_fCellSpacing;

    std::vector<float> m_vHeightMapData;
    std::vector<XMFLOAT2> m_vPatchBoundsY;
    std::vector<ID3D11ShaderResourceView*> m_vGroundTextureRV;
};

#endif