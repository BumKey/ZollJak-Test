#include "BasicMesh.h"
#include "LoadY2K.h"

BasicMesh::BasicMesh(ID3D11Device* device, const std::string& modelFilename, const std::wstring& texturePath)
{
	std::vector<Y2kMaterial> mats;
	Y2KLoader y2kLoader;
	y2kLoader.LoadY2K(modelFilename, Vertices, Indices, Subsets, mats);

	MeshData.SetVertices(device, &Vertices[0], Vertices.size());
	MeshData.SetIndices(device, &Indices[0], Indices.size());
	MeshData.SetSubsetTable(Subsets);

	SubsetCount = mats.size();

	for(UINT i = 0; i < SubsetCount; ++i)
	{
		Mat.push_back(mats[i].Mat);

		ID3D11ShaderResourceView* diffuseMapSRV = Texture_Mgr->CreateTexture(texturePath + mats[i].DiffuseMapName);
		DiffuseMapSRV.push_back(diffuseMapSRV);

		ID3D11ShaderResourceView* normalMapSRV = Texture_Mgr->CreateTexture(texturePath + mats[i].NormalMapName);
		NormalMapSRV.push_back(normalMapSRV);
	}

	XNA::ComputeBoundingAxisAlignedBoxFromPoints(&mMeshAABB,
		Vertices.size(), &Vertices[0].Pos, sizeof(Vertex::PosNormalTexTan));
}

BasicMesh::~BasicMesh()
{
}
