#include "stdafx.h"
#include "AnimatedModel.h"

AnimatedModel::AnimatedModel( std::string modelFile, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ShaderController* shaderControl )
    : m_sModelName( modelFile )
{
    M3DLoader loader;
    loader.LoadM3d( modelFile, m_vSkinVert, m_vIndex2, m_vSubsets, m_vMat, m_skeletonData );

     // Appearance
    m_pAppearance = new Appearance();
    m_pAppearance->SetVertexBuffer( pDevice, m_vSkinVert );
    m_pAppearance->SetIndices( pDevice, m_vIndex2 );

    // Shader for animation
    shaderControl->NewAnimationShader( "Animation", L"Animation.hlsl", pDevice, pContext );

    // Transform data
    m_pTransformData = new Transform();
    m_pTransformData->SetPosition( 3.0f, -2.0f, 0.0f );
    m_pTransformData->SetRotation( 0.0f, 180.0f, 0.0f );
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

    try
    {
        // Setup bone movement cbuffer
        HRESULT hr = m_finalTransformsCB.Initialize( pDevice, pContext );
        COM_ERROR_IF_FAILED( hr, "Failed to create ANIMATION constant buffer!" );
    }
    catch ( COMException& exception )
    {
        ErrorLogger::Log( exception );
        return;
    }

    m_vFinalTransforms.resize( m_skeletonData.BoneCount() );
    m_vFinalTransforms = m_skeletonData.GetFinalTransforms( m_sClipName, 0.0f );
    m_skeletonData.InverseKin( 18, XMFLOAT3( 0.0f, 0.0f, 0.0f ) );
}

AnimatedModel::~AnimatedModel()
{
    CleanUp();
}

void AnimatedModel::Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl, ConstantBuffer<MatrixBuffer>& buffer )
{
    if ( m_bDraw )
    {
        pContext->IASetInputLayout( shaderControl->GetShaderByName( "Animation" ).m_pVertexLayout );
        pContext->VSSetShader( shaderControl->GetShaderByName( "Animation" ).m_pVertexShader, nullptr, 0 );
        pContext->PSSetShader( shaderControl->GetShaderByName( "Animation" ).m_pPixelShader, nullptr, 0 );

        XMFLOAT4X4 WorldAsFloat = m_pTransformData->GetWorldMatrix();
        XMMATRIX mGO = XMLoadFloat4x4( &WorldAsFloat );
        buffer.data.mWorld = XMMatrixTranspose( mGO );
        if ( !buffer.ApplyChanges() )
            return;
        for ( size_t i = 0; i < m_vFinalTransforms.size(); i++ )
        {
            XMMATRIX mbone = XMLoadFloat4x4( &m_vFinalTransforms[i] );
            m_finalTransformsCB.data.m_mBoneTransforms[i] = XMMatrixTranspose( mbone );
        }

        if ( !m_finalTransformsCB.ApplyChanges() )
            return;
        pContext->VSSetConstantBuffers( 5, 1, m_finalTransformsCB.GetAddressOf() );

        for ( auto& subset : m_vSubsets )
        {
            pContext->PSSetShaderResources( 0, 1, &m_pTextureResourceView[subset.m_uId] );
            pContext->PSSetShaderResources( 1, 1, &m_pNormalMapResourceView[subset.m_uId] );
            m_pAppearance->Draw( pContext, subset.m_uFaceCount * 3, subset.m_uFaceStart * 3 );
        }
    }
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
}

void AnimatedModel::ProcessMesh( aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix, std::vector<SkinnedVertex>& verts, std::vector<WORD>& index )
{
    std::vector<SkinnedVertex> vertices;
    std::vector<WORD> indices;

    // Get vertices
    for ( UINT i = 0u; i < mesh->mNumVertices; i++ )
    {
        SkinnedVertex vertex;
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

void AnimatedModel::ProcessNode( aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix, std::vector<SkinnedVertex>& verts, std::vector<WORD>& index )
{
    XMMATRIX nodeTransformMatrix = XMMatrixTranspose( static_cast<XMMATRIX>( &node->mTransformation.a1 ) ) * parentTransformMatrix;

    for ( UINT i = 0u; i < node->mNumMeshes; i++ )
    {
        std::vector<SkinnedVertex> vertices;
        std::vector<WORD> indices;
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh( mesh, scene, nodeTransformMatrix, vertices, indices );
        verts.insert( verts.end(), vertices.begin(), vertices.end() );
        index.insert( index.end(), indices.begin(), indices.end() );
    }

    for ( UINT i = 0u; i < node->mNumChildren; i++ )
    {
        std::vector<SkinnedVertex> vertices;
        std::vector<WORD> indices;
        ProcessNode( node->mChildren[i], scene, nodeTransformMatrix, vertices, indices );
        verts.insert( verts.end(), vertices.begin(), vertices.end() );
        index.insert( index.end(), indices.begin(), indices.end() );
    }
}