#include "TerrainVoxel.h"

TerrainVoxel::TerrainVoxel(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, ShaderController* shader, int NumberOfChunks_X, int NumberOfChunks_Z):
    _DefaultChunkSize(XMFLOAT3(16,0,16)),
    _NumberOfChunksX(NumberOfChunks_X),
    _NumberOfChunksZ(NumberOfChunks_Z),
    _NumberOfChunks(NumberOfChunks_X * NumberOfChunks_Z)
{
    shader->NewShader("Voxle", L"CubeVoxel.fx", pd3dDevice, pContext);
    _ChunkData.resize(_NumberOfChunksX);

    vector<string> texGround;
    texGround.push_back("Textures513/grass.dds");
    texGround.push_back("Textures513/darkdirt.dds");
    texGround.push_back("Textures513/lightdirt.dds");
    texGround.push_back("Textures513/stone.dds");
    texGround.push_back("Textures513/snow.dds");
    ID3D11ShaderResourceView* res;

    for (auto texName : texGround)
    {
        wstring wide_string = wstring(texName.begin(), texName.end());
        const wchar_t* result = wide_string.c_str();
        CreateDDSTextureFromFile(pd3dDevice, result, nullptr, &res);
        _pGroundTextureRV.push_back(res);
    }

    for (float x = 0; x < _NumberOfChunksX; x++)
    {
        _ChunkData.push_back(std::vector<Chunk*>());
        for (float z = 0; z < _NumberOfChunksZ; z++)
        {
            _ChunkData[x].push_back(new Chunk(pd3dDevice, pContext, XMFLOAT3((_DefaultChunkSize.x * 2)*x, 0, (_DefaultChunkSize.z * 2) * z), _DefaultChunkSize));
        }
    }


    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(VoxleCube);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = pd3dDevice->CreateBuffer(&bd, nullptr, &_CubeInfoCB);
}

TerrainVoxel::~TerrainVoxel()
{
    CleanUp();
}
void ExtractFrustumPlanes2(XMFLOAT4 FrustumPlane[6], CXMMATRIX m)
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
bool AabbBehindPlaneTest(XMFLOAT3 center, XMFLOAT3 extents, XMFLOAT4 plane)
{
    XMFLOAT3 n = XMFLOAT3(abs(plane.x), abs(plane.y), abs(plane.z));

    // This is always positive.
    XMVECTOR a = XMVector3Dot(XMLoadFloat3(&extents), XMLoadFloat3(&n));
    float r = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&extents), XMLoadFloat3(&n)));
    XMFLOAT4 cnet = XMFLOAT4(center.x, center.y, center.z, 1.0f);
    // signed distance from center point to plane.
    float s = XMVectorGetX(XMVector4Dot(XMLoadFloat4(&cnet), XMLoadFloat4(&plane)));;

    // If the center point of the box is a distance of e or more behind the
    // plane (in which case s is negative since it is behind the plane),
    // then the box is completely in the negative half space of the plane.
    return (s + r) < 0.0f;
}

