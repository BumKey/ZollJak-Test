
#include "UI_Manager.h"

void calcimgputrc(D2D1_RECT_F* rcf, int x, int y, int w, int h)
{
	if (rcf != NULL)
	{
		rcf->left = float(x);
		rcf->top = float(y);
		rcf->right = float(x)+float(w);
		rcf->bottom = float(y)+float(h);
	}

}
UI_Manager::UI_Manager()
{
	input_ID = false;
	input_PW = false;
	UI_ClickOn = false;
	m_gamescene = Scene_Title;
	Set_Scene_UI(Scene_Title);
}


UI_Manager::~UI_Manager()
{
}

bool UI_Manager::LoadPNG2DDBitmap(const TCHAR* pngfn, ID2D1Bitmap* &out)
{
	if (pWICFactory == NULL)
		return false;

	IWICBitmapDecoder*      pImgLoader = { NULL };
	HRESULT                 hr = S_OK;

	hr = pWICFactory->CreateDecoderFromFilename(pngfn,  // 파일로부터 디코더 생성
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&pImgLoader);
	if (hr == S_OK)
	{
		IWICBitmapFrameDecode* pWICFdecoder = NULL;
		hr = pImgLoader->GetFrame(0, &pWICFdecoder); // 디코더로 부터 프레임추출

		if (hr == S_OK)
		{
			IWICFormatConverter *pFMTconv = NULL; // 컨버터를 만들어 프레임으로 초기화한다

			hr = pWICFactory->CreateFormatConverter(&pFMTconv);
			if (hr == S_OK)
			{
				pFMTconv->Initialize(pWICFdecoder,
					GUID_WICPixelFormat32bppPBGRA,
					WICBitmapDitherTypeNone,
					NULL,
					0.0f,
					WICBitmapPaletteTypeCustom);

				hr = m_d2dRenderTarget->CreateBitmapFromWicBitmap(pFMTconv, NULL, &out); // 프레임으로부터 d2d비트맵이미지 생성
				if (hr == S_OK)
				{
					SafeRelease(&pImgLoader);
					SafeRelease(&pFMTconv);
					SafeRelease(&pWICFdecoder);
					return true;
				}
			}
		}

		SafeRelease(&pImgLoader);
	}

	return false;
}
void UI_Manager::CreateFactorys()
{
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		&pD2DFactory_
		);


	if (SUCCEEDED(hr))
	{
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&pDWriteFactory)
			);
	}

	pDWriteFactory->CreateTextFormat(
		L"Arial",                // Font family name.
		NULL,                       // Font collection (NULL sets it to use the system font collection).
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		20.0f,
		L"en-us",
		&pTextFormat_
		);

	pDWriteFactory->CreateTextFormat(
		L"Arial",                // Font family name.
		NULL,                       // Font collection (NULL sets it to use the system font collection).
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		50.0f,
		L"en-us",
		&pTextFormat_WaveStart
		);



	// Center align (horizontally) the text.
	if (SUCCEEDED(hr))
	{
		hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		hr = pTextFormat_WaveStart->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}
	/*
	if (SUCCEEDED(hr))
	{
	hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}*/
}

void UI_Manager::CreateD2DrenderTarget(HWND hwnd, IDXGISwapChain* swapChain) {

	pD2DFactory_->GetDesktopDpi(&m_dpiX_, &m_dpiY_);

	IDXGISurface* backbuffer;
	HR(swapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(&backbuffer)));
	pD2DFactory_->CreateDxgiSurfaceRenderTarget(backbuffer,
	D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
	D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), m_dpiX_, m_dpiY_), &m_d2dRenderTarget);
	ReleaseCOM(backbuffer);

	hr = m_d2dRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&pBlackBrush_
		);

	hr = m_d2dRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&pWhiteBrush_
		);

	//비트맵 생성위한 부분
	CoInitialize(NULL);  // COM객체가 초기화가 안되서 에러가나는 경우를 위해 초기화 인자는 무조건 0 아니면 널
	CoCreateInstance(CLSID_WICImagingFactory,  // 인터페이스생성
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pWICFactory));

	GetClientRect(hwnd, &rc);
}

