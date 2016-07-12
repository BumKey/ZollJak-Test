#include "GameStateMgr.h"



GameStateMgr::GameStateMgr()
{
	mCurrState = GameState::GameWaiting;
}


GameStateMgr::~GameStateMgr()
{
}

void GameStateMgr::FlowAdvance()
{
	if (mCurrState == GameState::GameWaiting)
		mCurrState = GameState::WaveWaiting;
	else if (mCurrState == GameState::WaveWaiting && mCurrState != GameState::GameOver)
		mCurrState = GameState::WaveStart;
	else if (mCurrState == GameState::WaveStart && mCurrState != GameState::GameOver)
		mCurrState = GameState::Waving;
	else if (mCurrState == GameState::Waving && mCurrState != GameState::GameOver)
		mCurrState = GameState::WaveWaiting;
}
