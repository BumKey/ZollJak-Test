#pragma once
#ifndef GAMESTATEMGR_H
#define GAMESTATEMGR_H 
#include "stdafx.h"

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

#endif // !GAMESTATEMGR_H