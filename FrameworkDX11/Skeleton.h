#pragma once
#include<vector>
#include<map>
#include<string>

#include"DataStucts.h"
#include"Bone.h"
/// <summary>
/// bones 
/// animations
/// </summary>
class Skeleton
{
public:
    Skeleton();
    ~Skeleton();

    void Set(
        std::vector<int>& boneHierarchy,
        std::vector<XMFLOAT4X4>& boneOffsets,
        std::map<std::string, AnimationClip>& animations);

    
    std::vector<XMFLOAT4X4> GetFinalTransforms(const std::string& clipName, float timePos);
    
    std::vector<std::string> mAnimationClips();

    float GetClipStartTime(const std::string ClipName) {
       return _Animations.find(ClipName)->second.GetClipStartTime();
    }

    float GetClipEndTime(const std::string ClipName) {
        return _Animations.find(ClipName)->second.GetClipEndTime();
    }

    int BoneCount() { return _BoneData.size(); }
    std::vector<Bone*> GetBoneData() { return _BoneData; } 
    void InverKin(int Endfectro, XMFLOAT3 Tatget);
    void CleanUp();
    void RebuildBindPose();


    void SetBonePosition(int Bone, XMFLOAT3 Pos) {
       _BoneData[Bone]->SetPosition(Pos);
       if (_BoneData[Bone]->Getparent() > -1) {
           XMMATRIX world = XMMatrixMultiply(XMLoadFloat4x4(_BoneData[Bone]->GetWorld()), XMLoadFloat4x4(_BoneData[_BoneData[Bone]->Getparent()]->GetWorld()));
           XMFLOAT4X4 worldFloat;
           XMStoreFloat4x4(&worldFloat, world);
           _BoneData[Bone]->SetWorld(worldFloat);
       }
      SetChild(Bone);
    }
    void SetBoneScale(int Bone, XMFLOAT3 Scale) {
        _BoneData[Bone]->SetScale(Scale);
        if (_BoneData[Bone]->Getparent() > -1) {
            XMMATRIX world = XMMatrixMultiply(XMLoadFloat4x4(_BoneData[Bone]->GetWorld()), XMLoadFloat4x4(_BoneData[_BoneData[Bone]->Getparent()]->GetWorld()));
            XMFLOAT4X4 worldFloat;
            XMStoreFloat4x4(&worldFloat, world);
            _BoneData[Bone]->SetWorld(worldFloat);
        }
        SetChild(Bone);
    }
    void SetBoneRotQuat(int Bone, XMFLOAT4 RotQuat) {
        _BoneData[Bone]->SetRotation(RotQuat);
        if (_BoneData[Bone]->Getparent() > -1) {
            XMMATRIX world = XMMatrixMultiply(XMLoadFloat4x4(_BoneData[Bone]->GetWorld()), XMLoadFloat4x4(_BoneData[_BoneData[Bone]->Getparent()]->GetWorld()));
            XMFLOAT4X4 worldFloat;
            XMStoreFloat4x4(&worldFloat, world);
            _BoneData[Bone]->SetWorld(worldFloat);
        }
        SetChild(Bone);
    }
private:
   
    void SetChild(int Bone);
   
    XMFLOAT3 Rotate(XMFLOAT4 rotquat, int Currjoint);
    XMFLOAT3 CalculateError(XMFLOAT3 endEffector, XMFLOAT3 Target);
    

    std::map<std::string, AnimationClip> _Animations;
    std::vector<int> _BoneHierarchy;
    std::vector<Bone*> _BoneData;

};

