// Main
#include "stdafx.h"
#include "func.h"

// Globals
HWND MainWindow;
HICON hIcon1;
HINSTANCE hAppInstance = 0;
const wchar_t* ttitle = _T("Windows 7 Downloader");
bool IsItemSelected = false;
unsigned int FGColor = 0,BGColor = 0xFFFFFFFF;
XML* cfg = 0;
unsigned int LastRibbonHeight = 0;
A_FONT CurrentFont = {0};
vector<A_DOWNLOAD*> Downloads;
vector<wstring> RecentURLs;
const wchar_t* OurAppID = L"MichaelChourdakis.W7DL";
HANDLE PauseAll = CreateEvent(0,TRUE,TRUE,0);
POINTF3 Loc = {0};

// pragmas
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"sensorsapi.lib")
#pragma comment(lib,"locationapi.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"gdiplus.lib")

#ifdef _X64_
#pragma comment(lib,"xml64.lib")
#else
#pragma comment(lib,"xmlms.lib")
#endif

// Initialization of the Window
bool InitializeAll(HWND hh)
	{
	// XML Configuration
	wstring SaveF;
	if (!GetSaveFolder(SaveF))
		return 0;

	wchar_t d[10000] = {0};
	char cd[10000] = {0};
	swprintf_s(d,L"%s\\W7DL.XML",SaveF.c_str());
	cfg = new XML(d);

	// Load Configuration

	// Colors
	FGColor = cfg->GetRootElement()->FindElementZ("cfg",true)->FindVariableZ("fgc",true)->GetValueInt();
	BGColor = cfg->GetRootElement()->FindElementZ("cfg",true)->FindVariableZ("bgc",true)->GetValueInt();
	if (BGColor == FGColor)
		{
		FGColor = 0;
		BGColor = 0xFFFFFFFF;
		}

	// Fonts
	memset(d,0,sizeof(d));
	cfg->GetRootElement()->FindElementZ("cfg",true)->FindVariableZ("font",true)->GetBinaryValue((char*)d);
	memcpy(&CurrentFont,d,sizeof(CurrentFont));
	if (!wcslen(CurrentFont.Family) || !CurrentFont.Size)
		{
		// Set default tahoma
		wcscpy_s(CurrentFont.Family,L"Tahoma");
		CurrentFont.Size = 14;
		}

	// Recent URLs
	RecentURLs.clear();
	for(unsigned int i = 0 ; i < cfg->GetRootElement()->FindElementZ("cfg",true)->FindElementZ("recent",true)->GetChildrenNum() ; i++)
		{
		XMLElement* r = cfg->GetRootElement()->FindElementZ("cfg",true)->FindElementZ("recent",true)->GetChildren()[i];
		r->FindVariableZ("n",true)->GetValue(cd);
		wcscpy_s(d,W(cd));
		RecentURLs.push_back(wstring(d));
		}



	// Ribbon
	if (!RibbonInit(hh))
		return false;

	// D2D 
	if (!D2DInit(hh))
		return false;

	// Sensors
	if (!SensorsInit(hh))
		return false;

	// Anim
	if (!InitAnim(hh))
		return false;

	// Location?
	POINTF3 GetLocation();
	Loc = GetLocation();

	// Touch ?
	RegisterTouchWindow(hh,0);

	return true;
	}

void EndAll(HWND hh)
	{
	// Anim End
	EndAnim();

	// Touch End
	UnregisterTouchWindow(hh);

	// TaskBar End
	TBEnd();

	// Sensors End
	SensorsEnd();

	// D2D End
	D2DEnd();

	// Ribbon End
	RibbonEnd();

	// XML Save and End
	if (cfg)
		{
		cfg->Save();
		delete cfg;
		}
	cfg = 0;
	}

