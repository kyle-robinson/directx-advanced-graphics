#include "TerrainJsonLoad.h"

TerrainJsonLoad::TerrainJsonLoad() {}

TerrainJsonLoad::~TerrainJsonLoad() {}

void TerrainJsonLoad::LoadData( std::string fileName, TerrainData& output )
{
	rapidjson::Document document;
	std::ifstream fileStream( "Resources/JSON/" + fileName + ".json" );
	if ( !fileStream.is_open() )
	{
		document.SetNull();
	}
	else
	{
		rapidjson::IStreamWrapper streamWrapper( fileStream );
		document.ParseStream( streamWrapper );
		if ( document.HasParseError() )
			document.SetNull();
		fileStream.close();
	}

	TerrainData objectData;
	if ( !document.IsNull() )
	{
		// Load data from file
		for ( auto& m : document.GetObject() )
		{
			if ( m.value.HasMember( "Width" ) )
			{
				output.Width = m.value.FindMember( "Width" )->value.GetInt();
			}
			if ( m.value.HasMember( "Depth" ) )
			{
				output.Depth = m.value.FindMember( "Depth" )->value.GetInt();
			}
			if ( m.value.HasMember( "CellSpacing" ) )
			{
				output.CellSpacing = m.value.FindMember( "CellSpacing" )->value.GetFloat();
			}
			if ( m.value.HasMember( "Mode" ) )
			{
				output.Mode = m.value.FindMember( "Mode" )->value.GetInt();
				switch ( output.Mode )
				{
				case 0:
					output.HeightMapSettings = LoadHeightMapSettings( document );
					break;
				case 1:
					output.FaultLineSettings = LoadFaultLineSettings( document );
					break;
				case 2:
					output.NoiseSettings = LoadTerrainNoiseSettings( document );
					break;
				case 3:
					output.DiamondSquareSettings = LoadDiamondSquareSettings( document );
					break;
				}
				return;
			}
		}
	}
}

void TerrainJsonLoad::StoreData( std::string fileName, TerrainData dataToStore )
{
	rapidjson::Document document;
	document.SetObject();
	rapidjson::Document document1( &document.GetAllocator() );

	// Create first object
	document1.SetObject();
	document1.AddMember( "Width", dataToStore.Width, document1.GetAllocator() );
	document1.AddMember( "Depth", dataToStore.Depth, document1.GetAllocator() );
	document1.AddMember( "CellSpacing", dataToStore.CellSpacing, document1.GetAllocator() );
	document1.AddMember( "Mode", dataToStore.Mode, document1.GetAllocator() );
	document.AddMember( "Terrain_Data", document1, document.GetAllocator() );
	rapidjson::Document document2( &document.GetAllocator() );

	// Create mode object
	switch ( dataToStore.Mode )
	{
	case 0:
		StoreHeightMapSettings( document2, dataToStore.HeightMapSettings );
		document.AddMember( "Terrain_Data_HeightMap", document2, document.GetAllocator() );
		break;
	case 1:
		StoreFaultLineSettings( document2, dataToStore.FaultLineSettings );
		document.AddMember( "Terrain_Data_FaultLine", document2, document.GetAllocator() );
		break;
	case 2:
		StoreTerrainNoiseSettings( document2, dataToStore.NoiseSettings );
		document.AddMember( "Terrain_Data_Noise", document2, document.GetAllocator() );
		break;
	case 3:
		StoreDiamondSquareSettings( document2, dataToStore.DiamondSquareSettings );
		document.AddMember( "Terrain_Data_DiamondSquare", document2, document.GetAllocator() );
		break;
	}

	StoreFile( fileName, document );
}

HeightMapSettings TerrainJsonLoad::LoadHeightMapSettings( rapidjson::Document& doc )
{
	HeightMapSettings heightMapData;
	for ( auto& object : doc["Terrain_Data_HeightMap"].GetObject() )
	{
		std::string objectName = object.name.GetString();
		if ( objectName == "HeightMap_File" )
		{
			heightMapData.HeightMapFile = object.value.GetString();
		}
		else if ( objectName == "HeightMap_Scale" )
		{
			heightMapData.HeightScale = object.value.GetFloat();
		}
	}

	return heightMapData;
}

