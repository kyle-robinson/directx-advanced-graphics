#pragma once
#ifndef ANIMATEDMODEL_H
#define ANIMATEDMODEL_H

#include "Skeleton.h"
#include "M3dLoader.h"
#include "Transform.h"
#include "DataStucts.h"
#include "Appearance.h"
#include "ConstantBuffer.h"
#include "ShaderController.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class AnimatedModel
{
public:
    AnimatedModel( std::string modelFile, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ShaderController* shaderControl );
    ~AnimatedModel();

    void Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl, ConstantBuffer<MatrixBuffer>& buffer );
    void Update( float dt );

    inline Skeleton* GetSkeleton() noexcept { return &m_skeletonData; }

    inline void SetAnimation( std::string clipName )
    {
        if ( m_sClipName != clipName )
        {
            m_sPrevClipName = m_sClipName;
            m_fPrevTime = m_fTimePos;
            m_sClipName = clipName;
            m_fTimePos = 0;
        }
    }

    inline Transform* GetTransformData() const noexcept { return m_pTransformData; }
    inline std::string GetModelName() const noexcept { return m_sModelName; }
    inline std::vector<Subset> GetSubsets() const noexcept { return m_vSubsets; }
    inline std::vector<M3dMaterial> GetMaterialData() const noexcept { return m_vMat; }
    inline std::string GetClipName() const noexcept { return m_sClipName; }
    inline float GetTimePos() const noexcept { return m_fTimePos; }

    inline bool GetIsLoop() const noexcept { return m_bLoopAnimation; }
    inline void SetIsLoop( bool loop ) noexcept { m_bLoopAnimation = loop; }

    inline void SetTimePos( float time )
    {
        m_fTimePos = time;
        if ( m_skeletonData.GetClipEndTime( m_sClipName ) < m_fTimePos )
        {
            m_fTimePos = m_skeletonData.GetClipEndTime( m_sClipName );
        }
        if ( m_skeletonData.GetClipStartTime( m_sClipName ) > m_fTimePos )
        {
            m_fTimePos = 0;
        }
    }

private:
    void ProcessMesh( aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix, std::vector<SkinnedVertex>& verts, std::vector<WORD>& index );
    void ProcessNode( aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix, std::vector<SkinnedVertex>& verts, std::vector<WORD>& index );
    void CleanUp();

    Appearance* m_pAppearance;
    std::string m_sModelName;

    std::vector<WORD> m_vIndex;
    std::vector<WORD> m_vIndex2;
    std::vector<Subset> m_vSubsets;
    std::vector<M3dMaterial> m_vMat;
    std::vector<SkinnedVertex> m_vSkinVert;

    Skeleton m_skeletonData;
    Transform* m_pTransformData;
    std::vector<XMFLOAT4X4> m_vFinalTransforms;
    ConstantBuffer<SkinnedCB> m_finalTransformsCB;

    std::vector<ID3D11ShaderResourceView*> m_pTextureResourceView;
    std::vector<ID3D11ShaderResourceView*> m_pNormalMapResourceView;

    float m_fTimePos = 0.0f;
    bool m_bLoopAnimation = false;
    std::string m_sClipName = "BindPose";

    float m_fPrevTime;
    std::string m_sPrevClipName = "BindPose";
};

#endif