#include "Terrain.h"
#include<ostream>
#include<fstream>
#include<DirectXMath.h>
#include "DirectXPackedVector.h"
#include<algorithm>

// noise genrater userd : https://github.com/Auburn/FastNoiseLite
#include"fastNoiseLite\Cpp\FastNoiseLite.h"

Terrain::Terrain(std::string HightMapName, XMFLOAT2 size, double Scale, TerrainGenType GenType, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, ShaderController* ShaderControll):
    HightMapName(HightMapName),
    _HightMapHight(size.y),
    _HightMapWidth(size.x),
    _HightScale(Scale),
    _TerrainCreationType(GenType)
{
	_pTransform = new Transform();
    _pApperance = new TerrainAppearence(_HightMapHight, _HightMapWidth,1.0f, _HightmapData);

    ShaderControll->NewTessellationShader("Terrain", L"Terrain.hlsl", pd3dDevice, pContext);

    CreateHightData();
    BuildHightMap(pd3dDevice);

    _pApperance->setHightdata(_HightmapData);
    _pApperance->CalcAllPatchBoundsY();
    _pApperance->InitMeshGround(pd3dDevice);


    D3D11_BUFFER_DESC bd = {};
    // Create the light constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(TerrainCB);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
   HRESULT hr = pd3dDevice->CreateBuffer(&bd, nullptr, &_TerrainConstantBuffer);

   _TerrainCB.MaxDist = 500.0f;
   _TerrainCB.MinDist = 20.0f;
   _TerrainCB.MaxTess = 6.0f;
   _TerrainCB.MinTess = 0.0f;


   _TerrainCB.Layer1MaxHight = 20.0f;
   _TerrainCB.Layer2MaxHight = 30.0f;
   _TerrainCB.Layer3MaxHight = 40.0f;
   _TerrainCB.Layer4MaxHight = 50.0f;
   _TerrainCB.Layer5MaxHight = 55.0f;
}

Terrain::~Terrain()
{
    CleanUp();
}

void Terrain::Update()
{
}

void ExtractFrustumPlanes(XMFLOAT4 FrustumPlane[6], CXMMATRIX m)
{
    XMFLOAT4X4 viewProj;
    XMStoreFloat4x4(&viewProj, m);
    // Left Frustum Plane
// Add first column of the matrix to the fourth column
    FrustumPlane[0].x = viewProj._14 + viewProj._11;
    FrustumPlane[0].y = viewProj._24 + viewProj._21;
    FrustumPlane[0].z = viewProj._34 + viewProj._31;
    FrustumPlane[0].w = viewProj._44 + viewProj._41;

    // Right Frustum Plane
    // Subtract first column of matrix from the fourth column
    FrustumPlane[1].x = viewProj._14 - viewProj._11;
    FrustumPlane[1].y = viewProj._24 - viewProj._21;
    FrustumPlane[1].z = viewProj._34 - viewProj._31;
    FrustumPlane[1].w = viewProj._44 - viewProj._41;

    // Top Frustum Plane
    // Subtract second column of matrix from the fourth column
    FrustumPlane[2].x = viewProj._14 - viewProj._12;
    FrustumPlane[2].y = viewProj._24 - viewProj._22;
    FrustumPlane[2].z = viewProj._34 - viewProj._32;
    FrustumPlane[2].w = viewProj._44 - viewProj._42;

    // Bottom Frustum Plane
    // Add second column of the matrix to the fourth column
    FrustumPlane[3].x = viewProj._14 + viewProj._12;
    FrustumPlane[3].y = viewProj._24 + viewProj._22;
    FrustumPlane[3].z = viewProj._34 + viewProj._32;
    FrustumPlane[3].w = viewProj._44 + viewProj._42;

    // Near Frustum Plane
    // We could add the third column to the fourth column to get the near plane,
    // but we don't have to do this because the third column IS the near plane
    FrustumPlane[4].x = viewProj._13;
    FrustumPlane[4].y = viewProj._23;
    FrustumPlane[4].z = viewProj._33;
    FrustumPlane[4].w = viewProj._43;

    // Far Frustum Plane
    // Subtract third column of matrix from the fourth column
    FrustumPlane[5].x = viewProj._14 - viewProj._13;
    FrustumPlane[5].y = viewProj._24 - viewProj._23;
    FrustumPlane[5].z = viewProj._34 - viewProj._33;
    FrustumPlane[5].w = viewProj._44 - viewProj._43;

    // Normalize the plane equations.
    for (int i = 0; i < 6; ++i)
    {
        XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&FrustumPlane[i]));
        XMStoreFloat4(&FrustumPlane[i], v);
    }
}


