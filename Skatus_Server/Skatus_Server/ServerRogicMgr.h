#pragma once
#include "stdafx.h"
#include "server.h"
#include "GameStateMgr.h"
#include "GameTimer.h"
#include "ObjectMgr.h"

class ServerRogicMgr
{
public:
	ServerRogicMgr();
	~ServerRogicMgr();

	void Update();
	void WaveStart();
	void AddPlayer(const SOCKET& socket, const ObjectType::Types& oType, const UINT& id);
	void RemovePlayer(const UINT& id);
	void ProcessKeyInput(CS_Move& inPacket);
	void ProcessMouseInput(CS_Attack& inPacket);

	const SO_InitDesc& GetPlayerInfo(UINT id) { return mObjectMgr.GetPlayer(id); }

	void SendPacketFrameInfo();
	void SendPacektPlayerInfo();

private:
	void SendPacketToCreateMonsters();

	FLOAT Distance2D(const XMFLOAT3& a, const XMFLOAT3& b);

private:

	UINT mNewID;
	UINT mCurrWaveLevel;
	UINT mCurrPlayerNum;

	GameTimer		mGameTimer;
	GameTimer		mRogicTimer;
	GameStateMgr	mGameStateMgr;
	ObjectMgr		mObjectMgr;

	std::unordered_map<UINT, std::unordered_map<ObjectType::Types, UINT>> mPerWaveMonsterNum;
};