// Returns true if the box is completely outside the frustum.
bool AabbOutsideFrustumTest(XMFLOAT3 center, XMFLOAT3 min, XMFLOAT3 Max, vector<XMFLOAT4> frustumPlanes)
{
    
    bool cull = false;
    for (size_t planeID = 0; planeID < frustumPlanes.size(); planeID++)
    {
        XMVECTOR planeNormal = XMVectorSet(frustumPlanes[planeID].x, frustumPlanes[planeID].y, frustumPlanes[planeID].z, 0.0f);
        float planeConstant = frustumPlanes[planeID].w;
        XMFLOAT3 axisVert;
        // x-axis
        if (frustumPlanes[planeID].x < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the x axis
            axisVert.x = min.x + center.x; // min x plus tree positions x
        else
            axisVert.x = Max.x + center.x; // max x plus tree positions x

        // y-axis
        if (frustumPlanes[planeID].y < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the y axis
            axisVert.y = min.y + center.y; // min y plus tree positions y
        else
            axisVert.y = Max.y + center.y; // max y plus tree positions y

        // z-axis
        if (frustumPlanes[planeID].z < 0.0f)    // Which AABB vertex is furthest down (plane normals direction) the z axis
            axisVert.z = min.z + center.z; // min z plus tree positions z
        else
            axisVert.z = Max.z + center.z; // max z plus tree positions z

    // Now we get the signed distance from the AABB vertex that's furthest down the frustum planes normal,
    // and if the signed distance is negative, then the entire bounding box is behind the frustum plane, which means
    // that it should be culled
        XMFLOAT3 b;
        XMStoreFloat3(&b, XMVector3Dot(planeNormal, XMLoadFloat3(&axisVert)));
        if (b.x + planeConstant < 0.0f) {
            cull = true;
            return cull;
        }
    }
    return cull;
}
void TerrainVoxel::Draw(ID3D11DeviceContext* pContext, ShaderController* ShaderControll, ConstantBuffer* buffer, ID3D11Buffer* _pConstantBuffer, CameraController* camControll)
{
    if (_IsDraw) {
        pContext->VSSetShader(ShaderControll->GetShaderByName("Voxle")._pVertexShader, nullptr, 0);
        pContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
        pContext->PSSetShader(ShaderControll->GetShaderByName("Voxle")._pPixelShader, nullptr, 0);
        pContext->PSSetShaderResources(0, 5, _pGroundTextureRV.data());
        pContext->PSSetConstantBuffers(3, 1, &_CubeInfoCB);
        //frustrum Planes for culling terrain when not in view
        XMFLOAT4X4 viewAsFloats = camControll->GetCam(2)->GetView();
        XMFLOAT4X4 projectionAsFloats = camControll->GetCam(2)->GetProjection();

        XMMATRIX RTTview = XMLoadFloat4x4(&viewAsFloats);
        XMMATRIX RTTprojection = XMLoadFloat4x4(&projectionAsFloats);
        XMMATRIX viewProject = RTTview * RTTprojection;
        //for frustum culling
        XMFLOAT4 worldPlanes[6];
        ExtractFrustumPlanes2(worldPlanes, viewProject);
        vector<XMFLOAT4> planes;
        planes.push_back(worldPlanes[0]);
        planes.push_back(worldPlanes[1]);
        planes.push_back(worldPlanes[2]);
        planes.push_back(worldPlanes[3]);
        planes.push_back(worldPlanes[4]);
        planes.push_back(worldPlanes[5]);

        for (auto x : _ChunkData)
        {
            for (auto z : x)
            {
                XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
                XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);


                XMFLOAT3 CenterPoint = XMFLOAT3((z->GetTrnasfor()->GetPosition().x + _DefaultChunkSize.x), (z->GetTrnasfor()->GetPosition().y + z->GetMaxHight()), (z->GetTrnasfor()->GetPosition().z + _DefaultChunkSize.z));
                minVertex= XMFLOAT3(-_DefaultChunkSize.x, -1, -_DefaultChunkSize.x);
                maxVertex= XMFLOAT3(_DefaultChunkSize.x,  z->GetMaxHight(), _DefaultChunkSize.x);
                if (!AabbOutsideFrustumTest(CenterPoint, minVertex, maxVertex, planes)) {
                    z->Draw(pContext, ShaderControll, buffer, _pConstantBuffer, camControll, _CubeInfoCB);
                }
            }
        }
    }
}
//rebuil voxel with new data
void TerrainVoxel::RebuildMap(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, int Seed, int NumberOfChunks_X, int NumberOfChunks_Z, float Frequancy, int Octave)
{

    _Seed = Seed;
    _Frequancy = Frequancy;
    _Octaves = Octave;
    for (auto chunk : _ChunkData)
    {
        for (auto chunkData : chunk)
        {
            delete chunkData;
            chunkData = nullptr;
        }
        chunk.clear();
    }
    _ChunkData.clear();


    _NumberOfChunksX=NumberOfChunks_X;
    _NumberOfChunksZ=NumberOfChunks_Z;
    _NumberOfChunks=NumberOfChunks_X * NumberOfChunks_Z;

    _ChunkData.resize(_NumberOfChunksX);
    for (float x = 0; x < _NumberOfChunksX; x++)
    {
        _ChunkData.push_back(std::vector<Chunk*>());
        for (float z = 0; z < _NumberOfChunksZ; z++)
        {
            _ChunkData[x].push_back(new Chunk(pd3dDevice, pContext, XMFLOAT3((_DefaultChunkSize.x * 2) * x, 0, (_DefaultChunkSize.z * 2) * z), _DefaultChunkSize,_Seed, _Frequancy, _Octaves));
        }
    }

}

void TerrainVoxel::CleanUp()
{
    for (auto chunk: _ChunkData)
    {
        for (auto chunkData : chunk)
        {
            delete chunkData;
            chunkData = nullptr;
        }
        chunk.clear();
    }
    _ChunkData.clear();

    for (auto texRes : _pGroundTextureRV) {
       if(texRes) texRes->Release();
       texRes = nullptr;
    }
    _pGroundTextureRV.clear();

    if (_CubeInfoCB) {
        _CubeInfoCB->Release();
    }
}

Chunk::Chunk(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext,XMFLOAT3 pos, XMFLOAT3 Size):
    _XSize(Size.x),
    _ZSize(Size.z),
    _MaxHight(Size.y)
{
    _AllCubeInChuck.resize(_XSize);
    _ChunkTransform = new Transform();
    _ChunkTransform->SetPosition(pos);
    GenrateTerrain(pd3dDevice, pContext);
}

Chunk::Chunk(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext, XMFLOAT3 pos, XMFLOAT3 Size, int Seed, float Frequancy, int Octave)
{
    _Seed = Seed;
    _Frequancy = Frequancy;
    _Octaves = Octave;
    _AllCubeInChuck.resize(_XSize);
    _ChunkTransform = new Transform();
    _ChunkTransform->SetPosition(pos);
    GenrateTerrain(pd3dDevice, pContext);
}

Chunk::~Chunk()
{
    CleanUp();
}


void Chunk::Draw(ID3D11DeviceContext* pContext, ShaderController* ShaderControll, ConstantBuffer* buffer, ID3D11Buffer* _pConstantBuffer, CameraController* camControll, ID3D11Buffer* VoxleCB)
{
    
    //frustrum Planes for culling terrain when not in view
    XMFLOAT4X4 viewAsFloats = camControll->GetCam(2)->GetView();
    XMFLOAT4X4 projectionAsFloats = camControll->GetCam(2)->GetProjection();

    XMMATRIX RTTview = XMLoadFloat4x4(&viewAsFloats);
    XMMATRIX RTTprojection = XMLoadFloat4x4(&projectionAsFloats);
    XMMATRIX viewProject = RTTview * RTTprojection;

    XMFLOAT4 worldPlanes[6];
    ExtractFrustumPlanes2(worldPlanes, viewProject);
    vector<XMFLOAT4> planes;
    planes.push_back(worldPlanes[0]);
    planes.push_back(worldPlanes[1]);
    planes.push_back(worldPlanes[2]);
    planes.push_back(worldPlanes[3]);
    planes.push_back(worldPlanes[4]);
    planes.push_back(worldPlanes[5]);
   
    for (auto x : _CubeToDraw)
    {
       
                if (x->GetIsActive()) {
                    x->GetTransForm()->SetParent(_ChunkTransform->GetWorldMatrix());
                    XMFLOAT3 center = x->GetTransForm()->GetPosition();
                    XMFLOAT4X4 world = x->GetTransForm()->GetWorldMatrix();
                    XMFLOAT3 pos= XMFLOAT3(0,0,0);
                    XMFLOAT3 pos2;
                    XMStoreFloat3(&pos2, XMVector3Transform(XMLoadFloat3(&pos), XMLoadFloat4x4(&world)));
                    XMFLOAT3 minVertex = XMFLOAT3(-1,-1,-1);
                    XMFLOAT3 maxVertex = XMFLOAT3( 1,1,1);
                    
                    

                    if (!AabbOutsideFrustumTest(pos2, minVertex, maxVertex, planes)) {
                        XMFLOAT4X4 WorldAsFloat = x->GetTransForm()->GetWorldMatrix();
                        XMMATRIX mGO = XMLoadFloat4x4(&WorldAsFloat);
                        buffer->mWorld = XMMatrixTranspose(mGO);
                        pContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, buffer, 0, 0);
                        VoxleCube data = x->GetCubeData();
                        pContext->UpdateSubresource(VoxleCB, 0, nullptr, &data, 0, 0);

                        x->GetApparance()->Draw(pContext);
                    }
                    
                }
        
    }
 


}

