#pragma once
#include "Protocol.h"

class GameStateMgr
{
public:
	GameStateMgr();
	~GameStateMgr();

	void FlowAdvance();

	void		SetGameOver() { mCurrState = GameState::GameOver; }
	GameState	GetCurrState() const { return mCurrState; }
private:
	GameState mCurrState;
};

