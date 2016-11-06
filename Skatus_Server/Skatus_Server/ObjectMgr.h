#pragma once
#include "stdafx.h"

struct S_MonInfo
{
	XMFLOAT3 CurrPos;
	XMFLOAT3 TargetPos;
};

class ObjectMgr
{
public:
	ObjectMgr();
	~ObjectMgr();

	void UpdateMonsters();

	void AddObject(ObjectType::Types oType);
	void AddPlayer(ObjectType::Types oType, DWORD client_id);

	void RemovePlayer(const UINT& id);

	UINT GetCurrPlayerNum() const { return mCurrPlayerNum; }
	const XMFLOAT3 GetTemplePos() { return mTemplePos; }

	void SetPlayerRot(const UINT& id, const XMFLOAT3& rot) { mPlayers[id].Rot = rot; }
	void SetPlayerPos(const UINT& id, XMFLOAT3 pos) { pos.y = -0.1f; mPlayers[id].Pos = pos; }
	void SetCollsion(const UINT& id, const XMFLOAT3& pos);
	void SetTemple_Collision(const UINT& id, const XMFLOAT3& pos);
	std::unordered_map<UINT, SO_InitDesc>		GetPlayers();
	std::unordered_map<UINT, SO_InitDesc>&		GetMonsters() { return mMonsters; }
	XMFLOAT3*	GetCollisionPos() { return mCollisionPos; }
	std::vector<BO_InitDesc>& GetAllBasicObjects() { return mObstacles; }
	SO_InitDesc&				GetPlayer(const UINT& id);
	void ReleaseAllMonsters();

private:
	void CreateMap();
	const UINT SetMonstersTarget(const XMFLOAT3& pos);

private:
	UINT mStage;
	UINT mMaxMonsters;
	UINT mMaxStructures;
	UINT mTotalObjectNum;
	UINT mMonsterGeneratedNum;
	UINT mCurrPlayerNum;

	const XMFLOAT3 mTemplePos;

	XMFLOAT3 mCollisionPos[10];

	std::vector<BO_InitDesc>	mObstacles;						
	std::vector<SO_InitDesc>	mStructures, mProjectiles;		
	std::unordered_map<UINT, SO_InitDesc>	 mMonsters;						
	std::unordered_map<UINT, S_MonInfo>		 mMonInfo;
	SO_InitDesc				mPlayers[MAX_USER];		
	bool					mConnected[MAX_USER];
};