#pragma once
#include "protocol.h"

class GameStateMgr
{
public:
	GameStateMgr() { mCurrState = eGameState::GameWaiting; }
	~GameStateMgr();

	void Reset() { mCurrState = eGameState::GameWaiting; }
	void FlowAdvance();

	void SetGameOver() { mCurrState = eGameState::GameOver; }
	eGameState GetCurrState() const { return mCurrState; }

private:
	eGameState mCurrState;
};

