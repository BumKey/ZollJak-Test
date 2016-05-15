#include "RogicTimer.h"

int GameState;

RogicTimer::RogicTimer()
{
	nextwave_time = 2;//���� ���̺�ð�(���½ð�)
	remain_time = 150;// �����ִ� ���̺�(���� �ð�)
	wavetimer = 0;
	playtime_sec = 0;
	before_play = 0;
	before_wave = 0;
	attacktimer = 0;
}


RogicTimer::~RogicTimer()
{
}

bool RogicTimer::WaveTimer()
{


	 wavetimer = current_sec - before_wave;

	if(GameState == game_waiting_wave)//gamestate�� extern ������ ���ָ� �ذ�
	{
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
	else if (GameState == game_waving)
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
	if (attacktimer >= (1/attack_speed))
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