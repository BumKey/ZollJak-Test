#pragma once

#include "GameStateMgr.h"
Game_Wave_Waiting::Game_Wave_Waiting()
{
	game_state_type = game_waiting_wave;
}
Game_Wave_Waiting::~Game_Wave_Waiting()
{

}

Game_Wave_Waiting* Game_Wave_Waiting::Instance()
{
	static Game_Wave_Waiting instance;

	return &instance;
}
void Game_Wave_Waiting::Enter()
{
	Sound_Mgr->hpdown = false;
	Sound_Mgr->Stop_3DSound(Sound_p_almostdie);
	Time_Mgr->SetWaveTimer(); //웨이브스위치 작동
	Time_Mgr->Set_P_HP(1000);
}

void Game_Wave_Waiting::Execute()
{
	//UI_Mgr->Change_HP_TEXT(100);
	UI_Mgr->Change_Time_TEXT(game_waiting_wave);
	UI_Mgr->Change_HP_TEXT(Time_Mgr->Get_P_HP());
	if (Time_Mgr->gamestate == game_waving)
	{
		G_State_Mgr->ChangeState(Game_Waving::Instance());
	}


}

void Game_Wave_Waiting::Exit()
{

}