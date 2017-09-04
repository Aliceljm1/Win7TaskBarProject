// Main
#include "stdafx.h"



// Direct 2D
ID2D1Factory* d2_f = 0;
ID2D1HwndRenderTarget* d2_RT = 0;

// Write Factory and fonts
IDWriteFactory* d2_w = 0;
IDWriteTextFormat* fo2d_n = 0; // Normal 
IDWriteTextFormat* fo2d_b = 0; // Bold

HBITMAP OkImage = 0;
HBITMAP ErrorImage = 0;
HBITMAP DownloadingImage = 0;


// Function to get a D2 color from an ARGB
static D2D1_COLOR_F GetD2Color(unsigned long c)
	{
	D2D1_COLOR_F cc;
	cc.a = GetAValue(c)/255.0f;
	if (cc.a == 0)
		cc.a = 1.0f;
	cc.r = GetRValue(c)/255.0f;
	cc.g = GetGValue(c)/255.0f;
	cc.b = GetBValue(c)/255.0f;
	return cc;
	}

// Function to get a D2 solid brush from an ARGB
ID2D1SolidColorBrush* GetD2SolidBrush(unsigned long c)
	{
	if (!d2_f || !d2_RT)
		return 0;

	ID2D1SolidColorBrush* b = 0;
	D2D1_COLOR_F cc = GetD2Color(c);
	d2_RT->CreateSolidColorBrush(cc,&b);
	return b;
	}


void D2Image(float x1,float y1,HBITMAP hB,float Op,bool HasAlpha)
	{
	UNREFERENCED_PARAMETER(HasAlpha);
	BITMAP bo = {0}; 
	GetObject(hB,sizeof(bo),&bo);

	ID2D1HwndRenderTarget* pRT = d2_RT;
	if (!pRT)
		return;

	WICBitmapAlphaChannelOption ao = WICBitmapUseAlpha;
	IWICBitmap* wb = 0;
	IWICImagingFactory* pImageFactory = 0;
	CoCreateInstance(CLSID_WICImagingFactory,0,CLSCTX_ALL,__uuidof(IWICImagingFactory),(void**)&pImageFactory);
	if (!pImageFactory)
		return;

	pImageFactory->CreateBitmapFromHBITMAP(hB,0,ao,&wb);
	if (!wb)
		{
		pImageFactory->Release();
		return;
		}
	ID2D1Bitmap* b = 0;
	pRT->CreateBitmapFromWicBitmap(wb,0,&b);
	if (!b)
		{
		// Convert it
		IWICFormatConverter* spConverter = 0;
		pImageFactory->CreateFormatConverter(&spConverter);
		if (spConverter)
			{
			spConverter->Initialize(wb,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.f,WICBitmapPaletteTypeMedianCut);
			pRT->CreateBitmapFromWicBitmap(spConverter,0,&b);
			spConverter->Release();
			}
		}
	if (wb)
		{
		wb->Release();
		wb = 0;
		}
	if (b)
		{
		D2D1_RECT_F r;
		r.left = (FLOAT)x1;
		r.top = (FLOAT)y1;
		r.right = (FLOAT)(x1 + bo.bmWidth);
		r.bottom = (FLOAT)(y1 + bo.bmHeight);
		pRT->DrawBitmap(b,r,Op);
		b->Release();
		}
	pImageFactory->Release();
	}

// Function to get text's metrics
POINT GetD2TextSize(IDWriteTextFormat* ffo,wchar_t* txt,int l = -1)
	{
	POINT p = {0};
	if (!d2_w)
		return p;

	// Create a layout
	IDWriteTextLayout* lay = 0;
	d2_w->CreateTextLayout(txt,l == -1 ? wcslen(txt) : l,ffo,1000,1000,&lay);
	if (!lay)
		return p;
	DWRITE_TEXT_METRICS m = {0};
	lay->GetMaxWidth();
	lay->GetMetrics(&m);
	lay->Release();

	// Save the metrics
	int wi = (int)m.widthIncludingTrailingWhitespace;
	if (m.widthIncludingTrailingWhitespace > (float)wi)
		wi++;
	int he = (int)m.height;
	if (m.height > (float)he)
		he++;
	p.x = wi;
	p.y = he;
	return p;
	}


bool D2DCreateFonts()
	{
	// End Fonts
	if (fo2d_n)
		fo2d_n->Release();
	fo2d_n = 0;
	if (fo2d_b)
		fo2d_b->Release();
	fo2d_b = 0;

	// New Fonts
	FLOAT fs = (FLOAT)abs(CurrentFont.Size);
	d2_w->CreateTextFormat(CurrentFont.Family,0,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,fs,L"",&fo2d_n);
	if (!fo2d_n)
		return false;

	d2_w->CreateTextFormat(CurrentFont.Family,0,DWRITE_FONT_WEIGHT_BOLD,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,fs,L"",&fo2d_b);
	if (!fo2d_b)
		{
		fo2d_n->Release();
		fo2d_n = 0;
		return false;
		}

	return true;
	}

