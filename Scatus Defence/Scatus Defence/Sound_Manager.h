#pragma once
#include <dsound.h>
#include<list>
#include<vector>
#include "winwav.h"

#define Sound_Mgr  Sound_Manager::Instance()
#define DSVOLUME_TO_DB(volume)   ((DWORD)(-10000 +(-volume*100)))  // 입력값을 데시벨로 바꿔주는 매크로함수
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

enum Sound_list_type {
	Sound_p_warrior,//플레이어 전사
	Sound_p_archer,
	Sound_p_builder,


};

enum Sound_type {
	Sound_type_BG,
	Sound_type_effect,
	Sound_type_3D_effect

};

enum Sound_effect_type {

	Sound_click1,
	Sound_open,
	Sound_wave_start

};

enum Sound_3Deffect_type {
	Sound_null,

	
	Sound_idle,
	Sound_Goblin_footstep,
	Sound_Goblin_roar,
	Sound_Goblin_die,
	Sound_Goblin_damage,
	Sound_Goblin_roar2,
	Sound_p_walk,
	Sound_mop_growl,
	Sound_p_shout,
	Sound_p_shout2,
	Sound_p_almostdie,
	Sound_attack,
	Sound_p_damage,
	Sound_p_damage2,
	Sound_p_swing,
	Sound_p_die,
	Sound_p_footstep1,
	Sound_Giant_footstep,
	Sound_Giant_footstep2,
	Sound_Giant_roar1,
	Sound_Giant_damage1,
	Sound_Giant_damage2,
	Sound_Giant_roar2,
	Sound_Giant_idle,
	Sound_Giant_swing,
	Sound_Giant_swing2,
	Sound_Giant_attack1,
	Sound_Giant_attack2,
	Sound_Giant_attack3,
	Sound_Giant_die,
	Sound_impact,

	Sound_earthqueke


};

enum Sound_BG_type {
	BG_Title,
	BG_Boss,
	BG_Waving,
	BG_Wave_wait,
	BG_GameOver

};

struct Sound_BG
{
	int Sounds_BG_type;
	IDirectSoundBuffer* Sound_buffer;
	bool isActive;
	DSBUFFERDESC   dsBuffDesc;

};

struct Sound_Effects
{
	Sound_effect_type Sounds_effect_type;
	IDirectSoundBuffer* Sound_buffer;
	bool isActive;
	DSBUFFERDESC   dsBuffDesc;
	

};


struct Sound_3DEffects
{
	Sound_3Deffect_type Sounds_3Deffect_type;
	IDirectSoundBuffer8* Sound_buffer_3D;
	IDirectSound3DBuffer8* m_secondary3DBuffer1;
	bool isActive;
	DSBUFFERDESC   dsBuffDesc;
	Sound_3DEffects() {
		IDirectSoundBuffer8* Sound_buffer_3D= nullptr;
		IDirectSound3DBuffer8* m_secondary3DBuffer1 = nullptr;
	}



};



class Sound_Manager
{
private:

	IDirectSound8*                   pDSnd;
	HRESULT result;
	//We have a new listener interface for 3D sound.

	IDirectSound3DListener8* m_listener;
	IDirectSoundBuffer8* m_secondaryBuffer1;
	//IDirectSound3DBuffer8* m_secondary3DBuffer1;
	IDirectSoundBuffer* m_primaryBuffer;
	////
	IDirectSoundBuffer* tempBuffer;
	DWORD					m_dwDSBufferSize;//
	DWORD					m_dwNumBuffers;//
	std::vector<Sound_BG>   Sounds_BG_list;
	std::vector<Sound_Effects>   Sounds_Effects_list;
	std::vector<Sound_3DEffects>   Sounds_3DEffects_list;
	IDirectSoundBuffer* pDSSndBuffBack;
	
	DSBUFFERDESC                   bufferDesc;

	HRESULT hr;
	WinWave*                        wwBg = NULL;
	WinWave*                        wwEffect = NULL;
	WinWave*                        ww3DEffect = NULL;
	void Init(HWND hwnd);
	D3DVECTOR Calc_listener_Pos(float x, float y, float z);
	bool LoadWaveFile(char*, IDirectSoundBuffer8**, IDirectSound3DBuffer8**);
	void CreateBGBuffer_Wav(const TCHAR* strFilename, Sound_BG_type type);
	void CreateBuffer_Wav(const TCHAR* strFilename, Sound_effect_type type);
	void Create3DBuffer_Wav(const TCHAR* strFilename, Sound_3Deffect_type type);
	bool AllocDSound(IDirectSoundBuffer* &dsbuff, WinWave* ww, DSBUFFERDESC &dsdesc);
	bool AllocDSound(IDirectSoundBuffer8** dsbuff, WinWave* ww, DSBUFFERDESC &dsdesc, IDirectSound3DBuffer8** secondary3DBuffer);
public:
	Sound_Manager();
	~Sound_Manager();
	void Create_Sound(HWND hwnd);
	void Play(Sound_type type);
	void Turn_Sound_Effect(bool turn);
	void Turn_Sound_BG(bool turn);
	void PlayEffect(Sound_effect_type);
	void PlayBG(Sound_BG_type);

	//void ShutdownWaveFile(IDirectSoundBuffer8**, IDirectSound3DBuffer8**);
	void Play3DEffect(Sound_3Deffect_type type,FLOAT x, FLOAT y, FLOAT z);
	void Stop_3DSound(Sound_3Deffect_type type);
	void Stop_BGM(Sound_BG_type type);
	void SetSoundPos(Sound_3Deffect_type type, FLOAT x, FLOAT y, FLOAT z);
	void Set3DLinstenerPos( FLOAT x, FLOAT y, FLOAT z);


	static Sound_Manager* Instance();
};

