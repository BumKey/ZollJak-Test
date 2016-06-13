#ifndef LoadM3D_H
#define LoadM3D_H

#include "Vertex.h"
#include "SkinnedData.h"

struct Subset
{
	Subset() : Id(-1), VertexStart(0), VertexCount(0),
		FaceStart(0), FaceCount(0) { }

	UINT Id;
	UINT VertexStart;
	UINT VertexCount;
	UINT FaceStart;
	UINT FaceCount;
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT3 Ambient;
	XMFLOAT3 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT3 Reflect;

	bool AlphaClip;
	std::string EffectTypeName;
	std::string DiffuseMapName;
	std::string NormalMapName;
};

class M3DLoader
{
public:
	bool LoadM3D(const std::string& filename,
		std::vector<Vertex::PosNormalTexTan>& vertices,
		std::vector<USHORT>& indices,
		std::vector<Subset>& subsets,
		std::vector<Material>& mats);

	bool LoadM3D(const std::string& filename,
		std::vector<Vertex::PosNormalTexTanSkinned>& vertices,
		std::vector<USHORT>& indices,
		std::vector<Subset>& subsets,
		std::vector<Material>& mats,
		SkinnedData& skinInfo);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<Material>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets);
	void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexSkinned>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTanSkinned>& vertices);
	void ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
	void ReadBoneFinalTransform(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& finalTransform);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);
};



#endif // LoadM3D_H