bool D2DInit(HWND hh)
	{
	// CoCreate Factory
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory),0,(void**)&d2_f);
	if (!d2_f)
		return false;

	// HWND Target Render
	// First try hardware
	D2D1_RENDER_TARGET_PROPERTIES def = D2D1::RenderTargetProperties();
	def.type = D2D1_RENDER_TARGET_TYPE_HARDWARE;

	RECT rc = {0};
	GetClientRect(hh,&rc);
	d2_f->CreateHwndRenderTarget(def,D2D1::HwndRenderTargetProperties(hh,D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top)),&d2_RT);
	if (!d2_RT)
		{
		// Try again
		def.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;

		// Create a Direct2D render target	
		d2_f->CreateHwndRenderTarget(def,D2D1::HwndRenderTargetProperties(hh,D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top)),&d2_RT);
		}

	if (!d2_RT)
		{
		d2_f->Release();
		d2_f = 0;
		return false;
		}

	// DirectWrite
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),(IUnknown**)&d2_w);
	if (!d2_w)
		{
		d2_RT->Release();
		d2_RT = 0;
		d2_f->Release();
		d2_f = 0;
		return false;
		}

	if (!D2DCreateFonts())
		{
		d2_w->Release();
		d2_w = 0;
		d2_RT->Release();
		d2_RT = 0;
		d2_f->Release();
		d2_f = 0;
		return false;
		}

	// Load bitmaps
	HBITMAP LoadTransparentToolbarImage(HINSTANCE hI,const TCHAR* Name,unsigned long Color);
	OkImage = LoadTransparentToolbarImage(hAppInstance,_T("IMAGE_DONE"),0xFFFFFFFF);
	ErrorImage = LoadTransparentToolbarImage(hAppInstance,_T("IMAGE_ERROR"),0xFFFFFFFF);
	DownloadingImage = LoadTransparentToolbarImage(hAppInstance,_T("IMAGE_DOWNLOADING"),0xFFFFFFFF);


	return true;
	}

void D2DEnd()
	{
	if (DownloadingImage)
		DeleteObject(DownloadingImage);
	DownloadingImage = 0;
	if (ErrorImage)
		DeleteObject(ErrorImage);
	ErrorImage = 0;
	if (OkImage)
		DeleteObject(OkImage);
	OkImage = 0;

	// End Fonts
	if (fo2d_n)
		fo2d_n->Release();
	fo2d_n = 0;
	if (fo2d_b)
		fo2d_b->Release();
	fo2d_b = 0;

	// End DirectWrite
	if (d2_w)
		d2_w->Release();
	d2_w = 0;

	// End HWND Target
	if (d2_RT)
		d2_RT->Release();
	d2_RT = 0;

	// End Factory
	if (d2_f)
		d2_f->Release();
	d2_f = 0;
	}

