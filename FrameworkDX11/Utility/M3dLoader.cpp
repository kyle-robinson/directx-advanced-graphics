#include "stdafx.h"
#include "M3dLoader.h"

bool M3DLoader::LoadM3d( const std::string& filename,
	std::vector<SkinnedVertex>& vertices,
	std::vector<WORD>& indices,
	std::vector<Subset>& subsets,
	std::vector<M3dMaterial>& mats,
	Skeleton& skinInfo )
{
	std::ifstream fin( filename );

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numTriangles = 0;
	UINT numBones = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if ( fin )
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;

		std::vector<XMFLOAT4X4> boneOffsets;
		std::vector<int> boneIndexToParentIndex;
		std::map<std::string, AnimationClip> animations;

		ReadMaterials( fin, numMaterials, mats );
		ReadSubsetTable( fin, numMaterials, subsets );
		ReadSkinnedVertices( fin, numVertices, vertices );
		ReadTriangles( fin, numTriangles, indices );
		ReadBoneOffsets( fin, numBones, boneOffsets );
		ReadBoneHierarchy( fin, numBones, boneIndexToParentIndex );
		ReadAnimationClips( fin, numBones, numAnimationClips, animations );

		skinInfo.Set( boneIndexToParentIndex, boneOffsets, animations );
		index = indices;
		return true;
	}
	return false;
}

void M3DLoader::ReadMaterials( std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats )
{
	std::string ignore;
	mats.resize( numMaterials );

	std::string diffuseMapName;
	std::string normalMapName;
	XMFLOAT3 reflect;
	fin >> ignore; // materials header text
	for ( UINT i = 0; i < numMaterials; ++i )
	{
		fin >> ignore >> mats[i].Mat.Ambient.x >> mats[i].Mat.Ambient.y >> mats[i].Mat.Ambient.z;
		fin >> ignore >> mats[i].Mat.Diffuse.x >> mats[i].Mat.Diffuse.y >> mats[i].Mat.Diffuse.z;
		fin >> ignore >> mats[i].Mat.Specular.x >> mats[i].Mat.Specular.y >> mats[i].Mat.Specular.z;
		fin >> ignore >> mats[i].Mat.Specular.w;
		fin >> ignore >> reflect.x >> reflect.y >> reflect.z;
		fin >> ignore >> mats[i].AlphaClip;
		fin >> ignore >> mats[i].EffectTypeName;
		fin >> ignore >> diffuseMapName;
		fin >> ignore >> normalMapName;

		mats[i].DiffuseMapName.resize( diffuseMapName.size(), ' ' );
		mats[i].NormalMapName.resize( normalMapName.size(), ' ' );
		std::copy( diffuseMapName.begin(), diffuseMapName.end(), mats[i].DiffuseMapName.begin() );
		std::copy( normalMapName.begin(), normalMapName.end(), mats[i].NormalMapName.begin() );
	}
}

void M3DLoader::ReadSubsetTable( std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets )
{
	std::string ignore;
	subsets.resize( numSubsets );

	fin >> ignore; // subset header text
	for ( UINT i = 0; i < numSubsets; ++i )
	{
		fin >> ignore >> subsets[i].m_uId;
		fin >> ignore >> subsets[i].m_uVertexStart;
		fin >> ignore >> subsets[i].m_uVertexCount;
		fin >> ignore >> subsets[i].m_uFaceStart;
		fin >> ignore >> subsets[i].m_uFaceCount;
	}
}

void M3DLoader::ReadSkinnedVertices( std::ifstream& fin, UINT numVertices, std::vector<SkinnedVertex>& vertices )
{
	std::string ignore;
	vertices.resize( numVertices );

	fin >> ignore; // vertices header text
	int boneIndices[4];
	float weights[4];
	for ( UINT i = 0; i < numVertices; ++i )
	{
		fin >> ignore >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].Tangent.x >> vertices[i].Tangent.y >> vertices[i].Tangent.z >> vertices[i].Tangent.w;
		fin >> ignore >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		fin >> ignore >> vertices[i].Tex.x >> vertices[i].Tex.y;
		fin >> ignore >> weights[0] >> weights[1] >> weights[2] >> weights[3];
		fin >> ignore >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		vertices[i].Weights.x = weights[0];
		vertices[i].Weights.y = weights[1];
		vertices[i].Weights.z = weights[2];

		vertices[i].BoneIndices.x = boneIndices[0];
		vertices[i].BoneIndices.y = boneIndices[1];
		vertices[i].BoneIndices.z = boneIndices[2];
		vertices[i].BoneIndices.w = boneIndices[3];
	}
}

