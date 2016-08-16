#pragma once
#include "Protocol.h"

class GameStateMgr
{
public:
	GameStateMgr();
	~GameStateMgr();

	void Reset() { mCurrState = eGameState::GameWaiting; }
	void FlowAdvance();

	void		SetGameOver() { mCurrState = eGameState::GameOver; }
	eGameState	GetCurrState() const { return mCurrState; }
private:
	eGameState mCurrState;
	int Round_level;
};