bool NextClearD2D = false;
void D2DrawAnimation(double xx,double yy)
	{
	// HRESULT hr = S_OK;
	if (!d2_f || !d2_RT || !fo2d_n || !fo2d_b)
		return;

	d2_RT->BeginDraw();
	RECT rc;
	GetClientRect(MainWindow,&rc);


	for(unsigned int i = 0 ; i < Downloads.size() ; i++)
		{
		double x = xx,y = yy;

		A_DOWNLOAD& A = *Downloads[i];
		if (A.State != 1)
			continue; // Not downloading 
		if (A.PauseThis && WaitForSingleObject(A.PauseThis,0) != WAIT_OBJECT_0)
			continue;
		if (PauseAll && WaitForSingleObject(PauseAll,0) != WAIT_OBJECT_0)
			continue;


		RECT& r = A.AnimRect;
		if (r.right == 0)
			continue; // Not set yet

		unsigned int C = FGColor & 0x00FFFFFF;
		float Perc = (float)A.Percentage;
		if (Perc == 100)
			continue;

		Perc /= 100.0;
		Perc *= 255.0;
		unsigned int CC = (unsigned int)Perc;
		if (CC < 20)
			CC = 20;
		CC <<= 24;
		C |= CC;

		ID2D1SolidColorBrush* brs = GetD2SolidBrush(FGColor);
		if (!brs)
			continue;

		ID2D1SolidColorBrush* br = GetD2SolidBrush(CC);
		if (!br)
			{
			brs->Release();
			brs = 0;
			continue;
			}

		ID2D1SolidColorBrush* brb1 = 0;
		brb1 = GetD2SolidBrush(BGColor);
		if (!brb1)
			{
			br->Release();
			br = 0;
			brs->Release();
			brs = 0;
			continue;
			}
		ID2D1SolidColorBrush* brb2 = GetD2SolidBrush(AXRGB(i % 2 ? 0x20 : 0x10,0x200,0x200,0x200));
		if (!brb2)
			{
			brb1->Release();
			brb1 = 0;
			br->Release();
			br = 0;
			brs->Release();
			brs = 0;
			continue;
			}

		if (NextClearD2D)
			{
			D2D1_RECT_F rx = {(FLOAT)r.left,(FLOAT)r.top - 2.0f,(FLOAT)r.right,(FLOAT)r.bottom + 2.0f};
			d2_RT->FillRectangle(rx,brb1);
			if (A.Selected)
				d2_RT->FillRectangle(rx,brb2);

			A.PrevPoint.x = A.PrevPoint.y = 0;
			}
		D2D1_RECT_F rx = {(FLOAT)r.right,(FLOAT)r.top - 2.0f,(FLOAT)rc.right,(FLOAT)r.bottom + 2.0f};
		d2_RT->FillRectangle(rx,brb1);
		if (A.Selected)
			d2_RT->FillRectangle(rx,brb2);

		// Update Percentage below name

		// Erase Previous Point
		// x is from 0 to 100, y is from 0 to 1
		y *= (r.bottom - r.top); // Convert to available height
		y += r.top;

		x /= 100.0f; // Convert to [0,1]
		x *= (r.right - r.left); // Convert to available width
		x += r.left;

		D2D1_POINT_2F p2;
		p2.x = (FLOAT)x;
		p2.y = (FLOAT)y;
		if (A.PrevPoint.x == 0 && A.PrevPoint.y == 0)
			nop();
		else
			d2_RT->DrawLine(A.PrevPoint,p2,br,1);
		A.PrevPoint = p2;

		// Draw the text after anim rect
		TCHAR txt[1000] = {0};
		swprintf_s(txt,1000,L"%I64u/%I64u",A.DownloadedBytes,A.TotalBytes);
		GetD2TextSize(fo2d_n,txt);
		D2D1_RECT_F ry;
		ry.left = (FLOAT)A.AnimRect.right;
		ry.top = (FLOAT)A.AnimRect.top;
		ry.bottom = (FLOAT)A.AnimRect.bottom;
		ry.right = (FLOAT)rc.right;
		fo2d_n->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		fo2d_n->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
		d2_RT->DrawText(txt,wcslen(txt),fo2d_n,ry,brs);

		br->Release();
		brb2->Release();
		brb1->Release();
		}

	d2_RT->EndDraw();
	NextClearD2D = false;
	}


