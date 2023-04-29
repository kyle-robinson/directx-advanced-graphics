#include "stdafx.h"
#include "DataStucts.h"

template<typename T>
static T Min( const T& a, const T& b )
{
	return a < b ? a : b;
}

template<typename T>
static T Max( const T& a, const T& b )
{
	return a > b ? a : b;
}

Keyframe::Keyframe() :
	m_fTimePos( 0.0f ),
	m_fTranslation( 0.0f, 0.0f, 0.0f ),
	m_fScale( 1.0f, 1.0f, 1.0f ),
	m_fRotationQuat( 0.0f, 0.0f, 0.0f, 1.0f )
{}

Keyframe::~Keyframe() {}

float BoneAnimation::GetStartTime() const
{
	// Keyframes are sorted by time, so the first keyframe gives the start time.
	return m_vKeyframes.front().m_fTimePos;
}

float BoneAnimation::GetEndTime()const
{
	// Keyframes are sorted by time, so last keyframe gives the end time.
	float f = m_vKeyframes.back().m_fTimePos;
	return f;
}

/// <summary>
/// Interpolate between keyframes
/// </summary>
void BoneAnimation::Interpolate( float t, XMFLOAT4X4& mat ) const
{
	if ( t <= m_vKeyframes.front().m_fTimePos )
	{
		XMVECTOR S = XMLoadFloat3( &m_vKeyframes.front().m_fScale );
		XMVECTOR P = XMLoadFloat3( &m_vKeyframes.front().m_fTranslation );
		XMVECTOR Q = XMLoadFloat4( &m_vKeyframes.front().m_fRotationQuat );
		XMVECTOR zero = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
		XMStoreFloat4x4( &mat, XMMatrixAffineTransformation( S, zero, Q, P ) );
	}
	else if ( t >= m_vKeyframes.back().m_fTimePos )
	{
		XMVECTOR S = XMLoadFloat3( &m_vKeyframes.back().m_fScale );
		XMVECTOR P = XMLoadFloat3( &m_vKeyframes.back().m_fTranslation );
		XMVECTOR Q = XMLoadFloat4( &m_vKeyframes.back().m_fRotationQuat );
		XMVECTOR zero = XMVectorSet( 0.0f, 0.0f, 0.0f, -1.0f );
		XMStoreFloat4x4( &mat, XMMatrixAffineTransformation( S, zero, Q, P ) );
	}
	else
	{
		for ( UINT i = 0; i < m_vKeyframes.size() - 1; ++i )
		{
			if ( t >= m_vKeyframes[i].m_fTimePos && t <= m_vKeyframes[i + 1].m_fTimePos )
			{
				float lerpPercent = ( t - m_vKeyframes[i].m_fTimePos ) / ( m_vKeyframes[i + 1].m_fTimePos - m_vKeyframes[i].m_fTimePos );

				XMVECTOR s0 = XMLoadFloat3( &m_vKeyframes[i].m_fScale );
				XMVECTOR s1 = XMLoadFloat3( &m_vKeyframes[i + 1].m_fScale );

				XMVECTOR p0 = XMLoadFloat3( &m_vKeyframes[i].m_fTranslation );
				XMVECTOR p1 = XMLoadFloat3( &m_vKeyframes[i + 1].m_fTranslation );

				XMVECTOR q0 = XMLoadFloat4( &m_vKeyframes[i].m_fRotationQuat );
				XMVECTOR q1 = XMLoadFloat4( &m_vKeyframes[i + 1].m_fRotationQuat );

				XMVECTOR S = XMVectorLerp( s0, s1, lerpPercent );
				XMVECTOR P = XMVectorLerp( p0, p1, lerpPercent );
				XMVECTOR Q = XMQuaternionSlerp( q0, q1, lerpPercent );

				XMVECTOR zero = XMVectorSet( 0.0f, 0.0f, 0.0f, 1.0f );
				XMStoreFloat4x4( &mat, XMMatrixAffineTransformation( S, zero, Q, P ) );

				break;
			}
		}
	}
}

float AnimationClip::GetClipStartTime()const
{
	// Find the smallest start time over all bones in this clip.
	float t = FLT_MAX;
	for ( UINT i = 0; i < m_vBoneAnimations.size(); ++i )
	{
		t = Min( t, m_vBoneAnimations[i].GetStartTime() );
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	// Find the largest end time over all bones in this clip.
	float t = 0.0f;
	for ( UINT i = 0; i < m_vBoneAnimations.size(); ++i )
	{
		t = Max( t, m_vBoneAnimations[i].GetEndTime() );
	}

	return t;
}

void AnimationClip::Interpolate( float t, std::vector<XMFLOAT4X4>& boneTransforms )const
{
	for ( UINT i = 0; i < m_vBoneAnimations.size(); ++i )
	{
		m_vBoneAnimations[i].Interpolate( t, boneTransforms[i] );
	}
}