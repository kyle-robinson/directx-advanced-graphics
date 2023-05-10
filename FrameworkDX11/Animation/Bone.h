#pragma once
#ifndef BONE_H
#define BONE_H

#include <DirectXMath.h>
#include <vector>

class Bone
{
public:
    Bone( XMFLOAT4X4 offSet, int parent );
    ~Bone();

    inline XMFLOAT4X4* GetOffset() noexcept { return &m_mOffset; }
    inline XMFLOAT4 GetOffsetPos() const noexcept { return m_fPosOffset; }
    inline XMFLOAT4 GetOffsetRotation() const noexcept { return m_fRotQuatOffset; }
    inline XMFLOAT4 GetOffsetScale() const noexcept { return m_fScaleOffset; }

    inline XMFLOAT4X4* GetWorld() noexcept { return &m_mWorld; }
    inline XMFLOAT4 GetWorldPos() const noexcept { return m_fPosWorld; }
    inline XMFLOAT4 GetWorldRot() const noexcept { return m_fRotQuatWorld; }
    inline XMFLOAT4 GetWorldScale() const noexcept { return m_fScaleWorld; }

    inline void SetWorld( XMFLOAT4X4 world ) noexcept {
        m_mWorld = world;
        SetOffset();
    }
    inline XMFLOAT4X4* GetReal() noexcept { return &m_mReal; }
    inline XMFLOAT3 GetRealPos() const noexcept { return m_fPosReal; }
    inline XMFLOAT4 GetRealRot() const noexcept { return m_fRotQuatReal; }
    inline XMFLOAT3 GetRealScale() const noexcept { return m_fScaleReal; }

    inline void SetReal( XMFLOAT4X4 RealMat ) noexcept { m_mReal= RealMat; }
    inline void SetRealPos( XMFLOAT3 pos ) noexcept { m_fPosReal= pos; }
    inline void SetRealRot( XMFLOAT4 RotQuat ) noexcept { m_fRotQuatReal = RotQuat; }
    inline void SetRealScale( XMFLOAT3 Scale ) noexcept { m_fScaleReal = Scale; }

    inline int GetParent() const noexcept { return m_iParent; }
    inline std::vector<int> const GetChild()
    {
        if ( m_vChildren.size() != 0 )
            return m_vChildren;
        else
            return  std::vector<int>( 1, -1 );
    }

    inline void AddChild( int child ) noexcept { m_vChildren.push_back( child ); }

    void SetPosition( XMFLOAT3 pos );
    void SetRotation( XMFLOAT4 rotQuat );
    void SetScale( XMFLOAT3 scale );

private:
    void SetOffset();
    void SetWorld();

    XMFLOAT4X4 m_mOffset = XMFLOAT4X4();
    XMFLOAT4 m_fPosOffset = XMFLOAT4();
    XMFLOAT4 m_fRotQuatOffset = XMFLOAT4();
    XMFLOAT4 m_fScaleOffset = XMFLOAT4();

    XMFLOAT4X4 m_mWorld = XMFLOAT4X4();
    XMFLOAT4 m_fPosWorld = XMFLOAT4();
    XMFLOAT4 m_fRotQuatWorld = XMFLOAT4();
    XMFLOAT4 m_fScaleWorld = XMFLOAT4();

    XMFLOAT4X4 m_mReal = XMFLOAT4X4();
    XMFLOAT3 m_fPosReal = XMFLOAT3();
    XMFLOAT4 m_fRotQuatReal = XMFLOAT4();
    XMFLOAT3 m_fScaleReal = XMFLOAT3();

    std::vector<int> m_vChildren;
    int m_iParent;
};

#endif