void Terrain::Draw(ID3D11DeviceContext* pContext, ShaderController* ShaderControll, ConstantBuffer* buffer, ID3D11Buffer* _pConstantBuffer, CameraController* camControll)
{
    if (_IsDraw) {
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
        pContext->IASetInputLayout(ShaderControll->GetShaderByName("Terrain").m_pVertexLayout);

        XMFLOAT4X4 WorldAsFloat = _pTransform->GetWorldMatrix();
        XMMATRIX mGO = XMLoadFloat4x4(&WorldAsFloat);
        buffer->mWorld = XMMatrixTranspose(mGO);
        pContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, buffer, 0, 0);

        //frustrum Planes for culling terrain when not in view
        XMFLOAT4X4 viewAsFloats = camControll->GetCam(0)->GetView();
        XMFLOAT4X4 projectionAsFloats = camControll->GetCam(0)->GetProjection();

        XMMATRIX RTTview = XMLoadFloat4x4(&viewAsFloats);
        XMMATRIX RTTprojection = XMLoadFloat4x4(&projectionAsFloats);
        XMMATRIX viewProject = RTTview * RTTprojection;
        XMFLOAT4 worldPlanes[6];
        ExtractFrustumPlanes(worldPlanes, viewProject);
        _TerrainCB.WorldFrustumPlanes[0] = worldPlanes[0];
        _TerrainCB.WorldFrustumPlanes[1] = worldPlanes[1];
        _TerrainCB.WorldFrustumPlanes[2] = worldPlanes[2];
        _TerrainCB.WorldFrustumPlanes[3] = worldPlanes[3];
        _TerrainCB.WorldFrustumPlanes[4] = worldPlanes[4];
        _TerrainCB.WorldFrustumPlanes[5] = worldPlanes[5];

        _TerrainCB.gEyePosition = camControll->GetCam(0)->GetPositionFloat4();

        //_TerrainCB.gEyePosition = XMFLOAT4();
        pContext->UpdateSubresource(_TerrainConstantBuffer, 0, nullptr, &_TerrainCB, 0, 0);

        //Shader Set
        pContext->VSSetShader(ShaderControll->GetShaderByName("Terrain").m_pVertexShader, nullptr, 0);
        pContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
        pContext->HSSetConstantBuffers(4, 1, &_TerrainConstantBuffer);
        pContext->VSSetShaderResources(1, 1, &_HeightMapSRV);

        pContext->HSSetShader(ShaderControll->GetShaderByName("Terrain").m_pHullShader, nullptr, 0);
        pContext->HSSetConstantBuffers(0, 1, &_pConstantBuffer);
        pContext->HSSetConstantBuffers(4, 1, &_TerrainConstantBuffer);
        pContext->HSSetShaderResources(1, 1, &_HeightMapSRV);

        pContext->DSSetShader(ShaderControll->GetShaderByName("Terrain").m_pDomainShader, nullptr, 0);
        pContext->DSSetConstantBuffers(0, 1, &_pConstantBuffer);
        pContext->DSSetConstantBuffers(4, 1, &_TerrainConstantBuffer);
        pContext->DSSetShaderResources(1, 1, &_HeightMapSRV);

        pContext->PSSetShaderResources(0, 1, _pApperance->GetTextureResourceView().data());
        pContext->PSSetConstantBuffers(4, 1, &_TerrainConstantBuffer);
        pContext->PSSetShaderResources(0, 1, &_BlendMap);
        pContext->PSSetShaderResources(1, 1, &_HeightMapSRV);
        pContext->PSSetShaderResources(2, 5, _pApperance->GetTextureResourceView().data());
        pContext->PSSetShader(ShaderControll->GetShaderByName("Terrain").m_pPixelShader, nullptr, 0);

        _pApperance->Draw(pContext);

        // FX sets tessellation stages, but it does not disable them.  So do that here
        // to turn off tessellation.
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pContext->HSSetShader(0, 0, 0);
        pContext->DSSetShader(0, 0, 0);
    }
}



void Terrain::SetHightScale(float HightScale)
{
    _HightScale = HightScale;
}

