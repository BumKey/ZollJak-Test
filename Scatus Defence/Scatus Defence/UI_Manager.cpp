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
	UI_ClickOn = false;
	m_gamestat = 0;
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

	hr = pWICFactory->CreateDecoderFromFilename(pngfn,  // ���Ϸκ��� ���ڴ� ����
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		&pImgLoader);
	if (hr == S_OK)
	{
		IWICBitmapFrameDecode* pWICFdecoder = NULL;
		hr = pImgLoader->GetFrame(0, &pWICFdecoder); // ���ڴ��� ���� ����������

		if (hr == S_OK)
		{
			IWICFormatConverter *pFMTconv = NULL; // �����͸� ����� ���������� �ʱ�ȭ�Ѵ�

			hr = pWICFactory->CreateFormatConverter(&pFMTconv);
			if (hr == S_OK)
			{
				pFMTconv->Initialize(pWICFdecoder,
					GUID_WICPixelFormat32bppPBGRA,
					WICBitmapDitherTypeNone,
					NULL,
					0.0f,
					WICBitmapPaletteTypeCustom);

				hr = m_d2dRenderTarget->CreateBitmapFromWicBitmap(pFMTconv, NULL, &out); // ���������κ��� d2d��Ʈ���̹��� ����
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


void UI_Manager::CreateD2DrenderTarget(HWND hwnd) {



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


	// Center align (horizontally) the text.
	if (SUCCEEDED(hr))
	{
		hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}
	/*
	if (SUCCEEDED(hr))
	{
		hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}*/

		pD2DFactory_->GetDesktopDpi(&m_dpiX_, &m_dpiY_);



	pD2DFactory_->CreateDxgiSurfaceRenderTarget(m_backbuffer,
	D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
	D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), m_dpiX_, m_dpiY_), &m_d2dRenderTarget);
	if (m_backbuffer)m_backbuffer->Release();


	hr = m_d2dRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black),
		&pBlackBrush_
		);

	hr = m_d2dRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White),
		&pWhiteBrush_
		);

	//��Ʈ�� �������� �κ�
	CoInitialize(NULL);  // COM��ü�� �ʱ�ȭ�� �ȵǼ� ���������� ��츦 ���� �ʱ�ȭ ���ڴ� ������ 0 �ƴϸ� ��
	CoCreateInstance(CLSID_WICImagingFactory,  // �������̽�����
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pWICFactory));

	GetClientRect(hwnd, &rc);
	Load_All_Image();
	TextInit();



	

}



UI_Manager* UI_Manager::Instance()
{
	static UI_Manager instance;

	return &instance;
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
				pTextFormat_,    // The text format.
				layoutRect,       // The region of the window where the text will be rendered.
				pWhiteBrush_     // The brush used to draw the text.
			);
		}
	}


	HRESULT hr = m_d2dRenderTarget->EndDraw();











}

