#pragma once
#include<d3d11.h>
#include<DirectXMath.h>
#include<vector>
#include<cstddef>
using namespace DirectX;


typedef unsigned short USHORT;
typedef unsigned int UINT;

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
	Keyframe();
	~Keyframe();

	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};


///<summary>
/// A BoneAnimation is defined by a list of keyframes.
///</summary>
struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

	void Interpolate(float t, XMFLOAT4X4& M)const;

	std::vector<Keyframe> Keyframes;

};

///<summary>
/// animation clip data  
///</summary>
struct AnimationClip
{
	float GetClipStartTime()const;
	float GetClipEndTime()const;

	void Interpolate(float t, std::vector<XMFLOAT4X4>& boneTransforms)const;

	std::vector<BoneAnimation> BoneAnimations;
};

struct Subset
{
	Subset() :
		Id(-1),
		VertexStart(0), VertexCount(0),
		FaceStart(0), FaceCount(0)
	{
	}
	UINT Id;
	UINT VertexStart;
	UINT VertexCount;
	UINT FaceStart;
	UINT FaceCount;

	
};

//vertex Data
typedef unsigned char BYTE;
struct SkinedVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Tex;
	XMFLOAT4 Tangent;
	XMFLOAT3 Weights;
	XMFLOAT4 BoneIndices;
};

struct cbSkinned
{
	XMMATRIX gBoneTransforms[96];
};