void Terrain::SetCellSpacing(float cellSpaceing)
{
    _CellSpaceing = cellSpaceing;
}
/// <summary>
/// using lines to genrate hight map
/// </summary>
void Terrain::FaultHightFromation()
{
    _HightmapData.clear();
    _HightmapData.resize(_HightMapHight * _HightMapWidth, 0);
    RandomGen::random<int>(0, 255, 0);
    RandomGen::randomFloat<float>(0, 255, 0);
    for (size_t i = 0; i < _NumberOfIterations; i++)
    {
        //line formula ax + bz = c
        //get line data
       float  v = RandomGen::randomFloat<float>(0, RAND_MAX, _Seed);
       float  a = sin(v);
       float   b = cos(v);

        float d = std::sqrt(_HightMapHight * _HightMapHight + _HightMapWidth * _HightMapWidth);
        float  c = RandomGen::randomFloat<float>(-d / 2, d / 2, _Seed);

       for (UINT z = 0; z < _HightMapHight; ++z)
       {
           for (UINT x = 0; x < _HightMapWidth; ++x)
           {
               //one side of line go up
               if (a * x + b * z - c > 0)
                   _HightmapData[z * _HightMapWidth + x] += _Displacement;
               //other side of line go down
               else
                   _HightmapData[z * _HightMapWidth + x] -= _Displacement;
           }
       }

    }
}
/// <summary>
/// useing noise to genrate hight map
/// </summary>
void Terrain::HightFromNoise()
{
    FastNoiseLite noise;
    noise.SetSeed(_Seed);
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(_Frequancy);
    noise.SetFractalOctaves(_NumberOfOctaves);

    // Gather noise data
    _HightmapData.resize(_HightMapHight * _HightMapWidth, 0);
    int index = 0;

    for (int y = 0; y < _HightMapHight; y++)
    {
        for (int x = 0; x < _HightMapWidth; x++)
        {
            //get value between 0 and 1 * by hight scale
            _HightmapData[y * _HightMapWidth + x] =  ((noise.GetNoise((float)x, (float)y) + 1) / 2)* _HightScale;
        }
    }

}
/// <summary>
/// using dimonand squaer to get hight map
/// </summary>
void Terrain::DiamondSquareHightMap()
{
    _2DHightMap.clear();
    _2DHightMap.resize(_HightMapWidth);

    for (size_t i = 0; i < _2DHightMap.size(); i++)
    {
        _2DHightMap[i].resize(_HightMapWidth);
    }
    RandomGen::random<int>(0, 255, 0);
    //seed map
    _2DHightMap[0][0] = RandomGen::random<int>(0, 255,_Seed);
    _2DHightMap[0][_HightMapWidth - 1] = RandomGen::random<int>(0, 255, _Seed);
    _2DHightMap[_HightMapWidth - 1][0] = RandomGen::random<int>(0, 255, _Seed);
    _2DHightMap[_HightMapWidth - 1][_HightMapWidth - 1] = RandomGen::random<int>(0, 255, _Seed);

    int sideLength = _HightMapWidth / 2;

    Diamond(_HightMapWidth);
    Square(_HightMapWidth);

    _Range /= 2;

    while (sideLength >= 2)
    {
        Diamond(sideLength + 1);
        Square(sideLength + 1);
        sideLength /= 2;
        _Range /= 2;
    }

    //convert to 1d Hight Map

    _HightmapData.resize(_HightMapWidth * _HightMapWidth);
    for (int z = 0; z < _HightMapWidth; ++z)
    {
        for (int x = 0; x < _HightMapWidth; ++x)
        {

            _HightmapData[z * _HightMapWidth + x] = (_2DHightMap[x][z] / 255) * _HightScale;

        }
    }
}

//diceide on what genration type to use
void Terrain::CreateHightData()
{

    switch (_TerrainCreationType)
    {
    case TerrainGenType::HightMapLoad:
        LoadHightMap();
        break;
    case TerrainGenType::FaultLine:
        FaultHightFromation();
        break;
    case TerrainGenType::Noise:
        HightFromNoise();
        break;
    case TerrainGenType::DiamondSquare:
        DiamondSquareHightMap();
        break;
    default:
        LoadHightMap();
        break;
    }
    Smooth();
}

void Terrain::SetMaxTess(float maxTess)
{
    _TerrainCB.MaxTess = maxTess;
}

