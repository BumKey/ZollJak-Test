#pragma once

#include <DWrite.h>
#include <D2D1.h>
#include <wincodec.h>
#include<iostream>
#include "UI_Scene.h"
#include "Sound_Manager.h"
#include "RogicTimer.h"
#include "GameTimer.h"
#define UI_Mgr  UI_Manager::Instance()
using namespace std;
enum UI_ID {
	UI_title_button_login,
	UI_title_button_register,
	UI_title_button_option,
	UI_title_button_ID,
	UI_title_button_PW,
	UI_title_BG,
	UI_title_menubox,
	UI_title_BGMOn,
	UI_title_BGMOff,
	UI_title_SoundOff,
	UI_title_SoundOn,
	UI_ingame_hp,
	UI_ingame_time,
	UI_ingame_sys_msg,
	UI_ingame_chat_log,
	UI_ingame_chat_box,
	UI_ingame_demage,
	UI_ingame_cross,
	UI_ingame_Round,
	UI_Gameover,
	Text_title_login,
	Text_title_register,
	Text_title_ID,
	Text_title_PW,
	Text_title_PW_star,
	Text_TitleSet_BGM,
	Text_TitleSet_Effect,
	Text_ingame_hp,
	Text_ingame_Round,
	Text_ingame_time,
	Text_ingame_sys_msg,
	Text_ingame_chat_log,
	Text_ingame_chat_box,
	Text_ingame_demage,
	Text_GameOver_GO,//게임오버 메시지
	Text_Test // 테스트할때만 사용할 텍스트
};
enum UI_frame_type {
	UI_Frame_null,
	UI_Frame_White,
	UI_Frame_Black

};

template <class T> inline void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

struct Text_info {
	wchar_t* _wszText_;
	UI_ID UI_id;
	type_Scene m_type_scene;
	bool Active;
	FLOAT x;
	FLOAT y;
	FLOAT alpha;
	Text_info(wchar_t* wszText_, type_Scene type_scene, UI_ID ui_id, FLOAT x_, FLOAT y_, FLOAT alpha_) {
		_wszText_ = wszText_;
		m_type_scene = type_scene;
		x = x_;
		y = y_;
		alpha = alpha_;
		Active = false;
		UI_id = ui_id;
	}
};
struct Image_info {

	wchar_t* _wszText_;
	ID2D1Bitmap* Image;
	type_Scene m_type_scene;
	UI_ID UI_id;
	bool Active;
	FLOAT x;
	FLOAT y;
	FLOAT alpha;
	FLOAT scale_x;
	FLOAT scale_y;
	UI_frame_type m_frame_type;
	Image_info(wchar_t* wszText_, type_Scene type_scene, UI_ID ui_id, FLOAT x_, FLOAT y_, FLOAT scale_x_, FLOAT scale_y_, FLOAT alpha_,
		UI_frame_type frame_type) {
		UI_id = ui_id;
		_wszText_ = wszText_;
		x = x_;
		y = y_;
		scale_x = scale_x_;
		scale_y = scale_y_;
		m_type_scene = type_scene;
		m_frame_type = frame_type;
		alpha = alpha_;
		Image = NULL;
		Active = false;
	}
};

class UI_Manager
{
private:UI_Manager();
		~UI_Manager();

		HRESULT hr;
		//2D이미지 로더
		IWICImagingFactory*             pWICFactory;
		bool input_ID;
		bool input_PW;
		//텍스트 및 도형그리기
		ID2D1Factory* pD2DFactory_;
		IDWriteFactory *pDWriteFactory;
		ID2D1RenderTarget * m_d2dRenderTarget;
		IDWriteTextLayout * m_pdwTextLayout1;
		IDWriteTextLayout * m_pdwTextLayout2;
		IDWriteTextFormat* pTextFormat_;
		IDWriteFontCollection* pFontCollection;
		FLOAT m_dpiX_;
		FLOAT m_dpiY_;
		UINT32 cTextLength_;
		ID2D1SolidColorBrush* pBlackBrush_;
		ID2D1SolidColorBrush* pWhiteBrush_;
		UI_Scene Title;
		UI_Scene Title_Setting;
		UI_Scene Waving;
		WPARAM m_wParam;
		void TextInit();

		std::list<Image_info*> m_Image_list;
		bool UI_ClickOn;
		UINT m_nMessageID;
		type_Scene m_gamescene;
		GameTimer mDamage_Timer;

public:
	RECT rc;
	std::list<Text_info*> m_Text_list;
	void CreateD2DrenderTarget(HWND hwnd);
	bool LoadPNG2DDBitmap(const TCHAR* pngfn, ID2D1Bitmap* &out);
	void Print_All_UI();
	void Print_All_Text();
	void Print_All_Image();
	void Add_Text(wchar_t* _wszText_, type_Scene type_scene, UI_ID ui_id, FLOAT x, FLOAT y, FLOAT alpha);
	void Delete_Text(wchar_t* _wszText_);
	void Delete_Text_All();
	void Load_All_Image();
	void Set_Image_Active(wchar_t* _wszText_, bool boolean);
	void Set_Text_Active(wchar_t* _wszText_, bool boolean);
	void Delete_All_Image();
	void SetScene();
	bool MouseOn2D(wchar_t* _wszText_, int mouse_x, int mouse_y);
	void chageScene();
	void Set_input_ID_state(bool input) {
		input_ID = input;
	}
	bool Get_input_ID_state() {
		return input_ID;
	}
	void Set_input_PW_state(bool input) {
		input_PW = input;
		if (input == true)
		{
			//	GetTextptr(Text_title_PW)->Active = false;

		}
	}
	bool Get_input_PW_state() {
		return input_PW;
	}
	void Set_Scene_UI(type_Scene Scenetype);
	void InTitle_UI(int x, int y);
	void InGame_UI(int x, int y);
	void InTitle_Option(int x, int y);
	void UI_Scene_Mgr(int x, int y);
	void Active_damage_Screen(bool Active, bool Hp200 = false);
	bool Tick_dmage_Timer();
	IDXGISurface* m_backbuffer;
	void Set_nMessageID(UINT nMessageID) { m_nMessageID = nMessageID; }
	UINT Get_nMessageID() { return m_nMessageID; }
	void Set_wParam(WPARAM wParam) { m_wParam = wParam; }
	WPARAM Get_wParam() { return m_wParam; }
	void Set_Gamescene(type_Scene gamestat) { m_gamescene = gamestat; }
	int Get_Gamescene() { return m_gamescene; }
	
	Image_info* GetImagePtr(UI_ID ui_id);
	void Change_HP_TEXT(int player_hp);
	void Change_Round_TEXT(int Round);
	void Change_Time_TEXT(Gamestate_type type);
	void InputID_PW();
	Text_info* GetTextptr(UI_ID ui_id);
	static UI_Manager* Instance();
};

