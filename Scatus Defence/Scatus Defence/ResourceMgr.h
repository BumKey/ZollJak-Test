#pragma once
#include "BasicMesh.h"
#include "SkinnedMesh.h"
#include "TextureMgr.h"
#include "Properties.h"
#include <unordered_map>

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

	static ID3D11ShaderResourceView* GoblinDiffuseMapSRV[2];

	SkinnedMesh* GetSkinnedMesh(Object_type oType);
	void ReleaseMesh(Object_type oType);

private:
	ID3D11Device* mDevice;
	TextureMgr mTexMgr;

	// 참조 횟수가 의미있는 메쉬들
	std::unordered_map<Object_type, SkinnedMesh*> mMeshes;
	std::unordered_map<Object_type, UINT> mReferences;
};

