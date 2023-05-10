#include "stdafx.h"
#include "Bone.h"

Bone::Bone( XMFLOAT4X4 offSet, int parent ) : m_mOffset( offSet ), m_iParent( parent )
{
    XMVECTOR pos, rot, scale;
    XMMATRIX offsetMat = XMLoadFloat4x4( &offSet );
    XMMatrixDecompose( &scale, &rot, &pos, offsetMat );

    XMStoreFloat4( &m_fScaleOffset, scale );
    XMStoreFloat4( &m_fRotQuatOffset, rot );
    XMStoreFloat4( &m_fPosOffset, pos );

    XMMATRIX worldJoint = XMMatrixInverse( nullptr, offsetMat );
    XMStoreFloat4x4( &m_mWorld, worldJoint );
    XMMatrixDecompose( &scale, &rot, &pos, worldJoint );

    XMStoreFloat4( &m_fScaleWorld, scale );
    XMStoreFloat4( &m_fRotQuatWorld, rot );
    XMStoreFloat4( &m_fPosWorld, pos );
}

Bone::~Bone()
{
}

void Bone::SetPosition( XMFLOAT3 pos )
{
    m_fPosWorld = XMFLOAT4( pos.x, pos.y, pos.z, 1 );
    SetWorld();
}

void Bone::SetRotation( XMFLOAT4 rotQuat )
{
    m_fRotQuatWorld = rotQuat;
    SetWorld();
}

void Bone::SetScale( XMFLOAT3 scale )
{
    m_fScaleWorld = XMFLOAT4( scale.x, scale.y, scale.z, 1 );
    SetWorld();
}

void Bone::SetOffset()
{
    XMMATRIX worldMat = XMLoadFloat4x4( &m_mWorld );
    XMStoreFloat4x4( &m_mOffset, XMMatrixInverse( nullptr, worldMat ) );
}

void Bone::SetWorld()
{
    XMFLOAT3 pos = XMFLOAT3( m_fPosWorld.x, m_fPosWorld.y, m_fPosWorld.z );
    XMFLOAT3 scale = XMFLOAT3( m_fScaleWorld.x, m_fScaleWorld.y, m_fScaleWorld.z );
    XMVECTOR S = XMLoadFloat3( &scale );
    XMVECTOR P = XMLoadFloat3( &pos );
    XMVECTOR Q = XMLoadFloat4( &m_fRotQuatWorld );

    XMVECTOR zero = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
    XMStoreFloat4x4( &m_mWorld, XMMatrixAffineTransformation( S, zero, Q, P ) );
    SetOffset();
}