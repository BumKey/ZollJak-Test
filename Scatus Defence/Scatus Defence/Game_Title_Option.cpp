#pragma once

#include "GameStateMgr.h"
Game_Title_Option::Game_Title_Option()
{
	game_state_type = game_title_option;
}
Game_Title_Option::~Game_Title_Option()
{

}

Game_Title_Option* Game_Title_Option::Instance()
{
	static Game_Title_Option instance;

	return &instance;
}
void Game_Title_Option::Enter()
{

}

void Game_Title_Option::Execute()
{


}

void Game_Title_Option::Exit()
{
	
}