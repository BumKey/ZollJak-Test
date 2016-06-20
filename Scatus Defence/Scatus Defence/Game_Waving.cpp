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
	int wavelevel = State_Mgr->Get_Wavelevel();
	wavelevel++;
	State_Mgr->Set_Wavelevel(wavelevel);
	UI_Mgr->Change_Round_TEXT(State_Mgr->Get_Wavelevel());
	Time_Mgr->SetWaveTimer(); //웨이브스위치 작동
	Rogic_Mgr->Add_Monster(State_Mgr->Get_Wavelevel());
	UI_Mgr->GetTextptr(Text_ingame_start_wave)->Active = false;
}

void Game_Waving::Execute()
{
	UI_Mgr->Change_HP_TEXT(Player::GetInstance()->GetProperty().hp_now);
	UI_Mgr->Change_Time_TEXT(game_waving);
	Rogic_Mgr->AIManager(State_Mgr->Get_Wavelevel());
	if (Time_Mgr->WaveTimer())
	{
		if (State_Mgr->Get_Wavelevel() == 100)
		{
			State_Mgr->ChangeState(Game_GameOver::Instance()); // 게임엔드로 수정해야함
		}
		else
		{

			State_Mgr->ChangeState(Game_Wave_Waiting::Instance()); // 웨이브 대기상태로 회귀
		}
	};

}

void Game_Waving::Exit()
{
	Object_Mgr->ReleaseAllMonsters();
	//맵 내부의 모든적 삭제
}