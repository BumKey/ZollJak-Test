#pragma once
#include "stdafx.h"

class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void AddObject(ObjectType::Types oType);
	void AddPlayer(ObjectType::Types oType, DWORD client_id);

	void RemovePlayer(const UINT& id);

	UINT GetCurrPlayerNum() const { return mCurrPlayerNum; }
	const XMFLOAT3 GetTemplePos() { return mTemplePos; }

	void SetPlayerRot(const UINT& id, const XMFLOAT3& rot) { mPlayers[id].Rot = rot; }
	void SetPlayerPos(const UINT& id, XMFLOAT3 pos) { pos.y = -0.1f; mPlayers[id].Pos = pos; }
	void SetMonstersTarget();

	const std::unordered_map<UINT, SO_InitDesc>		GetPlayers();
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
	UINT mMonsterGeneratedNum;
	UINT mCurrPlayerNum;

	const XMFLOAT3 mTemplePos;

	std::vector<BO_InitDesc>	mObstacles;						
	std::vector<SO_InitDesc>	mStructures, mProjectiles;		
	std::unordered_map<UINT, SO_InitDesc>	 mMonsters;						
	SO_InitDesc				mPlayers[MAX_USER];		
	bool					mConnected[MAX_USER];
};