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

	Sound_click1

};

enum Sound_3D_effect_type {
	Sound_null,
	Sound_die,
	Sound_attack,
	Sound_idle,
	

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
	Sound_3D_effect_type Sounds_effect_type;
	IDirectSound3DBuffer* Sound_buffer;
	bool isActive;
	DSBUFFERDESC   dsBuffDesc;



};



class Sound_Manager
{
private:
	IDirectSound*                   pDSnd;
	DS3DLISTENER			        m_DsLsn;                                    //// Listener properties (Dest)
	LPDIRECTSOUND3DBUFFER	m_pDsrc;											// 3D sound buffer
	LPDIRECTSOUND3DLISTENER	m_pDlsn;											// 3D listener object
	DS3DBUFFER				m_DsSrc;											// 3D buffer properties (Source)
	LPDIRECTSOUNDBUFFER*	m_apDSBuffer;                                       //무어에 쓰는 물건인고
	LPDIRECTSOUND8 m_pDS;//
	DWORD					m_dwDSBufferSize;//
	DWORD					m_dwNumBuffers;//
	std::vector<Sound_BG>   Sounds_BG_list;
	std::vector<Sound_Effects>   Sounds_Effects_list;
	IDirectSoundBuffer* pDSSndBuffBack;
	DSBUFFERDESC                    dsBuffDescBack;
	DSBUFFERDESC                    dsBuffDescFront;
	HRESULT hr;
	WinWave*                        wwBg = NULL;
	WinWave*                        wwSelect = NULL;
public:
	Sound_Manager();
	~Sound_Manager();
	void Create_Sound(HWND hwnd);
	void Play(Sound_type type);
	void Turn_Sound_Effect(bool turn);
	void Turn_Sound_BG(bool turn);
	void PlayEffect(Sound_effect_type);
	void PlayBG(Sound_BG_type);
	bool AllocDSound(IDirectSoundBuffer* &dsbuff, WinWave* ww, DSBUFFERDESC &dsdesc);
	bool AllocDSound(IDirectSound3DBuffer* &dsbuff, WinWave* ww, DSBUFFERDESC &dsdesc);
	HRESULT Get3DBufferInterface(DWORD dwIndex, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer);
	void Creae_3DSound(HWND hwnd);
	static Sound_Manager* Instance();
};

