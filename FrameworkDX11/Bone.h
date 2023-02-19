#pragma once
#include<DirectXMath.h>
#include<vector>
using namespace DirectX;
/// <summary>
/// all of a bones data
/// </summary>
class Bone
{
public:
    Bone(XMFLOAT4X4 offSet, int parent);
    ~Bone();

    XMFLOAT4X4* GetOffSet() { return &_OffSet; }
    XMFLOAT4 GetOffSetPos() { return _PosOffSet; }
    XMFLOAT4 GetOffSetRot() { return _RotQuatOffSet; }
    XMFLOAT4 GetOffSetScale() { return _ScaleOffSet; }
    

    XMFLOAT4X4* GetWorld() { return &_World; }
    XMFLOAT4 GetWorldPos() { return _PosWorld; }
    XMFLOAT4 GetWorldRot() { return _RotQuatWorld; }
    XMFLOAT4 GetWorldScale() { return _ScaleWorld; }
  
    void SetWorld(XMFLOAT4X4 world) {
        _World = world;
        SetOffSet();
    }
    XMFLOAT4X4* GetReal() { return &_Real; }
    XMFLOAT3 GetRealPos() { return _PosReal; }
    XMFLOAT4 GetRealRot() { return _RotQuatReal; }
    XMFLOAT3 GetRealScale() { return _ScaleReal; }


    void SetReal(XMFLOAT4X4 RealMat) {  _Real= RealMat; }
    void SetRealPos(XMFLOAT3 pos) {  _PosReal= pos; }
    void SetRealRot(XMFLOAT4 RotQuat) {  _RotQuatReal= RotQuat; }
    void SetRealScale(XMFLOAT3 Scale) {  _ScaleReal= Scale; }

    int Getparent() { return _Parent; }
    std::vector<int> GetChild() {
        if (_Child.size() != 0) {
            return _Child;
        }
        else {
            return  std::vector<int>(1, -1);
        }
    }

    void AddChild(int Child) { _Child.push_back(Child); }

    void SetPosition(XMFLOAT3 pos);
    void SetRotation(XMFLOAT4 rotQuat);
    void SetScale(XMFLOAT3 scale);

private:
    void SetOffSet();
    void SetWorld();

    XMFLOAT4X4 _OffSet=XMFLOAT4X4();
    XMFLOAT4 _PosOffSet= XMFLOAT4();
    XMFLOAT4 _RotQuatOffSet= XMFLOAT4();
    XMFLOAT4 _ScaleOffSet= XMFLOAT4();

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

