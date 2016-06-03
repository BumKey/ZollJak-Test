#pragma once

#include <DWrite.h>
#include <D2D1.h>
#include <wincodec.h>
#include <list>



#define UI_Mgr  UI_Manager::Instance()

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
	FLOAT x;
	FLOAT y;
	FLOAT alpha;
	Text_info(wchar_t* wszText_, FLOAT x_, FLOAT y_, FLOAT alpha_) {
		_wszText_ = wszText_;
		x = x_;
		y = y_;
		alpha = alpha_;
	}
};
struct Image_info {
	wchar_t* _wszText_;
	ID2D1Bitmap* Image;
	bool Active;
	FLOAT x;
	FLOAT y;
	FLOAT alpha;
	FLOAT scale_x;
	FLOAT scale_y;
	Image_info(wchar_t* wszText_, FLOAT x_, FLOAT y_, FLOAT scale_x_, FLOAT scale_y_, FLOAT alpha_) {
		_wszText_ = wszText_;
		x = x_;
		y = y_;
		scale_x = scale_x_;
		scale_y = scale_y_;

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
		IWICImagingFactory*             pWICFactory = NULL;
		bool input_ID;
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

		std::list<Text_info*> m_Text_list;
		std::list<Image_info*> m_Image_list;

public:
	RECT rc;
	void CreateD2DrenderTarget(HWND hwnd);
	bool LoadPNG2DDBitmap(const TCHAR* pngfn, ID2D1Bitmap* &out);
	void Print_All_UI();
	void Print_All_Text();
	void Print_All_Image();
	void Add_Text(wchar_t* _wszText_, FLOAT x, FLOAT y, FLOAT alpha);
	void Delete_Text(wchar_t* _wszText_);
	void Delete_Text_All();
	void Load_All_Image();
	void Set_Image_Active(wchar_t* _wszText_, bool boolean);
	void Delete_All_Image();
	bool MouseOn2D(wchar_t* _wszText_, int mouse_x, int mouse_y);
	void Set_input_ID_state(bool input) {
		input_ID = input;
	}
	bool Get_input_ID_state(bool input) {
		return input_ID;
	}
	IDXGISurface* m_backbuffer;



	static UI_Manager* Instance();
};

