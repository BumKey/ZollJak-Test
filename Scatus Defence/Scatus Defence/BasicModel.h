#ifndef BASICMODEL_H
#define BASICMODEL_H

#include "MeshData.h"
#include "TextureMgr.h"
#include "Vertex.h"

// 모델을 불러오고 매쉬데이터를 저장해놓은 클래스
class BasicModel
{
public:
	BasicModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath);
	~BasicModel();

	UINT SubsetCount;

	std::vector<Material> Mat;
	std::vector<ID3D11ShaderResourceView*> DiffuseMapSRV;
	std::vector<ID3D11ShaderResourceView*> NormalMapSRV;

	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex::PosNormalTexTan> Vertices;
	std::vector<USHORT> Indices;
	std::vector<MeshData::Subset> Subsets;

	MeshData ModelMesh;
};

//struct BasicModelInstance
//{
//	BasicModel* Model;
//	XMFLOAT4X4 World;
//};

#endif // SKINNEDMODEL_H