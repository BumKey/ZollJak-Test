#ifndef SKINNEDMODEL_H
#define SKINNEDMODEL_H

#include "SkinnedMeshData.h"
#include "BasicMeshData.h"
#include "TextureMgr.h"
#include "Vertex.h"
#include "GameModel.h"

class SkinnedModel : public GameModel
{
public:
	SkinnedModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath);
	~SkinnedModel();

	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex::PosNormalTexTanSkinned> Vertices;
	SkinnedMeshData SkinnedMeshData;
};

#endif // SKINNEDMODEL_H