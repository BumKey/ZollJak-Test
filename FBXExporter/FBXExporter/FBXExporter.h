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
#include <algorithm>

typedef std::unordered_map<std::string, std::pair<UINT, UINT>> FrameInfo;

class FBXExporter
{
public:
	FBXExporter();
	~FBXExporter();

public:
	bool LoadFile(const std::string& fileName);
	void Export(const std::string& fileName, const std::string& clipName, std::ofstream& fout);

	void SetClipNum(UINT num) { mClipNum = num; }
	void AllInOneTakeMode(const FrameInfo& frameInfo);

private:	// You should strict to ordoer of processes
	void ProcessGeometry(FbxNode* node);	
	void ProcessControlPoints(FbxMesh* mesh);
	void ProcessMesh(FbxMesh* mesh);

	void ReadUV(FbxMesh* mesh, const UINT triangleIndex, const UINT positionInTriangle, Triangle::SurfaceData& triangle);
	void ReadNormal(FbxMesh* mesh, int ctrlPointIndex, int indexCounter, Triangle::SurfaceData& data);
	void ReadTangent(FbxMesh* mesh, int ctrlPointIndex, int indexCounter, Triangle::SurfaceData& data);
	
	void ProcessMaterials(FbxNode* node);
	void ProcessMaterialAttribute(FbxSurfaceMaterial* inMaterial, UINT inMaterialIndex);
	void ProcessMaterialTexture(FbxSurfaceMaterial * inMaterial, Material * outMaterial);
	void AssociateMaterialToMesh(FbxNode* node);

	void ProcessBoneHierachy(FbxNode* rootNode);
	void GetAllBones(FbxNode* none);
	void ReadSkinnedData(FbxMesh* mesh);
	void RemoveUnskinnedBones();
	void ReadKeyframes();
	int FindBoneIndexUsingName(const std::string& boneName);

	void FinalProcedure();
	void WriteMesh(std::ostream& inStream);
	void WriteAnimation(std::ostream& inStream);

private:
	FbxManager* mFbxMgr;
	FbxScene* mScene;

	LARGE_INTEGER mCPUFreq;

	UINT mMeshCount;
	UINT mPrevCtrlPointCount;
	UINT mPrevTriangleCount;

	bool mAllInOneTakeMode;
	bool mHasAnimation;

	UINT mClipNum;
	UINT mExportingCount;
	std::string mClipName;

	std::vector<Bone> mBones;

	std::vector<Triangle> mTriangles;
	std::vector<Vertex::Skinned> mVertices;
	std::vector<UINT> mIndices;
	std::vector<Subset> mSubsets;

	std::unordered_map<UINT, Material*> mMaterials;
	std::unordered_map<UINT, CtrlPoint> mControlPoints;
	FrameInfo mFrameInfo;
};