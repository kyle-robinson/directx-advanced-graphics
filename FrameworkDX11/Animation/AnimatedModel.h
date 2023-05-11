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

/* REFERENCE: Frank Luna, Introduction to 3D Game Programming with DirectX 11, Chapter 25 Character Animation */

class AnimatedModel
{
public:
    AnimatedModel( std::string modelFile, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ShaderController* shaderControl );
    ~AnimatedModel();

    void Draw( ID3D11DeviceContext* pContext, ShaderController* shaderControl, ConstantBuffer<MatrixBuffer>& buffer );
    void Update( float dt );

    inline Skeleton* GetSkeleton() noexcept { return &m_skeleton; }
    inline Transform* GetTransform() const noexcept { return m_pTransform.get(); }
    inline std::string GetModelName() const noexcept { return m_sModelName; }
    inline std::vector<Subset> GetSubsets() const noexcept { return m_vSubsets; }
    inline std::vector<M3dMaterial> GetMaterialData() const noexcept { return m_vMat; }
    inline std::string GetClipName() const noexcept { return m_sClipName; }

    inline bool GetIsLoop() const noexcept { return m_bLoopAnimation; }
    inline void SetIsLoop( bool loop ) noexcept { m_bLoopAnimation = loop; }

    inline float GetTimePos() const noexcept { return m_fTimePos; }
    inline void SetTimePos( float time )
    {
        m_fTimePos = time;
        if ( m_skeleton.GetClipEndTime( m_sClipName ) < m_fTimePos )
        {
            m_fTimePos = m_skeleton.GetClipEndTime( m_sClipName );
        }
        if ( m_skeleton.GetClipStartTime( m_sClipName ) > m_fTimePos )
        {
            m_fTimePos = 0;
            m_fTimePos = 0;
        }
    }

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

	inline void Show() noexcept { m_bDraw = true; }
	inline void Hide() noexcept { m_bDraw = false; }
	inline bool IsVisible() const noexcept { return m_bDraw; }

private:
    void ProcessMesh( aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix, std::vector<SkinnedVertex>& verts, std::vector<WORD>& index );
    void ProcessNode( aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix, std::vector<SkinnedVertex>& verts, std::vector<WORD>& index );
    void CleanUp();

    float m_fTimePos = 0.0f;
    bool m_bLoopAnimation = false;
    std::string m_sClipName = "T-Pose";

    float m_fPrevTime;
    std::string m_sPrevClipName = "T-Pose";

    bool m_bDraw = false;
    std::string m_sModelName;
    std::unique_ptr<Appearance> m_pAppearance;

    std::vector<WORD> m_vIndex;
    std::vector<WORD> m_vIndex2;
    std::vector<Subset> m_vSubsets;
    std::vector<M3dMaterial> m_vMat;
    std::vector<SkinnedVertex> m_vSkinVert;

    Skeleton m_skeleton;
    std::unique_ptr<Transform> m_pTransform;
    std::vector<XMFLOAT4X4> m_vFinalTransforms;
    ConstantBuffer<SkinnedCB> m_finalTransformsCB;

    std::vector<ID3D11ShaderResourceView*> m_pTextureResourceView;
    std::vector<ID3D11ShaderResourceView*> m_pNormalMapResourceView;
};

#endif