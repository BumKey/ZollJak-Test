#pragma once
#include "stdafx.h"
#include "server.h"
#include "GameStateMgr.h"
#include "GameTimer.h"
#include "Server_Collision.h"
#include "ObjectMgr.h"

class ServerRogicMgr
{
public:
	ServerRogicMgr();
	~ServerRogicMgr();

	void WaveStart();
	void Update();
	void AddPlayer(const SOCKET& socket, const ObjectType::Types& oType, const UINT& id);
	void ProcessKeyInput(CS_Move& inPacket);
	void ProcessMouseInput(CS_Attack& inPacket);
	UINT GetWaveLevel() { return mCurrWaveLevel; }
	UINT SetWaveLevel(int num) { mCurrWaveLevel= num; }
	const SO_InitDesc& GetPlayerInfo(UINT id) { return mObjectMgr.GetPlayer()[id]; }
private:
	FLOAT Distance2D(const XMFLOAT3& a, const XMFLOAT3& b);
	void SendPacketPerFrame();
	void SendPacketToCreateMonsters();

private:
	UINT mCurrWaveLevel;
	UINT mCurrPlayerNum;

	GameTimer		mGameTimer;
	GameTimer		mRogicTimer;
	GameStateMgr	mGameStateMgr;
	ObjectMgr		mObjectMgr;
	Server_Collision  mCollisionMgr;

	std::unordered_map<UINT, std::unordered_map<ObjectType::Types, UINT>> mPerWaveMonsterNum;
};

