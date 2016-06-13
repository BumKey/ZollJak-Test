#include "LoadM3D.h"

bool M3DLoader::LoadM3D(const std::string& filename,
	std::vector<Vertex::PosNormalTexTan>& vertices,
	std::vector<USHORT>& indices,
	std::vector<Subset>& subsets,
	std::vector<Material>& mats)
{
	std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numTriangles = 0;
	UINT numBones = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;

		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
		ReadVertices(fin, numVertices, vertices);
		ReadTriangles(fin, numTriangles, indices);

		return true;
	}

	for (auto& iter : vertices)
	{
		if (abs(iter.Pos.x - 0.001f) <= 0.001f)
			iter.Pos.x = 0.0f;

		if (abs(iter.Pos.y - 0.001f) <= 0.001f)
			iter.Pos.y = 0.0f;

		if (abs(iter.Pos.z - 0.001f) <= 0.001f)
			iter.Pos.z = 0.0f;
	}
	return false;
}

bool M3DLoader::LoadM3D(const std::string& filename,
	std::vector<Vertex::PosNormalTexTanSkinned>& vertices,
	std::vector<USHORT>& indices,
	std::vector<Subset>& subsets,
	std::vector<Material>& mats,
	SkinnedData& skinInfo)
{
	std::ifstream fin(filename);

	UINT numMaterials = 0;
	UINT numVertices = 0;
	UINT numTriangles = 0;
	UINT numBones = 0;
	UINT numAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;

		std::vector<XMFLOAT4X4> finalTransform;
		std::vector<int> boneIndexToParentIndex;
		std::map<std::string, AnimationClip> animations;

		ReadMaterials(fin, numMaterials, mats);
		ReadSubsetTable(fin, numMaterials, subsets);
		ReadSkinnedVertices(fin, numVertices, vertices);
		ReadTriangles(fin, numTriangles, indices);
		ReadBoneFinalTransform(fin, numBones, finalTransform);
		ReadBoneHierarchy(fin, numBones, boneIndexToParentIndex);
		ReadAnimationClips(fin, numBones, numAnimationClips, animations);

		skinInfo.Set(boneIndexToParentIndex, finalTransform, animations);

		return true;
	}

	for (auto& iter : vertices)
	{
		if (abs(iter.Pos.x - 0.001f) <= 0.001f)
			iter.Pos.x = 0.0f;

		if (abs(iter.Pos.y - 0.001f) <= 0.001f)
			iter.Pos.y = 0.0f;

		if (abs(iter.Pos.z - 0.001f) <= 0.001f)
			iter.Pos.z = 0.0f;
	}
	return false;
}

void M3DLoader::ReadMaterials(std::ifstream& fin, UINT numMaterials, std::vector<Material>& mats)
{
	std::string ignore;
	mats.resize(numMaterials);

	fin >> ignore; // materials header text
	for (UINT i = 0; i < numMaterials; ++i)
	{
		fin >> ignore >> mats[i].Ambient.x >> mats[i].Ambient.y >> mats[i].Ambient.z;
		fin >> ignore >> mats[i].Diffuse.x >> mats[i].Diffuse.y >> mats[i].Diffuse.z;
		fin >> ignore >> mats[i].Specular.x >> mats[i].Specular.y >> mats[i].Specular.z;
		fin >> ignore >> mats[i].Specular.w;
		fin >> ignore >> mats[i].Reflect.x >> mats[i].Reflect.y >> mats[i].Reflect.z;
		fin >> ignore >> mats[i].AlphaClip;
		fin >> ignore >> mats[i].EffectTypeName;
		fin >> ignore >> mats[i].DiffuseMapName;
		fin >> ignore >> mats[i].NormalMapName;
	}
}

void M3DLoader::ReadSubsetTable(std::ifstream& fin, UINT numSubsets, std::vector<Subset>& subsets)
{
	std::string ignore;
	subsets.resize(numSubsets);

	fin >> ignore; // subset header text
	for (UINT i = 0; i < numSubsets; ++i)
	{
		fin >> ignore >> subsets[i].Id;
		fin >> ignore >> subsets[i].VertexStart;
		fin >> ignore >> subsets[i].VertexCount;
		fin >> ignore >> subsets[i].FaceStart;
		fin >> ignore >> subsets[i].FaceCount;
	}
}

void M3DLoader::ReadVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTan>& vertices)
{
	std::string ignore;
	vertices.resize(numVertices);

	fin >> ignore; // vertices header text
	for (UINT i = 0; i < numVertices; ++i)
	{
		fin >> ignore >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].TangentU.x >> vertices[i].TangentU.y >> vertices[i].TangentU.z >> vertices[i].TangentU.w;
		fin >> ignore >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		fin >> ignore >> vertices[i].Tex.x >> vertices[i].Tex.y;
	}
}

