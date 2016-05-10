#ifndef LoadY2K_H
#define LoadY2K_H

#include "BasicMeshData.h"
#include "LightHelper.h"
#include "SkinnedMeshData.h"
#include "Vertex.h"

struct Y2kMaterial
{
	Material Mat;
	bool AlphaClip;
	std::string EffectTypeName;
	std::wstring DiffuseMapName;
	std::wstring NormalMapName;
};

class Y2KLoader
{
public:
	bool LoadY2K(const std::string& filename,
		std::vector<Vertex::PosNormalTexTan>& vertices,
		std::vector<USHORT>& indices,
		std::vector<BasicMeshData::Subset>& subsets,
		std::vector<Y2kMaterial>& mats);
	bool LoadY2K(const std::string& filename,
		std::vector<Vertex::PosNormalTexTanSkinned>& vertices,
		std::vector<USHORT>& indices,
		std::vector<BasicMeshData::Subset>& subsets,
		std::vector<Y2kMaterial>& mats,
		SkinnedMeshData& skinInfo);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<Y2kMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<BasicMeshData::Subset>& subsets);
	void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTanSkinned>& vertices);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
	void ReadBoneFinalTransform(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& finalTransform);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};



#endif // LoadY2K_H