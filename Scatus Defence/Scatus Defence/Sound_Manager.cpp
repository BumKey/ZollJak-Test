
#include "Sound_Manager.h"






Sound_Manager::Sound_Manager()
{
	m_listener = 0;
	m_secondaryBuffer1 = 0;
	tempBuffer = nullptr;
	hpdown = false;
		//m_secondary3DBuffer1 = 0;
}


Sound_Manager::~Sound_Manager()
{
}


void Sound_Manager::Init(HWND hwnd)
{
	IDirectSoundBuffer8** ppDsb8 = nullptr;
	WAVEFORMATEX waveFormat;
	HRESULT result;


	// Setup the primary buffer description.
	ZeroMemory(&bufferDesc, sizeof(DSBUFFERDESC));
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	hr = DirectSoundCreate8(NULL, &pDSnd, NULL);
	// Setup the format of the primary sound bufffer.
	// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0;

	pDSnd->CreateSoundBuffer(&bufferDesc, &m_primaryBuffer, NULL);
	// Set the primary buffer to be the wave format specified.
	result = m_primaryBuffer->SetFormat(&waveFormat);
	// Obtain a listener interface.
	result = m_primaryBuffer->QueryInterface(IID_IDirectSound3DListener8, (LPVOID*)&m_listener);
	if (FAILED(result))
	{

	}



	// Set the initial position of the listener to be in the middle of the scene.
	m_listener->SetPosition(0.0f, 0.0f, 0.0f, DS3D_IMMEDIATE);


	pDSnd->SetCooperativeLevel(hwnd, DSSCL_NORMAL);
	//배경음들 초기화

}

void Sound_Manager::CreateBGBuffer_Wav( const TCHAR* strFilename, Sound_BG_type type)
{
	Sound_BG temp_bg;


	wwBg = new WinWave();
	if (wwBg != NULL)
	{
		if (wwBg->LoadWaveFile(strFilename) == S_OK)
		{
			AllocDSound(temp_bg.Sound_buffer, wwBg, temp_bg.dsBuffDesc);

			delete wwBg;
			wwBg = NULL;
		}
	}
	temp_bg.Sounds_BG_type = type;
	Sounds_BG_list.push_back(temp_bg);
}

void Sound_Manager::CreateBuffer_Wav(const TCHAR* strFilename, Sound_effect_type type)
{

	Sound_Effects temp_effect;

	wwEffect = new WinWave();
	if (wwEffect != NULL)
	{
		if (wwEffect->LoadWaveFile(strFilename) == S_OK)
		{
			AllocDSound(temp_effect.Sound_buffer, wwEffect, temp_effect.dsBuffDesc);

			delete wwEffect;
			wwEffect = NULL;
		}
	}
	
	temp_effect.Sounds_effect_type = type;
	Sounds_Effects_list.push_back(temp_effect);
}
void Sound_Manager::Create3DBuffer_Wav(const TCHAR* strFilename, Sound_3Deffect_type type)
{


	Sound_3DEffects temp_3D_effect;
	ww3DEffect = new WinWave();
	if (ww3DEffect->LoadWaveFile(strFilename) == S_OK)
	{
		
		AllocDSound(&temp_3D_effect.Sound_buffer_3D, ww3DEffect, temp_3D_effect.dsBuffDesc, &temp_3D_effect.m_secondary3DBuffer1);

		delete ww3DEffect;
		ww3DEffect = NULL;

		temp_3D_effect.Sounds_3Deffect_type = type;
		Sounds_3DEffects_list.push_back(temp_3D_effect);
	}

}