void UI_Manager::Load_All_Image()
{

	Image_info *temp = new Image_info(L"Ÿ��Ʋȭ��", Scene_Title, UI_title_BG,rc.left , rc.top,rc.right-rc.left,rc.bottom-rc.top, 1, UI_Frame_null);
	hr= LoadPNG2DDBitmap(TEXT("title.jpg"),temp->Image);
	if (SUCCEEDED(hr))
	{
			m_Image_list.push_back(temp);
	}
	Image_info *temp2 = new Image_info(L"�޽����ڽ�", Scene_nothing, UI_ingame_chat_box,100, 100, 400,400,0.0, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("��ô��ô�Ŀ���.jpg"), temp2->Image);
	if (SUCCEEDED(hr))
	{
	m_Image_list.push_back(temp2);
	}

	/* ����� �ڽ�
	Image_info *temp3 = new Image_info(L"Title_Menu_Box", (rc.left+rc.right)/2 -150 , (rc.top + rc.bottom) / 2 -150 , 
		300, +300, 1);
	hr = LoadPNG2DDBitmap(TEXT("sony.jpg"), temp3->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp3);
	}
	*/
	Image_info *temp3 = new Image_info(L"Title_Menu_Box", Scene_Title, UI_title_menubox,(rc.left + rc.right) / 2 - 150, (rc.top + rc.bottom) / 2 - 50,
		300, +200, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("sony.jpg"), temp3->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp3);
	}

	Image_info *temp4 = new Image_info(L"���̵� �Է�", Scene_Title, UI_title_button_ID, (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 - 25,
		260, 40, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("ui_box_basic.png"), temp4->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp4);
	}

	Image_info *temp5 = new Image_info(L"�н����� �Է�", Scene_Title, UI_title_button_PW, (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 +25,
		260, 40, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("ui_box_basic.png"), temp5->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp5);
	}


	Image_info *temp6 = new Image_info(L"�α���", Scene_Title, UI_title_button_login, (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 +75,
		125, 40, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("ui_box_basic.png"), temp6->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp6);
	}

	Image_info *temp7 = new Image_info(L"ȸ������", Scene_Title, UI_title_button_register,(rc.left + rc.right) / 2+5, (rc.top + rc.bottom) / 2 +75,
		125, 40, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("ui_box_basic.png"), temp7->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp7);
	}

	Image_info *temp8 = new Image_info(L"Ÿ��Ʋ_�ɼǾ�����", Scene_Title, UI_title_button_option,rc.right - 50, rc.top +  10,
		40, 40, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("option.png"), temp8->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp8);
	}
	Image_info *temp9 = new Image_info(L"ũ�ν����", Scene_Ingame,UI_ingame_cross, (rc.left + rc.right)/2-15, (rc.top + rc.bottom)/2-15,
		30, 30, 0.7, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("ũ�ν����.png"), temp9->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp9);
	}
	Image_info *temp10 = new Image_info(L"ü��", Scene_Ingame, UI_ingame_hp, rc.left +30, (rc.bottom-60),
		100, 30, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("ui_box_basic.png"), temp10->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp10);
	}

	Image_info *temp11 = new Image_info(L"����", Scene_Ingame, UI_ingame_Round, rc.left + 30, (rc.top + 10),
		100, 30, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("ui_box_basic.png"), temp11->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp11);
	}

	Image_info *temp12 = new Image_info(L"�ð�", Scene_Ingame, UI_ingame_time, rc.right - 145, (rc.top + 10),
		130, 30, 0.8, UI_Frame_Black);
	hr = LoadPNG2DDBitmap(TEXT("ui_box_basic.png"), temp12->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp12);
	}

	Image_info *temp13 = new Image_info(L"�����on", Scene_Title_Setting, UI_title_BGMOn, (rc.left + rc.right) / 2 +30, (rc.top + rc.bottom) / 2 - 15,
		50, 50, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("MusicOn.png"), temp13->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp13);
	}

	Image_info *temp14 = new Image_info(L"�����off", Scene_Title_Setting,UI_title_BGMOff, (rc.left + rc.right) / 2 + 30, (rc.top + rc.bottom) / 2 - 15,
		50, 50, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("MusicOff.png"), temp14->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp14);
	}

	Image_info *temp15 = new Image_info(L"ȿ����on", Scene_Title_Setting, UI_title_SoundOn, (rc.left + rc.right) / 2 + 30, (rc.top + rc.bottom) / 2 +50,
		50, 50, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("ȿ����.png"), temp15->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp15);
	}
	Image_info *temp16 = new Image_info(L"ȿ����off", Scene_Title_Setting, UI_title_SoundOff, (rc.left + rc.right) / 2 + 30, (rc.top + rc.bottom) / 2 +50,
		50, 50, 0.8, UI_Frame_null);
	hr = LoadPNG2DDBitmap(TEXT("ȿ��������.png"), temp16->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp16);
	}


	
}

