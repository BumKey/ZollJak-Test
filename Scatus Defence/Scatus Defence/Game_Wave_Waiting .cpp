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
	Time_Mgr->SetWaveTimer(); //웨이브스위치 작동

}

void Game_Wave_Waiting::Execute()
{

	UI_Mgr->Change_Time_TEXT(game_waiting_wave);

	UI_Mgr->Change_HP_TEXT(Player::GetInstance()->GetProperty().hp_now);
	if (Time_Mgr->Get_remain_Wait_time() < 2)
	{
		UI_Mgr->GetTextptr(Text_ingame_start_wave)->Active = true;
	}
	if (Time_Mgr->WaveTimer())
	{
		
		State_Mgr->ChangeState(Game_Waving::Instance()); // 웨이브 상태로 회귀

	};

}

void Game_Wave_Waiting::Exit()
{

}