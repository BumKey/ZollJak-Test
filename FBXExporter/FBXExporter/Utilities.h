#pragma once
#include <fbxsdk.h>
#include <iostream>
#include <assert.h>
#include <string>
#include <unordered_map>
#include "Vertex.h"

struct BlendingIndexWeightPair
{
	unsigned int mBlendingIndex;
	double mBlendingWeight;

	BlendingIndexWeightPair() : 
		mBlendingIndex(0),
		mBlendingWeight(0)
	{}
};

// Each Control Point in FBX is basically a vertex
// in the physical world. For example, a cube has 8
// vertices(Control Points) in FBX
// Bones are associated with Control Points in FBX
// The mapping is one Bone corresponding to 4
// Control Points(Reverse of what is done in a game engine)
// As a result, this struct stores a XMFLOAT3 and a 
// vector of Bone indices
struct CtrlPoint
{
	CtrlPoint() { BlendingInfo.reserve(4); }

	XMFLOAT3 Position;
	std::vector<BlendingIndexWeightPair> BlendingInfo;
	std::vector<XMFLOAT3> Normal;
	std::vector<XMFLOAT4> Tangent;
	std::vector<XMFLOAT2> UV;

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
	std::vector<UINT> Indices; 
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

std::ostream& operator<<(std::ostream& os, const XMFLOAT2& f2);
std::ostream& operator<<(std::ostream& os, const XMFLOAT3& f3);
std::ostream& operator<<(std::ostream& os, const XMFLOAT4& f4);
XMFLOAT3 operator-(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT2 operator-(XMFLOAT2 l, XMFLOAT2 r);

XMFLOAT3 Float3Normalize(const XMFLOAT3& in);
XMFLOAT2 Float2Normalize(const XMFLOAT2& in);