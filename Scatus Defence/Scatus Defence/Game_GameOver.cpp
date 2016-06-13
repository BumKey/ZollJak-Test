#pragma once

#include "GameStateMgr.h"
Game_GameOver::Game_GameOver()
{
	game_state_type = game_gameover;
}
Game_GameOver::~Game_GameOver()
{

}

Game_GameOver* Game_GameOver::Instance()
{
	static Game_GameOver instance;

	return &instance;
}
void Game_GameOver::Enter()
{


}

void Game_GameOver::Execute()
{


}

void Game_GameOver::Exit()
{

}