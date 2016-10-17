#pragma once

#include "GameStateMgr.h"
Game_Title::Game_Title()
{
	game_state_type = game_title ;
}
Game_Title::~Game_Title()
{
	
}	

Game_Title* Game_Title::Instance()
{
	static Game_Title instance;

	return &instance;
}

void Game_Title::Enter()
{

}
void Game_Title::Execute()
{

	if (UI_Mgr->Get_Gamescene() == Scene_Ingame) {}
		G_State_Mgr->ChangeState(Game_Begin::Instance());
	


}
void Game_Title::Exit()
{
	Sound_Mgr->PlayEffect(Sound_open);
}