#pragma once

#include "GameStateMgr.h"
Game_Begin::Game_Begin()
{
	game_state_type = game_begin;
}
Game_Begin::~Game_Begin()
{

}

Game_Begin* Game_Begin::Instance()
{
	static Game_Begin instance;

	return &instance;
}
void Game_Begin::Enter()
{
	Time_Mgr->SetBeforeTime(); //플레이타임 측정시작


}

void Game_Begin::Execute()
{

	State_Mgr->ChangeState(Game_Wave_Waiting::Instance()); // 게임엔드로 수정해야함
}

void Game_Begin::Exit()
{

}