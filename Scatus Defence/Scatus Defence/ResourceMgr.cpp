#include "ResourceMgr.h"

ID3D11ShaderResourceView* ResourceMgr::GoblinDiffuseMapSRV[] = { 0 };

ResourceMgr::ResourceMgr() : mGoblinReferences(0)
{
}

ResourceMgr::~ResourceMgr()
{
	SafeDelete(TreeMesh);
	/*SafeDelete(BaseMesh);
	SafeDelete(StairsMesh);
	SafeDelete(Pillar1Mesh);
	SafeDelete(Pillar2Mesh);
	SafeDelete(Pillar3Mesh);
	SafeDelete(Pillar4Mesh);*/
	SafeDelete(RockMesh);
}

void ResourceMgr::Init(ID3D11Device * dc)
{
	mDevice = dc;
	mTexMgr.Init(mDevice);

	Temple = new BasicMesh(mDevice, mTexMgr, "Models\\house1.y2k", L"Textures\\house1\\");
	TreeMesh = new BasicMesh(mDevice, mTexMgr, "Models\\tree.m3d", L"Textures\\");
	//BaseMesh = new BasicMesh(dc, texMgr, "Meshs\\base.m3d", L"Textures\\");
	//StairsMesh = new BasicMesh(dc, texMgr, "Meshs\\stairs.m3d", L"Textures\\");
	//Pillar1Mesh = new BasicMesh(dc, texMgr, "Meshs\\pillar1.m3d", L"Textures\\");
	//Pillar2Mesh = new BasicMesh(dc, texMgr, "Meshs\\pillar2.m3d", L"Textures\\");
	//Pillar3Mesh = new BasicMesh(dc, texMgr, "Meshs\\pillar5.m3d", L"Textures\\");
	//Pillar4Mesh = new BasicMesh(dc, texMgr, "Meshs\\pillar6.m3d", L"Textures\\");
	RockMesh = new BasicMesh(mDevice, mTexMgr, "Models\\rock.m3d", L"Textures\\");

}

SkinnedMesh * ResourceMgr::GetGoblinMesh()
{
	if (mGoblinReferences <= 0) {
		mGoblinMesh = new SkinnedMesh(mDevice, mTexMgr, "Models\\goblin.y2k", L"");
		GoblinDiffuseMapSRV[0] = mTexMgr.CreateTexture(L"Textures\\goblin\\goblin_diff_R.jpg");
		GoblinDiffuseMapSRV[1] = mTexMgr.CreateTexture(L"Textures\\goblin\\goblin_diff_B.jpg");
		++mGoblinReferences;
	}
	else
		++mGoblinReferences;

	return mGoblinMesh;
}

void ResourceMgr::ReleaseGoblinMesh()
{
	--mGoblinReferences; 
	if (mGoblinReferences <= 0)
	{
		SafeDelete(mGoblinMesh);
		for (auto i : GoblinDiffuseMapSRV)
			mTexMgr.Delete(i);
	}
}
