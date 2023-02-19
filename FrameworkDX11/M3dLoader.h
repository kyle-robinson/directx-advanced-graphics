#pragma once
#include<vector>
#include<fstream>
#include<map>

#include"DataStucts.h"
#include"Skeleton.h"
#include"structures.h"

//most From https://github.com/jjuiddong/Introduction-to-3D-Game-Programming-With-DirectX11/blob/master/Chapter%2025%20Character%20Animation/SkinnedMesh/LoadM3d.h with eddits to fit into created data stuctuers
///loade file
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
		
		bool LoadM3d(const std::string& filename,
			std::vector<SkinedVertex>& vertices,
			std::vector<USHORT>& indices,
			std::vector<Subset >& subsets,
			std::vector<M3dMaterial>& mats,
			Skeleton& skinInfo);
			std::vector<USHORT> index;
	private:
		void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<M3dMaterial>& mats);
		void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets);
		void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<SkinedVertex>& vertices);
		void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
		void ReadBoneOffsets(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& boneOffsets);
		void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
		void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations);
		void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);	
	};