void Terrain::SetMinTess(float minTess)
{
    _TerrainCB.MinTess = minTess;
}

void Terrain::SetMaxTessDist(float maxTessDist)
{
    _TerrainCB.MaxDist = maxTessDist;
}

void Terrain::SetMinTessDist(float minTessDist)
{
    _TerrainCB.MinDist = minTessDist;
}



bool Terrain::InBounds(int i, int j)
{
    // True if ij are valid indices; false otherwise.
    return
        i >= 0 && i < (int)_HightMapHight&&
        j >= 0 && j < (int)_HightMapWidth;
}

float Terrain::Average(int i, int j)
{

    //use values around to smoot value
    float avg = 0.0f;
    float num = 0.0f;
    // Use int to allow negatives. If we use UINT, @ i=0, m=i-1=UINT_MAX
    // and no iterations of the outer for loop occur.
    for (int m = i - 1; m <= i + 1; ++m)
    {
        for (int n = j - 1; n <= j + 1; ++n)
        {
            if (InBounds(m, n))
            {
                avg += _HightmapData[m * _HightMapWidth + n];
                num += 1.0f;
            }
        }
    }
    return avg / num;
}

//smooth hight data to from better terrain
void Terrain::Smooth()
{
    std::vector<float> dest(_HightmapData.size());
    for (UINT i = 0; i < _HightMapHight; ++i)
    {
        for (UINT j = 0; j < _HightMapWidth; ++j)
        {
            dest[i * _HightMapWidth + j] = Average(i, j);
        }
    }
    // Replace the old heightmap with the filtered one.
    _HightmapData = dest;
}

void Terrain::SetBlendMap(std::string name, ID3D11Device* pd3dDevice)
{
    wstring wide_string = wstring(name.begin(), name.end());
    const wchar_t* result = wide_string.c_str();
    CreateDDSTextureFromFile(pd3dDevice, result, nullptr, &_BlendMap);
}

float Terrain::GetHightWorld(float x, float z)
{
    return 0.0f;
}

float Terrain::GetHight(float x, float z)
{
    return _HightmapData[x * _HightMapWidth + z];
}

void Terrain::ReBuildTerrain(XMFLOAT2 size, double Scale, float CellSpaceing, TerrainGenType GenType, ID3D11Device* pd3dDevice)
{



    _HightMapHight=size.y;
    _HightMapWidth=size.x;
    _HightScale=Scale;
    _TerrainCreationType=GenType;
    _CellSpaceing = CellSpaceing;
    CreateHightData();
    BuildHightMap(pd3dDevice);

    _pApperance->SetWidth(size.x);
    _pApperance->SetDepth(size.y);
    _pApperance->SetCellSpaceing(CellSpaceing);

    _pApperance->setHightdata(_HightmapData);
    _pApperance->CalcAllPatchBoundsY();
    _pApperance->InitMeshGround(pd3dDevice);


}

void Terrain::SetTexHights(float Hight1, float Hight2, float Hight3, float Hight4, float Hight5)
{
    _TerrainCB.Layer1MaxHight = Hight1;
    _TerrainCB.Layer2MaxHight = Hight2;
    _TerrainCB.Layer3MaxHight = Hight3;
    _TerrainCB.Layer4MaxHight = Hight4;
    _TerrainCB.Layer5MaxHight = Hight5;
}

void Terrain::SetTex(vector<string> texGroundName, ID3D11Device* pd3dDevice)
{
    _pApperance->SetTex(texGroundName, pd3dDevice);
    _TexGround = texGroundName;
}

//hight map into shader resrouce to give vertex hight in shader
void Terrain::BuildHightMap(ID3D11Device* pd3dDevice)
{
    HRESULT hr;
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = _HightMapWidth;
    texDesc.Height = _HightMapHight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    // HALF is defined in xnamath.h, for storing 16-bit float.
    std::vector<DirectX::PackedVector::HALF> hmap(_HightmapData.size());
    std::transform(_HightmapData.begin(), _HightmapData.end(),
        hmap.begin(), DirectX::PackedVector::XMConvertFloatToHalf);
    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &hmap[0];
    data.SysMemPitch = _HightMapWidth * sizeof(DirectX::PackedVector::HALF);
    data.SysMemSlicePitch = 0;
    ID3D11Texture2D* hmapTex = 0;
    hr = pd3dDevice->CreateTexture2D(&texDesc, &data, &hmapTex);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;
    hr= pd3dDevice->CreateShaderResourceView(
        hmapTex, &srvDesc, &_HeightMapSRV);
    // SRV saves reference.
   hmapTex->Release();
}