void M3DLoader::ReadTriangles( std::ifstream& fin, UINT numTriangles, std::vector<WORD>& indices )
{
	std::string ignore;
	indices.resize( numTriangles * 3 );

	fin >> ignore; // triangles header text
	for ( UINT i = 0; i < numTriangles; ++i )
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
}

void M3DLoader::ReadBoneOffsets( std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& boneOffsets )
{
	std::string ignore;
	boneOffsets.resize( numBones );

	fin >> ignore; // BoneOffsets header text
	for ( UINT i = 0; i < numBones; ++i )
	{
		fin >> ignore >>
			boneOffsets[i]( 0, 0 ) >> boneOffsets[i]( 0, 1 ) >> boneOffsets[i]( 0, 2 ) >> boneOffsets[i]( 0, 3 ) >>
			boneOffsets[i]( 1, 0 ) >> boneOffsets[i]( 1, 1 ) >> boneOffsets[i]( 1, 2 ) >> boneOffsets[i]( 1, 3 ) >>
			boneOffsets[i]( 2, 0 ) >> boneOffsets[i]( 2, 1 ) >> boneOffsets[i]( 2, 2 ) >> boneOffsets[i]( 2, 3 ) >>
			boneOffsets[i]( 3, 0 ) >> boneOffsets[i]( 3, 1 ) >> boneOffsets[i]( 3, 2 ) >> boneOffsets[i]( 3, 3 );
	}
}

void M3DLoader::ReadBoneHierarchy( std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex )
{
	std::string ignore;
	boneIndexToParentIndex.resize( numBones );

	fin >> ignore; // BoneHierarchy header text
	for ( UINT i = 0; i < numBones; ++i )
	{
		fin >> ignore >> boneIndexToParentIndex[i];
	}
}

void M3DLoader::ReadAnimationClips( std::ifstream& fin, UINT numBones, UINT numAnimationClips,
	std::map<std::string, AnimationClip>& animations )
{
	std::string ignore;
	fin >> ignore; // AnimationClips header text
	for ( UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex )
	{
		std::string clipName;
		fin >> ignore >> clipName;
		fin >> ignore;

		AnimationClip clip;
		clip.m_vBoneAnimations.resize( numBones );

		for ( UINT boneIndex = 0; boneIndex < numBones; ++boneIndex )
		{
			ReadBoneKeyframes( fin, numBones, clip.m_vBoneAnimations[boneIndex] );
		}
		fin >> ignore;
		animations[clipName] = clip;
	}
}

void M3DLoader::ReadBoneKeyframes( std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation )
{
	std::string ignore;
	UINT numKeyframes = 0;
	fin >> ignore >> ignore >> numKeyframes;
	fin >> ignore;

	boneAnimation.m_vKeyframes.resize( numKeyframes );
	for ( UINT i = 0; i < numKeyframes; ++i )
	{
		float t = 0.0f;
		XMFLOAT3 p( 0.0f, 0.0f, 0.0f );
		XMFLOAT3 s( 1.0f, 1.0f, 1.0f );
		XMFLOAT4 q( 0.0f, 0.0f, 0.0f, 1.0f );
		fin >> ignore >> t;
		fin >> ignore >> p.x >> p.y >> p.z;
		fin >> ignore >> s.x >> s.y >> s.z;
		fin >> ignore >> q.x >> q.y >> q.z >> q.w;

		boneAnimation.m_vKeyframes[i].m_fTimePos = t;
		boneAnimation.m_vKeyframes[i].m_fTranslation = p;
		boneAnimation.m_vKeyframes[i].m_fScale = s;
		boneAnimation.m_vKeyframes[i].m_fRotationQuat = q;
	}

	fin >> ignore;
}