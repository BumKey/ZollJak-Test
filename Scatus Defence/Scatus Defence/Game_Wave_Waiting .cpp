#pragma once
#include "ObjectMgr.h"
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
	//Sound_Mgr->Stop_3DSound(Sound_p_almostdie);
	Time_Mgr->SetWaveTimer(); //웨이브스위치 작동
	Time_Mgr->Set_P_HP(1000);
	if (Time_Mgr->Get_Wavelevel() != 0)
	{
		UI_Mgr->Set_Text_Active(L"웨이브 종료", true);
		UI_wavetimer.Reset();
	}
}

void Game_Wave_Waiting::Execute()
{
	UI_wavetimer.Tick();
	if (UI_wavetimer.TotalTime() > 1.0f)
	{
		UI_Mgr->Delete_Text(L"웨이브 종료");
	}
	//UI_Mgr->Change_HP_TEXT(100);
	UI_Mgr->Change_Time_TEXT(game_waiting_wave);
	UI_Mgr->Change_HP_TEXT(Time_Mgr->Get_P_HP());
	if (Time_Mgr->Get_Wavelevel()==10)
	{
		UI_Mgr->Change_Round_TEXT(Time_Mgr->Get_Wavelevel());
		UI_Mgr->Set_Text_Active(L"게임 클리어", true);
	}
	else {
		if (Time_Mgr->gamestate == game_waving)
		{
			G_State_Mgr->ChangeState(Game_Waving::Instance());
		}
	}

}

void Game_Wave_Waiting::Exit()
{
	UI_Mgr->Delete_Text(L"웨이브 종료");
}