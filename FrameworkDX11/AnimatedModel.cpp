#include "AnimatedModel.h"
#include"DDSTextureLoader.h"

void ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix, std::vector<SkinedVertex>& verts, std::vector<USHORT>& index)
{
    std::vector<SkinedVertex> vertices;
    std::vector<USHORT> indices;

    // get vertices
    for (UINT i = 0u; i < mesh->mNumVertices; i++)
    {
        SkinedVertex vertex;

        vertex.Pos.x = mesh->mVertices[i].x;
        vertex.Pos.y = mesh->mVertices[i].y;
        vertex.Pos.z = mesh->mVertices[i].z;

        if (mesh->mTextureCoords[0])
        {
            vertex.Tex.x = static_cast<float>(mesh->mTextureCoords[0][i].x);
            vertex.Tex.y = static_cast<float>(mesh->mTextureCoords[0][i].y);
        }

        vertex.Normal.x = mesh->mNormals[i].x;
        vertex.Normal.y = mesh->mNormals[i].y;
        vertex.Normal.z = mesh->mNormals[i].z;

        vertices.push_back(vertex);
    }

    for (UINT i = 0u; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (UINT j = 0u; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    verts = vertices;
    index= indices;
   
}
void ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix, std::vector<SkinedVertex>& verts, std::vector<USHORT>& index)
{
    XMMATRIX nodeTransformMatrix = XMMatrixTranspose(static_cast<XMMATRIX>(&node->mTransformation.a1)) * parentTransformMatrix;

    for (UINT i = 0u; i < node->mNumMeshes; i++)
    {
        std::vector<SkinedVertex> vertices;
        std::vector<USHORT> indices;
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene, nodeTransformMatrix, vertices, indices);

        verts.insert(verts.end(), vertices.begin(), vertices.end());
        index.insert(index.end(), indices.begin(), indices.end());
       
    }

    for (UINT i = 0u; i < node->mNumChildren; i++) {
        std::vector<SkinedVertex> vertices;
        std::vector<USHORT> indices;
        ProcessNode(node->mChildren[i], scene, nodeTransformMatrix, vertices, indices);

      

        verts.insert(verts.end(), vertices.begin(), vertices.end());
        index.insert(index.end(), indices.begin(), indices.end());
       
        
    }
}




AnimatedModel::AnimatedModel(std::string ModelFile, ID3D11Device* device, ID3D11DeviceContext* pImmediateContext, ShaderController* Controll):
    _ModelName(ModelFile)
{
    M3DLoader loader;
    loader.LoadM3d(ModelFile, _SkinVert, _Index2, _Subsets, _Mat, SkeletonData);

  /*  Assimp::Importer importera;
    const aiScene* pScene = importera.ReadFile("AnimationModel/StandingDisarmUnderarmoptimusprime.fbx",
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
    if (pScene == nullptr) {
        int aa = 123;
    }
   */

    //ProcessNode(pScene->mRootNode, pScene, XMMatrixIdentity(), skinVert, index2);

    //apprance
    _Apparance = new Appearance();
    _Apparance->SetVertexBuffer(device, _SkinVert, (UINT)_SkinVert.size());
    _Apparance->SetIndices(device, &_Index2[0], _Index2.size());
    
    //shader for animation
    Controll->NewAnimationShader("animation", L"AnnimationShader.fx", device, pImmediateContext);

    //transform data
    TransformData = new Transform();
    TransformData->SetPosition(3, -2, 0);
    TransformData->SetScale(0.05f, 0.05f, 0.05f);

    //textuers
    wstring FileLoacation = L"AnimationModel\\";
    m_pTextureResourceView.resize(_Mat.size(),nullptr);
    m_pNormalMapResourceView.resize(_Mat.size(),nullptr);
    for (size_t i = 0; i < _Mat.size(); i++)
    {
        std::vector<int> indexToAddDiff;
        std::vector<int> indexToAddNormMap;
        for (size_t j = 0; j < _Mat.size(); j++) {
            if (_Mat[i].DiffuseMapName == _Mat[j].DiffuseMapName) {
                indexToAddDiff.push_back(j);
            }
            if (_Mat[i].NormalMapName == _Mat[j].NormalMapName) {
                indexToAddNormMap.push_back(j);
            }
        }

        ID3D11ShaderResourceView* diffuseMapTex;
        wstring DiffMap = FileLoacation + _Mat[i].DiffuseMapName;
        CreateDDSTextureFromFile(device, DiffMap.c_str(), nullptr, &diffuseMapTex);
        for (size_t j = 0; j < indexToAddDiff.size(); j++) {
            if (m_pTextureResourceView[indexToAddDiff[j]]) {
                continue;
            }
            m_pTextureResourceView[indexToAddDiff[j]] = diffuseMapTex;
        }

        ID3D11ShaderResourceView* NormMapTex;
        wstring NormMap = FileLoacation + _Mat[i].NormalMapName;
        CreateDDSTextureFromFile(device, NormMap.c_str(), nullptr, &NormMapTex);
        for (size_t j = 0; j < indexToAddNormMap.size(); j++) {
            if (m_pNormalMapResourceView[indexToAddNormMap[j]]) {
                continue;
            }
            m_pNormalMapResourceView[indexToAddNormMap[j]] = NormMapTex;
        }
    }

    //bone movement CB
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(cbSkinned);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = device->CreateBuffer(&bd, nullptr, &FinalTransformsCB);

    FinalTransforms.resize(SkeletonData.BoneCount());
    FinalTransforms=SkeletonData.GetFinalTransforms(_ClipName, 0);

    SkeletonData.InverKin(18, XMFLOAT3(0, 0, 0));
}

