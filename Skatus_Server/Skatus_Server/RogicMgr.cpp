#include "RogicMgr.h"

RogicMgr::RogicMgr() : mCurrWaveLevel(0)
{
	mPerWaveMonsterNum[1][ObjectType::Goblin] = 15;
	mPerWaveMonsterNum[1][ObjectType::Cyclop] = 2;

	mPerWaveMonsterNum[2][ObjectType::Goblin] = 20;
	mPerWaveMonsterNum[2][ObjectType::Cyclop] = 5;

	mPerWaveMonsterNum[3][ObjectType::Goblin] = 40;
	mPerWaveMonsterNum[3][ObjectType::Cyclop] = 10;
}


RogicMgr::~RogicMgr()
{
}

void RogicMgr::Update()
{
	if (mGameStateMgr.GetCurrState == GameState::WaveStart)
	{
		// ���� �޴������� ������ ���ְ�
		for (auto oType : mPerWaveMonsterNum[mCurrWaveLevel]) {
			for (UINT i = 0; i < oType.second; ++i)
				mObjectMgr.AddObject(oType.first);
		}
	}
}
