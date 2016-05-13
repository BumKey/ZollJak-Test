#include "ResourceMgr.h"

BasicModel* ResourceMgr::TreeModel = 0;
BasicModel* ResourceMgr::BaseModel = 0;
BasicModel* ResourceMgr::StairsModel = 0;
BasicModel* ResourceMgr::Pillar1Model = 0;
BasicModel* ResourceMgr::Pillar2Model = 0;
BasicModel* ResourceMgr::Pillar3Model = 0;
BasicModel* ResourceMgr::Pillar4Model = 0;
BasicModel* ResourceMgr::RockModel = 0;

void ResourceMgr::InitAll(ID3D11Device * dc, TextureMgr& texMgr)
{
	TreeModel = new BasicModel(dc, texMgr, "Models\\tree.m3d", L"Textures\\");
	BaseModel = new BasicModel(dc, texMgr, "Models\\base.m3d", L"Textures\\");
	StairsModel = new BasicModel(dc, texMgr, "Models\\stairs.m3d", L"Textures\\");
	Pillar1Model = new BasicModel(dc, texMgr, "Models\\pillar1.m3d", L"Textures\\");
	Pillar2Model = new BasicModel(dc, texMgr, "Models\\pillar2.m3d", L"Textures\\");
	Pillar3Model = new BasicModel(dc, texMgr, "Models\\pillar5.m3d", L"Textures\\");
	Pillar4Model = new BasicModel(dc, texMgr, "Models\\pillar6.m3d", L"Textures\\");
	RockModel = new BasicModel(dc, texMgr, "Models\\rock.m3d", L"Textures\\");
}

void ResourceMgr::DestroyAll()
{
	SafeDelete(TreeModel);
	SafeDelete(BaseModel);
	SafeDelete(StairsModel);
	SafeDelete(Pillar1Model);
	SafeDelete(Pillar2Model);
	SafeDelete(Pillar3Model);
	SafeDelete(Pillar4Model);
	SafeDelete(RockModel);
}
