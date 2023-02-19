#pragma once

#include"DataStucts.h"
#include"Appearance.h"
#include"M3dLoader.h"
#include"ShaderController.h"
#include"Transform.h"
#include"Skeleton.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

/// <summary>
/// controll all the data to animate a model
/// Skining
/// skelaton
/// </summary>
class AnimatedModel
{
public:
    AnimatedModel(std::string ModelFile, ID3D11Device* device, ID3D11DeviceContext* pImmediateContext, ShaderController* Controll);
    ~AnimatedModel();

    void Draw(ID3D11DeviceContext* pImmediateContext, ShaderController* Controll, ConstantBuffer* buffer, ID3D11Buffer* _pConstantBuffer);
    void Update(float dt);

    Skeleton* GetSkeleton() { return &SkeletonData; }


    void SetAnnimation(string ClipName) {

        if (_ClipName != ClipName) {
            _PrevClipName = _ClipName;
            prevTime = TimePos;
            _ClipName = ClipName;
            TimePos = 0;
        }
        
    }

    Transform* GetTransformData(){return TransformData;}


    std::string GetModelName() { return _ModelName; }
    std::vector<Subset> GetSubsets() { return _Subsets; }
    std::vector<M3dMaterial> GetMaterrialData(){ return _Mat; }

    std::string GetClipName() { 
        return _ClipName; 
    }
    float GetTimePos() { return TimePos; }


    bool GetIsLoop() { return isLoopAnimation; }
    void SetIsLoop(bool IsLoop) { isLoopAnimation = IsLoop; }

    void SetTimePos(float Time) { 
        TimePos = Time; 
        if (SkeletonData.GetClipEndTime(_ClipName) < TimePos) {
            TimePos = SkeletonData.GetClipEndTime(_ClipName);
        }
        if (SkeletonData.GetClipStartTime(_ClipName) > TimePos) {
            TimePos = 0;
        }
    }

private:
    void CleanUp();
    Appearance* _Apparance;

    std::string _ModelName;


    std::vector<SkinedVertex> _SkinVert;
    std::vector<USHORT> index;
    std::vector<USHORT> _Index2;
    std::vector<Subset> _Subsets;
    std::vector<M3dMaterial> _Mat;
   

    Skeleton SkeletonData;


    Transform* TransformData;
    std::vector<XMFLOAT4X4> FinalTransforms;
    cbSkinned data;
    ID3D11Buffer* FinalTransformsCB = nullptr;

   std::vector<ID3D11ShaderResourceView*> m_pTextureResourceView;
   std::vector<ID3D11ShaderResourceView*> m_pNormalMapResourceView;

   bool isLoopAnimation=false;
   float TimePos= 0.0f;
   std::string _ClipName = "BindPose";

   float prevTime;
   std::string _PrevClipName = "BindPose";




};

