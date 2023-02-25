#include "stdafx.h"
#include "AnimatedModel.h"
#include "DDSTextureLoader.h"

AnimatedModel::AnimatedModel( std::string modelFile, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ShaderController* shaderControl )
    : m_sModelName( modelFile )
{
    M3DLoader loader;
    loader.LoadM3d( modelFile, m_vSkinVert, m_vIndex2, m_vSubsets, m_vMat, m_skeletonData );

     // Appearance
    m_pAppearance = new Appearance();
    m_pAppearance->SetVertexBuffer( pDevice, m_vSkinVert, (UINT)m_vSkinVert.size() );
    m_pAppearance->SetIndices( pDevice, &m_vIndex2[0], m_vIndex2.size() );

    // Shader for animation
    shaderControl->NewAnimationShader( "Animation", L"Animation.hlsl", pDevice, pContext );

    // Transform data
    m_pTransformData = new Transform();
    m_pTransformData->SetPosition( 3, -2, 0 );
    m_pTransformData->SetScale( 0.05f, 0.05f, 0.05f );

    // Textures
    std::wstring filePath = L"Resources/AnimModel/";
    m_pTextureResourceView.resize( m_vMat.size(), nullptr );
    m_pNormalMapResourceView.resize( m_vMat.size(), nullptr );
    for ( size_t i = 0; i < m_vMat.size(); i++ )
    {
        std::vector<int> indexToAddDiff;
        std::vector<int> indexToAddNormMap;
        for ( size_t j = 0; j < m_vMat.size(); j++ )
        {
            if ( m_vMat[i].DiffuseMapName == m_vMat[j].DiffuseMapName )
            {
                indexToAddDiff.push_back( j );
            }
            if ( m_vMat[i].NormalMapName == m_vMat[j].NormalMapName )
            {
                indexToAddNormMap.push_back( j );
            }
        }

        ID3D11ShaderResourceView* diffuseMapTex;
        std::wstring diffuseMap = filePath + m_vMat[i].DiffuseMapName;
        CreateDDSTextureFromFile( pDevice, diffuseMap.c_str(), nullptr, &diffuseMapTex );
        for ( size_t j = 0; j < indexToAddDiff.size(); j++ )
        {
            if ( m_pTextureResourceView[indexToAddDiff[j]] )
            {
                continue;
            }
            m_pTextureResourceView[indexToAddDiff[j]] = diffuseMapTex;
        }

        ID3D11ShaderResourceView* NormMapTex;
        std::wstring normalMap = filePath + m_vMat[i].NormalMapName;
        CreateDDSTextureFromFile( pDevice, normalMap.c_str(), nullptr, &NormMapTex );
        for ( size_t j = 0; j < indexToAddNormMap.size(); j++ )
        {
            if ( m_pNormalMapResourceView[indexToAddNormMap[j]] )
            {
                continue;
            }
            m_pNormalMapResourceView[indexToAddNormMap[j]] = NormMapTex;
        }
    }

    // Setup bone movement cbuffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( cbSkinned );
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HRESULT hr = pDevice->CreateBuffer( &bd, nullptr, &m_pFinalTransformsCB );

    m_vFinalTransforms.resize( m_skeletonData.BoneCount() );
    m_vFinalTransforms = m_skeletonData.GetFinalTransforms( m_sClipName, 0 );
    m_skeletonData.InverseKin( 18, XMFLOAT3( 0, 0, 0 ) );
}

AnimatedModel::~AnimatedModel()
{
    CleanUp();
}

void AnimatedModel::Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl, ConstantBuffer* buffer, ID3D11Buffer* cbuffer )
{
    pContext->IASetInputLayout( shaderControl->GetShaderByName( "Animation" ).m_pVertexLayout );
    pContext->VSSetShader( shaderControl->GetShaderByName( "Animation" ).m_pVertexShader, nullptr, 0 );
    pContext->PSSetShader( shaderControl->GetShaderByName( "Animation" ).m_pPixelShader, nullptr, 0 );

    XMFLOAT4X4 WorldAsFloat = m_pTransformData->GetWorldMatrix();
    XMMATRIX mGO = XMLoadFloat4x4( &WorldAsFloat );
    buffer->mWorld = XMMatrixTranspose( mGO );
    pContext->UpdateSubresource( cbuffer, 0, nullptr, buffer, 0, 0 );
    for ( size_t i = 0; i < m_vFinalTransforms.size(); i++ )
    {
        XMMATRIX mbone = XMLoadFloat4x4( &m_vFinalTransforms[i] );
        m_skinData.m_mBoneTransforms[i] = XMMatrixTranspose( mbone );
    }

    pContext->UpdateSubresource( m_pFinalTransformsCB, 0, nullptr, &m_skinData, 0, 0 );
    pContext->VSSetConstantBuffers( 5, 1, &m_pFinalTransformsCB );
    for ( auto& subset : m_vSubsets )
    {
        pContext->PSSetShaderResources( 0, 1, &m_pTextureResourceView[subset.m_uId] );
        pContext->PSSetShaderResources( 1, 1, &m_pNormalMapResourceView[subset.m_uId] );
        m_pAppearance->Draw( pContext, subset.m_uFaceCount * 3, subset.m_uFaceStart * 3 );
    }

    pContext->IASetInputLayout( shaderControl->GetShaderByName( "Basic" ).m_pVertexLayout );
    pContext->VSSetShader( shaderControl->GetShaderData().m_pVertexShader, nullptr, 0 );
    pContext->PSSetShader( shaderControl->GetShaderData().m_pPixelShader, nullptr, 0 );
}

