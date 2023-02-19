#pragma once
#include"../rapidjson/include/rapidjson/rapidjson.h"
#include <../rapidjson/include/rapidjson/writer.h>
#include <../rapidjson/include/rapidjson/document.h>
#include <../rapidjson/include/rapidjson/istreamwrapper.h>
#include <../rapidjson/include/rapidjson/ostreamwrapper.h>
#include<string>
#include<fstream>

//Data stucts to store data 
struct HightMapSettings
{
    std::string HightMapFile;
    float HightScale;
};

struct FualtLineSettings
{
    int Seed;
    int iterationCount;
    float Displacement;
};

struct DimondSquareSettings
{
    int Seed;
    int range;
    float HightScale;
};

struct TerrainNoiseSettings
{
    int Seed;
    int NumberOfOctaves;
    float Frequancy;
    float HightScale;
};
struct TerrainData
{
    int Width;
    int Depth;
    float CellSpaceing;
    int mode;
    HightMapSettings _HightMapSettings;
    FualtLineSettings _FaultLineSettings;
    DimondSquareSettings _DimondSquareSettings;
    TerrainNoiseSettings _NoiseSettings;
};


using namespace std;
using namespace rapidjson;


// Check node is there
template <typename DataFormat>
bool CheckDataIsThere(const std::string& objectName, const DataFormat& document)
{
    return document.HasMember(objectName.c_str());;
}
//store the file 
inline bool StoreFile(const std::string& fileName, const Document& document)
{
    // Write back to file
    std::ofstream fileStream("JSON\\" + fileName +".json");
    OStreamWrapper wrapperStream(fileStream);
    Writer<OStreamWrapper> writer(wrapperStream);
    document.Accept(writer);
    fileStream.close();
    return true;
}



/// <summary>
/// loade and save data through the use of JSON
/// </summary>
class TerrainJsonLoad
{
public:
    TerrainJsonLoad();
    ~TerrainJsonLoad();


    static void LoadData(std::string FileName, TerrainData& output);
    static void StoreData(std::string FileName, TerrainData DataToStore);

private:
    static HightMapSettings LoadHightMapSettings(Document& Doc);
    static void StoreHightMapSettings(Document& Doc, const HightMapSettings Data);

    static FualtLineSettings LoadFualtLineSettings(Document& Doc);
    static void StoreFualtLineSettings(Document& Doc,const FualtLineSettings Data);

    static DimondSquareSettings LoadDimondSquareSettings(Document& Doc);
    static void StoreDimondSquareSettings(Document& Doc, const DimondSquareSettings Data);

    static TerrainNoiseSettings LoadTerrainNoiseSettings(Document& Doc);
    static void StoreTerrainNoiseSettings(Document& Doc, const TerrainNoiseSettings Data);

};

