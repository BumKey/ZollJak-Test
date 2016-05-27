#pragma once
#include <fbxsdk.h>
#include <iostream>
#include <assert.h>
#include <string>
#include <unordered_map>
#include "Vertex.h"

std::ostream& operator<<(std::ostream& os, const XMFLOAT2& f2);
std::ostream& operator<<(std::ostream& os, const XMFLOAT3& f3);
std::ostream& operator<<(std::ostream& os, const XMFLOAT4& f4);

XMFLOAT3 operator+(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator-(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator/(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator/(XMFLOAT3 l, FLOAT r);
bool operator==(XMFLOAT3 l, XMFLOAT3 r);

XMFLOAT2 operator+(XMFLOAT2 l, XMFLOAT2 r);
XMFLOAT2 operator-(XMFLOAT2 l, XMFLOAT2 r);
bool operator==(XMFLOAT2 l, XMFLOAT2 r);

XMFLOAT3 Float3Normalize(const XMFLOAT3& in);
XMFLOAT2 Float2Normalize(const XMFLOAT2& in);

struct BlendingIndexWeightPair
{
	unsigned int mBlendingIndex;
	double mBlendingWeight;

	BlendingIndexWeightPair() : 
		mBlendingIndex(0),
		mBlendingWeight(0)
	{}
};

struct UVInfo
{
	XMFLOAT2 UV;
	std::vector<UINT> TriangleIndex, PositionInTriangle;

	bool operator==(const UVInfo& rhs)
	{
		if (abs(UV.x - rhs.UV.x) <= 0.001f &&
			abs(UV.y - rhs.UV.y) <= 0.001f)
			return true;
		else
			return false;
	}
};

struct CtrlPoint
{
	CtrlPoint() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT3 Position;
	std::vector<XMFLOAT3> Normals;
	std::vector<UVInfo> UVInfos;
	std::vector<BlendingIndexWeightPair> BlendingInfo;
};

///<summary>
/// A Keyframe defines the bone transformation at an instant in time.
///</summary>
struct Keyframe
{
	Keyframe() { ZeroMemory(this, sizeof(this)); }
	float TimePos;
	XMFLOAT3 Translation;
	XMFLOAT3 Scale;
	XMFLOAT4 RotationQuat;
};

struct Bone
{
	int ParentIndex;
	std::string Name;
	FbxAMatrix BoneOffset;

	FbxNode* Node;
	std::vector<Keyframe> Keyframes;

	Bone() : ParentIndex(-1), Node(nullptr)
	{ BoneOffset.SetIdentity(); }
};

struct Triangle
{
	std::string MaterialName;
	UINT SubsetID;

	bool operator<(const Triangle& rhs)
	{
		return SubsetID < rhs.SubsetID;
	}
};

class Utilities
{
public:

	// This function should be changed if exporting to another format
	static void WriteMatrix(std::ostream& inStream, FbxAMatrix& inMatrix, bool inIsRoot);

	static void PrintMatrix(FbxMatrix& inMatrix);
	
	static FbxAMatrix GetGeometryTransformation(FbxNode* inNode);

	static std::string GetFileName(const std::string& inInput);

	static std::string RemoveSuffix(const std::string& inInput);
};

struct Subset
{
	Subset() : ID(-1), VertexStart(0), VertexCount(0),
		FaceStart(0), FaceCount(0) {}

	int ID;
	UINT VertexStart;
	UINT VertexCount;
	UINT FaceStart;
	UINT FaceCount;
};
