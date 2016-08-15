#pragma once
#include "stdafx.h"

class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObject(ObjectType::Types oType);
	void AddPlayer(ObjectType::Types oType, DWORD client_id);

	void SetPlayerRot(const UINT& id, const XMFLOAT3& rot) { mPlayers[id].Rot = rot; }
	void SetPlayerPosXZ(const UINT& id, const XMFLOAT3& pos) { mPlayers[id].Pos = pos; }
	void SetPlayerPosY(const UINT& id, const FLOAT& y) { mPlayerYpos[id] = y; }

	const std::unordered_map<UINT, SO_InitDesc>	GetAllSkinnedObjects();
	const std::unordered_map<UINT, SO_InitDesc>&	GetMonsters() { return mMonsters; }
	const std::vector<BO_InitDesc>& GetAllBasicObjects() { return mObstacles; }
	const SO_InitDesc&				GetPlayer(const UINT& id);
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
	float					mPlayerYpos[MAX_USER];
};