void Sound_Manager::Create_Sound(HWND hwnd)
{
	

	

		Init(hwnd);
	

		//배경음
		CreateBGBuffer_Wav(TEXT("Sound\\background.wav"), BG_Waving);
		CreateBGBuffer_Wav(TEXT("Sound\\왕좌의게임.wav"), BG_Title);
		//2D이펙트
		CreateBuffer_Wav(TEXT("Sound\\select.wav"), Sound_click1);
		CreateBuffer_Wav(TEXT("Sound\\Olddoor.wav"), Sound_open);
		//3D이펙트// 피격음
		Create3DBuffer_Wav(TEXT("Sound\\impact.wav"), Sound_impact);
		//플레이어_전사
		Create3DBuffer_Wav(TEXT("Sound\\Footstep01.wav"), Sound_p_footstep1);
		Create3DBuffer_Wav(TEXT("Sound\\전사사망.wav"), Sound_p_die);
		Create3DBuffer_Wav(TEXT("Sound\\전사피격.wav"), Sound_p_damage);
		Create3DBuffer_Wav(TEXT("Sound\\죽어갈.wav"), Sound_p_almostdie);
		Create3DBuffer_Wav(TEXT("Sound\\기합.wav"), Sound_p_shout);
		Create3DBuffer_Wav(TEXT("Sound\\swing.wav"), Sound_p_swing);
		//자이언트
		Create3DBuffer_Wav(TEXT("Sound\\giant\\giant_roar_mono.wav"), Sound_Giant_roar1);
		Create3DBuffer_Wav(TEXT("Sound\\giant\\giant_roar2_mono.wav"), Sound_Giant_roar2);
		Create3DBuffer_Wav(TEXT("Sound\\giant\\giant_attack_mono.wav"), Sound_Giant_attack1);
		Create3DBuffer_Wav(TEXT("Sound\\giant\\giant_attack2_mono.wav"), Sound_Giant_attack2);
		Create3DBuffer_Wav(TEXT("Sound\\giant\\giant_attack3_mono.wav"), Sound_Giant_attack3);
		Create3DBuffer_Wav(TEXT("Sound\\giant\\giant_pain_mono.wav"), Sound_Giant_damage1);
		Create3DBuffer_Wav(TEXT("Sound\\giant\\giant_pain2_mono.wav"), Sound_Giant_damage2);
		Create3DBuffer_Wav(TEXT("Sound\\giant\\giant_idle_mono.wav"), Sound_Giant_idle);
		
		Create3DBuffer_Wav(TEXT("Sound\\giant\\싸이클롭스발.wav"), Sound_Giant_footstep);
		// 고블린
	//	Create3DBuffer_Wav(TEXT("Sound\\goblin\\고블린피격.wav"), Sound_Goblin_damage);





	//	Create3DBuffer_Wav(TEXT("전사피격2.wav"), Sound_p_damage2);
	//	Create3DBuffer_Wav(TEXT("전사피격2.wav"), Sound_p_swing);
	//	Create3DBuffer_Wav(TEXT("전사피격2.wav"), Sound_p_shout);
	//  Create3DBuffer_Wav(TEXT("전사피격2.wav"), Sound_p_shout);

		//pDSSndBuffBack->Play(0, 0, DSBPLAY_LOOPING);
	
		for (auto iter : Sounds_BG_list)
		{
			if(iter.Sounds_BG_type == BG_Title)
			{ 
				iter.Sound_buffer->SetVolume(DSBVOLUME_MAX -4000);
				iter.Sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
			}
		}
		//temp_bg.Sound_buffer->Play(0, 0, 0);
		//Play3DEffect(Sound_Giant_footstep, 0,0,0);

	//pDSSndBuffBack->SetCurrentPosition(0);
}
bool Sound_Manager::AllocDSound(IDirectSoundBuffer8** dsbuff, WinWave* ww, DSBUFFERDESC &dsdesc, IDirectSound3DBuffer8** secondary3DBuffer)//3D초기화
{


	if (pDSnd == NULL)
		return false;

	if (ww == NULL)
		return false;

	ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
	dsdesc.dwSize = sizeof(DSBUFFERDESC);
	dsdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D ;
	dsdesc.dwBufferBytes = ww->WaveSize();
	dsdesc.lpwfxFormat = ww->WaveFormat();
	dsdesc.dwReserved = 0;
	dsdesc.guid3DAlgorithm = GUID_NULL;
	//2.임시버퍼를 만들어 공간을 한당하고 세컨드버퍼로 그 공간에 접근하는 인터페이스 생성, 그후 임시버퍼가 있던 메모리에 웨이브파일을 할당
	 hr = pDSnd->CreateSoundBuffer(&dsdesc,&tempBuffer, NULL);
	if (FAILED(hr))
	 {
		
		 return false;
	 }
	//pDSnd->CreateSoundBuffer(&dsdesc, &dsbuff, NULL);
	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&*dsbuff);
	//tempBuffer->QueryInterface(IID_IDirectSound3DBuffer8, (void**)&*secondary3DBuffer);
	if (FAILED(result))
	{
		//return false;
	}
	tempBuffer->Release();
	tempBuffer = 0;
	if (result == S_OK)
	{
		void* pwData1 = NULL;
		void* pwData2 = NULL;
		DWORD lenData1 = 0;
		DWORD lenData2 = 0;

		(*dsbuff)->Lock(0,
			dsdesc.dwBufferBytes,
			&pwData1,
			&lenData1,
			&pwData2,
			&lenData2,
			0);

		memcpy(pwData1, ww->WaveData(), dsdesc.dwBufferBytes);
		(*dsbuff)->Unlock(pwData1, dsdesc.dwBufferBytes, NULL, 0);
		
	}

	// 세컨드버퍼에 3D사운드를 제어 할 수 있는 인터페이스를 얻는다.
	result = (*dsbuff)->QueryInterface(IID_IDirectSound3DBuffer8, (void**)&*secondary3DBuffer);

	
	if (FAILED(result))
	{
		return false;
	}

	return true;

}


	bool Sound_Manager::AllocDSound(IDirectSoundBuffer* &dsbuff, WinWave* ww, DSBUFFERDESC &dsdesc)
	{
		if (pDSnd == NULL)
			return false;

		if (ww == NULL)
			return false;

		ZeroMemory(&dsdesc, sizeof(DSBUFFERDESC));
		dsdesc.dwSize = sizeof(DSBUFFERDESC);
		dsdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME, DSBCAPS_CTRLFREQUENCY, DSBCAPS_CTRLPAN | DSBCAPS_CTRL3D;
		dsdesc.dwBufferBytes = ww->WaveSize();
		dsdesc.lpwfxFormat = ww->WaveFormat();

		HRESULT hr = pDSnd->CreateSoundBuffer(&dsdesc, &dsbuff, NULL);

		if (hr == S_OK)
		{
			void* pwData1 = NULL;
			void* pwData2 = NULL;
			DWORD lenData1 = 0;
			DWORD lenData2 = 0;

			dsbuff->Lock(0,
				dsdesc.dwBufferBytes,
				&pwData1,
				&lenData1,
				&pwData2,
				&lenData2,
				0);

			memcpy(pwData1, ww->WaveData(), dsdesc.dwBufferBytes);
			dsbuff->Unlock(pwData1, dsdesc.dwBufferBytes, NULL, 0);
		}
	}


	Sound_Manager* Sound_Manager::Instance()
	{
		static Sound_Manager instance;

		return &instance;
	}

	void Sound_Manager::Turn_Sound_Effect(bool turn)
	{
		for (auto i : Sounds_Effects_list)
		{
			if (turn)
				i.Sound_buffer->SetVolume(DSBVOLUME_MAX);
			else
				i.Sound_buffer->SetVolume(DSBVOLUME_MIN);

		}
	}

	void Sound_Manager::Turn_Sound_BG(bool turn)
	{
		DWORD a = 100;
		for (auto i : Sounds_BG_list)
		{
			if (turn)
			{
				if (i.Sounds_BG_type == BG_Title)
					i.Sound_buffer->SetVolume(DSBVOLUME_MAX);
			}
			else
			{
				if (i.Sounds_BG_type == BG_Title)
					i.Sound_buffer->SetVolume(DSBVOLUME_MIN);
	
				int v;
		
			}
		}
	}

	void Sound_Manager::PlayEffect(Sound_effect_type effect_id)
	{
		for (auto i : Sounds_Effects_list)
		{
			if (i.Sounds_effect_type == effect_id)
			{
				i.Sound_buffer->Play(0, 0, 0);
			}
		}
	}
	void Sound_Manager::PlayBG(Sound_BG_type)
	{
		
	}

	void Sound_Manager::Play3DEffect(Sound_3Deffect_type type, FLOAT x, FLOAT y, FLOAT z)
	{
		
		
		for (auto i : Sounds_3DEffects_list)
		{
			if (i.Sounds_3Deffect_type == type)
			{
				D3DVALUE dis;
				result = i.m_secondary3DBuffer1->SetPosition(x, y, z, DS3D_IMMEDIATE);
				result = i.m_secondary3DBuffer1->GetMaxDistance(&dis);
			//	result = i.m_secondary3DBuffer1->SetMaxDistance(10000.0f, DS3D_IMMEDIATE);
				//i.Sound_buffer_3D->SetFrequency(1000.0f);
				//i.Sound_buffer_3D->Play(0, 0, 0);
				i.Sound_buffer_3D->SetVolume(DSBVOLUME_MAX);
				i.Sound_buffer_3D->Play(0, 0, 0);
			}
		}
	}

	void Sound_Manager::Stop_3DSound(Sound_3Deffect_type type)
	{
		for (auto i : Sounds_3DEffects_list)
		{
			if (i.Sounds_3Deffect_type == type)
			{
			
				i.Sound_buffer_3D->SetFrequency(1000.0f);
				//i.Sound_buffer_3D->Play(0, 0, 0);
				i.Sound_buffer_3D->Stop();
			}
		}
	}
	void Sound_Manager::Stop_BGM(Sound_BG_type type)
	{
		for (auto i : Sounds_BG_list)
		{
			if (i.Sounds_BG_type == type)
			{
				i.Sound_buffer->Stop();
			}
		}
	}
	void Sound_Manager::SetSoundPos(Sound_3Deffect_type type, FLOAT x, FLOAT y, FLOAT z)
	{
		for (auto i : Sounds_3DEffects_list)
		{
			if (i.Sounds_3Deffect_type == type)
			{
				result = i.m_secondary3DBuffer1->SetPosition(x, y, z, DS3D_IMMEDIATE);

		
			}
		}
	}
	void Sound_Manager::Set3DLinstenerPos(FLOAT x, FLOAT y, FLOAT z)
	{
		m_listener->SetPosition(x, y, z, DS3D_IMMEDIATE);
	}