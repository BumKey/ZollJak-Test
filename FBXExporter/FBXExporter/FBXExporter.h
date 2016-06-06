#pragma once
#include "Utilities.h"
#include <unordered_map>
#include <map>
#include "Material.h"
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

class FBXExporter
{
public:
	FBXExporter();
	~FBXExporter();

public:
	void SetClipNum(UINT num) { mClipNum = num; }

	bool LoadFile(const std::string& fileName);
	void Export(const std::string& fileName, const std::string& clipName,
		std::ofstream& fout, bool overWrite);

private:	// You should strict to ordoer of processes
	void ProcessGeometry(FbxNode* node);	
	void ProcessControlPoints(FbxMesh* mesh);
	void ProcessMesh(FbxMesh* mesh);

	void ReadUV(FbxMesh* inMesh, int inCtrlPointIndex, int inTextureUVIndex, int inUVLayer, XMFLOAT2& outUV);
	void ReadNormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outNormal);
	void ReadBinormal(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT3& outBinormal);
	void ReadTangent(FbxMesh* inMesh, int inCtrlPointIndex, int inVertexCounter, XMFLOAT4& outTangent);

	void ProcessMaterials(FbxNode* node);
	void ProcessMaterialAttribute(FbxSurfaceMaterial* inMaterial, UINT inMaterialIndex);
	void ProcessMaterialTexture(FbxSurfaceMaterial * inMaterial, Material * ioMaterial);
	void AssociateMaterialToMesh(FbxNode* node);

	void ProcessBoneHierachy(FbxNode* node);
	void ProcessBoneHierachy(FbxNode* none, int inDepth, int myIndex, int inParentIndex);
	void ReadSkinnedData(FbxMesh* mesh);
	void ReadBoneWeightsAndIndices(FbxMesh* mesh);
	void ReadBoneOffset(FbxMesh* mesh);
	void RemoveUnSkinnedBonesFromHierachy();
	void ReadKeyframes(FbxNode* none, Bone& bone);
	UINT FindBoneIndexUsingName(const std::string& inJointName);

	void FinalProcedure();
	void WriteMesh(std::ostream& inStream);
	void WriteAnimation(std::ostream& inStream);


private:
	FbxManager* mFbxMgr;
	FbxScene* mScene;

	LARGE_INTEGER mCPUFreq;

	UINT mPrevCtrlPointCount;
	UINT mPrevTriangleCount;

	bool mOverWrite;
	bool mHasAnimation;

	UINT mClipNum;
	std::string mClipName;

	std::vector<Bone> mBones;

	std::vector<Triangle> mTriangles;
	std::vector<Vertex::Skinned> mVertices;
	std::vector<Subset> mSubsets;

	std::unordered_map<UINT, Material*> mMaterials;
	std::unordered_map<UINT, CtrlPoint> mControlPoints;
};