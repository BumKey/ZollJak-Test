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

	void WaveStart();
	void Update();
	void ProcessKeyInput(cs_packet_move& inPacket);
	void ProcessMouseInput(cs_packet_attack& inPacket);

private:
	FLOAT Distance2D(const XMFLOAT3& a, const XMFLOAT3& b);
	void SendPacket();

private:

	UINT mCurrWaveLevel;
	
	GameTimer		mGameTimer;
	GameTimer		mRogicTimer;
	GameStateMgr	mGameStateMgr;
	ObjectMgr		mObjectMgr;

	std::unordered_map<UINT, std::unordered_map<ObjectType::Types, UINT>> mPerWaveMonsterNum;
};