void UI_Manager::Load_All_UI()
{

	Load_All_Image();
	TextInit();
}

UI_Manager* UI_Manager::Instance()
{
	static UI_Manager instance;

	return &instance;
}

void UI_Manager::OnSize(HWND hwnd)
{
	for (auto i : m_Image_list)
	{
		i->Image->GetSize().width;
		i->Image->GetSize().height;
		i->Image->GetPixelSize().height;
		i->Image->GetPixelSize().width;
	}

	//모든 이미지 크기 재조정
}

void UI_Manager::Print_All_UI()
{
	Print_All_Image();
	Print_All_Text();

	
}
void UI_Manager::Print_All_Text()
{

	

	//GetClientRect(hwnd_, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);


	m_d2dRenderTarget->BeginDraw();

	for (auto i : m_Text_list)
	{
		if (i->Active == true)
		{
			wchar_t* wszText_ = i->_wszText_;
			cTextLength_ = (UINT32)wcslen(wszText_);
			/*
			D2D1_RECT_F layoutRect = D2D1::RectF(
				static_cast<FLOAT>((rc.left) / m_dpiX_) + 100,
				static_cast<FLOAT>((rc.top) / m_dpiY_) + 100,
				static_cast<FLOAT>(rc.right - rc.left) / m_dpiX_,
				static_cast<FLOAT>(rc.bottom - rc.top) / m_dpiY_
				);*/
			D2D1_RECT_F layoutRect = D2D1::RectF(
				static_cast<FLOAT>(i->x),
				static_cast<FLOAT>(i->y),
				static_cast<FLOAT>(i->x+300),
				static_cast<FLOAT>(i->y+100)
				); 
			
			m_d2dRenderTarget->DrawText(
				wszText_,        // The string to render.
				cTextLength_,    // The string's length.
				i->m_pTextFormat_,    // The text format.
				layoutRect,       // The region of the window where the text will be rendered.
				pWhiteBrush_     // The brush used to draw the text.
			);
		}
	}


	HRESULT hr = m_d2dRenderTarget->EndDraw();











}

