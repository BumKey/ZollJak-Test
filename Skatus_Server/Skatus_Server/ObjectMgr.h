#pragma once
#include "stdafx.h"

class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObject(ObjectType::Types oType);
	void AddPlayer(ObjectType::Types oType, DWORD client_id);

	void SetPlayerPos(const UINT& id, const XMFLOAT3& pos) { mPlayers[id].Pos = pos; }

	const std::unordered_map<UINT, SO_InitDesc>	GetAllSkinnedObjects();
	const std::unordered_map<UINT, SO_InitDesc>&	GetMonsters() { return mMonsters; }
	const std::vector<BO_InitDesc>& GetAllBasicObjects() { return mObstacles; }
	const auto						GetPlayer() { return mPlayers; }
	void ReleaseAllMonsters();

private:
	void CreateMap();

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;
	UINT mObjectGeneratedNum;
	UINT mCurrPlayerNum;

	std::vector<BO_InitDesc>	mObstacles;						
	std::vector<SO_InitDesc>	mStructures, mProjectiles;		
	std::unordered_map<UINT, SO_InitDesc>	 mMonsters;						
	SO_InitDesc				mPlayers[MAX_USER];				
};