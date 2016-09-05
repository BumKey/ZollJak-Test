#pragma once

#include "GameStateMgr.h"
Game_Waving::Game_Waving()
{
	game_state_type = game_waving;
}
Game_Waving::~Game_Waving()
{

}

Game_Waving* Game_Waving::Instance()
{
	static Game_Waving instance;

	return &instance;
}
void Game_Waving::Enter()
{

	
	UI_Mgr->Change_Round_TEXT(Time_Mgr->Get_Wavelevel());
	//Time_Mgr->SetWaveTimer(); //���̺꽺��ġ �۵�
	
}

void Game_Waving::Execute()
{

	//UI_Mgr->Change_HP_TEXT(50);
	UI_Mgr->Change_Time_TEXT(game_waving);
	if (Time_Mgr->gamestate == game_waiting_wave)
	{
		G_State_Mgr->ChangeState(Game_Wave_Waiting::Instance());
	}
	
		if (Time_Mgr->Get_Wavelevel() == 100)
		{
			//G_State_Mgr->ChangeState(Game_GameOver::Instance()); // ���ӿ���� �����ؾ���
		}
		
	

}

void Game_Waving::Exit()
{


	//�� ������ ����� ����

}