// load hight map data
void Terrain::LoadHightMap()
{
    // A height for each vertex
    std::vector<unsigned char> in(_HightMapWidth * _HightMapHight);

    // Open the file.
    std::ifstream inFile;
    inFile.open(HightMapName, std::ios_base::binary);

    if (inFile)
    {
        // Read the RAW bytes.
        inFile.read((char*)&in[0], (std::streamsize)in.size());
        // Done with file.

        inFile.close();
    }

    // Copy the array data into a float array and scale it. mHeightmap.resize(heightmapHeight * heightmapWidth, 0);
    _HightmapData.resize(_HightMapHight * _HightMapWidth,0);
    for (UINT i = 0; i < _HightMapHight * _HightMapWidth; ++i)
    {
        _HightmapData[i] = (in[i] / 255.0f) * _HightScale;
    }


}

/// <summary>
/// dimond step
/// </summary>
/// <param name="sideLength"></param>
void Terrain::Diamond(int sideLength)
{
    int halfSide = sideLength / 2;

    for (int y = 0; y < _HightMapWidth / (sideLength - 1); y++)
    {
        for (int x = 0; x < _HightMapWidth / (sideLength - 1); x++)
        {
            int center_x = x * (sideLength - 1) + halfSide;
            int center_y = y * (sideLength - 1) + halfSide;

            //advrage of points on vertex
            int avg = (_2DHightMap[x * (sideLength - 1)][y * (sideLength - 1)] +
                _2DHightMap[x * (sideLength - 1)][(y + 1) * (sideLength - 1)] +
                _2DHightMap[(x + 1) * (sideLength - 1)][y * (sideLength - 1)] +
                _2DHightMap[(x + 1) * (sideLength - 1)][(y + 1) * (sideLength - 1)]) / 4.0f;

            //add randome number to it
            _2DHightMap[center_x][center_y] = avg + RandomGen::random<int>(-_Range, _Range,_Seed);
        }
    }
}
// Averaging helper function for square step to ignore out of bounds points
void Terrain::average(int x, int y, int sideLength)
{
    float counter = 0;
    float accumulator = 0;

    int halfSide = sideLength / 2;
    //get points of data
    if (x != 0)
    {
        counter += 1.0f;
        accumulator += _2DHightMap[y][x - halfSide];
    }
    if (y != 0)
    {
        counter += 1.0f;
        accumulator += _2DHightMap[y - halfSide][x];
    }
    if (x != _HightMapWidth - 1)
    {
        counter += 1.0f;
        accumulator += _2DHightMap[y][x + halfSide];
    }
    if (y != _HightMapWidth - 1)
    {
        counter += 1.0f;
        accumulator += _2DHightMap[y + halfSide][x];
    }

    //avrage then add random valuae to it
    _2DHightMap[y][x] = (accumulator / counter) + RandomGen::random<int>(-_Range, _Range,_Seed);
}
//squaer data
void Terrain::Square(int sideLength)
{
    int halfLength = sideLength / 2;

    for (int y = 0; y < _HightMapWidth / (sideLength - 1); y++)
    {
        for (int x = 0; x < _HightMapWidth / (sideLength - 1); x++)
        {
            // Top
            average(x * (sideLength - 1) + halfLength, y * (sideLength - 1), sideLength);
            // Right
            average((x + 1) * (sideLength - 1), y * (sideLength - 1) + halfLength,
                sideLength);
            // Bottom
            average(x * (sideLength - 1) + halfLength, (y + 1) * (sideLength - 1), sideLength);
            // Left
            average(x * (sideLength - 1), y * (sideLength - 1) + halfLength, sideLength);
        }
    }
}

void Terrain::CleanUp()
{
    if (_pTransform)delete _pTransform;
    _pTransform = nullptr;

    if (_pApperance)delete _pApperance;
    _pApperance = nullptr;
    if (_HeightMapSRV)_HeightMapSRV->Release();
    _HeightMapSRV = nullptr;
    if (_BlendMap)_BlendMap->Release();
    _BlendMap = nullptr;
    if (_TerrainConstantBuffer)_TerrainConstantBuffer->Release();
    _TerrainConstantBuffer = nullptr;

}
