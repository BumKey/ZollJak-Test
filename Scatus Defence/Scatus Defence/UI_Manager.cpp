
#include "UI_Manager.h"

void calcimgputrc(D2D1_RECT_F* rcf, int x, int y, int w, int h)
{
	if (rcf != NULL)
	{
		rcf->left = float(x);
		rcf->top = float(y);
		rcf->right = float(x) + float(w);
		rcf->bottom = float(y) + float(h);
	}

}
UI_Manager::UI_Manager()
{
	input_ID = false;
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
	UI_Mgr->Add_Text(L"���̵� �Է�â", (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 150, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 - 25, 1);

	UI_Mgr->Add_Text(L"��й�ȣ �Է�â", (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 150, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 + 25, 1);

	UI_Mgr->Add_Text(L"�α���", (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 215, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 + 80, 1);

	UI_Mgr->Add_Text(L"ȸ������", (UI_Mgr->rc.left + UI_Mgr->rc.right) /
		2 - 80, (UI_Mgr->rc.top + UI_Mgr->rc.bottom) / 2 + 80, 1);






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
			static_cast<FLOAT>(i->x + 300),
			static_cast<FLOAT>(i->y + 100)
			);

		m_d2dRenderTarget->DrawText(
			wszText_,        // The string to render.
			cTextLength_,    // The string's length.
			pTextFormat_,    // The text format.
			layoutRect,       // The region of the window where the text will be rendered.
			pWhiteBrush_     // The brush used to draw the text.
			);

	}


	HRESULT hr = m_d2dRenderTarget->EndDraw();











}

void UI_Manager::Load_All_Image()
{

	Image_info *temp = new Image_info(L"Ÿ��Ʋȭ��", rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 1);
	hr = LoadPNG2DDBitmap(TEXT("title.jpg"), temp->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp);
	}
	Image_info *temp2 = new Image_info(L"�޽����ڽ�", 100, 100, 400, 400, 0.5);
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
	Image_info *temp3 = new Image_info(L"Title_Menu_Box", (rc.left + rc.right) / 2 - 150, (rc.top + rc.bottom) / 2 - 50,
		300, +200, 0.8);
	hr = LoadPNG2DDBitmap(TEXT("sony.jpg"), temp3->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp3);
	}

	Image_info *temp4 = new Image_info(L"���̵� �Է�", (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 - 25,
		260, 40, 0.8);
	hr = LoadPNG2DDBitmap(TEXT("��ô��ô�Ŀ���.jpg"), temp4->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp4);
	}

	Image_info *temp5 = new Image_info(L"�н����� �Է�", (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 + 25,
		260, 40, 0.8);
	hr = LoadPNG2DDBitmap(TEXT("��ô��ô�Ŀ���.jpg"), temp5->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp5);
	}


	Image_info *temp6 = new Image_info(L"�α���", (rc.left + rc.right) / 2 - 130, (rc.top + rc.bottom) / 2 + 75,
		125, 40, 0.8);
	hr = LoadPNG2DDBitmap(TEXT("��ô��ô�Ŀ���.jpg"), temp6->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp6);
	}

	Image_info *temp7 = new Image_info(L"ȸ������", (rc.left + rc.right) / 2 + 5, (rc.top + rc.bottom) / 2 + 75,
		125, 40, 0.8);
	hr = LoadPNG2DDBitmap(TEXT("��ô��ô�Ŀ���.jpg"), temp7->Image);
	if (SUCCEEDED(hr))
	{
		m_Image_list.push_back(temp7);
	}
	Set_Image_Active(L"Ÿ��Ʋȭ��", true);
	Set_Image_Active(L"Title_Menu_Box", true);
	Set_Image_Active(L"���̵� �Է�", true);
	Set_Image_Active(L"�н����� �Է�", true);
	Set_Image_Active(L"�α���", true);
	Set_Image_Active(L"ȸ������", true);


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
			rect.right = i->x + i->scale_x;
			rect.top = i->y;
			rect.bottom = i->y + i->scale_y;
			D2D1_RECT_F layoutRect = D2D1::RectF(
				static_cast<FLOAT>((rect.left)),
				static_cast<FLOAT>((rect.top)),
				static_cast<FLOAT>(rect.right),
				static_cast<FLOAT>(rect.bottom)
				);

			m_d2dRenderTarget->DrawRectangle(&layoutRect, pBlackBrush_);

		}
	}
	HRESULT hr = m_d2dRenderTarget->EndDraw();
}
void UI_Manager::Add_Text(wchar_t* _wszText_, FLOAT x, FLOAT y, FLOAT alpha)
{
	Text_info *temp = new Text_info(_wszText_, x, y, alpha);
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
			i->_wszText_ = L"";
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
	for (auto i : m_Image_list)
	{
		i->Active = false;
	}
}

void  UI_Manager::Delete_Text_All()
{
	m_Text_list.clear();
}
bool UI_Manager::MouseOn2D(wchar_t* _wszText_, int mouse_x, int mouse_y)
{

	for (auto i : m_Image_list)
	{
		if (i->_wszText_ == _wszText_ && i->Active == true)
		{
			if (mouse_x > i->x && mouse_y > i->y)
			{
				if (mouse_x < i->x + i->scale_x && mouse_y < i->y + i->scale_y)
				{
					return true;
				}
			}
		}
	}
	return false;

}