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

    inline XMFLOAT4X4* GetOffSet() noexcept { return &_OffSet; }
    inline XMFLOAT4 GetOffSetPos() const noexcept { return _PosOffSet; }
    inline XMFLOAT4 GetOffSetRotation() const noexcept { return _RotQuatOffSet; }
    inline XMFLOAT4 GetOffSetScale() const noexcept { return _ScaleOffSet; }

    inline XMFLOAT4X4* GetWorld() noexcept { return &_World; }
    inline XMFLOAT4 GetWorldPos() const noexcept { return _PosWorld; }
    inline XMFLOAT4 GetWorldRot() const noexcept { return _RotQuatWorld; }
    inline XMFLOAT4 GetWorldScale() const noexcept { return _ScaleWorld; }
  
    inline void SetWorld( XMFLOAT4X4 world ) noexcept {
        _World = world;
        SetOffSet();
    }
    inline XMFLOAT4X4* GetReal() noexcept { return &_Real; }
    inline XMFLOAT3 GetRealPos() const noexcept { return _PosReal; }
    inline XMFLOAT4 GetRealRot() const noexcept { return _RotQuatReal; }
    inline XMFLOAT3 GetRealScale() const noexcept { return _ScaleReal; }

    inline void SetReal( XMFLOAT4X4 RealMat ) noexcept {  _Real= RealMat; }
    inline void SetRealPos( XMFLOAT3 pos ) noexcept {  _PosReal= pos; }
    inline void SetRealRot( XMFLOAT4 RotQuat ) noexcept {  _RotQuatReal = RotQuat; }
    inline void SetRealScale( XMFLOAT3 Scale ) noexcept {  _ScaleReal = Scale; }

    inline int GetParent() const noexcept { return _Parent; }
    inline std::vector<int> const GetChild() {
        if ( _Child.size() != 0 ) {
            return _Child;
        }
        else {
            return  std::vector<int>( 1, -1 );
        }
    }

    inline void AddChild( int Child ) noexcept { _Child.push_back( Child ); }

    void SetPosition( XMFLOAT3 pos );
    void SetRotation( XMFLOAT4 rotQuat );
    void SetScale( XMFLOAT3 scale );

private:
    void SetOffSet();
    void SetWorld();

    XMFLOAT4X4 _OffSet = XMFLOAT4X4();
    XMFLOAT4 _PosOffSet = XMFLOAT4();
    XMFLOAT4 _RotQuatOffSet = XMFLOAT4();
    XMFLOAT4 _ScaleOffSet = XMFLOAT4();

    XMFLOAT4X4 _World = XMFLOAT4X4();
    XMFLOAT4 _PosWorld = XMFLOAT4();
    XMFLOAT4 _RotQuatWorld = XMFLOAT4();
    XMFLOAT4 _ScaleWorld = XMFLOAT4();

    XMFLOAT4X4 _Real = XMFLOAT4X4();
    XMFLOAT3 _PosReal = XMFLOAT3();
    XMFLOAT4 _RotQuatReal = XMFLOAT4();
    XMFLOAT3 _ScaleReal = XMFLOAT3();
    int _Parent;
    std::vector<int> _Child;
};

#endif