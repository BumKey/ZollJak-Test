#pragma once
#include "BasicMeshData.h"
#include "Vertex.h"


class GameMesh
{
public:
	GameMesh();
	~GameMesh();

public:
	UINT SubsetCount;

	std::vector<Material> Mat;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<XMFLOAT3> VerticesPos;
	std::vector<USHORT> Indices;
	std::vector<BasicMeshData::Subset> Subsets;

	BasicMeshData MeshData;
};

