#include "TerrainAppearence.h"
#include"DDSTextureLoader.h"
TerrainAppearence::TerrainAppearence()
{
}

TerrainAppearence::TerrainAppearence(UINT Width, UINT Length, float CellSpacing, vector<float> HightMap):
	_HightMapWidth(Width),
	_HightMapHight(Length),
	_CellSpaceing(CellSpacing),
	_HightmapData(HightMap)
{
	// Divide heightmap into patches such that each patch has CellsPerPatch.
	_NumPatchVertRows = ((_HightMapHight - 1) / _CellsPerPatch) + 1;
	_NumPatchVertCols = ((_HightMapWidth - 1) / _CellsPerPatch) + 1;

	_NumPatchVertices = _NumPatchVertRows * _NumPatchVertCols;
	_NumPatchQuadFaces = (_NumPatchVertRows - 1) * (_NumPatchVertCols - 1);

}

TerrainAppearence::~TerrainAppearence()
{
}

HRESULT TerrainAppearence::InitMeshGround(ID3D11Device* pd3dDevice)
{
	HRESULT hr= S_OK;
	
	BuildPatchVertex(pd3dDevice);
	BuildPatchIndex(pd3dDevice);


	m_material.Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_material.Material.Specular = XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f);
	m_material.Material.SpecularPower = 32.0f;
	m_material.Material.UseTexture = true;
	m_material.Material.Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_material.Material.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_material.Material.HightScale = 0.1f;
	m_material.Material.MaxLayers = 15.0f;
	m_material.Material.MinLayers = 10.0f;

    return hr;
}

void TerrainAppearence::CreateCube(bool lXNegative, bool lXPositive, bool lYNegative, bool lYPositive, bool lZNegative, bool lZPositive, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{

	if (lXNegative && lXPositive && lYNegative && lYPositive && lZNegative && lZPositive) {
		_ToDraw = false;
		return;
	}

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		//top
		
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },//3
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//1
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },//0

		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },//2
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//1
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },//3
		
		//bot
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },//6
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//4
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },//5

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },//7
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//4
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },//6

		//left

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//11
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },//9
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },//8


		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },//10
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },//9
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//11

		//right
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//14
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },//12
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },//13

		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },//15
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },//12
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },//14

		//front
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },//19
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) , XMFLOAT2(1.0f, 1.0f) },//17
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },//16

		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },//18
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f) , XMFLOAT2(1.0f, 1.0f) },//17
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },//19

		//back
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },//22
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },//20
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },//21

		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },//23
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },//20
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },//22

	};

	//cacluate binormal and tangent
	//CalculateModelVectors(vertices, NUM_VERTICES);

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * NUM_VERTICES;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	HRESULT hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	//if (FAILED(hr))
		//return hr;



	// Create index buffer
	std::vector<WORD> ind;

	if (!lYPositive) {
		ind.push_back(0);
		ind.push_back(1);
		ind.push_back(2);

		ind.push_back(3);
		ind.push_back(4);
		ind.push_back(5);
	}
	if (!lYNegative) {
		ind.push_back(6);
		ind.push_back(7);
		ind.push_back(8);

		ind.push_back(9);
		ind.push_back(10);
		ind.push_back(11);
	}
	if (!lXNegative) {
		ind.push_back(12);
		ind.push_back(13);
		ind.push_back(14);

		ind.push_back(15);
		ind.push_back(16);
		ind.push_back(17);
	}
	if (!lXPositive) {
		ind.push_back(18);
		ind.push_back(19);
		ind.push_back(20);

		ind.push_back(21);
		ind.push_back(22);
		ind.push_back(23);
	}
	if (!lZNegative) {
		ind.push_back(24);
		ind.push_back(25);
		ind.push_back(26);

		ind.push_back(27);
		ind.push_back(28);
		ind.push_back(29);
	}
	if (!lZPositive) {
		ind.push_back(30);
		ind.push_back(31);
		ind.push_back(32);

		ind.push_back(33);
		ind.push_back(34);
		ind.push_back(35);
	}

	if (ind.size() > 0) {
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(WORD) * ind.size();        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = &ind[0];
		hr = pd3dDevice->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
		//if (FAILED(hr))
			//return hr;

		// Set index buffer
		pContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	}
	else {
		_ToDraw = false;
	}
	// Set primitive topology
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	NumberOfVert = ind.size();
}

void TerrainAppearence::SetTex(std::vector<std::string> texName, ID3D11Device* pd3dDevice)
{
	ID3D11ShaderResourceView* res;
	
	for (auto texName: texName)
	{
		wstring wide_string = wstring(texName.begin(), texName.end());
		const wchar_t* result = wide_string.c_str();
		CreateDDSTextureFromFile(pd3dDevice, result, nullptr, &res);
		_pGroundTextureRV.push_back(res);
	}
	
}

float TerrainAppearence::GetWidth()
{
	// Total terrain width.
	return (_HightMapWidth - 1) * _CellSpaceing;
}

void TerrainAppearence::SetWidth(float Width)
{
	_HightMapWidth = Width;
	// Divide heightmap into patches such that each patch has CellsPerPatch.
	_NumPatchVertCols = ((_HightMapWidth - 1) / _CellsPerPatch) + 1;
	

	_NumPatchVertices = _NumPatchVertRows * _NumPatchVertCols;
	_NumPatchQuadFaces = (_NumPatchVertRows - 1) * (_NumPatchVertCols - 1);



}

