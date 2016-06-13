#ifndef LoadY2K_H
#define LoadY2K_H

#include "BasicMeshData.h"
#include "LightHelper.h"
#include "SkinnedData.h"
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
		SkinnedData& skinInfo);

private:
	void ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<Y2kMaterial>& mats);
	void ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<BasicMeshData::Subset>& subsets);
	void ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexSkinned>& vertices);
	void ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTanSkinned>& vertices);
	void ReadIndicies(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices);
	void ReadBoneFinalTransform(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& finalTransform);
	void ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex);
	void ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips, std::map<std::string, AnimationClip>& animations);
	void ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation);

	template<class T>
	void ReadBinary(std::ifstream& fin, T& out);

	template<class T>
	void ReadBinary(std::ifstream& fin, T& out, UINT strLen);
};



#endif // LoadY2K_H

template<class T>
inline void Y2KLoader::ReadBinary(std::ifstream& fin, T & out)
{
	UINT bufferLen = sizeof(out) / sizeof(BYTE);
	BYTE* buffer = new BYTE[bufferLen];

	fin.read((char*)buffer, bufferLen);
	out = *reinterpret_cast<T*>(buffer);
	delete buffer;
}

template<class T>
inline void Y2KLoader::ReadBinary(std::ifstream & fin, T & out, UINT strLen)
{
	BYTE* buffer = new BYTE[strLen + 1];

	fin.read((char*)buffer, strLen);
	buffer[strLen] = '\0';
	out = reinterpret_cast<char*>(buffer);
	delete buffer;
}

