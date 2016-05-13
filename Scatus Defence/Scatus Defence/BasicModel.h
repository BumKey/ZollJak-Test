#ifndef BASICMODEL_H
#define BASICMODEL_H

#include "BasicMeshData.h"
#include "TextureMgr.h"
#include "Vertex.h"
#include "GameModel.h"

class BasicModel : public GameModel
{
public:
	BasicModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath);
	~BasicModel();

	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex::PosNormalTexTan> Vertices;
};

#endif // SKINNEDMODEL_H