#pragma once
#include "BasicModel.h"
#include "SkinnedModel.h"
#include <TextureMgr.h>


// 하나만 존재하고 여러 곳에서 참조해야하는 리소스들을
// 생성하고 참조할 수 있도록 한다.
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