float TerrainAppearence::GetDepth()
{
		// Total terrain depth.
		return (_HightMapHight - 1) * _CellSpaceing;
}

void TerrainAppearence::SetDepth(float Depth)
{
	_HightMapHight = Depth;
	// Divide heightmap into patches such that each patch has CellsPerPatch.
	_NumPatchVertRows = ((_HightMapHight - 1) / _CellsPerPatch) + 1;
	

	_NumPatchVertices = _NumPatchVertRows * _NumPatchVertCols;
	_NumPatchQuadFaces = (_NumPatchVertRows - 1) * (_NumPatchVertCols - 1);
}

void TerrainAppearence::BuildPatchVertex(ID3D11Device* pd3dDevice)
{
	HRESULT hr;
	//build the grid
	std::vector<TerrainVert> patchVertices(_NumPatchVertRows * _NumPatchVertCols);
	float halfWidth = 0.5f * GetWidth();
	float halfDepth = 0.5f * GetDepth();
	float patchWidth = GetWidth() / (_NumPatchVertCols - 1);
	float patchDepth = GetDepth() / (_NumPatchVertRows - 1);
	float du = 1.0f / (_NumPatchVertCols - 1);
	float dv = 1.0f / (_NumPatchVertRows - 1);
	for (UINT i = 0; i < _NumPatchVertRows; ++i)
	{
		float z = halfDepth - i * patchDepth;
		for (UINT j = 0; j < _NumPatchVertCols; ++j)
		{
			float x = -halfWidth + j * patchWidth;
			patchVertices[i * _NumPatchVertCols + j].Pos = XMFLOAT3(x, 0.0f, z);
			// Stretch texture over grid.
			patchVertices[i * _NumPatchVertCols + j].Tex.x = j * du;
			patchVertices[i * _NumPatchVertCols + j].Tex.y = i * dv;
		}
	}

	// Store axis-aligned bounding box y-bounds in upper-left patch corner.
	for (UINT i = 0; i < _NumPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < _NumPatchVertCols - 1; ++j)
		{
			UINT patchID = i * (_NumPatchVertCols - 1) + j;
			patchVertices[i * _NumPatchVertCols + j].BoundsY = _PatchBoundsY[patchID];
		}
	}
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(TerrainVert) * patchVertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &patchVertices[0];
	hr= pd3dDevice->CreateBuffer(&vbd, &vinitData, &m_pVertexBuffer);


}

void TerrainAppearence::BuildPatchIndex(ID3D11Device* pd3dDevice)
{
	HRESULT hr;
	std::vector<USHORT> indices(_NumPatchQuadFaces * 4); // 4 indices per quad face
// Iterate over each quad and compute indices.
	int k = 0;
	for (UINT i = 0; i < _NumPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < _NumPatchVertCols - 1; ++j)
		{
			// Top row of 2x2 quad patch
			indices[k] = i * _NumPatchVertCols + j;
			indices[k + 1] = i * _NumPatchVertCols + j + 1;
			// Bottom row of 2x2 quad patch
			indices[k + 2] = (i + 1) * _NumPatchVertCols + j;
			indices[k + 3] = (i + 1) * _NumPatchVertCols + j + 1;
			k += 4; // next quad
		}
	}
	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(USHORT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	hr= pd3dDevice->CreateBuffer(&ibd, &iinitData, &m_pIndexBuffer);

	NumberOfVert = _NumPatchQuadFaces * 4;
}

void TerrainAppearence::SetCellSpaceing(float CellSpace)
{
	_CellSpaceing = _CellSpaceing;
}

void TerrainAppearence::CalcAllPatchBoundsY()
{
	_PatchBoundsY.resize(_NumPatchQuadFaces);

	// For each patch
	for (UINT i = 0; i < _NumPatchVertRows - 1; ++i)
	{
		for (UINT j = 0; j < _NumPatchVertCols - 1; ++j)
		{
			CalcPatchBoundsY(i, j);
		}
	}
}
void TerrainAppearence::Draw(ID3D11DeviceContext* pImmediateContext)
{
	if (_ToDraw) {
		ID3D11Buffer* materialCB = getMaterialConstantBuffer();
		pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

		// Set vertex buffer
		UINT stride = sizeof(SimpleVertex);
		UINT offset = 0;

		pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		//pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

		pImmediateContext->DrawIndexed(NumberOfVert, 0, 0);
	}
}
static const float Infinity = FLT_MAX;


template<typename T>
static T Min(const T& a, const T& b)
{
	return a < b ? a : b;
}

template<typename T>
static T Max(const T& a, const T& b)
{
	return a > b ? a : b;
}

void TerrainAppearence::CalcPatchBoundsY(UINT i, UINT j)
{
	// Scan the heightmap values this patch covers and compute the min/max height.

	UINT x0 = j * _CellsPerPatch;
	UINT x1 = (j + 1) * _CellsPerPatch;

	UINT y0 = i * _CellsPerPatch;
	UINT y1 = (i + 1) * _CellsPerPatch;

	float minY = +Infinity;
	float maxY = -Infinity;
	for (UINT y = y0; y <= y1; ++y)
	{
		for (UINT x = x0; x <= x1; ++x)
		{
			UINT k = y * _HightMapWidth + x;
			minY = Min<float>(minY, _HightmapData[k]);
			maxY = Max<float>(maxY, _HightmapData[k]);
		}
	}

	UINT patchID = i * (_NumPatchVertCols - 1) + j;
	//mPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
	_PatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}
