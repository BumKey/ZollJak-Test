#pragma once
#include "BasicMesh.h"
#include "SkinnedMesh.h"
#include "TextureMgr.h"
#include "Properties.h"
#include "Singletone.h"
#include <unordered_map>

#define Resource_Mgr ResourceMgr::GetInstance()

// 하나만 존재하고 여러 곳에서 참조해야하는 리소스들을
// 생성하고 참조할 수 있도록 한다.
class ResourceMgr : public Singletone<ResourceMgr>
{
private:
	ResourceMgr();
	~ResourceMgr();

	friend class Singletone<ResourceMgr>;
public:
	void Init(ID3D11Device* dc);

	static ID3D11ShaderResourceView* GoblinDiffuseMapSRV[2];

	SkinnedMesh*			GetSkinnedMesh(ObjectType::Types oType);
	BasicMesh*				GetBasicMesh(ObjectType::Types oType);

	void ReleaseMesh(ObjectType::Types oType);

private:
	ID3D11Device* mDevice;

	// Obstacles
	BasicMesh* mTreeMesh;
	BasicMesh* mBaseMesh;
	BasicMesh* mStairsMesh;
	BasicMesh* mPillar1Mesh;
	BasicMesh* mPillar2Mesh;
	BasicMesh* mPillar3Mesh;
	BasicMesh* mPillar4Mesh;
	BasicMesh* mRockMesh;
	BasicMesh* mTempleMesh;

	// 참조 횟수가 의미있는 메쉬들
	std::unordered_map<ObjectType::Types, GameMesh*> mMeshes;
	std::unordered_map<ObjectType::Types, UINT> mReferences;
};

