
#include "GameStateMgr.h"


GameStateMgr::GameStateMgr()
{
	m_wave_level = 0;
	m_pCurrentState = Game_Title::Instance();//�ʱ����
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

								 //printf("\n\n\n\n\n\n\n���� �÷��� Ÿ�� : %d��",mRogicTimer.GetPlayTime());
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
	printf("������ ���۵Ǿ����ϴ�.");
	mRogicTimer.SetBeforeTime(); //�÷���Ÿ�� ��������
	mRogicTimer.SetWaveTimer(); //���̺꽺��ġ �۵�
	
}
*/