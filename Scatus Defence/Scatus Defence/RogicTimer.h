#pragma once
#include "time.h"
#include <string>

enum Gamestate_type
{
	game_title,
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
extern int GameState;
class RogicTimer
{
private : 
		   
		  double nextwave_time;
		  double remain_time;
		  double wavetimer;
		  double attacktimer;

public:
	
	clock_t current_tick;
	double before_wave;
	clock_t before_play;
	double current_sec;
	double playtime_sec;
	double before_attack;
	
	double Get_remain_Wait_time() { return nextwave_time - wavetimer; }
	double Get_remain_Wave_time() { return remain_time - wavetimer; }
	void SetBeforeTime();
	void SetCurrentTime();
	void SetWaveTimer();
	void SetAttackTimer();
	bool AttackTimer(double attack_speed);
	int GetPlayTime();
	RogicTimer();
	~RogicTimer();
	bool WaveTimer();
	


};

