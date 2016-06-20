
#include "Sound_Manager.h"



HRESULT LnDms_Get3DListener(LPDIRECTSOUND8 pDS, LPDIRECTSOUND3DLISTENER* ppDSListener)
{
	HRESULT             hr;
	DSBUFFERDESC        dsbdesc;
	LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

	if (NULL == pDS || NULL == ppDSListener)
		return E_INVALIDARG;

	*ppDSListener = NULL;

	// Obtain primary buffer, asking it for 3D control
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;

	if (FAILED(hr = pDS->CreateSoundBuffer(&dsbdesc, &pDSBPrimary, NULL)))
		return hr;

	if (FAILED(hr = pDSBPrimary->QueryInterface(IID_IDirectSound3DListener, (void**)ppDSListener)))
	{
		SAFE_RELEASE(pDSBPrimary);
		return hr;
	}

	// Release the primary buffer, since it is not need anymore
	SAFE_RELEASE(pDSBPrimary);

	return S_OK;
}


HRESULT Sound_Manager::Get3DBufferInterface(DWORD dwIndex, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer)
{
	if (m_apDSBuffer == NULL)
		return CO_E_NOTINITIALIZED;

	if (dwIndex >= m_dwNumBuffers)
		return E_INVALIDARG;

	*ppDS3DBuffer = NULL;

	return m_apDSBuffer[dwIndex]->QueryInterface(IID_IDirectSound3DBuffer, (void**)ppDS3DBuffer);
}


Sound_Manager::Sound_Manager()
{
}


Sound_Manager::~Sound_Manager()
{
}
void Sound_Manager::Create_Sound(HWND hwnd)
{

	Sound_BG temp_bg;
	Sound_Effects temp_effect;
	Sound_3DEffects temp_3D_effect;

	hr = DirectSoundCreate(NULL, &pDSnd, NULL);
	if (hr == S_OK)
	{
		pDSnd->SetCooperativeLevel(hwnd, DSSCL_NORMAL);
		//배경음들 초기화
		wwBg = new WinWave();
	

		if (wwBg != NULL)
		{
			if (wwBg->LoadWaveFile(TEXT("Resource\\background.wav")) == S_OK)
			{
				AllocDSound(temp_bg.Sound_buffer, wwBg, temp_bg.dsBuffDesc);

				delete wwBg;
				wwBg = NULL;
			}
		}
		temp_bg.Sounds_BG_type = BG_Waving;
		Sounds_BG_list.push_back(temp_bg);
	
		wwBg = new WinWave();


		if (wwBg != NULL)
		{
			if (wwBg->LoadWaveFile(TEXT("Resource\\왕좌의게임.wav")) == S_OK)
			{
				AllocDSound(temp_bg.Sound_buffer, wwBg, temp_bg.dsBuffDesc);

				delete wwBg;
				wwBg = NULL;
			}
		}
		temp_bg.Sounds_BG_type = BG_Title;
		Sounds_BG_list.push_back(temp_bg);

		//이펙트에 사용할 사운드 초기화
		wwSelect = new WinWave();
		if (wwSelect != NULL)
		{
			if (wwSelect->LoadWaveFile(TEXT("Resource\\select.wav")) == S_OK)
			{
				AllocDSound(temp_effect.Sound_buffer, wwSelect, temp_effect.dsBuffDesc);

				delete wwSelect;
				wwSelect = NULL;
			}
		}
	
		temp_effect.Sounds_effect_type = Sound_click1;
		Sounds_Effects_list.push_back(temp_effect);

		wwSelect = new WinWave();
		if (wwSelect != NULL)
		{
			if (wwSelect->LoadWaveFile(TEXT("Resource\\select.wav")) == S_OK)
			{
				AllocDSound(temp_3D_effect.Sound_buffer, wwSelect, temp_effect.dsBuffDesc);

				delete wwSelect;
				wwSelect = NULL;
			}
		}
	}

	

		//pDSSndBuffBack->Play(0, 0, DSBPLAY_LOOPING);
	
		for (auto iter : Sounds_BG_list)
		{
			if(iter.Sounds_BG_type == BG_Title)
			iter.Sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
		}
		//temp_bg.Sound_buffer->Play(0, 0, 0);
	

	//pDSSndBuffBack->SetCurrentPosition(0);
}

bool Sound_Manager::AllocDSound(IDirectSound3DBuffer* &dsbuff, WinWave* ww, DSBUFFERDESC &dsdesc)//3D초기화
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
		dsdesc.guid3DAlgorithm = DS3DALG_HRTF_FULL;
		

		//2. Create 3D Buffer
		if (FAILED(hr = this->Get3DBufferInterface(0, &m_pDsrc)))

			

		//3. Create 3D Listener
		if (FAILED(hr = LnDms_Get3DListener(m_pDS, &m_pDlsn)))
	
		if (hr == S_OK)
		{
			memset(&m_DsSrc, 0, sizeof m_DsSrc);
			memset(&m_DsLsn, 0, sizeof m_DsLsn);

			m_DsSrc.dwSize = sizeof m_DsSrc;
			m_DsSrc.dwMode = DS3DMODE_NORMAL;
			m_DsSrc.flMinDistance = 0.f;
			m_DsSrc.flMaxDistance = 0.1f;
			//여기서부터코딩

			m_pDsrc->SetAllParameters(&m_DsSrc, DS3D_IMMEDIATE);


			m_DsLsn.dwSize = sizeof m_DsLsn;
			m_pDlsn->GetAllParameters(&m_DsLsn);

			m_DsLsn.vPosition.x = 0.f;
			m_DsLsn.vPosition.y = 0.f;
			m_DsLsn.vPosition.z = 0.f;
			m_DsLsn.flDopplerFactor = 0.F;
			m_DsLsn.flRolloffFactor = 0.1F;
			m_DsLsn.flDistanceFactor = 0.F;						// 이것도 중요

			return m_pDlsn->SetAllParameters(&m_DsLsn, DS3D_IMMEDIATE);
		}
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
