#include "TerrainJsonLoad.h"

TerrainJsonLoad::TerrainJsonLoad()
{
}

TerrainJsonLoad::~TerrainJsonLoad()
{
}

void TerrainJsonLoad::LoadData(std::string FileName, TerrainData& output)
{

	Document document;


	std::ifstream fileStream("JSON\\"+FileName + ".json");
	if (!fileStream.is_open())
	{
		document.SetNull();
	}
	else
	{
		IStreamWrapper streamWrapper(fileStream);
		document.ParseStream(streamWrapper);
		if (document.HasParseError())
			document.SetNull();
		fileStream.close();
	}

	TerrainData objectData;
	

	if (!document.IsNull())
	{
		// Load from file

		for (auto& m : document.GetObject())
		{
			
			if (m.value.HasMember("Width"))
			{
				output.Width = m.value.FindMember("Width")->value.GetInt();
			}
			if (m.value.HasMember("Depth"))
			{
				output.Depth = m.value.FindMember("Depth")->value.GetInt();
			}
			 if (m.value.HasMember("CellSpaceing"))
			{
				 output.CellSpaceing = m.value.FindMember("CellSpaceing")->value.GetFloat();
			}
			if (m.value.HasMember("Mode"))
			{
				output.mode= m.value.FindMember("Mode")->value.GetInt();
				
				switch (output.mode)
				{
				case 0:
					output._HightMapSettings = LoadHightMapSettings(document);
					break;
				case 1:
					output._FaultLineSettings= LoadFualtLineSettings(document);
					break;
				case 2:
					output._NoiseSettings= LoadTerrainNoiseSettings(document);
					break;
				case 3:
					output._DimondSquareSettings= LoadDimondSquareSettings(document);
					break;
				}
				return;
			}
		}
	}

	
}

void TerrainJsonLoad::StoreData(std::string FileName, TerrainData DataToStore)
{
	Document document;
	document.SetObject();
	Document document1(&document.GetAllocator());

	//creat object one
	document1.SetObject();
	document1.AddMember("Width", DataToStore.Width, document1.GetAllocator());
	document1.AddMember("Depth", DataToStore.Depth, document1.GetAllocator());
	document1.AddMember("CellSpaceing", DataToStore.CellSpaceing, document1.GetAllocator());
	document1.AddMember("Mode", DataToStore.mode, document1.GetAllocator());
	document.AddMember("Terrain_Data", document1, document.GetAllocator());
	Document document2(&document.GetAllocator());

	//create mode object
	switch (DataToStore.mode)
	{
	case 0:
		StoreHightMapSettings(document2, DataToStore._HightMapSettings);
		document.AddMember("Terrain_Data_HighMap", document2, document.GetAllocator());
		break;
	case 1:
		StoreFualtLineSettings(document2, DataToStore._FaultLineSettings);
		document.AddMember("Terrain_Data_FualtLine", document2, document.GetAllocator());
		break;
	case 2:
		StoreTerrainNoiseSettings(document2, DataToStore._NoiseSettings);
		document.AddMember("Terrain_Data_Noise", document2, document.GetAllocator());
		break;
	case 3:
		StoreDimondSquareSettings(document2, DataToStore._DimondSquareSettings);
		document.AddMember("Terrain_Data_DimondSquare", document2, document.GetAllocator());
		break;
	}
	
	
	StoreFile(FileName, document);

}

HightMapSettings TerrainJsonLoad::LoadHightMapSettings(Document& Doc)
{
	HightMapSettings Data;
	
	for (auto& Object : Doc["Terrain_Data_HighMap"].GetObject())
	{
		string ObjectName = Object.name.GetString();
		if (ObjectName =="HightMapFile")
		{
			Data.HightMapFile = Object.value.GetString();
		}
		else if (ObjectName =="Hight_Scale")
		{
			Data.HightScale = Object.value.GetFloat();
		}
	}

	return Data;
}

void TerrainJsonLoad::StoreHightMapSettings(Document& Doc, const HightMapSettings Data)
{
	
	Doc.SetObject();
	Value s;
	s.SetString(Data.HightMapFile.c_str(),Doc.GetAllocator() );
	Doc.AddMember("HightMapFile", s, Doc.GetAllocator());
	Doc.AddMember("Hight_Scale", Data.HightScale, Doc.GetAllocator());
	
}

FualtLineSettings TerrainJsonLoad::LoadFualtLineSettings(Document& Doc)
{
	FualtLineSettings Data;
	for (auto& Object : Doc["Terrain_Data_FualtLine"].GetObject())
	{
		string ObjectName = Object.name.GetString();
		if (ObjectName=="Seed")
		{
			Data.Seed = Object.value.GetInt();
		}
		else if (ObjectName=="Displacement")
		{
			Data.Displacement = Object.value.GetFloat();
		}
		else if (ObjectName=="Iteration_Count")
		{
			Data.iterationCount = Object.value.GetInt();
		}
	}

	return Data;
}

void TerrainJsonLoad::StoreFualtLineSettings(Document& Doc, const FualtLineSettings Data)
{
	Doc.SetObject();
	Doc.AddMember("Seed", Data.Seed, Doc.GetAllocator());
	Doc.AddMember("Displacement", Data.Displacement, Doc.GetAllocator());
	Doc.AddMember("Iteration_Count", Data.iterationCount, Doc.GetAllocator());

}

DimondSquareSettings TerrainJsonLoad::LoadDimondSquareSettings(Document& Doc)
{
	DimondSquareSettings Data;
	for (auto& Object : Doc["Terrain_Data_DimondSquare"].GetObject())
	{
		string ObjectName = Object.name.GetString();
		if (ObjectName=="Seed")
		{
			Data.Seed = Object.value.GetInt();
		}
		else if (ObjectName=="HightScale")
		{
			Data.HightScale = Object.value.GetFloat();
		}
		else if (ObjectName=="Range")
		{
			Data.range = Object.value.GetInt();
		}
	}
	return Data;
}

void TerrainJsonLoad::StoreDimondSquareSettings(Document& Doc, const DimondSquareSettings Data)
{

	Doc.SetObject();
	Doc.AddMember("Seed", Data.Seed, Doc.GetAllocator());
	Doc.AddMember("HightScale", Data.HightScale, Doc.GetAllocator());
	Doc.AddMember("Range", Data.range, Doc.GetAllocator());
	
}

TerrainNoiseSettings TerrainJsonLoad::LoadTerrainNoiseSettings(Document& Doc)
{
	TerrainNoiseSettings Data;
	for (auto& Object : Doc["Terrain_Data_Noise"].GetObject())
	{
		string ObjectName = Object.name.GetString();
		if (ObjectName=="Seed")
		{
			Data.Seed = Object.value.GetInt();
		}
		if (ObjectName == "HightScale")
		{
			Data.HightScale = Object.value.GetFloat();
		}
		if (ObjectName == "Number_Of_Octaves")
		{
			Data.NumberOfOctaves = Object.value.GetInt();
		}
		if (ObjectName == "Frequancy")
		{
			Data.Frequancy = Object.value.GetFloat();
		}
	}
	return Data;
}

void TerrainJsonLoad::StoreTerrainNoiseSettings(Document& Doc, const TerrainNoiseSettings Data)
{
	Doc.SetObject();
	Doc.AddMember("Seed", Data.Seed, Doc.GetAllocator());
	Doc.AddMember("HightScale", Data.HightScale, Doc.GetAllocator());
	Doc.AddMember("Frequancy", Data.Frequancy, Doc.GetAllocator());
	Doc.AddMember("Number_Of_Octaves", Data.NumberOfOctaves, Doc.GetAllocator());
}
