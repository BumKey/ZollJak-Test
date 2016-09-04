#include "GameStateMgr.h"



GameStateMgr::GameStateMgr()
{
	mCurrState = eGameState::GameWaiting;
}


GameStateMgr::~GameStateMgr()
{
}

void GameStateMgr::FlowAdvance()
{
	if (mCurrState == eGameState::GameWaiting)		mCurrState = eGameState::WaveWaiting;
	else if (mCurrState == eGameState::WaveWaiting) mCurrState = eGameState::WaveStart;
	else if (mCurrState == eGameState::WaveStart)	mCurrState = eGameState::Waving;
	// else if (mCurrState == eGameState::Waving)		mCurrState = eGameState::WaveWaiting;
}
