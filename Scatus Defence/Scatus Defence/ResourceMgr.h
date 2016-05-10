#pragma once
#include "BasicModel.h"
#include "SkinnedModel.h"
#include <TextureMgr.h>


// �ϳ��� �����ϰ� ���� ������ �����ؾ��ϴ� ���ҽ�����
// �����ϰ� ������ �� �ֵ��� �Ѵ�.
class ResourceMgr
{
public:
	static void InitAll(ID3D11Device* dc, TextureMgr& texMgr);
	static void DestroyAll();

	static BasicModel* TreeModel;
	static BasicModel* BaseModel;
	static BasicModel* StairsModel;
	static BasicModel* Pillar1Model;
	static BasicModel* Pillar2Model;
	static BasicModel* Pillar3Model;
	static BasicModel* Pillar4Model;
	static BasicModel* RockModel;

	static SkinnedModel* Goblin;
};