void UI_Manager::Load_All_Image()
{

	Image_info *temp = new Image_info(L"타이틀화면", Scene_Title, UI_title_BG,rc.left , rc.top,rc.right-rc.left,rc.bottom-rc.top, 1, UI_Frame_null);
	hr= LoadPNG2DDBitmap(TEXT("Resource\\title.jpg"),temp->Image);
	if (SUCCEEDED(hr))
	{
			m_Image_list.push_back(temp);
	}
	Image_info *temp2 = new Image_info(L"메시지박스", Scene_NULL, UI_ingame_chat_box,100, 100, 400,400,0.9, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\효과음.png"), temp2->Image);
	if (SUCCEEDED(hr))
	{
	m_Image_list.push_back(temp2);
	}

	/* 정가운데 박스
	Image_info *temp3 = new Image_info(L"Title_Menu_Box", (rc.left+rc.right)/2 -150 , (rc.top + rc.bottom) / 2 -150 , 
		300, +300, 1);
	hr = LoadPNG2DDBitmap(TEXT("sony.jpg"), temp3->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp3);
	}
	*/
	Image_info *temp3 = new Image_info(L"Title_Menu_Box", Scene_Title, UI_title_menubox,(rc.left + rc.right) / 2 - 150, (rc.top + rc.bottom) / 2 - 50,
		300, +200, 0.4, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\sony.jpg"), temp3->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp3);
	}

	Image_info *temp4 = new Image_info(L"아이디 입력", Scene_Title, UI_title_button_ID, (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 - 25,
		260, 40, 0.5, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\ui_box_basic.png"), temp4->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp4);
	}

	Image_info *temp5 = new Image_info(L"패스워드 입력", Scene_Title, UI_title_button_PW, (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 +25,
		260, 40, 0.5, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\ui_box_basic.png"), temp5->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp5);
	}


	Image_info *temp6 = new Image_info(L"로그인", Scene_Title, UI_title_button_login, (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 +75,
		125, 40, 0.5, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\ui_box_basic.png"), temp6->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp6);
	}

	Image_info *temp7 = new Image_info(L"회원가입", Scene_Title, UI_title_button_register,(rc.left + rc.right) / 2+5, (rc.top + rc.bottom) / 2 +75,
		125, 40, 0.5, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\ui_box_basic.png"), temp7->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp7);
	}

	Image_info *temp8 = new Image_info(L"타이틀_옵션아이콘", Scene_Title, UI_title_button_option,rc.right - 50, rc.top +  10,
		40, 40, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\option.png"), temp8->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp8);
	}
	Image_info *temp9 = new Image_info(L"크로스헤더", Scene_Ingame,UI_ingame_cross, (rc.left + rc.right)/2-15, (rc.top + rc.bottom)/2-15,
		30, 30, 0.7, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\크로스헤더.png"), temp9->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp9);
	}
	Image_info *temp10 = new Image_info(L"체력", Scene_Ingame, UI_ingame_hp, rc.left +30, (rc.bottom-60),
		100, 30, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\ui_box_basic.png"), temp10->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp10);
	}

	Image_info *temp11 = new Image_info(L"라운드", Scene_Ingame, UI_ingame_Round, rc.left + 30, (rc.top + 10),
		100, 30, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\ui_box_basic.png"), temp11->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp11);
	}

	Image_info *temp12 = new Image_info(L"시간", Scene_Ingame, UI_ingame_time, rc.right - 145, (rc.top + 10),
		130, 30, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\ui_box_basic.png"), temp12->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp12);
	}

	Image_info *temp13 = new Image_info(L"배경음on", Scene_Title_Setting, UI_title_BGMOn, (rc.left + rc.right) / 2 +30, (rc.top + rc.bottom) / 2 - 15,
		50, 50, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\MusicOn.png"), temp13->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp13);
	}

	Image_info *temp14 = new Image_info(L"배경음off", Scene_Title_Setting,UI_title_BGMOff, (rc.left + rc.right) / 2 + 30, (rc.top + rc.bottom) / 2 - 15,
		50, 50, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\MusicOff.png"), temp14->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp14);
	}

	Image_info *temp15 = new Image_info(L"효과음on", Scene_Title_Setting, UI_title_SoundOn, (rc.left + rc.right) / 2 + 30, (rc.top + rc.bottom) / 2 +50,
		50, 50, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\효과음.png"), temp15->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp15);
	}
	Image_info *temp16 = new Image_info(L"효과음off", Scene_Title_Setting, UI_title_SoundOff, (rc.left + rc.right) / 2 + 30, (rc.top + rc.bottom) / 2 +50,
		50, 50, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("Resource\\효과음제거.png"), temp16->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp16);
	}


	
}

void UI_Manager::TextInit()
{
	UI_Mgr->Add_Text(L"아이디 입력창", Scene_Title, Text_title_ID, (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 150, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 - 20, 1,pTextFormat_);

	UI_Mgr->Add_Text(L"비밀번호 입력창", Scene_Title, Text_title_PW, (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 150, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 + 30, 1,  pTextFormat_);

	UI_Mgr->Add_Text(L"", Scene_Title, Text_title_PW_star, (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 150, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 + 30, 1,  pTextFormat_);

	UI_Mgr->Add_Text(L"로그인", Scene_Title, Text_title_login, (rc.left + rc.right) /
		2 - 215, (rc.top + rc.bottom) / 2 + 80, 1,  pTextFormat_);

	UI_Mgr->Add_Text(L"회원가입", Scene_Title, Text_title_register, (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 80, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 + 80, 1, pTextFormat_);
	UI_Mgr->Add_Text(L"배경음 설정 : ", Scene_Title_Setting, Text_TitleSet_BGM, (rc.left + rc.right) /
		2 - 200, (rc.top + rc.bottom) / 2 , 1, pTextFormat_);
	UI_Mgr->Add_Text(L"효과음 설정 : ", Scene_Title_Setting, Text_TitleSet_Effect, (rc.left + rc.right) /
		2 - 200, (rc.top + rc.bottom) / 2+60, 1,  pTextFormat_);
	UI_Mgr->Add_Text(L"HP : 32", Scene_Ingame, Text_ingame_hp, rc.left -70, (rc.bottom - 55), 1, pTextFormat_);
	UI_Mgr->Add_Text(L"Round 0", Scene_Ingame, Text_ingame_Round, rc.left - 70, (rc.top +15), 1, pTextFormat_);
	UI_Mgr->Add_Text(L"Next 03:21", Scene_Ingame, Text_ingame_time, rc.right - 230, (rc.top + 15), 1, pTextFormat_);
	UI_Mgr->Add_Text(L"웨이브가 시작됩니다!", Scene_NULL, Text_ingame_start_wave, (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 150, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2, 1, pTextFormat_WaveStart);
}

void UI_Manager::Print_All_Image()
{


	D2D1_RECT_F imgputrc = { 0 };
	
	m_d2dRenderTarget->BeginDraw();
	for (auto i : m_Image_list)
	{
		if (i->Active == true)
		{
			calcimgputrc(&imgputrc, i->x, i->y, i->scale_x, i->scale_y);
			m_d2dRenderTarget->DrawBitmap(i->Image, imgputrc,/*투명도*/i->alpha,/*이미지 품질*/D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				/*그릴 이미지의 부분, 널이면 전체 그림*/NULL);
		
				RECT rect;
				rect.left = i->x;
				rect.right = i->x+i->scale_x;
				rect.top = i->y;
				rect.bottom = i->y+i->scale_y;
				D2D1_RECT_F layoutRect = D2D1::RectF(
					static_cast<FLOAT>((rect.left) ),
					static_cast<FLOAT>((rect.top)),
					static_cast<FLOAT>(rect.right),
					static_cast<FLOAT>(rect.bottom)
					);
				switch (i->m_frame_type)
				{	
					
					case UI_Frame_null: break;
					case UI_Frame_Black: 	m_d2dRenderTarget->DrawRectangle(&layoutRect, pBlackBrush_); break;
					case UI_Frame_White : m_d2dRenderTarget->DrawRectangle(&layoutRect, pWhiteBrush_); break;
				}
			
			
		}
	}
	HRESULT hr = m_d2dRenderTarget->EndDraw();
}
void UI_Manager::Add_Text(wchar_t* _wszText_, type_Scene type_scene, UI_ID ui_id , FLOAT x, FLOAT y, FLOAT alpha, IDWriteTextFormat* TextForMat)
{
	
	Text_info *temp = new Text_info(_wszText_, type_scene, ui_id, x, y, alpha, TextForMat);
	m_Text_list.push_back(temp);
}
void UI_Manager::Delete_Text(wchar_t* _wszText_)
{
	/*
	for (auto i = m_Text_list.begin(); i == m_Text_list.end();)
	{
		
	
		else
		{
			i++;
		}
	
	}
	*/
	for (auto i : m_Text_list)
	{
		if (i->_wszText_ == _wszText_)
		{
			i->Active = false;
		}
	}
}
void UI_Manager::Set_Image_Active(wchar_t* _wszText_, bool boolean)
{


	for (auto i : m_Image_list)
	{
		if (i->_wszText_ == _wszText_)
		{
			i->Active = boolean;
		}
	}
	
}
void UI_Manager::Delete_All_Image()
{
	//m_Image_list.clear();
	for (auto i :m_Image_list)
	{
		i->Active = false;
	}
}

void  UI_Manager::Delete_Text_All()
{
	for (auto i : m_Text_list)
	{
		i->Active = false;
	}
}
bool UI_Manager::MouseOn2D(wchar_t* _wszText_,int mouse_x, int mouse_y)
{
	
	for (auto i : m_Image_list)
	{

		if (i->_wszText_ == _wszText_ && i->Active == true)
		{


			switch (Get_nMessageID())
			{
			case WM_LBUTTONDOWN:
				if ((mouse_x > i->x && mouse_y > i->y) && (mouse_x < i->x + i->scale_x && mouse_y < i->y + i->scale_y))
				{
					i->alpha = 0.3;
					UI_ClickOn = true;
					Sound_Mgr->PlayEffect(Sound_click1);
				}
				break;
			case WM_MOUSEMOVE:
				if ((mouse_x > i->x && mouse_y > i->y) && (mouse_x < i->x + i->scale_x && mouse_y < i->y + i->scale_y))
				{
					if (UI_ClickOn)
						i->alpha = 0.3;

				}
				else
				{
					if ((i->UI_id == (UI_title_button_PW) || (i->UI_id == UI_title_button_ID)))
					{

					}
					else {
						i->alpha = 0.9;
					}
				
				}
				break;
			case WM_LBUTTONUP:
				if ((i->UI_id == (UI_title_button_PW) || (i->UI_id == UI_title_button_ID)))
				{

				}
				else {
					i->alpha = 0.9;
				}

				if ((mouse_x > i->x && mouse_y > i->y) && (mouse_x < i->x + i->scale_x && mouse_y < i->y + i->scale_y))
				{

					UI_ClickOn = false;
					return true;

				}
				else
				{


				}

				UI_ClickOn = false;
				break;
			}

		}

	}			
	return false;

}

void UI_Manager::chageScene()
{
	Delete_All_Image();
	Delete_Text_All();
}

void UI_Manager::SetScene()
{
	for (auto i : m_Image_list)
	{
		switch (i->m_type_scene)
		{

		case Scene_Title:
			Title.UI_Image_list.push_back(i->UI_id);
			break;
		case Scene_Ingame:
			Title.UI_Image_list.push_back(i->UI_id);
			break;
		default: break;
		}
	}
	for (auto i : m_Text_list)
	{
		switch (i->m_type_scene)
		{

		case Scene_Title:
			Title.UI_Image_list.push_back(i->UI_id);
			break;
		case Scene_Ingame:
			Title.UI_Image_list.push_back(i->UI_id);
			break;
		default: break;
		}
	}

	
}

void UI_Manager::Set_Scene_UI(type_Scene Scenetype)
{
	//타이틀에 있는 모든 이미지와 텍스트의 상태를 세팅
\
		for (auto i_image : m_Image_list)
		{
			if (Scenetype == i_image->m_type_scene)
			{
				i_image->Active = true;
			
			}

		}
		for (auto i_Text : m_Text_list)
		{
			if (Scenetype == i_Text->m_type_scene)
			{
				i_Text->Active = true;

			}
		}
		if (Get_input_ID_state())
		{
			Delete_Text(L"아이디 입력창");
			GetImagePtr(UI_title_button_ID)->alpha = 0.2;
		}

		if (Get_input_PW_state())
		{
			Delete_Text(L"비밀번호 입력창");
			GetImagePtr(UI_title_button_PW)->alpha = 0.2;
		}

	
	
}
void UI_Manager::UI_Scene_Mgr(int x, int y)
{
	
	InputID_PW();
	switch (m_gamescene)
	{

	case 0: 
		Set_Scene_UI(Scene_Title);
		
		//ShowCursor(FALSE);
		InTitle_UI(x, y); 
			break;
	case 1:

		InTitle_Option(x, y);
		break;
	case 2: 
		Set_Scene_UI(Scene_Ingame);
		InGame_UI(x, y);


		break;
	}
}
void UI_Manager::InTitle_UI(int x, int y)
{
	if (Get_input_ID_state())
	{
		if (GetTextptr(Text_title_PW_star)->_wszText_[0] != '\0')
		{
		

			GetTextptr(Text_title_PW)->Active = false;
			GetTextptr(Text_title_PW_star)->Active = true;
		}
		GetImagePtr(UI_title_button_ID)->alpha = 0.2;
	}

	if (Get_input_PW_state())
	{
		GetImagePtr(UI_title_button_PW)->alpha = 0.2;
		GetTextptr(Text_title_PW)->Active = false;
		GetTextptr(Text_title_PW_star)->Active = true;
	}

	if (MouseOn2D(L"아이디 입력", x, y))
	{
		Set_input_ID_state(true);
		Set_input_PW_state(false);
	//	Delete_Text(L"아이디 입력창");
		
		

	}  
	if (MouseOn2D(L"패스워드 입력", x, y))
	{
	//	Set_input_PW_state(true);
		Delete_Text(L"비밀번호 입력창");

		Set_input_PW_state(true);
		Set_input_ID_state(false);

	}


	
	if (MouseOn2D(L"타이틀_옵션아이콘", x, y))
	{
		chageScene();
		m_gamescene=Scene_Title_Setting;
		Set_Image_Active(L"타이틀화면",true);
		Set_Image_Active(L"타이틀_옵션아이콘", true);
		Set_Image_Active(L"Title_Menu_Box", true);

		Set_Scene_UI(Scene_Title_Setting);
		Set_Image_Active(L"효과음off", false);
		Set_Image_Active(L"배경음off", false);
	}
	

	if (UI_Mgr->MouseOn2D(L"로그인", x, y))
	{
		chageScene();
		SetCursor(LoadCursor(0, IDC_ARROW));
		ShowCursor(false);
		Set_input_ID_state(false);
		Set_input_PW_state(false);
		Set_Scene_UI(Scene_Ingame);
		m_gamescene=Scene_Ingame;
		//	UI_Mgr->Set_Image_Active(L"메시지박스", true);

	}

}
	

	


void UI_Manager::InTitle_Option(int x, int y)
{
	if (MouseOn2D(L"타이틀_옵션아이콘", x, y))
	{
		chageScene();

		Set_Scene_UI(Scene_Title);
		m_gamescene=Scene_Title;
	}
	if (MouseOn2D(L"효과음off", x, y))
	{
		Set_Image_Active(L"효과음off", false);
		Set_Image_Active(L"효과음on", true);

		Sound_Mgr->Turn_Sound_Effect(true);
	}
	else if (MouseOn2D(L"효과음on", x, y))
	{
		Set_Image_Active(L"효과음on", false);
		Set_Image_Active(L"효과음off", true);
		Sound_Mgr->Turn_Sound_Effect(false);
	}
	if (MouseOn2D(L"배경음off", x, y))
	{
		Set_Image_Active(L"배경음off", false);
		Set_Image_Active(L"배경음on", true);
		Sound_Mgr->Turn_Sound_BG(true);
	}
	else if (MouseOn2D(L"배경음on", x, y))
	{
		Set_Image_Active(L"배경음on", false);
		Set_Image_Active(L"배경음off", true);
		Sound_Mgr->Turn_Sound_BG(false);
	}
}
void UI_Manager::InGame_UI(int x, int y)
{
	/*
	if (MouseOn2D(L"메시지박스", x, y))
	{
		Set_Image_Active(L"메시지박스", false);
		SetCursor(LoadCursor(0, IDC_ARROW));
		ShowCursor(true);
	
		//Change_HP_TEXT(43);
	}
	*/
	

	


}

void UI_Manager::Change_HP_TEXT(int player_hp)
{
	
	int myIntValue = player_hp;
	Text_info* temp;
	wchar_t temp_wchar;
	static wchar_t m_reportFileName[10];

	swprintf_s(m_reportFileName, L"체력: %d", myIntValue);
	temp = GetTextptr(Text_ingame_hp);

	temp->_wszText_ = m_reportFileName;

	

}

void UI_Manager::Change_Round_TEXT(int Round)
{

	int myIntValue = Round;
	Text_info* temp;
	wchar_t temp_wchar;
	static wchar_t m_reportFileName[10];

	swprintf_s(m_reportFileName, L"Round %d", myIntValue);
	temp = GetTextptr(Text_ingame_Round);


	temp->_wszText_ = m_reportFileName; //수정, 동적할당 삭제


}

void UI_Manager::Change_Time_TEXT(Gamestate_type type)
{
	
	int myIntValue;
	Text_info* temp;
	wchar_t temp_wchar;
	static wchar_t m_reportFileName[20];

	if (type==game_waiting_wave)
	{
		myIntValue = Time_Mgr->Get_remain_Wait_time();
		if (myIntValue % 60 < 10)
		{
			swprintf_s(m_reportFileName, L"Next   %d : 0%d ", myIntValue / 60, myIntValue % 60);

		}
		else
		{
			swprintf_s(m_reportFileName, L"Next   %d : %d ", myIntValue / 60, myIntValue % 60);
		}
	
	}
	else if(type==game_waving)
	{
		myIntValue = Time_Mgr->Get_remain_Wave_time();
		if (myIntValue % 60 < 10)
		{
			swprintf_s(m_reportFileName, L"End   %d : 0%d", myIntValue / 60, myIntValue % 60);

		}
		else
		{
			swprintf_s(m_reportFileName, L"End   %d : %d", myIntValue / 60, myIntValue % 60);
		}

	}


	temp = GetTextptr(Text_ingame_time);
	temp->_wszText_ = m_reportFileName;
	


}

Text_info* UI_Manager::GetTextptr(UI_ID ui_id)
{
	for (auto i : m_Text_list)
	{
		
		if (ui_id == i->UI_id)
		{
			
			return i;
			
		}
	}
}

Image_info* UI_Manager::GetImagePtr(UI_ID ui_id)
{
	for (auto i : m_Image_list)
	{
		

		if (ui_id == i->UI_id)
		{

			return i;

		}
	}
}


void UI_Manager::InputID_PW()
{
	Text_info * temp;
	Text_info * temp2;
	Text_info * temp3;
	static wchar_t wiputIDstr[50];
	static int ID_count = 0;
	static int PW_count = 0;
	static wchar_t wiputPWstr[50];
	static wchar_t wiputPWstr_star[50];
	_wsetlocale(LC_ALL, L"korean");
	std::wcin.imbue(std::locale("kor"));
	/* 패스워드 마지막 부분 감추기
	if (Get_input_ID_state() == true && Get_input_PW_state() == false)
	{
		temp3->_wszText_[PW_count] = '\0';
		if (PW_count > 0)
		{
			temp3->_wszText_[PW_count - 1] = '*';
		}
	}
	*/
	switch (m_nMessageID)
	{
	case WM_CHAR:
		if (m_wParam ==  VK_TAB || m_wParam == VK_SPACE)
		{
		}
		else if (Get_input_ID_state())
		{
			temp = GetTextptr(Text_title_ID);

	

			if (m_wParam == VK_RETURN)
			{
				Set_input_PW_state(true);
				Set_input_ID_state(false);
				ID_count = 0;
				break;
			}
			else if (m_wParam == VK_BACK)
			{
				if (ID_count > 0)
					ID_count--;
				wiputIDstr[ID_count] = '\0';

			}
			else
			{

				wiputIDstr[ID_count++] = m_wParam;
				wiputIDstr[ID_count] = '\0';
				//	//wsprintf(wiputIDstr, L"%s", iputIDstr);


			}

			temp->_wszText_ = wiputIDstr;

		}
		else if (Get_input_PW_state()) {
			temp2 = GetTextptr(Text_title_PW);
			temp3 = GetTextptr(Text_title_PW_star);


			if (m_wParam == VK_RETURN)
			{
				Set_input_PW_state(false);
				PW_count = 0;
				break;
			}
			else if (m_wParam == VK_BACK)
			{
				if (PW_count > 0)
					PW_count--;
				wiputPWstr[PW_count] = '\0';
				wiputPWstr_star[PW_count] = '\0';

			}
			else
			{

				wiputPWstr[PW_count] = m_wParam;
				wiputPWstr_star[PW_count] = m_wParam;
				if (PW_count>0)
				wiputPWstr_star[PW_count-1] = '*';
				PW_count++;
			
				wiputPWstr[PW_count] = '\0';
				if (PW_count>0)
				wiputPWstr_star[PW_count] = '\0';

			}
			
			temp2->_wszText_ = wiputPWstr;
			temp3->_wszText_ = wiputPWstr_star;
		}

		break;
	
	}

}
