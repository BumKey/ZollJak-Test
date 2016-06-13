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

XMFLOAT4 operator+(XMFLOAT4 l, XMFLOAT4 r);

XMFLOAT3 operator+(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator-(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator/(XMFLOAT3 l, XMFLOAT3 r);
XMFLOAT3 operator/(XMFLOAT3 l, FLOAT r);
bool operator==(XMFLOAT3 l, XMFLOAT3 r);

XMFLOAT2 operator+(XMFLOAT2 l, XMFLOAT2 r);
XMFLOAT2 operator-(XMFLOAT2 l, XMFLOAT2 r);
bool operator==(XMFLOAT2 l, XMFLOAT2 r);
bool operator!=(XMFLOAT2 l, XMFLOAT2 r);

XMFLOAT4 Float4Normalize(const XMFLOAT4& in);
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

struct CtrlPoint
{
	CtrlPoint() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT3 Position;
	std::vector<BlendingIndexWeightPair> BlendingInfo;
	std::vector<XMFLOAT2> PolygonVertex;
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

	void operator=(const Bone& rhs)
	{
		ParentIndex = rhs.ParentIndex;
		Name = rhs.Name;
		BoneOffset = rhs.BoneOffset;
		Node = rhs.Node;
		Keyframes = rhs.Keyframes;
	}
};

struct Triangle
{
	struct SurfaceData
	{
		XMFLOAT3 Normal;
		// XMFLOAT3 Binormal;
		XMFLOAT4 Tangent;
		XMFLOAT2 UV;
	};

	std::string MaterialName;
	UINT SubsetID;
	SurfaceData Data[3];

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
