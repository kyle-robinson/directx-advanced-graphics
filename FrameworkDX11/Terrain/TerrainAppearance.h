#pragma once
#ifndef TERRAINAPPEARENCE_H
#define TERRAINAPPEARENCE_H

#include "Appearance.h"

struct TerrainVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
    XMFLOAT2 BoundsY;
};

class TerrainAppearance : public Appearance
{
public:
    TerrainAppearance();
    TerrainAppearance( UINT width, UINT length, float cellSpacing, std::vector<float> heightMap );
    ~TerrainAppearance();

    void InitMesh_Terrain( ID3D11Device* pDevice );
    void InitMesh_Cube(
        bool lXNegative, bool lXPositive,
        bool lYNegative, bool lYPositive,
        bool lZNegative, bool lZPositive,
        ID3D11Device* pDevice, ID3D11DeviceContext* pContext );

    inline std::vector<ID3D11ShaderResourceView*> GetTextureResourceView() const noexcept { return m_vGroundTextureRV; }
    void SetTextures( std::vector<std::string> texName, ID3D11Device* pDevice );
    void SetTexture( int index, ID3D11ShaderResourceView* texture );

    inline void SetHeightData( std::vector<float> heightMap ) noexcept { m_vHeightMapData = heightMap; }

    float GetWidth();
    void SetWidth( float width );

    float GetDepth();
    void SetDepth( float depth );

    inline float GetCellSpacing() const noexcept { return m_fCellSpacing; }
    inline void SetCellSpacing( float cellSpace ) noexcept { m_fCellSpacing = cellSpace; }

    void CalcAllPatchBoundsY();
    void DrawTerrain( ID3D11DeviceContext* pContext );

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

    VertexBuffer<TerrainVertex> m_terrainVB;
    IndexBuffer m_terrainIB;

    std::vector<float> m_vHeightMapData;
    std::vector<XMFLOAT2> m_vPatchBoundsY;
    std::vector<ID3D11ShaderResourceView*> m_vGroundTextureRV;
};

#endif