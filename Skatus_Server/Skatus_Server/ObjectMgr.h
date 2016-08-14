#pragma once
#include "stdafx.h"

class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObject(ObjectType::Types oType);
	void AddPlayer(ObjectType::Types oType, DWORD client_id);

	const std::vector<ObjectInfo>&		GetAllObjects();
	std::vector<ObjectInfo>&				GetMonsters() { return mMonsters; }
	ObjectInfo							GetPlayer(const DWORD& client_id) { return mPlayers[client_id]; }
	void ReleaseAllMonsters();

private:
	void CreateMap();

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	std::vector<ObjectInfo>	mAllObjects;					
	std::vector<ObjectInfo>	mObstacles;						
	std::vector<ObjectInfo>	mStructures, mProjectiles;		
	std::vector<ObjectInfo>	mMonsters;						
	ObjectInfo				mPlayers[MAX_USER];				
};