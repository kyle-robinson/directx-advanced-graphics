#pragma once
#ifndef M3DLOADER_H
#define M3DLOADER_H

#include <fstream>
#include "Skeleton.h"
#include "Structures.h"
#include "DataStucts.h"

// REFERENCE : https://github.com/jjuiddong/Introduction-to-3D-Game-Programming-With-DirectX11/blob/master/Chapter%2025%20Character%20Animation/SkinnedMesh/LoadM3d.h
struct M3dMaterial
{
	_Material Mat;
	bool AlphaClip;
	std::string EffectTypeName;
	std::wstring DiffuseMapName;
	std::wstring NormalMapName;
};

class M3DLoader
{
public:
	bool LoadM3d( const std::string& filename,
		std::vector<SkinnedVertex>& vertices,
		std::vector<WORD>& indices,
		std::vector<Subset >& subsets,
		std::vector<M3dMaterial>& mats,
		Skeleton& skinInfo );
	std::vector<WORD> index;
private:
	void ReadMaterials( std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats );
	void ReadSubsetTable( std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets );
	void ReadSkinnedVertices( std::ifstream& fin, UINT numVertices, std::vector<SkinnedVertex>& vertices );
	void ReadTriangles( std::ifstream& fin, UINT numTriangles, std::vector<WORD>& indices );
	void ReadBoneOffsets( std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& boneOffsets );
	void ReadBoneHierarchy( std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex );
	void ReadAnimationClips( std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations );
	void ReadBoneKeyframes( std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation );
};

#endif