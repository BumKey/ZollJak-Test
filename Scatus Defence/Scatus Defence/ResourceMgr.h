#pragma once
#include "BasicMesh.h"
#include "SkinnedMesh.h"
#include "TextureMgr.h"

// 하나만 존재하고 여러 곳에서 참조해야하는 리소스들을
// 생성하고 참조할 수 있도록 한다.
class ResourceMgr
{
public:
	ResourceMgr();
	~ResourceMgr();

	void Init(ID3D11Device* dc);

	// Obstacles
	BasicMesh* TreeMesh;
	BasicMesh* BaseMesh;
	BasicMesh* StairsMesh;
	BasicMesh* Pillar1Mesh;
	BasicMesh* Pillar2Mesh;
	BasicMesh* Pillar3Mesh;
	BasicMesh* Pillar4Mesh;
	BasicMesh* RockMesh;
	BasicMesh* Temple;

	SkinnedMesh* GetGoblinMesh();
	static ID3D11ShaderResourceView* GoblinDiffuseMapSRV[2];
	void ReleaseGoblinMesh();

private:
	ID3D11Device* mDevice;
	TextureMgr mTexMgr;

	UINT mGoblinReferences;

	// 참조 횟수가 의미있는 메쉬들
	SkinnedMesh* mGoblinMesh;
};

