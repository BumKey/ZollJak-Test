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
	int wavelevel = G_State_Mgr->Get_Wavelevel();
	wavelevel++;
	G_State_Mgr->Set_Wavelevel(wavelevel);
	UI_Mgr->Change_Round_TEXT(G_State_Mgr->Get_Wavelevel());
	Time_Mgr->SetWaveTimer(); //���̺꽺��ġ �۵�
	G_Rogic_Mgr->Add_Monster(G_State_Mgr->Get_Wavelevel());
	UI_Mgr->GetTextptr(Text_ingame_start_wave)->Active = false;
}

void Game_Waving::Execute()
{

	UI_Mgr->Change_HP_TEXT(G_Rogic_Mgr->GetPlayer()->GetProperty().hp_now);
	UI_Mgr->Change_Time_TEXT(game_waving);
	G_Rogic_Mgr->AIManager(G_State_Mgr->Get_Wavelevel());
	if (Time_Mgr->WaveTimer())
	{
		if (G_State_Mgr->Get_Wavelevel() == 100)
		{
			G_State_Mgr->ChangeState(Game_GameOver::Instance()); // ���ӿ���� �����ؾ���
		}
		else
		{

			G_State_Mgr->ChangeState(Game_Wave_Waiting::Instance()); // ���̺� �����·� ȸ��
		}
	};

}

void Game_Waving::Exit()
{

	G_Rogic_Mgr->mObjectMgr.ReleaseAllMonsters();
	//�� ������ ����� ����

}