void AnimatedModel::Update( float dt )
{
    if ( m_bLoopAnimation )
    {
        m_fTimePos += dt;
        m_vFinalTransforms = m_skeletonData.GetFinalTransforms( m_sClipName, m_fTimePos );

        // Loop animation
        if ( m_fTimePos > m_skeletonData.GetClipEndTime( m_sClipName ) )
            m_fTimePos = 0.0f;
    }
    else
    {
        m_vFinalTransforms = m_skeletonData.GetFinalTransforms( m_sClipName, m_fTimePos );
    }
}

void AnimatedModel::CleanUp()
{
    for ( size_t i = 0; i < m_pTextureResourceView.size(); i++ )
    {
        std::vector<int> indexToAddDiff;
        for ( size_t j = 0; j < m_pTextureResourceView.size(); j++ )
        {
            if ( m_pTextureResourceView[i] == m_pTextureResourceView[j] )
            {
                indexToAddDiff.push_back( j );
            }
        }
        if ( m_pTextureResourceView[i] )
        {
            m_pTextureResourceView[i]->Release();
        }
        for ( size_t j = 0; j < indexToAddDiff.size(); j++ )
        {
            m_pTextureResourceView[indexToAddDiff[j]] = nullptr;
        }
    }
    m_pTextureResourceView.clear();

    for ( size_t i = 0; i < m_pNormalMapResourceView.size(); i++ )
    {
        std::vector<int> indexToAddNormMap;
        for ( size_t j = 0; j < m_pNormalMapResourceView.size(); j++ )
        {
            if ( m_pNormalMapResourceView[i] == m_pNormalMapResourceView[j] )
            {
                indexToAddNormMap.push_back( j );
            }
        }
        if ( m_pNormalMapResourceView[i] )
        {
            m_pNormalMapResourceView[i]->Release();
        }
        for ( size_t j = 0; j < indexToAddNormMap.size(); j++ )
        {
            m_pNormalMapResourceView[indexToAddNormMap[j]] = nullptr;
        }
    }
    m_pNormalMapResourceView.clear();

    if ( m_pTransformData )
    {
        delete m_pTransformData;
        m_pTransformData = nullptr;
    }

    if ( m_pAppearance )
    {
        delete m_pAppearance;
        m_pAppearance = nullptr;
    }

    m_skeletonData.CleanUp();
    m_pFinalTransformsCB->Release();
}

void AnimatedModel::ProcessMesh( aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix, std::vector<SkinedVertex>& verts, std::vector<USHORT>& index )
{
    std::vector<SkinedVertex> vertices;
    std::vector<USHORT> indices;

    // Get vertices
    for ( UINT i = 0u; i < mesh->mNumVertices; i++ )
    {
        SkinedVertex vertex;
        vertex.Pos.x = mesh->mVertices[i].x;
        vertex.Pos.y = mesh->mVertices[i].y;
        vertex.Pos.z = mesh->mVertices[i].z;

        if ( mesh->mTextureCoords[0] )
        {
            vertex.Tex.x = static_cast<float>( mesh->mTextureCoords[0][i].x );
            vertex.Tex.y = static_cast<float>( mesh->mTextureCoords[0][i].y );
        }

        vertex.Normal.x = mesh->mNormals[i].x;
        vertex.Normal.y = mesh->mNormals[i].y;
        vertex.Normal.z = mesh->mNormals[i].z;
        vertices.push_back( vertex );
    }

    for ( UINT i = 0u; i < mesh->mNumFaces; i++ )
    {
        aiFace face = mesh->mFaces[i];
        for ( UINT j = 0u; j < face.mNumIndices; j++ )
            indices.push_back( face.mIndices[j] );
    }

    verts = vertices;
    index = indices;
}

void AnimatedModel::ProcessNode( aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix, std::vector<SkinedVertex>& verts, std::vector<USHORT>& index )
{
    XMMATRIX nodeTransformMatrix = XMMatrixTranspose( static_cast<XMMATRIX>( &node->mTransformation.a1 ) ) * parentTransformMatrix;

    for ( UINT i = 0u; i < node->mNumMeshes; i++ )
    {
        std::vector<SkinedVertex> vertices;
        std::vector<USHORT> indices;
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh( mesh, scene, nodeTransformMatrix, vertices, indices );
        verts.insert( verts.end(), vertices.begin(), vertices.end() );
        index.insert( index.end(), indices.begin(), indices.end() );
    }

    for ( UINT i = 0u; i < node->mNumChildren; i++ )
    {
        std::vector<SkinedVertex> vertices;
        std::vector<USHORT> indices;
        ProcessNode( node->mChildren[i], scene, nodeTransformMatrix, vertices, indices );
        verts.insert( verts.end(), vertices.begin(), vertices.end() );
        index.insert( index.end(), indices.begin(), indices.end() );
    }
}