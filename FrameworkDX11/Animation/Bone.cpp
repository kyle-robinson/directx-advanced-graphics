#include "Bone.h"

Bone::Bone(XMFLOAT4X4 offSet, int parent):
    _OffSet(offSet),
    _Parent(parent)
{
    XMVECTOR scalevec;
    XMVECTOR rot;
    XMVECTOR pos;
    XMMATRIX offsetMat = XMLoadFloat4x4(&offSet);
    XMMatrixDecompose(&scalevec, &rot, &pos, offsetMat);
    
    XMStoreFloat4(&_ScaleOffSet,scalevec);
    XMStoreFloat4(&_RotQuatOffSet,rot);
    XMStoreFloat4(&_PosOffSet,pos);

    XMMATRIX worldJoint = XMMatrixInverse(nullptr, offsetMat);
    XMStoreFloat4x4(&_World, worldJoint);
    XMMatrixDecompose(&scalevec, &rot, &pos, worldJoint);

    XMStoreFloat4(&_ScaleWorld, scalevec);
    XMStoreFloat4(&_RotQuatWorld, rot);
    XMStoreFloat4(&_PosWorld, pos);

    
}

Bone::~Bone()
{
}

void Bone::SetPosition(XMFLOAT3 pos)
{
    _PosWorld = XMFLOAT4(pos.x, pos.y, pos.z, 1);
    SetWorld();
}

void Bone::SetRotation(XMFLOAT4 rotQuat)
{
    _RotQuatWorld = rotQuat;
    SetWorld();
}

void Bone::SetScale(XMFLOAT3 scale)
{
    _ScaleWorld = XMFLOAT4(scale.x, scale.y, scale.z, 1);
    SetWorld();
}

void Bone::SetOffSet()
{
    XMMATRIX worldMat = XMLoadFloat4x4(&_World);
    XMStoreFloat4x4(&_OffSet, XMMatrixInverse(nullptr, worldMat));
}

void Bone::SetWorld()
{
    XMFLOAT3 pos = XMFLOAT3(_PosWorld.x, _PosWorld.y, _PosWorld.z);
    XMFLOAT3 scale = XMFLOAT3(_ScaleWorld.x, _ScaleWorld.y, _ScaleWorld.z);
    XMVECTOR S = XMLoadFloat3(&scale);
    XMVECTOR P = XMLoadFloat3(&pos);
    XMVECTOR Q = XMLoadFloat4(&_RotQuatWorld);

    XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMStoreFloat4x4(&_World, XMMatrixAffineTransformation(S, zero, Q, P));
    SetOffSet();
    
}
