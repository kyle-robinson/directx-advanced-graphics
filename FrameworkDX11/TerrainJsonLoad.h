#pragma once
#ifndef TERRAINJSONLOAD_H
#define TERRAINJSONLOAD_H

#include "../rapidjson/include/rapidjson/rapidjson.h"
#include <../rapidjson/include/rapidjson/writer.h>
#include <../rapidjson/include/rapidjson/document.h>
#include <../rapidjson/include/rapidjson/istreamwrapper.h>
#include <../rapidjson/include/rapidjson/ostreamwrapper.h>
#include <fstream>

struct HeightMapSettings
{
    std::string HeightMapFile;
    float HeightScale;
};

struct FaultLineSettings
{
    int Seed;
    int IterationCount;
    float Displacement;
};

struct DiamondSquareSettings
{
    int Seed;
    int Range;
    float HeightScale;
};

struct TerrainNoiseSettings
{
    int Seed;
    int NumOfOctaves;
    float Frequency;
    float HeightScale;
};

struct TerrainData
{
    int Width;
    int Depth;
    int Mode;
    float CellSpacing;
    HeightMapSettings HeightMapSettings;
    FaultLineSettings FaultLineSettings;
    DiamondSquareSettings DiamondSquareSettings;
    TerrainNoiseSettings NoiseSettings;
};

template <typename DataFormat>
bool CheckDataIsThere( const std::string& objectName, const DataFormat& document )
{
    return document.HasMember( objectName.c_str() );;
}

inline bool StoreFile( const std::string& fileName, const rapidjson::Document& document )
{
    // Write back to file
    std::ofstream fileStream( "Resources/JSON/" + fileName + ".json" );
    rapidjson::OStreamWrapper wrapperStream( fileStream );
    rapidjson::Writer<rapidjson::OStreamWrapper> writer( wrapperStream );
    document.Accept( writer );
    fileStream.close();
    return true;
}

class TerrainJsonLoad
{
public:
    TerrainJsonLoad();
    ~TerrainJsonLoad();

    static void LoadData( std::string fileName, TerrainData& output );
    static void StoreData( std::string fileName, TerrainData dataToStore );

private:
    static HeightMapSettings LoadHeightMapSettings( rapidjson::Document& Doc );
    static void StoreHeightMapSettings( rapidjson::Document& Doc, const HeightMapSettings heightMapData );

    static FaultLineSettings LoadFaultLineSettings( rapidjson::Document& Doc );
    static void StoreFaultLineSettings( rapidjson::Document& Doc, const FaultLineSettings faultLineData );

    static DiamondSquareSettings LoadDiamondSquareSettings( rapidjson::Document& Doc );
    static void StoreDiamondSquareSettings( rapidjson::Document& Doc, const DiamondSquareSettings diamondSquareData );

    static TerrainNoiseSettings LoadTerrainNoiseSettings( rapidjson::Document& Doc );
    static void StoreTerrainNoiseSettings( rapidjson::Document& Doc, const TerrainNoiseSettings terrainNoiseData );
};

#endif