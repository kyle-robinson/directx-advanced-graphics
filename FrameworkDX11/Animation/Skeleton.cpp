#include "stdafx.h"
#include "Skeleton.h"

Skeleton::Skeleton() {}

Skeleton::~Skeleton()
{
	CleanUp();
}

void Skeleton::Set( std::vector<int>& boneHierarchy, std::vector<XMFLOAT4X4>& boneOffsets, std::map<std::string, AnimationClip>& animations )
{
	m_mAnimations = animations;
	m_vBoneHierarchy = boneHierarchy;

	for ( size_t i = 0; i < boneOffsets.size(); i++ )
	{
		m_vBoneData.push_back( new Bone( boneOffsets[i], m_vBoneHierarchy[i] ) );
	}

	for ( size_t bone = 0; bone < boneHierarchy.size(); bone++ )
	{
		if ( boneHierarchy[bone] < 0 || boneHierarchy[bone] >= m_vBoneData.size() )
		{
			continue;
		}
		m_vBoneData[boneHierarchy[bone]]->AddChild( bone );
	}

	RebuildPose();
}

std::vector<XMFLOAT4X4> Skeleton::GetFinalTransforms( const std::string& clipName, float timePos )
{
	std::vector<XMFLOAT4X4> finalTransforms;
	finalTransforms.resize( m_vBoneData.size() );

	UINT numBones = m_vBoneData.size();
	std::vector<XMFLOAT4X4> toParentTransforms( numBones );

	// Interpolate all the bones of this clip at the given time instance.
	auto clip = m_mAnimations.find( clipName );
	clip->second.Interpolate( timePos, toParentTransforms );

	// Traverse the hierarchy and transform all the bones to the root space.
	std::vector<XMFLOAT4X4> toRootTransforms( numBones );
	toRootTransforms[0] = toParentTransforms[0];

	// Now find the toRootTransform of the children.
	for ( UINT i = 1; i < numBones; ++i )
	{
		XMMATRIX toParent = XMLoadFloat4x4( &toParentTransforms[i] );
		int parentIndex = m_vBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4( &toRootTransforms[parentIndex] );
		XMMATRIX toRoot = XMMatrixMultiply( toParent, parentToRoot );
		XMStoreFloat4x4( &toRootTransforms[i], toRoot );
	}

	// Move animation data into a useable transfrom space
	for ( UINT i = 0; i < numBones; ++i )
	{
		XMMATRIX offset = XMLoadFloat4x4( m_vBoneData[i]->GetOffSet() );
		XMMATRIX toRoot = XMLoadFloat4x4( &toRootTransforms[i] );
		XMStoreFloat4x4( &finalTransforms[i], XMMatrixMultiply( offset, toRoot ) );
	}

	return finalTransforms;
}

std::vector<std::string> Skeleton::AnimationClips()
{
	std::vector<std::string> animationNames;
	for ( auto animation : m_mAnimations )
	{
		animationNames.push_back( animation.first );
	}

	return animationNames;
}

void Skeleton::CleanUp()
{
	for ( size_t i = 0; i < m_vBoneData.size(); i++ )
	{
		if ( m_vBoneData[i] )
		{
			delete m_vBoneData[i];
			m_vBoneData[i] = nullptr;
		}
	}
	m_vBoneData.clear();
}

void Skeleton::RebuildPose()
{
	AnimationClip Clip;
	Clip.m_vBoneAnimations.resize( BoneCount() );

	for ( size_t i = 0; i < BoneCount(); i++ )
	{
		BoneAnimation BoneAnimation;
		BoneAnimation.m_vKeyframes.resize( 2 );

		Keyframe Frame;
		if ( i == 0 )
		{
			Frame.m_fTranslation= XMFLOAT3( m_vBoneData[i]->GetWorldPos().x, m_vBoneData[i]->GetWorldPos().y, m_vBoneData[i]->GetWorldPos().z );
			Frame.m_fScale = XMFLOAT3( m_vBoneData[i]->GetWorldScale().x, m_vBoneData[i]->GetWorldScale().y, m_vBoneData[i]->GetWorldScale().z );
			Frame.m_fRotationQuat = m_vBoneData[i]->GetWorldRot();
		}
		else
		{
			XMMATRIX RealMat = XMMatrixMultiply(
				XMLoadFloat4x4( m_vBoneData[i]->GetWorld() ),
				XMMatrixInverse( nullptr, XMLoadFloat4x4( m_vBoneData[m_vBoneData[i]->Getparent()]->GetWorld() ) ) );

			XMVECTOR scalevec, rot, pos;
			XMMatrixDecompose( &scalevec, &rot, &pos, RealMat );

			XMFLOAT4X4 real = XMFLOAT4X4();
			XMFLOAT4 posReal = XMFLOAT4();
			XMFLOAT4 rotQuatReal = XMFLOAT4();
			XMFLOAT4 scaleReal = XMFLOAT4();

			XMStoreFloat4( &scaleReal, scalevec );
			XMStoreFloat4( &rotQuatReal, rot );
			XMStoreFloat4( &posReal, pos );
			XMStoreFloat4x4( &real, RealMat );
			Frame.m_fTranslation = XMFLOAT3( posReal.x, posReal.y, posReal.z );
			Frame.m_fScale = XMFLOAT3( scaleReal.x, scaleReal.y, scaleReal.z );
			Frame.m_fRotationQuat = rotQuatReal;
			m_vBoneData[i]->SetReal( real );
		}

		m_vBoneData[i]->SetRealPos( Frame.m_fTranslation );
		m_vBoneData[i]->SetRealRot( Frame.m_fRotationQuat );
		m_vBoneData[i]->SetRealScale( Frame.m_fScale );

		Frame.m_fTimePos = 0;
		BoneAnimation.m_vKeyframes[0] = Frame;
		Frame.m_fTimePos = 1;
		BoneAnimation.m_vKeyframes[1] = Frame;
		Clip.m_vBoneAnimations[i] = BoneAnimation;
	}

	m_mAnimations["T-Pose"] = Clip;
}

void Skeleton::SetChild( int bone )
{
	if ( m_vBoneData[bone]->GetChild()[0] == -1 )
	{
		return;
	}
	else
	{
		for ( size_t i = 0; i < m_vBoneData[bone]->GetChild().size(); i++ )
		{
			XMMATRIX world = XMMatrixMultiply(
				XMLoadFloat4x4( m_vBoneData[m_vBoneData[bone]->GetChild()[i]]->GetReal() ),
				XMLoadFloat4x4( m_vBoneData[bone]->GetWorld() ) );
			XMFLOAT4X4 worldFloat;
			XMStoreFloat4x4( &worldFloat, world );
			m_vBoneData[m_vBoneData[bone]->GetChild()[i]]->SetWorld( worldFloat );
			SetChild( m_vBoneData[bone]->GetChild()[i] );
		}
	}
}

XMFLOAT3 Skeleton::CalculateError( XMFLOAT3 endEffector, XMFLOAT3 target )
{
	XMFLOAT3 dist = XMFLOAT3( target.x - endEffector.x, target.z - endEffector.z, target.z - endEffector.z );
	XMVECTOR distVec = XMLoadFloat3( &dist );
	distVec = XMVector3Length( distVec );
	dist.x = XMVectorGetX( distVec );
	dist.y = XMVectorGetY( distVec );
	dist.z = XMVectorGetZ( distVec );
	return dist;
}