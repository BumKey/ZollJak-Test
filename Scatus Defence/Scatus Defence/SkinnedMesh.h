#ifndef SkinnedMesh_H
#define SkinnedMesh_H

#include "SkinnedData.h"
#include "TextureMgr.h"
#include "Vertex.h"
#include "GameMesh.h"

class SkinnedMesh : public GameMesh
{
public:
	SkinnedMesh(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath);
	~SkinnedMesh();

	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex::PosNormalTexTanSkinned> Vertices;
	SkinnedData SkinnedData;
};

#endif // SkinnedMesh_H