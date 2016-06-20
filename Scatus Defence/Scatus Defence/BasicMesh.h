#ifndef BasicMesh_H
#define BasicMesh_H

#include "BasicMeshData.h"
#include "TextureMgr.h"
#include "Vertex.h"
#include "GameMesh.h"

class BasicMesh : public GameMesh
{
public:
	BasicMesh(ID3D11Device* device, const std::string& modelFilename, const std::wstring& texturePath);
	~BasicMesh();

public:
	// Keep CPU copies of the mesh data to read from.  
	std::vector<Vertex::PosNormalTexTan> Vertices;
};

#endif // SKINNEDMODEL_H