void UI_Manager::TextInit()
{
	UI_Mgr->Add_Text(L"���̵� �Է�â", Scene_Title, Text_title_ID, (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 150, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 - 25, 1);

	UI_Mgr->Add_Text(L"��й�ȣ �Է�â", Scene_Title, Text_title_PW, (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 150, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 + 25, 1);

	UI_Mgr->Add_Text(L"�α���", Scene_Title, Text_title_login, (rc.left + rc.right) /
		2 - 215, (rc.top + rc.bottom) / 2 + 80, 1);

	UI_Mgr->Add_Text(L"ȸ������", Scene_Title, Text_title_register, (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 80, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 + 80, 1);
	UI_Mgr->Add_Text(L"����� ���� : ", Scene_Title_Setting, Text_ingame_hp, (rc.left + rc.right) /
		2 - 200, (rc.top + rc.bottom) / 2 , 1);
	UI_Mgr->Add_Text(L"ȿ���� ���� : ", Scene_Title_Setting, Text_ingame_hp, (rc.left + rc.right) /
		2 - 200, (rc.top + rc.bottom) / 2+60, 1);
	UI_Mgr->Add_Text(L"HP : 32", Scene_Ingame, Text_ingame_hp, rc.left -70, (rc.bottom - 55), 1);
	UI_Mgr->Add_Text(L"Round 1", Scene_Ingame, Text_ingame_Round, rc.left - 70, (rc.top +15), 1);
	UI_Mgr->Add_Text(L"Next 03:21", Scene_Ingame, Text_ingame_time, rc.right - 230, (rc.top + 15), 1);
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
			m_d2dRenderTarget->DrawBitmap(i->Image, imgputrc,/*����*/i->alpha,/*�̹��� ǰ��*/D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				/*�׸� �̹����� �κ�, ���̸� ��ü �׸�*/NULL);
		
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
void UI_Manager::Add_Text(wchar_t* _wszText_, type_Scene type_scene, UI_ID ui_id , FLOAT x, FLOAT y, FLOAT alpha)
{
	
	Text_info *temp = new Text_info(_wszText_, type_scene, ui_id, x, y, alpha);
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
				if ((mouse_x > i->x && mouse_y > i->y) && (mouse_x < i->x + i->scale_x && mouse_y < i->y + i->scale_y))
				{
					
						switch (Get_nMessageID())
						{
						case WM_LBUTTONDOWN:
							i->alpha = 0.5;
							UI_ClickOn = true;
							Sound_Mgr->PlayEffect(Sound_click1);
							break;
						case WM_MOUSEMOVE:
							if (UI_ClickOn)
								i->alpha = 0.5;
							break;
						case WM_LBUTTONUP:
							i->alpha = 0.8;
							UI_ClickOn = false;
							return true;
							break;
						
					}
					
			
				}
				else
				{

				
						i->alpha = 0.8;
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
	//Ÿ��Ʋ�� �ִ� ��� �̹����� �ؽ�Ʈ�� ���¸� ����
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
		
	
}
void UI_Manager::UI_Scene_Mgr(int x, int y)
{
	
	if (UI_Mgr->Get_input_ID_state())
	{
		for (auto i : UI_Mgr->m_Text_list)
		{
			if (i->UI_id == Text_title_ID)
				std::wcin >> i->_wszText_;
		}

	}
	switch (m_gamestat)
	{

	case 0: Set_Scene_UI(Scene_Title);
		
		//ShowCursor(FALSE);
		InTitle_UI(x, y); 
			break;
	case 1:

		InTitle_Option(x, y);
		break;
	case 2: 
		Set_Scene_UI(Scene_Ingame);
		InGame_UI(x, y);
		//SetCursor(LoadCursor(0, IDC_ARROW));
		//ShowCursor(true);


		break;
	}
}
void UI_Manager::InTitle_UI(int x, int y)
{


	if (MouseOn2D(L"���̵� �Է�", x, y))
	{
		Set_input_ID_state(true);
		Delete_Text(L"���̵� �Է�â");
		

	}  


	
	if (MouseOn2D(L"Ÿ��Ʋ_�ɼǾ�����", x, y))
	{
		chageScene();
		m_gamestat=1;
		Set_Image_Active(L"Ÿ��Ʋȭ��",true);
		Set_Image_Active(L"Ÿ��Ʋ_�ɼǾ�����", true);
		Set_Image_Active(L"Title_Menu_Box", true);

		Set_Scene_UI(Scene_Title_Setting);
		Set_Image_Active(L"ȿ����off", false);
		Set_Image_Active(L"�����off", false);
	}
	

	if (UI_Mgr->MouseOn2D(L"�α���", x, y))
	{
		chageScene();
		
		Set_Scene_UI(Scene_Ingame);
		m_gamestat=2;
		//	UI_Mgr->Set_Image_Active(L"�޽����ڽ�", true);

	}

}
	

	


void UI_Manager::InTitle_Option(int x, int y)
{
	if (MouseOn2D(L"Ÿ��Ʋ_�ɼǾ�����", x, y))
	{
		chageScene();

		Set_Scene_UI(Scene_Title);
		m_gamestat=0;
	}
	if (MouseOn2D(L"ȿ����off", x, y))
	{
		Set_Image_Active(L"ȿ����off", false);
		Set_Image_Active(L"ȿ����on", true);

		Sound_Mgr->Turn_Sound_Effect(true);
	}
	else if (MouseOn2D(L"ȿ����on", x, y))
	{
		Set_Image_Active(L"ȿ����on", false);
		Set_Image_Active(L"ȿ����off", true);
		Sound_Mgr->Turn_Sound_Effect(false);
	}
	if (MouseOn2D(L"�����off", x, y))
	{
		Set_Image_Active(L"�����off", false);
		Set_Image_Active(L"�����on", true);
		Sound_Mgr->Turn_Sound_BG(true);
	}
	else if (MouseOn2D(L"�����on", x, y))
	{
		Set_Image_Active(L"�����on", false);
		Set_Image_Active(L"�����off", true);
		Sound_Mgr->Turn_Sound_BG(false);
	}
}
void UI_Manager::InGame_UI(int x, int y)
{

	if (MouseOn2D(L"�޽����ڽ�", x, y))
	{
		Set_Image_Active(L"�޽����ڽ�", false);
	}
	




}