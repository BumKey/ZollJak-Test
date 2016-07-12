#include "RogicTimer.h"


RogicTimer::RogicTimer()
{
	nextwave_time = 4;//다음 웨이브시간(쉬는시간)
	remain_time = 100;// 남아있는 웨이브(전투 시간)
	wavetimer = 0;
	playtime_sec = 0;
	before_play = 0;
	before_wave = 0;
	attacktimer = 0;
	gamestate = game_title;
	
}


RogicTimer::~RogicTimer()
{
}
RogicTimer* RogicTimer::Instance()
{
	static RogicTimer instance;

	return &instance;
}
bool RogicTimer::WaveTimer()
{


	wavetimer = current_sec - before_wave;

	if (gamestate == game_waiting_wave)//gamestate를 extern 선언을 해주면 해결
	{
		int a=4;
		int b = 4;
		//printf("\n웨이브대기%d초", wavetimer);
		if (wavetimer >= nextwave_time)
		{

			return true;
		}
		else
		{

			return false;
		}
	}
	else if (gamestate == game_waving)
	{
		//	printf("%\n웨이브진행d초", wavetimer);
		if (wavetimer >= remain_time)
		{

			return true;
		}
		else
		{

			return false;
		}
	}




};

bool RogicTimer::AttackTimer(double attack_speed)
{

	attacktimer = current_sec - before_attack;
	if (attacktimer >= (1 / attack_speed))
	{

		return true;
	}
	else
	{

		return false;
	}
}
void RogicTimer::SetAttackTimer()
{
	before_attack = current_sec;

}

void RogicTimer::SetBeforeTime()
{
	before_play = current_tick;//실질적 플레이 타임 측정시작

}

void RogicTimer::SetCurrentTime() {

	current_tick = clock();
	current_sec = current_tick / CLOCKS_PER_SEC;

}
void RogicTimer::SetWaveTimer() {
	before_wave = current_sec;
}

int RogicTimer::GetPlayTime()
{
	playtime_sec = current_sec - (before_play / CLOCKS_PER_SEC);
	return playtime_sec;
}