void PaintWindow()
	{
	HRESULT hr = S_OK;
	wchar_t txt[2000] = {0};

	if (!d2_f || !d2_RT || !fo2d_n || !fo2d_b)
		return;

	RECT rc;
	GetClientRect(MainWindow,&rc);

	// Begin Drawing
	d2_RT->BeginDraw();

	float GetLightConditionLux();
	float LLUX = GetLightConditionLux();

	unsigned int NewFGColor = FGColor;
	if (LLUX != -1)
		{
		NewFGColor &= 0xFFFFFF;
		NewFGColor |= 0x80000000;
		}
	if (LLUX > 10000)
		{
		// LLUX from 0 to 100000
		
		float LL = min(LLUX,90000)/90000.0f; // to convert [0,1]
		LL *= 0x80; // convert [0,0x80]
		unsigned int X = (int)LL;
		X <<= 24;
		NewFGColor += X;
		}


	// Clear Background with the BG Color
	d2_RT->Clear(GetD2Color(BGColor));


	// Get the solid brush of the FG
	ID2D1SolidColorBrush* br = GetD2SolidBrush(NewFGColor);
	if (!br)
		{
		// duh
		d2_RT->EndDraw();
		return;
		}


	// {20,10,right,50} Display "Downloads" text
	swprintf_s(txt,1000,L"Downloads (%u)",Downloads.size());
	POINT ts = GetD2TextSize(fo2d_b,txt);
	D2D1_RECT_F r;
	r.left = 10;
	r.top = LastRibbonHeight + 10.0f;
	r.bottom = r.top + ts.y + 2;
	r.right = (FLOAT)rc.right;
	fo2d_b->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	fo2d_b->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	d2_RT->DrawText(txt,wcslen(txt),fo2d_b,r,br);



	D2D1_POINT_2F l1,l2;
	l1.x = 10.0f;
	l1.y = r.top + ts.y + 4;
	l2.x = rc.right - 10.0f;
	l2.y = l1.y;
	d2_RT->DrawLine(l1,l2,br,1);



	float FromY = l2.y + 2.0f;

	// Footer light and our location from sensors
	swprintf_s(txt,1000,L"Light Lux: ");
	if (LLUX == -1)
		wcscat_s(txt,1000,L"Not Available");
	else
		swprintf_s(txt + wcslen(txt),1000,L" %.2f ",LLUX);


	if (Loc.x && Loc.y)
		swprintf_s(txt + wcslen(txt),1000,L" - Location: %f,%f,%f",Loc.x,Loc.y,Loc.z);
	else
		swprintf_s(txt + wcslen(txt),1000,L" - Location: Not Available.",Loc.x,Loc.y,Loc.z);


	ts = GetD2TextSize(fo2d_b,txt);
	r.left = 10;
	r.bottom = rc.bottom - 10.0f;
	r.top = r.bottom - ts.y - 2.0f;
	r.right = (FLOAT)rc.right;
	fo2d_b->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	fo2d_b->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	d2_RT->DrawText(txt,wcslen(txt),fo2d_b,r,br);

	// Line above it
	l1.x = 10.0f;
	l1.y = r.top - 4;
	l2.x = rc.right - 10.0f;
	l2.y = l1.y;
	d2_RT->DrawLine(l1,l2,br,1);
//	float ToY = l2.y - 2.0f;

	// All downloads from FromY to ToY
	// float AvailableHeight = ToY - FromY;
	unsigned int ds = Downloads.size();
	if (ds)
		{
		float HPerDownload = 50;

		for(unsigned int i = 0 ; i < ds ; i++)
			{
			A_DOWNLOAD& A = *Downloads[i];
			D2D1_RECT_F r;

			// Draw local file
			const wchar_t* rr = wcsrchr(A.Local.c_str(),'\\');
			if (!rr)
				rr = A.Local.c_str();
			else
				rr++;

/*
			// 16x16 box to indicate status, green downloading, blue finished, red error
			r.left = 10;
			r.top = FromY;
			r.bottom = r.top + 16;
			r.right = r.left + 16;
			ID2D1SolidColorBrush* b = GetD2SolidBrush(A.State== 0 ? AXRGB(0,128,128,0) : A.State == 1 ? AXRGB(0,0,128,0) : A.State == 2 ? AXRGB(0,0,0,128) : AXRGB(0,128,0,0));
			if (b)
				{
				d2_RT->FillRectangle(r,b);
				b->Release();
				}
*/
			// 16x16 bitmap to indicate status
			D2Image(10.0f,FromY,A.State == 2 ? OkImage : A.State == 3 ? ErrorImage : DownloadingImage,1.0f,true);


			swprintf_s(txt,L"%s",rr);

			ts = GetD2TextSize(fo2d_n,txt);
			r.left = 30;
			r.top = FromY;
			r.bottom = r.top + HPerDownload;
			r.right = (FLOAT)rc.right;
			fo2d_n->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			fo2d_n->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			d2_RT->DrawText(txt,wcslen(txt),fo2d_b,r,br);
			A.AnimRect.left = (long)r.left + ts.x + 200;
			A.AnimRect.right = (long)rc.right - 150;
			A.AnimRect.top = (long)r.top + 3;
			A.AnimRect.bottom = (long)r.bottom - 3;


			A.HitTest.left = 0;
			A.HitTest.right = rc.right;
			A.HitTest.top = (int)FromY;
			A.HitTest.bottom = (int)(A.HitTest.top + HPerDownload);

			if (A.Selected)
				{
				D2D1_RECT_F r2;
				r2.left = 10;
				r2.top = FromY;
				r2.bottom = r2.top + HPerDownload;
				r2.right = rc.right - 10.0f;

				ID2D1SolidColorBrush* b2 = GetD2SolidBrush(AXRGB(i % 2 ? 0x20 : 0x10,0x200,0x200,0x200));
				if (b2)
					{
					d2_RT->FillRectangle(r2,b2);
					b2->Release();
					b2 = 0;
					}
				}

			FromY += HPerDownload;
			}

		}

	

	// Brush bye
	br->Release();

	// End Rendering
	D2D1_TAG tg1 = 0,tg2 = 0;
	d2_RT->Flush(&tg1,&tg2);
	if (tg1 || tg2)
		{
		nop();
		// Errors occur
		}
	hr = d2_RT->EndDraw(&tg1,&tg2);
	if (FAILED(hr))
		{
		nop(); // errors
		}
	}

void ResizeWindow()
	{
	if (!d2_f || !d2_RT)
		return;

	RECT rc = {0};
	GetClientRect(MainWindow,&rc);
	D2D1_SIZE_U us;
	us.width = rc.right;
	us.height = rc.bottom;
	d2_RT->Resize(us);
	}
