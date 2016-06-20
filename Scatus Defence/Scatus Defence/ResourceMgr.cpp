#include "ResourceMgr.h"

ID3D11ShaderResourceView* ResourceMgr::GoblinDiffuseMapSRV[] = { 0 };

ResourceMgr::ResourceMgr()
{
}

ResourceMgr::~ResourceMgr()
{
	SafeDelete(mTreeMesh);
	/*SafeDelete(BaseMesh);
	SafeDelete(StairsMesh);
	SafeDelete(Pillar1Mesh);
	SafeDelete(Pillar2Mesh);
	SafeDelete(Pillar3Mesh);
	SafeDelete(Pillar4Mesh);*/
	SafeDelete(mRockMesh);
}

void ResourceMgr::Init(ID3D11Device * dc)
{
	mDevice = dc;

	mTempleMesh = new BasicMesh(mDevice, "Models\\house1.y2k", L"Textures\\house1\\");
	mTreeMesh = new BasicMesh(mDevice, "Models\\tree.y2k", L"Textures\\");
	//BaseMesh = new BasicMesh(dc, texMgr, "Meshs\\base.m3d", L"Textures\\");
	//StairsMesh = new BasicMesh(dc, texMgr, "Meshs\\stairs.m3d", L"Textures\\");
	//Pillar1Mesh = new BasicMesh(dc, texMgr, "Meshs\\pillar1.m3d", L"Textures\\");
	//Pillar2Mesh = new BasicMesh(dc, texMgr, "Meshs\\pillar2.m3d", L"Textures\\");
	//Pillar3Mesh = new BasicMesh(dc, texMgr, "Meshs\\pillar5.m3d", L"Textures\\");
	//Pillar4Mesh = new BasicMesh(dc, texMgr, "Meshs\\pillar6.m3d", L"Textures\\");
	mRockMesh = new BasicMesh(mDevice, "Models\\rock.y2k", L"Textures\\");

}

SkinnedMesh * ResourceMgr::GetSkinnedMesh(ObjectType::Types oType)
{
	if (mReferences[oType] <= 0) {
		++mReferences[oType];
		switch (oType)
		{
		case ObjectType::Goblin:
			mMeshes[oType] = new SkinnedMesh(mDevice, "Models\\goblin.y2k", L"Textures\\goblin\\");
			GoblinDiffuseMapSRV[0] = Texture_Mgr->CreateTexture(L"Textures\\goblin\\goblin_diff_R.jpg");
			GoblinDiffuseMapSRV[1] = Texture_Mgr->CreateTexture(L"Textures\\goblin\\goblin_diff_B.jpg");
			break;
		case ObjectType::Cyclop:
			mMeshes[oType] = new SkinnedMesh(mDevice, "Models\\cyclop.y2k", L"Textures\\cyclop\\");
			break;
	
		default:
			assert(false, "Uncorrect SkinnedMesh Type");
		}	
	}
	else
		++mReferences[oType];

	return static_cast<SkinnedMesh*>(mMeshes[oType]);
}

BasicMesh * ResourceMgr::GetBasicMesh(ObjectType::Types oType)
{
	switch (oType)
	{
	case ObjectType::Temple:
		return mTempleMesh;
		break;
	case ObjectType::Tree:
		return mTreeMesh;
		break;
	case ObjectType::Rock:
		return mRockMesh;
		break;

	default:
		assert(false, "Uncorrect BasicMesh Type");
	}
}

void ResourceMgr::ReleaseMesh(ObjectType::Types oType)
{
	--mReferences[oType]; 
	if (mReferences[oType] <= 0)
	{
		SafeDelete(mMeshes[oType]);
		mMeshes.erase(oType);
			if (oType == ObjectType::Goblin) {
			for (auto i : GoblinDiffuseMapSRV)
				Texture_Mgr->Delete(i);
		}
	}
}

