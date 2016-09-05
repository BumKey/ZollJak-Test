
#include "GameStateMgr.h"


GameStateMgr::GameStateMgr()
{
	m_wave_level = 0;
	m_pCurrentState = Game_Title::Instance();//초기상태
	ChangeState(Game_Title::Instance());
	m_pGlobalState = Game_GlobalState::Instance();
	
}


GameStateMgr::~GameStateMgr()
{
}

GameStateMgr* GameStateMgr::Instance()
{
	static GameStateMgr instance;

	return &instance;
}

void GameStateMgr::Update()
{

								 //printf("\n\n\n\n\n\n\n현재 플레이 타임 : %d초",mRogicTimer.GetPlayTime());
	m_pCurrentState->Execute();
	m_pGlobalState->Execute();
}
void GameStateMgr::ChangeState(GameState* pNewState)
{

	m_pCurrentState->Exit();

	m_pCurrentState = pNewState;

	gamestate_cur = pNewState->Get_GameState_type();

	m_pCurrentState->Enter();
}
/*
GameStateMgr::GameStateMgr()
{

	GameState = game_waiting_wave;
	printf("게임이 시작되었습니다.");
	mRogicTimer.SetBeforeTime(); //플레이타임 측정시작
	mRogicTimer.SetWaveTimer(); //웨이브스위치 작동
	
}
*/