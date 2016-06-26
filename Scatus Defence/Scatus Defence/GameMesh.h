#pragma once
#include "BasicMeshData.h"
#include "xnacollision.h"
#include "Vertex.h"


class GameMesh
{
public:
	GameMesh();
	~GameMesh();

public:
	XNA::AxisAlignedBox GetAABB() const { return mMeshAABB; }

public:
	UINT SubsetCount;

	std::vector<Material> Mat;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<USHORT> Indices;
	std::vector<BasicMeshData::Subset> Subsets;

	BasicMeshData MeshData;

protected:
	XNA::AxisAlignedBox mMeshAABB;
};

