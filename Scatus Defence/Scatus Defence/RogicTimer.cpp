#include "RogicTimer.h"


RogicTimer::RogicTimer()
{
	nextwave_time = 4;//���� ���̺�ð�(���½ð�)
	remain_time = 100;// �����ִ� ���̺�(���� �ð�)
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

	if (gamestate == game_waiting_wave)//gamestate�� extern ������ ���ָ� �ذ�
	{
		int a=4;
		int b = 4;
		//printf("\n���̺���%d��", wavetimer);
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
		//	printf("%\n���̺�����d��", wavetimer);
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
	before_play = current_tick;//������ �÷��� Ÿ�� ��������

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