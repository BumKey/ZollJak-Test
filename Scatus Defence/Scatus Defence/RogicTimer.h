#pragma once
#include "time.h"
#include <string>
#define Time_Mgr  RogicTimer::Instance()

enum Gamestate_type
{
	game_global_state,
	game_title,
	game_title_option,
	game_begin,
	game_loading,
	game_waving,
	game_waiting_wave,
	game_ending,
	game_opening,
	game_gameover,
	game_exit,
	game_option,
	game_start


};

class RogicTimer
{
private : 
		   
		  double nextwave_time;
		  double remain_time;
		  double wavetimer;
		  double attacktimer;
		  int m_wave_level;
		  int m_time;
		  int m_p_hp;

public:
	
	clock_t current_tick;
	double before_wave;
	clock_t before_play;
	double current_sec;
	double playtime_sec;
	double before_attack;
	Gamestate_type gamestate;
	double Get_remain_Wait_time() { return nextwave_time - wavetimer; }
	double Get_remain_Wave_time() { return remain_time - wavetimer; }
	void SetBeforeTime();
	void SetCurrentTime();
	void SetWaveTimer();
	void SetAttackTimer();
	int GetWaveTime() { return wavetimer; }
	bool AttackTimer(double attack_speed);
	int GetPlayTime();
	int Get_Wavelevel() { return m_wave_level; }
	int Get_time() { return m_time; }
	int Get_P_HP() { return m_p_hp; }
	Gamestate_type GetGameState() { return gamestate; }
	void Set_Wavelevel(int num) { m_wave_level = num; }
	void Set_time(int num) {m_time = num; }
	void Set_P_HP(int num) { m_p_hp = num; }
	RogicTimer();
	~RogicTimer();
	bool WaveTimer();
	static RogicTimer* Instance();


};

