#include "LoadY2K.h"

bool Y2KLoader::LoadY2K(const std::string& filename,
	std::vector<Vertex::PosNormalTexTan>& vertices,
	std::vector<USHORT>& indices,
	std::vector<BasicMeshData::Subset>& subsets,
	std::vector<Y2kMaterial>& mats)
{
	std::ifstream fin(filename, std::ios::binary);

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numIndices = 0;

	std::string ignore;

	if (fin)
	{
		ReadBinary(fin, numMaterials);
		ReadBinary(fin, numVertices);
		ReadBinary(fin, numIndices);

		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
		ReadVertices(fin, numVertices, vertices);
		ReadIndicies(fin, numIndices, indices);

		return true;
	}
	return false;
}

bool Y2KLoader::LoadY2K(const std::string& filename,
	std::vector<Vertex::PosNormalTexTanSkinned>& vertices,
	std::vector<USHORT>& indices,
	std::vector<BasicMeshData::Subset>& subsets,
	std::vector<Y2kMaterial>& mats,
	SkinnedData& skinInfo)
{
	std::ifstream fin(filename, std::ios::binary);

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numIndices = 0;
	UINT numBones = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		ReadBinary(fin, numMaterials);
		ReadBinary(fin, numVertices);
		ReadBinary(fin, numIndices);
		ReadBinary(fin, numBones);
		ReadBinary(fin, numAnimationClips);

		std::vector<XMFLOAT4X4> finalTransform;
		std::vector<int> boneIndexToParentIndex;
		std::map<std::string, AnimationClip> animations;

		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
		ReadSkinnedVertices(fin, numVertices, vertices);
		ReadIndicies(fin, numIndices, indices);
		ReadBoneFinalTransform(fin, numBones, finalTransform);
		ReadBoneHierarchy(fin, numBones, boneIndexToParentIndex);
		ReadAnimationClips(fin, numBones, numAnimationClips, animations);

		skinInfo.Set(boneIndexToParentIndex, finalTransform, animations);

		return true;
	}
	return false;
}

void Y2KLoader::ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<Y2kMaterial>& mats)
{
	mats.resize(numMaterials);

	std::string diffuseMapName;
	std::string normalMapName;

	UINT strLen = 0;
	for (UINT i = 0; i < numMaterials; ++i)
	{
		ReadBinary(fin, mats[i].Mat.Ambient);
		ReadBinary(fin, mats[i].Mat.Diffuse);
		ReadBinary(fin, mats[i].Mat.Specular);
		ReadBinary(fin, mats[i].Mat.Reflect);
		ReadBinary(fin, mats[i].AlphaClip);

		ReadBinary(fin, strLen);
		ReadBinary(fin, mats[i].EffectTypeName, strLen);

		ReadBinary(fin, strLen);
		ReadBinary(fin, diffuseMapName, strLen);

		ReadBinary(fin, strLen);
		ReadBinary(fin, normalMapName, strLen);

		mats[i].DiffuseMapName.resize(diffuseMapName.size(), ' ');
		mats[i].NormalMapName.resize(normalMapName.size(), ' ');
		std::copy(diffuseMapName.begin(), diffuseMapName.end(), mats[i].DiffuseMapName.begin());
		std::copy(normalMapName.begin(), normalMapName.end(), mats[i].NormalMapName.begin());
	}
}

void Y2KLoader::ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<BasicMeshData::Subset>& subsets)
{
	subsets.resize(numSubsets);

	for (UINT i = 0; i < numSubsets; ++i)
	{
		ReadBinary(fin, subsets[i].Id);
		ReadBinary(fin, subsets[i].FaceStart);
		ReadBinary(fin, subsets[i].FaceCount);
	}
}

void Y2KLoader::ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices)
{
	vertices.resize(numVertices);

	for (UINT i = 0; i < numVertices; ++i)
	{
		ReadBinary(fin, vertices[i].Pos);
		ReadBinary(fin, vertices[i].TangentU);
		ReadBinary(fin, vertices[i].Normal);
		ReadBinary(fin, vertices[i].Tex);
	}
}

void Y2KLoader::ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTanSkinned>& vertices)
{
	vertices.resize(numVertices);

	for (UINT i = 0; i < numVertices; ++i)
	{
		ReadBinary(fin, vertices[i].Pos);
		ReadBinary(fin, vertices[i].TangentU);
		ReadBinary(fin, vertices[i].Normal);
		ReadBinary(fin, vertices[i].Tex);
		ReadBinary(fin, vertices[i].Weights);
		ReadBinary(fin, vertices[i].BoneIndices[0]);
		ReadBinary(fin, vertices[i].BoneIndices[1]);
		ReadBinary(fin, vertices[i].BoneIndices[2]);
		ReadBinary(fin, vertices[i].BoneIndices[3]);
	}
}

void Y2KLoader::ReadIndicies(std::ifstream& fin, UINT numIndices, std::vector<USHORT>& indices)
{
	indices.resize(numIndices);

	for (UINT i = 0; i < numIndices; ++i)
		ReadBinary(fin, indices[i]);
}

void Y2KLoader::ReadBoneFinalTransform(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& finalTransform)
{
	finalTransform.resize(numBones);

	for (UINT i = 0; i < numBones; ++i)
		ReadBinary(fin, finalTransform[i]);
}

void Y2KLoader::ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex)
{
	boneIndexToParentIndex.resize(numBones);

	for (UINT i = 0; i < numBones; ++i)
		ReadBinary(fin, boneIndexToParentIndex[i]);
}

void Y2KLoader::ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips,
	std::map<std::string, AnimationClip>& animations)
{
	for (UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
	{
		UINT strLen;
		std::string clipName;

		ReadBinary(fin, strLen);
		ReadBinary(fin, clipName, strLen);

		AnimationClip clip;
		clip.BoneAnimations.resize(numBones);

		for (UINT boneIndex = 0; boneIndex < numBones; ++boneIndex)
			ReadBoneKeyframes(fin, numBones, clip.BoneAnimations[boneIndex]);
		
		animations[clipName] = clip;
	}
}

void Y2KLoader::ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation)
{
	UINT numKeyframes;
	ReadBinary(fin, numKeyframes);

	boneAnimation.Keyframes.resize(numKeyframes);
	for (UINT i = 0; i < numKeyframes; ++i)
	{
		ReadBinary(fin, boneAnimation.Keyframes[i].TimePos);
		ReadBinary(fin, boneAnimation.Keyframes[i].Translation);
		ReadBinary(fin, boneAnimation.Keyframes[i].RotationQuat);
		boneAnimation.Keyframes[i].Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	}
}