void TerrainJsonLoad::StoreHeightMapSettings( rapidjson::Document& doc, const HeightMapSettings heightMapData )
{
	rapidjson::Value s;
	doc.SetObject();
	s.SetString( heightMapData.HeightMapFile.c_str(), doc.GetAllocator() );
	doc.AddMember( "HeightMap_File", s, doc.GetAllocator() );
	doc.AddMember( "HeightMap_Scale", heightMapData.HeightScale, doc.GetAllocator() );
}

FaultLineSettings TerrainJsonLoad::LoadFaultLineSettings( rapidjson::Document& doc )
{
	FaultLineSettings faultLineData;
	for ( auto& object : doc["Terrain_Data_FaultLine"].GetObject() )
	{
		std::string objectName = object.name.GetString();
		if ( objectName == "Seed" )
		{
			faultLineData.Seed = object.value.GetInt();
		}
		else if ( objectName == "Displacement" )
		{
			faultLineData.Displacement = object.value.GetFloat();
		}
		else if ( objectName == "Iteration_Count" )
		{
			faultLineData.IterationCount = object.value.GetInt();
		}
	}

	return faultLineData;
}

void TerrainJsonLoad::StoreFaultLineSettings( rapidjson::Document& doc, const FaultLineSettings faultLineData )
{
	doc.SetObject();
	doc.AddMember( "Seed", faultLineData.Seed, doc.GetAllocator() );
	doc.AddMember( "Displacement", faultLineData.Displacement, doc.GetAllocator() );
	doc.AddMember( "Iteration_Count", faultLineData.IterationCount, doc.GetAllocator() );
}

DiamondSquareSettings TerrainJsonLoad::LoadDiamondSquareSettings( rapidjson::Document& doc )
{
	DiamondSquareSettings diamondSquareData;
	for ( auto& object : doc["Terrain_Data_DiamondSquare"].GetObject() )
	{
		std::string objectName = object.name.GetString();
		if ( objectName == "Seed" )
		{
			diamondSquareData.Seed = object.value.GetInt();
		}
		else if ( objectName == "HeightMap_Scale" )
		{
			diamondSquareData.HeightScale = object.value.GetFloat();
		}
		else if ( objectName == "Range" )
		{
			diamondSquareData.Range = object.value.GetInt();
		}
	}
	return diamondSquareData;
}

void TerrainJsonLoad::StoreDiamondSquareSettings( rapidjson::Document& doc, const DiamondSquareSettings diamondSquareData )
{
	doc.SetObject();
	doc.AddMember( "Seed", diamondSquareData.Seed, doc.GetAllocator() );
	doc.AddMember( "HeightMap_Scale", diamondSquareData.HeightScale, doc.GetAllocator() );
	doc.AddMember( "Range", diamondSquareData.Range, doc.GetAllocator() );
}

TerrainNoiseSettings TerrainJsonLoad::LoadTerrainNoiseSettings( rapidjson::Document& doc )
{
	TerrainNoiseSettings terrainNoiseData;
	for ( auto& object : doc["Terrain_Data_Noise"].GetObject() )
	{
		std::string objectName = object.name.GetString();
		if ( objectName == "Seed" )
		{
			terrainNoiseData.Seed = object.value.GetInt();
		}
		if ( objectName == "HeightMap_Scale" )
		{
			terrainNoiseData.HeightScale = object.value.GetFloat();
		}
		if ( objectName == "Number_Of_Octaves" )
		{
			terrainNoiseData.NumOfOctaves = object.value.GetInt();
		}
		if ( objectName == "Frequency" )
		{
			terrainNoiseData.Frequency = object.value.GetFloat();
		}
	}

	return terrainNoiseData;
}

void TerrainJsonLoad::StoreTerrainNoiseSettings( rapidjson::Document& doc, const TerrainNoiseSettings terrainNoiseData )
{
	doc.SetObject();
	doc.AddMember( "Seed", terrainNoiseData.Seed, doc.GetAllocator() );
	doc.AddMember( "HightScale", terrainNoiseData.HeightScale, doc.GetAllocator() );
	doc.AddMember( "Frequancy", terrainNoiseData.Frequency, doc.GetAllocator() );
	doc.AddMember( "Number_Of_Octaves", terrainNoiseData.NumOfOctaves, doc.GetAllocator() );
}