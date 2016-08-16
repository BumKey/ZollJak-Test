#include "GameStateMgr.h"



GameStateMgr::GameStateMgr()
{
	mCurrState = eGameState::GameWaiting;
	Round_level = 1;
}


GameStateMgr::~GameStateMgr()
{
}

void GameStateMgr::FlowAdvance()
{
	if (mCurrState == eGameState::GameWaiting)
		mCurrState = eGameState::WaveWaiting;
	else if (mCurrState == eGameState::WaveWaiting && mCurrState != eGameState::GameOver)
		mCurrState = eGameState::WaveStart;
	else if (mCurrState == eGameState::WaveStart && mCurrState != eGameState::GameOver)
		mCurrState = eGameState::Waving;
	else if (mCurrState == eGameState::Waving && mCurrState != eGameState::GameOver)
		mCurrState = eGameState::WaveWaiting;
	
}
