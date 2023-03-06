#pragma once
#ifndef SKELETON_H
#define SKELETON_H

#include"Bone.h"
#include"DataStucts.h"

class Skeleton
{
public:
    Skeleton();
    ~Skeleton();

    void Set(
        std::vector<int>& boneHierarchy,
        std::vector<XMFLOAT4X4>& boneOffsets,
        std::map<std::string, AnimationClip>& animations );

    std::vector<XMFLOAT4X4> GetFinalTransforms( const std::string& clipName, float timePos );
    std::vector<std::string> AnimationClips();

    float GetClipStartTime( const std::string clipName )
    {
        return m_mAnimations.find( clipName )->second.GetClipStartTime();
    }
    float GetClipEndTime( const std::string clipName )
    {
        return m_mAnimations.find( clipName )->second.GetClipEndTime();
    }

    inline std::vector<Bone*> GetBoneData() const noexcept{ return m_vBoneData; }
    inline int BoneCount() const noexcept { return m_vBoneData.size(); }

    void CleanUp();
    void RebuildPose();

    inline void SetBonePosition( int Bone, XMFLOAT3 Pos )
    {
        m_vBoneData[Bone]->SetPosition( Pos );
        if ( m_vBoneData[Bone]->Getparent() > -1 )
        {
            XMMATRIX world = XMMatrixMultiply( XMLoadFloat4x4( m_vBoneData[Bone]->GetWorld() ), XMLoadFloat4x4( m_vBoneData[m_vBoneData[Bone]->Getparent()]->GetWorld() ) );
            XMFLOAT4X4 worldFloat;
            XMStoreFloat4x4( &worldFloat, world );
            m_vBoneData[Bone]->SetWorld( worldFloat );
        }
        SetChild( Bone );
    }

    inline void SetBoneScale( int Bone, XMFLOAT3 Scale )
    {
        m_vBoneData[Bone]->SetScale( Scale );
        if ( m_vBoneData[Bone]->Getparent() > -1 )
        {
            XMMATRIX world = XMMatrixMultiply( XMLoadFloat4x4( m_vBoneData[Bone]->GetWorld() ), XMLoadFloat4x4( m_vBoneData[m_vBoneData[Bone]->Getparent()]->GetWorld() ) );
            XMFLOAT4X4 worldFloat;
            XMStoreFloat4x4( &worldFloat, world );
            m_vBoneData[Bone]->SetWorld( worldFloat );
        }
        SetChild( Bone );
    }

    inline void SetBoneRotQuat( int bone, XMFLOAT4 qRot )
    {
        m_vBoneData[bone]->SetRotation( qRot );
        if ( m_vBoneData[bone]->Getparent() > -1 )
        {
            XMMATRIX world = XMMatrixMultiply(
                XMLoadFloat4x4( m_vBoneData[bone]->GetWorld() ),
                XMLoadFloat4x4( m_vBoneData[m_vBoneData[bone]->Getparent()]->GetWorld() ) );
            XMFLOAT4X4 worldFloat;
            XMStoreFloat4x4( &worldFloat, world );
            m_vBoneData[bone]->SetWorld( worldFloat );
        }
        SetChild( bone );
    }

private:
    void SetChild( int Bone );
    XMFLOAT3 CalculateError( XMFLOAT3 endEffector, XMFLOAT3 target );
    inline XMFLOAT3 Rotate( XMFLOAT4 qRot, int currJoint )const noexcept { return XMFLOAT3(); }

    std::vector<Bone*> m_vBoneData;
    std::vector<int> m_vBoneHierarchy;
    std::map<std::string, AnimationClip> m_mAnimations;
};

#endif