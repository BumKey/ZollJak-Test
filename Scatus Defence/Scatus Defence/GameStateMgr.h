#pragma once
//#include "RogicTimer.h"
#define G_State_Mgr  GameStateMgr::Instance()
#include "RogicTimer.h"
#include "UI_Manager.h"
//template <class entity_type>
class GameState {
	protected : 		
	 Gamestate_type game_state_type;
	 
	public:
		Gamestate_type  Get_GameState_type() { return game_state_type; };
		GameState(){};
		~GameState(){};
		virtual void Execute() {};
		virtual void Enter() {};
		virtual void Exit() {};

};

class GameStateMgr
{
	
private:// Gamestate_type gamestate;
		 GameState* m_pCurrentState;
		 GameState* m_pGlobalState;
		 Gamestate_type gamestate_cur;
		 int m_wave_level;
		 int m_wavetime;
public:
	void Update();
	void ChangeState( GameState* pNewState);
	GameStateMgr();
	~GameStateMgr();
	void SetState(Gamestate_type state) {
		gamestate_cur = state;
	};
	Gamestate_type GetState() { return gamestate_cur; }
	static GameStateMgr* Instance();

};


class Game_Title : public GameState
{
	;
	public :
	Game_Title();
	~Game_Title();
	void Execute();
	void Enter();
	void Exit();

	static Game_Title* Instance();
};

class Game_Title_Option : public GameState
{

public:
	Game_Title_Option();
	~Game_Title_Option();
	void Execute();
	void Enter();
	void Exit();

	static Game_Title_Option* Instance();
};

class Game_Begin: public GameState
{

public:
	Game_Begin();
	~Game_Begin();
	void Execute();
	void Enter();
	void Exit();
	static Game_Begin* Instance();
};

class Game_Waving : public GameState
{

public:
	Game_Waving();
	~Game_Waving();
	void Execute();
	void Enter();
	void Exit();
	static Game_Waving* Instance();
};

class Game_Wave_Waiting : public GameState
{

public:
	Game_Wave_Waiting();
	~Game_Wave_Waiting();
	void Execute();
	void Enter();
	void Exit();
	static Game_Wave_Waiting* Instance();
};

class Game_GameOver : public GameState
{

public:
	Game_GameOver();
	~Game_GameOver();
	void Execute();
	void Enter();
	void Exit();
	static Game_GameOver* Instance();
};


class Game_GlobalState : public GameState
{

public:
	Game_GlobalState();
	~Game_GlobalState();
	void Execute();
	void Enter();
	void Exit();
	static Game_GlobalState* Instance();
};