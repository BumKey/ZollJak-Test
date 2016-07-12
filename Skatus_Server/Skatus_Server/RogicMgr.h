#pragma once
#include "stdafx.h"
#include "GameStateMgr.h"
#include "ObjectMgr.h"

class RogicMgr
{
public:
	RogicMgr();
	~RogicMgr();

	void Update();

private:
	UINT mCurrWaveLevel;
	
	GameStateMgr	mGameStateMgr;
	ObjectMgr		mObjectMgr;

	std::unordered_map<UINT, std::unordered_map<ObjectType::Types, UINT>> mPerWaveMonsterNum;
};