AnimatedModel::~AnimatedModel()
{
    CleanUp();
}

void AnimatedModel::Draw(ID3D11DeviceContext* pImmediateContext, ShaderController* Controll, ConstantBuffer* buffer, ID3D11Buffer* _pConstantBuffer)
{
    pImmediateContext->IASetInputLayout(Controll->GetShaderByName("animation")._pVertexLayout);
    pImmediateContext->VSSetShader(Controll->GetShaderByName("animation")._pVertexShader, nullptr, 0);
    pImmediateContext->PSSetShader(Controll->GetShaderByName("animation")._pPixelShader, nullptr, 0);
   
    XMFLOAT4X4 WorldAsFloat = TransformData->GetWorldMatrix();
    XMMATRIX mGO = XMLoadFloat4x4(&WorldAsFloat);
    buffer->mWorld = XMMatrixTranspose(mGO);
    pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, buffer, 0, 0);

    for (size_t i = 0; i < FinalTransforms.size(); i++)
    {
        XMMATRIX mbone = XMLoadFloat4x4(&FinalTransforms[i]);
        data.gBoneTransforms[i] = XMMatrixTranspose(mbone);
    }

    pImmediateContext->UpdateSubresource(FinalTransformsCB, 0, nullptr, &data, 0, 0);

    pImmediateContext->VSSetConstantBuffers(5,1, &FinalTransformsCB);
    //draw
    for (auto& subset : _Subsets)
    {

        pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureResourceView[subset.Id]);
        pImmediateContext->PSSetShaderResources(1, 1, &m_pNormalMapResourceView[subset.Id]);

        _Apparance->Draw(pImmediateContext, subset.FaceCount*3, subset.FaceStart * 3);
     
    }

    
    pImmediateContext->IASetInputLayout(Controll->GetShaderByName("NoEffects")._pVertexLayout);

    pImmediateContext->VSSetShader(Controll->GetShaderData()._pVertexShader, nullptr, 0);
    pImmediateContext->PSSetShader(Controll->GetShaderData()._pPixelShader, nullptr, 0);
}

void AnimatedModel::Update(float dt)
{
    if (isLoopAnimation) {
        TimePos += dt;
        FinalTransforms = SkeletonData.GetFinalTransforms(_ClipName, TimePos);

        // Loop animation
        if (TimePos > SkeletonData.GetClipEndTime(_ClipName))
            TimePos = 0.0f;
    }
    else
    {
        FinalTransforms = SkeletonData.GetFinalTransforms(_ClipName, TimePos);
    }
}

void AnimatedModel::CleanUp()
{
    for (size_t i = 0; i < m_pTextureResourceView.size(); i++)
    {
        std::vector<int> indexToAddDiff;
        for (size_t j = 0; j < m_pTextureResourceView.size(); j++)
        {
            if (m_pTextureResourceView[i] == m_pTextureResourceView[j]) {
                indexToAddDiff.push_back(j);
            }
        }
        if (m_pTextureResourceView[i]) {
            m_pTextureResourceView[i]->Release();
        }
        for (size_t j = 0; j < indexToAddDiff.size(); j++)
        {
            m_pTextureResourceView[indexToAddDiff[j]] = nullptr;
        }
    }
    m_pTextureResourceView.clear();
    for (size_t i = 0; i < m_pNormalMapResourceView.size(); i++)
    {
        std::vector<int> indexToAddNormMap;
        for (size_t j = 0; j < m_pNormalMapResourceView.size(); j++)
        {
            if (m_pNormalMapResourceView[i] == m_pNormalMapResourceView[j]) {
                indexToAddNormMap.push_back(j);
            }
        }
        if (m_pNormalMapResourceView[i]) {
            m_pNormalMapResourceView[i]->Release();
        }
        for (size_t j = 0; j < indexToAddNormMap.size(); j++)
        {
            m_pNormalMapResourceView[indexToAddNormMap[j]] = nullptr;
        }
    }
    m_pNormalMapResourceView.clear();

    if (TransformData) {
        delete TransformData;
        TransformData = nullptr;
    }


    if (_Apparance) {
        delete _Apparance;
        _Apparance = nullptr;
    }

    SkeletonData.CleanUp();
    FinalTransformsCB->Release();
}
