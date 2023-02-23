#pragma once
#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

using namespace DirectX;
#include <DirectXMath.h>
#include <vector>

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
	Keyframe();
	~Keyframe();

	float m_fTimePos;
	XMFLOAT3 m_fTranslation;
	XMFLOAT3 m_fScale;
	XMFLOAT4 m_fRotationQuat;
};

///<summary>
/// A BoneAnimation is defined by a list of keyframes.
///</summary>
struct BoneAnimation
{
	float GetStartTime() const;
	float GetEndTime() const;
	void Interpolate( float t, XMFLOAT4X4& mat ) const;
	std::vector<Keyframe> m_vKeyframes;
};

/// <summary>
/// An AnimationClip is defined by a list of bone animations
/// </summary>
struct AnimationClip
{
	float GetClipStartTime() const;
	float GetClipEndTime() const;
	void Interpolate( float t, std::vector<XMFLOAT4X4>& boneTransforms ) const;
	std::vector<BoneAnimation> m_vBoneAnimations;
};

struct Subset
{
	Subset() :
		m_uId( -1u ),
		m_uVertexStart( 0u ),
		m_uVertexCount( 0u ),
		m_uFaceStart( 0u ),
		m_uFaceCount( 0u )
	{}

	UINT m_uId;
	UINT m_uVertexStart;
	UINT m_uVertexCount;
	UINT m_uFaceStart;
	UINT m_uFaceCount;
};

// Vertex data
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
	XMMATRIX m_mBoneTransforms[96];
};

#endif