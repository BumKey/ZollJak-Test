#include "Y2KExporter.h"
#include <iostream>

Y2KExporter::Y2KExporter() : mHasAnim(false), mAxisCorrect(false)
{
}


Y2KExporter::~Y2KExporter()
{
}

void Y2KExporter::AxisCorrect(const XMFLOAT3 & value)
{
	mAxisCorrect = true;
	mAxisCorrecter = value;
}

void Y2KExporter::LoadM3D(const std::string & inFileName, bool hasAnim)
{
	mHasAnim = hasAnim;

	bool result;
	if (hasAnim)
		result = mLoader.LoadM3D(inFileName, mSkinnedVertices, mIndices, mSubsets, mMats, mSkinnedData);
	else
		result = mLoader.LoadM3D(inFileName, mBasicVertices, mIndices, mSubsets, mMats);

	assert(result, inFileName);
}

void Y2KExporter::ExportY2K(const std::string & outFileName)
{
	std::ofstream fout(outFileName, std::ios::binary);
	UINT strLen;

	if (mAxisCorrect)
	{
		XMVECTOR v = XMLoadFloat3(&mAxisCorrecter);
		XMMATRIX R = XMMatrixRotationRollPitchYawFromVector(v);
		
		if (mHasAnim) {
			for (auto& v : mSkinnedVertices)
			{
				XMVECTOR p = XMLoadFloat3(&v.Pos);
				p = XMVector3TransformCoord(p, R);

				XMStoreFloat3(&v.Pos, p);
			}
		}
		else {
			for (auto& v : mBasicVertices)
			{
				XMVECTOR p = XMLoadFloat3(&v.Pos);
				p = XMVector3TransformCoord(p, R);

				XMStoreFloat3(&v.Pos, p);
			}
		}
	}

	if (mHasAnim)
	{
		UINT numMats = mMats.size();
		UINT numVers = mSkinnedVertices.size();
		UINT numIns = mIndices.size();
		UINT numBones = mSkinnedData.mBoneHierarchy.size();
		UINT numClips = mSkinnedData.mAnimations.size();

		// File_Header
		fout.write(reinterpret_cast<char*>(&numMats), sizeof(UINT));
		fout.write(reinterpret_cast<char*>(&numVers), sizeof(UINT));
		fout.write(reinterpret_cast<char*>(&numIns), sizeof(UINT));
		fout.write(reinterpret_cast<char*>(&numBones), sizeof(UINT));
		fout.write(reinterpret_cast<char*>(&numClips), sizeof(UINT));

		// Materials
		for (auto iter : mMats)
		{
			fout.write(reinterpret_cast<char*>(&iter.Ambient), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.Diffuse), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.Specular), sizeof(XMFLOAT4));
			fout.write(reinterpret_cast<char*>(&iter.Reflect), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.AlphaClip), sizeof(bool));

			strLen = iter.EffectTypeName.length();
			fout.write(reinterpret_cast<char*>(&strLen),sizeof(UINT));
			fout.write(reinterpret_cast<const char*>(iter.EffectTypeName.c_str()), strLen * sizeof(char));

			strLen = iter.DiffuseMapName.length();
			fout.write(reinterpret_cast<char*>(&strLen), sizeof(UINT));
			fout.write(reinterpret_cast<const char*>(iter.DiffuseMapName.c_str()), strLen * sizeof(char));

			strLen = iter.NormalMapName.length();
			fout.write(reinterpret_cast<char*>(&strLen), sizeof(UINT));
			fout.write(reinterpret_cast<const char*>(iter.NormalMapName.c_str()), strLen * sizeof(char));
		}

		for (auto iter : mSubsets) 
		{
			fout.write(reinterpret_cast<char*>(&iter.Id), sizeof(UINT));
			fout.write(reinterpret_cast<char*>(&iter.FaceStart), sizeof(UINT));
			fout.write(reinterpret_cast<char*>(&iter.FaceCount), sizeof(UINT));
		}

		for (auto iter : mSkinnedVertices)
		{
			fout.write(reinterpret_cast<char*>(&iter.Pos), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.TangentU), sizeof(XMFLOAT4));
			fout.write(reinterpret_cast<char*>(&iter.Normal), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.Tex), sizeof(XMFLOAT2));
			fout.write(reinterpret_cast<char*>(&iter.Weights), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(iter.BoneIndices), 4 * sizeof(BYTE));
		}

		for (auto iter : mIndices)
			fout.write(reinterpret_cast<char*>(&iter), sizeof(USHORT));

		for(auto iter : mSkinnedData.mBoneOffsets)
			fout.write(reinterpret_cast<char*>(&iter), sizeof(XMFLOAT4X4));

		for (auto iter : mSkinnedData.mBoneHierarchy)
			fout.write(reinterpret_cast<char*>(&iter), sizeof(int));

		for (auto iterA : mSkinnedData.mAnimations) {
			strLen = iterA.first.length();
			fout.write(reinterpret_cast<char*>(&strLen), sizeof(UINT));
			fout.write(reinterpret_cast<const char*>(iterA.first.c_str()), strLen * sizeof(char));
	
			for (auto iterB : iterA.second.BoneAnimations) {
				UINT numKeyframes = iterB.Keyframes.size();
				fout.write(reinterpret_cast<char*>(&numKeyframes), sizeof(UINT));
				for (auto iterK : iterB.Keyframes)
				{
					fout.write(reinterpret_cast<char*>(&iterK.TimePos), sizeof(float));
					fout.write(reinterpret_cast<char*>(&iterK.Translation), sizeof(XMFLOAT3));
					fout.write(reinterpret_cast<char*>(&iterK.RotationQuat), sizeof(XMFLOAT4));
				}
			}
		}
	}
	else
	{
		UINT numMats = mMats.size();
		UINT numVers = mBasicVertices.size();
		UINT numIns = mIndices.size();

		// File_Header
		fout.write(reinterpret_cast<char*>(&numMats), sizeof(UINT));
		fout.write(reinterpret_cast<char*>(&numVers), sizeof(UINT));
		fout.write(reinterpret_cast<char*>(&numIns), sizeof(UINT));

		// Materials
		for (auto iter : mMats)
		{
			fout.write(reinterpret_cast<char*>(&iter.Ambient), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.Diffuse), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.Specular), sizeof(XMFLOAT4));
			fout.write(reinterpret_cast<char*>(&iter.Reflect), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.AlphaClip), sizeof(bool));

			strLen = iter.EffectTypeName.length();
			fout.write(reinterpret_cast<char*>(&strLen), sizeof(UINT));
			fout.write(reinterpret_cast<const char*>(iter.EffectTypeName.c_str()), strLen * sizeof(char));

			strLen = iter.DiffuseMapName.length();
			fout.write(reinterpret_cast<char*>(&strLen), sizeof(UINT));
			fout.write(reinterpret_cast<const char*>(iter.DiffuseMapName.c_str()), strLen * sizeof(char));

			strLen = iter.NormalMapName.length();
			fout.write(reinterpret_cast<char*>(&strLen), sizeof(UINT));
			fout.write(reinterpret_cast<const char*>(iter.NormalMapName.c_str()), strLen * sizeof(char));
		}

		for (auto iter : mSubsets)
		{
			fout.write(reinterpret_cast<char*>(&iter.Id), sizeof(UINT));
			fout.write(reinterpret_cast<char*>(&iter.FaceStart), sizeof(UINT));
			fout.write(reinterpret_cast<char*>(&iter.FaceCount), sizeof(UINT));
		}

		for (auto iter : mBasicVertices)
		{
			fout.write(reinterpret_cast<char*>(&iter.Pos), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.TangentU), sizeof(XMFLOAT4));
			fout.write(reinterpret_cast<char*>(&iter.Normal), sizeof(XMFLOAT3));
			fout.write(reinterpret_cast<char*>(&iter.Tex), sizeof(XMFLOAT2));
		}

		for (auto iter : mIndices)
		{
			fout.write(reinterpret_cast<char*>(&iter), sizeof(USHORT));
		}
	}

	std::cout << outFileName << " 파일 Exporting 성공" << std::endl << std::endl;

	mIndices.clear();
	mSubsets.clear();
	mMats.clear();
	mSkinnedData.mAnimations.clear();
	mSkinnedData.mBoneHierarchy.clear();
	mSkinnedData.mBoneHierarchy.clear();
	mSkinnedVertices.clear();
	mBasicVertices.clear();

	fout.close();
}
