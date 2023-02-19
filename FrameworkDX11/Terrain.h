#pragma once
#include"C++HelperFunctions.h"
#include"TerrainAppearence.h"
#include"Transform.h"
#include"ShaderController.h"
#include"structures.h"
#include"CameraController.h"

enum class TerrainGenType
{
	HightMapLoad=0,
	FaultLine,
	Noise,
	DiamondSquare
};

/// <summary>
/// Cotroller of plane created terrain
/// </summary>
class Terrain
{
public:
	Terrain(std::string HightMapName,XMFLOAT2 size,double Scale, TerrainGenType GenType, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, ShaderController* ShaderControll);
	~Terrain();

	void Update();
	void Draw(ID3D11DeviceContext* pContext,ShaderController* ShaderControll, ConstantBuffer* buffer, ID3D11Buffer* _pConstantBuffer, CameraController* camControll);

	TerrainGenType GetGenType() { return _TerrainCreationType; }
	void CreateHightData();

	TerrainCB GetTerrainData() { return _TerrainCB; }
	void SetMaxTess(float maxTess);
	void SetMinTess(float minTess);
	void SetMaxTessDist(float maxTessDist);
	void SetMinTessDist(float minTessDist);

	bool InBounds(int i, int j);
	float Average(int i, int j);
	void Smooth();


	TerrainAppearence* GetApperance() { return _pApperance; }
	Transform* GetTransfrom() { return _pTransform; }
	void SetBlendMap(std::string name, ID3D11Device* pd3dDevice);

	std::string GetHightMapName() { return HightMapName; }
	int GetHightMapWidth() { return _HightMapWidth; }
	int GetHightMapHight() { return _HightMapHight; }

	void SetHightScale(float HightScale);
	int GetHightScale() { return _HightScale; }

	void SetCellSpacing(float cellSpaceing);
	float GetCellSpacing() { return _CellSpaceing; }
	
	float GetHightWorld(float x, float z);
	float GetHight(float x, float z);


	bool* GetIsDraw() { return &_IsDraw; }

	void ReBuildTerrain(XMFLOAT2 size, double Scale,float CellSpaceing, TerrainGenType GenType, ID3D11Device* pd3dDevice);

	void SetTexHights(float Hight1, float Hight2, float Hight3, float Hight4, float Hight5);
	void SetTex(vector<string> texGroundName, ID3D11Device* pd3dDevice);
	vector<string> GetTexNames() { return _TexGround; }


	void SetDimondSquaerData(int seed, int range) {
		_Seed = seed;
		RandomGen::random<int>(0, 255, 0);
		_Range = range;
		_RangeStore = range;
	}
	void SetNoiseData(int seed, float frequancy, int numberOfOctaves) {
		_Seed = seed;
		_Frequancy = frequancy;
		_NumberOfOctaves = numberOfOctaves;
	}
	
	void SetFualtLineData(int seed, int numberOfIteration, float Displacement) {
		_Seed = seed;
		RandomGen::random<int>(0, 255, 0);
		_NumberOfIterations = numberOfIteration;
		_Displacement = Displacement;
	}

	void SetHightMapLoadData() {

	}

	int GetSeed() { return _Seed; }
	int GetRange() { return _RangeStore; }
	int GetNumberOfOcatives() {
		return _NumberOfOctaves;
	}
	float GetFequancy() { return _Frequancy; }
	int GetNumberOfIterations() { return _NumberOfIterations; }
	float GetDisplacment() { return _Displacement; }
private:
	void BuildHightMap(ID3D11Device* pd3dDevice);

	void LoadHightMap();
	void FaultHightFromation();
	void HightFromNoise();


	void DiamondSquareHightMap();
	void Diamond(int sideLength);
	void Square(int sideLength);
	void average(int x, int y, int sideLength);

	void CleanUp();
private:
	
	bool _IsDraw = false;


	TerrainGenType _TerrainCreationType;
	//Hight map data
	std::string HightMapName;
	

	//genration Data;
	int _Seed;

	//DiamondSquare Data
	vector<vector<float>> _2DHightMap;
	int _Range = 196;
	int _RangeStore = 196;

	//fualt line
	int _NumberOfIterations=800;
	int _Displacement=0.5f;
	//noise
	float _Frequancy=0.1f;
	int _NumberOfOctaves=3;

	//Hight Data
	vector<float> _HightmapData;
	float _HightScale;
	int _HightMapWidth;
	int _HightMapHight;
	ID3D11ShaderResourceView* _HeightMapSRV;

	//Grid Data
	XMFLOAT2 _GridSize;

	Transform* _pTransform;
	TerrainAppearence* _pApperance;

	//Tex data
	float _CellSpaceing = 1.0f;
	ID3D11ShaderResourceView* _BlendMap;
	vector<string> _TexGround;
	//Shader Data
	TerrainCB _TerrainCB;
	ID3D11Buffer* _TerrainConstantBuffer=  nullptr;

};

