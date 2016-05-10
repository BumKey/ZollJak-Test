#ifndef SKINNEDMODEL_H
#define SKINNEDMODEL_H

#include "SkinnedMeshData.h"
#include "BasicMeshData.h"
#include "TextureMgr.h"
#include "Vertex.h"

class SkinnedModel
{
public:
	SkinnedModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath);
	~SkinnedModel();

	UINT SubsetCount;

	std::vector<Material> Mat;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex::PosNormalTexTanSkinned> Vertices;
	std::vector<USHORT> Indices;
	std::vector<BasicMeshData::Subset> Subsets;

	BasicMeshData ModelMesh;
	SkinnedMeshData SkinnedMeshData;
};

#endif // SKINNEDMODEL_H