// Window Proc
DWORD g_wmTBC = (DWORD)-1;
LRESULT CALLBACK Main_DP(HWND hh,UINT mm,WPARAM ww,LPARAM ll)
	{
	if (mm == g_wmTBC)
		{
		// Taskbar Created
		TBInit(hh);
		return 0;
		}

	switch(mm)
		{
		case WM_USER + 1: 
			{
			// Start Animation Again
			void StartAnim();
			StartAnim();
			return 0;
			}

		case WM_TOUCH:
			{
			return TouchHandler(hh,mm,ww,ll);
			}

		case WM_CREATE:
			{
			MainWindow = hh;
			if (!InitializeAll(hh))
				{
				MessageBox(hh,_T("A Windows 7 Component failed to be initialized."),ttitle,MB_ICONWARNING);
				EndAll(hh);
				DestroyWindow(hh);
				}
			return 0;
			}

		case WM_CLOSE:
			{
			for(signed int i = Downloads.size() - 1 ; i >= 0 ; i--)
				{
				if (Downloads[i]->State == 1)
					{
					MessageBox(hh,L"There are active downloads. Cancel them first",ttitle,MB_ICONWARNING);
					return 0;
					}
				}

			// End this one
			EndAll(hh);

			// Delete download vector
			for(signed int i = Downloads.size() - 1 ; i >= 0 ; i--)
				{
				A_DOWNLOAD* A = Downloads[i];
				Downloads.erase(Downloads.begin() + i);
				delete A;
				}

			DestroyWindow(hh);
			return 0;
			}

		case WM_DESTROY:
			{
			// Request loop off
			PostQuitMessage(0);
			return 0;
			}

		case WM_COMMAND:
			{
			int LW = LOWORD(ww);

			if (LW == 10010)
				{
				MessageBox(hh,L"No options yet!",ttitle,MB_ICONINFORMATION);
				return 0;
				}

			if (LW == 10012)
				{
				MessageBox(hh,L"W7DL - Copyright (C) Chourdakis Michael.",ttitle,MB_ICONINFORMATION);
				return 0;
				}

			if (LW == 10013)
				{
				MessageBox(hh,L"Help file not yet available :)",ttitle,MB_ICONINFORMATION);
				return 0;
				}

			if (LW == 10022)
				{
				if (Downloads.empty())
					{
					MessageBox(hh,L"No downloads yet. Start a download first!",ttitle,MB_ICONINFORMATION);
					return 0;
					}
				// Google maps of our location and downloads
				// Sample: http://www.turboirc.com/php/w7dl.php?w=900&h=600&x=23&y=37&p=25_59_Loc1,27_40_Loc2
				wchar_t url[10000] = {0};
				swprintf_s(url,10000,L"http://www.turboirc.com/php/w7dl.php?w=1000&h=800&x=%f&y=%f",Loc.x,Loc.y);

				swprintf_s(url + wcslen(url),5000,L"&p=");

				for(unsigned int i = 0 ; i < Downloads.size() ; i++)
					{
					A_DOWNLOAD& A = *Downloads[i];
					swprintf_s(url + wcslen(url),5000,L"%f_%f_%s,",A.lx,A.ly,A.Remote.c_str());
					}
				// Remove last comma
				url[wcslen(url) - 1] = 0;
				
				ShellExecute(0,L"open",url,0,0,0);
				return 0;
				}


			if (LW >= 15000 && LW <= 15020)
				{
				// Recent items add download
				unsigned int ID = LW - 15000;
				if (ID >= RecentURLs.size())
					return 0;

				LoadDownload(RecentURLs[ID].c_str());
				return 0;
				}

			if (LW == 10025 || LW == 10026 || LW == 10027) // Stop/Start selected
				{
				vector<unsigned int> rm;
				bool OnceAsk = false;				
				for(unsigned int i = 0 ; i < Downloads.size() ; i++)
					{
					A_DOWNLOAD& A = *Downloads[i];

					if (!A.Selected)
						continue;

					if (LW == 10025)
						ResetEvent(A.PauseThis);
					else
					if (LW == 10026)
						SetEvent(A.PauseThis);
					else
					if (LW == 10027)
						{
						if (A.State == 1)
							{
							if (OnceAsk == false && MessageBox(hh,L"Are you sure?",ttitle,MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDNO)
								return 0;
							OnceAsk = true;
							}
						A.CancelThis = true;
						// Remove it from vector
						rm.push_back(i);
						}
					}

				for(signed int j = rm.size() - 1 ; j >= 0 ; j--)
					{
					Downloads.erase(Downloads.begin() + rm[j]);
					}
				Update();
				return 0;
				}

			if (LW == 701)
				{
				// Stop All Downloads (TaskBar button)
				ResetEvent(PauseAll);
				return 0;
				}

			if (LW == 702)
				{
				// Start All Idle Downloads (TaskBar) button
				SetEvent(PauseAll);
				return 0;
				}

			if (LW == 101)
				{
				AddDownload(0,0);
				return 0;
				}
			if (LW == 102)
				{
				AddDownloads();
				return 0;
				}

			return 0;
			}


		case WM_SIZE:
			{
			// On resize we must notify Direct2D
			ResizeWindow();
			InvalidateRect(MainWindow,0,0);
			UpdateWindow(MainWindow);
			return 0;
			}

		case WM_PAINT:
			{
			// Paint it with Direct2D
			PaintWindow();
			ValidateRect(hh,0);
			return 0;
			}

		case WM_LBUTTONDBLCLK:
			{
			int XX = LOWORD(ll);
			int YY = HIWORD(ll);
			// Open a finished download

			// Select clicked
			for(unsigned int i = 0 ; i < Downloads.size() ; i++)
				{
				if (InRect(XX,YY,Downloads[i]->HitTest))
					{
					if (Downloads[i]->State == 2)
						{
						ShellExecute(hh,L"open",Downloads[i]->Local.c_str(),0,0,SW_SHOWNORMAL);
						}
					}
				}
			return 0;
			}


		case WM_LBUTTONDOWN:
			{
			int XX = LOWORD(ll);
			int YY = HIWORD(ll);
			// Select or unselect a download
			BOOL Shift = GetKeyState(VK_SHIFT) >> 16;
			// All off if not shift
			if (!Shift)
				{
				for(unsigned int i = 0 ; i < Downloads.size() ; i++)
					{
					if (!InRect(XX,YY,Downloads[i]->HitTest))
						Downloads[i]->Selected = false;
					}
				}
			
			// Select clicked
			for(unsigned int i = 0 ; i < Downloads.size() ; i++)
				{
				if (InRect(XX,YY,Downloads[i]->HitTest))
					Downloads[i]->Selected = !Downloads[i]->Selected;
				}

			// Invalidate Ribbon Commands to enable them 
			void InvalidateRibbonCommand(int cmd);
			InvalidateRibbonCommand(10025);
			InvalidateRibbonCommand(10026);
			InvalidateRibbonCommand(10027);


			// Redraw
			Update();
			return 0;
			}

		}

	return DefWindowProc(hh,mm,ww,ll);
	}


// WinMain
int __stdcall WinMain(HINSTANCE h,HINSTANCE,LPSTR tt,int)
	{
	HRESULT hr = S_OK;
	// OLE Init
	OleInitialize(0);


	// Set this process Explicit AppUserModeID
	// CompanyName.ProductName.SubProduct.VersionInformation
	hr = SetCurrentProcessExplicitAppUserModelID(OurAppID);


	// WS Init
	WSADATA wData = {0};
	WSAStartup(MAKEWORD(2,2),&wData);

	// CC init
	INITCOMMONCONTROLSEX icex = {0};
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES | ICC_WIN95_CLASSES;
	icex.dwSize = sizeof(icex);
	InitCommonControlsEx(&icex);

	// GDI+ Init (to load png resources)
	using namespace Gdiplus;
	ULONG_PTR tk = 0;
	GdiplusStartupInput gsi;
	GdiplusStartup(&tk,&gsi,0);


	// Running under Windows 7 ?
	OSVERSIONINFO oex = {0};
	oex.dwOSVersionInfoSize = sizeof(oex);
	GetVersionEx(&oex);
	if (oex.dwMajorVersion < 6 || (oex.dwMajorVersion == 6 && oex.dwMinorVersion < 1))
		{
		MessageBox(0,_T("Windows 7 Downloader requires Windows 7!"),ttitle,MB_OK | MB_ICONERROR);
		return 0;
		}

	// Register Extension W7DL (Under HKCU, doesn't need admin)
	RegisterExtension(L"W7DL");


	// Init variables
	hIcon1 = LoadIcon(h,_T("ICON_1"));
	hAppInstance = h;

	// Window Class
	WNDCLASSEX wClass = {0};
	wClass.cbSize = sizeof(wClass);
	wClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_PARENTDC;
	wClass.lpfnWndProc = (WNDPROC)Main_DP;
	wClass.hInstance = h;
	wClass.hIcon = hIcon1;
	wClass.hCursor = LoadCursor(0, IDC_HAND);
	wClass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wClass.lpszClassName = _T("CLASS");
	wClass.hIconSm = hIcon1;
	RegisterClassEx(&wClass);

	// Window
	HACCEL acc = LoadAccelerators(hAppInstance,_T("MENU_1"));
	CreateWindowEx(0,
		_T("CLASS"),
		ttitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
		0, CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		0,0, h, 0);

	g_wmTBC = RegisterWindowMessage(L"TaskbarButtonCreated");
	ShowWindow(MainWindow,SW_SHOWNORMAL);


	// Param ?
	if (tt)
		LoadDownload(W(tt).operator wchar_t*());


	// Message Loop
	MSG msg;
	while(GetMessage(&msg,0,0,0) > 0)
		{
		if (!TranslateAccelerator(MainWindow,acc,&msg))
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}

	GdiplusShutdown(tk);
	return 0;
	}