void M3DLoader::ReadSkinnedVertices(std::ifstream& fin, UINT numVertices, std::vector<Vertex::PosNormalTexTanSkinned>& vertices)
{
	std::string ignore;
	vertices.resize(numVertices);

	fin >> ignore; // vertices header text
	int boneIndices[4];
	float weights[4];
	for (UINT i = 0; i < numVertices; ++i)
	{
		fin >> ignore >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].TangentU.x >> vertices[i].TangentU.y >> vertices[i].TangentU.z >> vertices[i].TangentU.w;
		fin >> ignore >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		fin >> ignore >> vertices[i].Tex.x >> vertices[i].Tex.y;
		fin >> ignore >> weights[0] >> weights[1] >> weights[2] >> weights[3];
		fin >> ignore >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		vertices[i].Weights.x = weights[0];
		vertices[i].Weights.y = weights[1];
		vertices[i].Weights.z = weights[2];

		vertices[i].BoneIndices[0] = (BYTE)boneIndices[0];
		vertices[i].BoneIndices[1] = (BYTE)boneIndices[1];
		vertices[i].BoneIndices[2] = (BYTE)boneIndices[2];
		vertices[i].BoneIndices[3] = (BYTE)boneIndices[3];
	}
}

void M3DLoader::ReadTriangles(std::ifstream& fin, UINT numTriangles, std::vector<USHORT>& indices)
{
	std::string ignore;
	indices.resize(numTriangles * 3);

	fin >> ignore; // triangles header text
	for (UINT i = 0; i < numTriangles; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
}

void M3DLoader::ReadBoneFinalTransform(std::ifstream& fin, UINT numBones, std::vector<XMFLOAT4X4>& finalTransform)
{
	std::string ignore;
	finalTransform.resize(numBones);

	fin >> ignore; // finalTransform header text
	for (UINT i = 0; i < numBones; ++i)
	{
		fin >> ignore >>
			finalTransform[i](0, 0) >> finalTransform[i](0, 1) >> finalTransform[i](0, 2) >> finalTransform[i](0, 3) >>
			finalTransform[i](1, 0) >> finalTransform[i](1, 1) >> finalTransform[i](1, 2) >> finalTransform[i](1, 3) >>
			finalTransform[i](2, 0) >> finalTransform[i](2, 1) >> finalTransform[i](2, 2) >> finalTransform[i](2, 3) >>
			finalTransform[i](3, 0) >> finalTransform[i](3, 1) >> finalTransform[i](3, 2) >> finalTransform[i](3, 3);
	}
}

void M3DLoader::ReadBoneHierarchy(std::ifstream& fin, UINT numBones, std::vector<int>& boneIndexToParentIndex)
{
	std::string ignore;
	boneIndexToParentIndex.resize(numBones);

	fin >> ignore; // BoneHierarchy header text
	for (UINT i = 0; i < numBones; ++i)
	{
		fin >> ignore >> boneIndexToParentIndex[i];
	}
}

void M3DLoader::ReadAnimationClips(std::ifstream& fin, UINT numBones, UINT numAnimationClips,
	std::map<std::string, AnimationClip>& animations)
{
	std::string ignore;
	fin >> ignore; // AnimationClips header text
	for (UINT clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
	{
		std::string clipName;
		fin >> ignore >> clipName;
		fin >> ignore; // {

		AnimationClip clip;
		clip.BoneAnimations.resize(numBones);

		for (UINT boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			ReadBoneKeyframes(fin, numBones, clip.BoneAnimations[boneIndex]);
		}
		fin >> ignore; // }

		animations[clipName] = clip;
	}
}

void M3DLoader::ReadBoneKeyframes(std::ifstream& fin, UINT numBones, BoneAnimation& boneAnimation)
{
	std::string ignore;
	UINT numKeyframes = 0;
	fin >> ignore >> ignore >> numKeyframes;
	fin >> ignore; // {

	boneAnimation.Keyframes.resize(numKeyframes);
	for (UINT i = 0; i < numKeyframes; ++i)
	{
		float t = 0.0f;
		XMFLOAT3 p(0.0f, 0.0f, 0.0f);
		XMFLOAT3 s(1.0f, 1.0f, 1.0f);
		XMFLOAT4 q(0.0f, 0.0f, 0.0f, 1.0f);
		fin >> ignore >> t;
		fin >> ignore >> p.x >> p.y >> p.z;
		fin >> ignore >> s.x >> s.y >> s.z;
		fin >> ignore >> q.x >> q.y >> q.z >> q.w;

		boneAnimation.Keyframes[i].TimePos = t;
		boneAnimation.Keyframes[i].Translation = p;
		boneAnimation.Keyframes[i].Scale = s;
		boneAnimation.Keyframes[i].RotationQuat = q;
	}

	fin >> ignore; // }
}