#pragma once

#include "GameStateMgr.h"
Game_GlobalState::Game_GlobalState()
{
	game_state_type = game_global_state;
}
Game_GlobalState::~Game_GlobalState()
{

}

Game_GlobalState* Game_GlobalState::Instance()
{
	static Game_GlobalState instance;

	return &instance;
}
void Game_GlobalState::Enter()
{
	
}

void Game_GlobalState::Execute()
{
	
	//Time_Mgr->Set_time(G_State_Mgr->Get_WaveTime());
	//Time_Mgr->SetCurrentTime();
	Time_Mgr->gamestate = G_State_Mgr->GetState();

	
	
}

void Game_GlobalState::Exit()
{
	
}