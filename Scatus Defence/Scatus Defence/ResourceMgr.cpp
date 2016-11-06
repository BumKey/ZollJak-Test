#include "ResourceMgr.h"

ID3D11ShaderResourceView* ResourceMgr::GoblinDiffuseMapSRV[] = { 0 };

ResourceMgr::ResourceMgr()
{
}

ResourceMgr::~ResourceMgr()
{
	SafeDelete(mTreeMesh);
	SafeDelete(mTempleMesh);
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

	//mTempleMesh = new BasicMesh(mDevice, "Models\\house1.y2k", L"Textures\\house1\\");
	mTreeMesh = new BasicMesh(mDevice, "Models\\tree.y2k", L"Textures\\");
	mBaseMesh = new BasicMesh(mDevice, "Models\\base.y2k", L"Textures\\");
	mStairsMesh = new BasicMesh(mDevice, "Models\\stairs.y2k", L"Textures\\");
	mPillar1Mesh = new BasicMesh(mDevice, "Models\\pillar1.y2k", L"Textures\\");
	mPillar2Mesh = new BasicMesh(mDevice, "Models\\pillar2.y2k", L"Textures\\");
	mPillar3Mesh = new BasicMesh(mDevice, "Models\\pillar5.y2k", L"Textures\\");
	mPillar4Mesh = new BasicMesh(mDevice, "Models\\pillar6.y2k", L"Textures\\");
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

		case ObjectType::Warrior:
			mMeshes[oType] = new SkinnedMesh(mDevice, "Models\\warrior.y2k", L"Textures\\warrior\\");
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
		return mBaseMesh;
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