void Chunk::GenrateTerrain(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
    int MaxHight = 100;
    //World Creation
    FastNoiseLite noise;
    noise.SetSeed(_Seed);
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(_Frequancy);
    noise.SetFractalType(FastNoiseLite::FractalType::FractalType_None);
    noise.SetFractalOctaves(_Octaves);
   
    for (float x = 0; x < _XSize; x++)
    {
        for (float z = 0; z < _ZSize; z++)
        {
            int  Active = ((noise.GetNoise(x+ _ChunkTransform->GetPosition().x, z+ _ChunkTransform->GetPosition().z)+1)/2)*20;
            if (Active > MaxHight) {
                Active = MaxHight;
            }
            if (_MaxHight < Active) {
                _MaxHight = Active;
            }
            //Active = 10;
            _AllCubeInChuck[x].resize(_ZSize);
            for(float y = 0; y < MaxHight; y++) {
                _AllCubeInChuck[x][z].push_back(new Block(pd3dDevice, pContext));
            }
            for (float y = 0; y < Active; y++)
            {
                //CubeData[x][z].push_back(new Block(pd3dDevice, pContext));
                _AllCubeInChuck[x][z][y]->SetIsActive(true);
                
            }
        }
 
    }

   
    //creat caves 
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetFrequency(0.09);
    noise.SetFractalType(FastNoiseLite::FractalType::FractalType_None);
    noise.SetFractalOctaves(6);
    for (float x = 0; x < _XSize; x++)
    {
        for (float z = 0; z < _ZSize; z++)
        {
           
            for (float y = 0; y < MaxHight; y++)
            { float  Active = ((noise.GetNoise(x + _ChunkTransform->GetPosition().x, y, z + _ChunkTransform->GetPosition().z) + 1) / 2);
                //CubeData[x][z].push_back(new Block(pd3dDevice, pContext));

                if (Active < 0.5f) {
                    _AllCubeInChuck[x][z][y]->SetIsActive(false);
                }
            }
        }

    }
   
    //noise for cube types
    FastNoiseLite noise2;
    //noise.SetSeed(1338);
    noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    //noise.SetFrequency(0.01);
    noise.SetFractalType(FastNoiseLite::FractalType::FractalType_None);
    noise.SetFractalOctaves(3);

    //face cube creation
    bool lDefault = false;
    for (int x = 0; x < _XSize; x++) {
        for (int y = 0; y < _XSize; y++) {
            //remove unseen facees
            int faceCount = 0;
            for (int z = 0; z < _AllCubeInChuck[x][y].size(); z++) {
                if (_AllCubeInChuck[x][y][z]->GetIsActive() == false) {
                    continue;
                }
                bool lXNegative = lDefault;
                if (x > 0) {
                    lXNegative = _AllCubeInChuck[x - 1][y][z]->GetIsActive();
                    if (lXNegative) {
                        faceCount++;
                    }
                }

                bool lXPositive = lDefault;
                if (x < _XSize - 1) {
                    lXPositive = _AllCubeInChuck[x + 1][y][z]->GetIsActive();
                    if (lXPositive) {
                        faceCount++;
                    }
                }

                bool lYNegative = lDefault;
                if (z > 0) {
                    lYNegative = _AllCubeInChuck[x][y][z - 1]->GetIsActive();
                    if (lYNegative) {
                        faceCount++;
                    }
                }

                bool lYPositive = lDefault;
                if (z < _AllCubeInChuck[x][y].size()) {
                    lYPositive = _AllCubeInChuck[x][y][z + 1]->GetIsActive();
                    if (lYPositive) {
                        faceCount++;
                    }
                }

                bool lZNegative = lDefault;
                if (y > 0) {
                    lZNegative = _AllCubeInChuck[x][y - 1][z]->GetIsActive();
                    if (lZNegative) {
                        faceCount++;
                    }
                }

                bool lZPositive = lDefault;
                if (y < _XSize - 1) {
                    lZPositive = _AllCubeInChuck[x][y + 1][z]->GetIsActive();
                    if (lZPositive) {
                        faceCount++;
                    }
                }

                if (!lXNegative || !lXPositive || !lYNegative || !lYPositive || !lZNegative || !lZPositive) {
                    _AllCubeInChuck[x][y][z]->CreatCube(lXNegative, lXPositive, lYNegative, lYPositive, lZNegative, lZPositive, pd3dDevice, pContext);
                    _AllCubeInChuck[x][y][z]->GetTransForm()->SetPosition(x / 0.5f, z / 0.5f, y / 0.5f);
                    
                        //set cube types
                       float a= noise2.GetNoise((float)x, (float)y, (float)z);
                       if (a < 0) {
                            _AllCubeInChuck[x][y][z]->SetBlockType(BlockType::Stone);
                       }
                       else if (a < 0.5f) {
                           _AllCubeInChuck[x][y][z]->SetBlockType(BlockType::Grass);
                       }
                       else if (a < 1.0f) {
                           _AllCubeInChuck[x][y][z]->SetBlockType(BlockType::Snow);
                       }
                }
                
            }
        }
    }

    // manage cubes so list to draw is small
    for (int x = 0; x < _XSize; x++) {
        for (int y = 0; y < _XSize; y++) {
            for (int z = 0; z < _AllCubeInChuck[x][y].size(); z++) {
                if (_AllCubeInChuck[x][y][z]->GetIsActive() == false) {
                    continue;
                }
                if (_AllCubeInChuck[x][y][z]->GetApparance() == nullptr) {
                    _AllCubeInChuck[x][y][z]->SetIsActive(false);
                }
                if (_AllCubeInChuck[x][y][z]->GetIsActive()) {
                    _CubeToDraw.push_back(_AllCubeInChuck[x][y][z]);
                }
            }
        }
    }

}

void Chunk::CleanUp()
{
    delete _ChunkTransform;
    _ChunkTransform = nullptr;

    for (auto cubeX: _AllCubeInChuck)
    {
        for (auto cubeY : cubeX)
        {
            for (auto cubeZ : cubeY)
            {
                delete cubeZ;
                cubeZ = nullptr;
            }
        }
    }
}

Block::Block(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
   
}

Block::~Block()
{
    CleanUp();
}

void Block::CreatCube(bool lXNegative, bool lXPositive, bool lYNegative, bool lYPositive, bool lZNegative, bool lZPositive, ID3D11Device* pd3dDevice, ID3D11DeviceContext* pContext)
{
    _CubeTransform = new Transform();
    _CubeApparace = new TerrainAppearence();
    _CubeApparace->CreateCube(lXNegative, lXPositive, lYNegative, lYPositive, lZNegative, lZPositive, pd3dDevice, pContext);
    
}

void Block::SetBlockType(BlockType Block)
{
    _BlockType = Block;

    _CubeData.CubeType = (int)Block;

}

void Block::CleanUp()
{
    delete _CubeApparace;
    _CubeApparace = nullptr;
    delete _CubeTransform;
    _CubeTransform = nullptr;
}
