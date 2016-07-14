#pragma once
#include "stdafx.h"

class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObject(ObjectType::Types oType);
	void AddPlayer(ObjectType::Types oType, DWORD client_id);

	const std::vector<ForClientInfo>&		GetAllObjects();
	std::vector<ForClientInfo>&				GetMonsters() { return mMonsters; }
	ForClientInfo							GetPlayer(const DWORD& client_id) { return mPlayers[client_id]; }
	void ReleaseAllMonsters();

private:
	void CreateMap();

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;

	std::vector<ForClientInfo>	mAllObjects;					
	std::vector<ForClientInfo>	mObstacles;						
	std::vector<ForClientInfo>	mStructures, mProjectiles;		
	std::vector<ForClientInfo>	mMonsters;						
	ForClientInfo				mPlayers[MAX_USER];				
};