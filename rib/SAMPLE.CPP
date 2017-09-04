#define STRICT
#define _WIN32_WINNT 0x601
#define _WIN32_IE 0x800

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <tchar.h>
#include <shlobj.h>
#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>

using namespace std;

// Globals
HINSTANCE hAppInstance = 0;
HWND MainWindow = 0;
HINSTANCE hRibInstance = 0;

// IUICommandHandler for Implementation
class MyUICommandHandler : public IUICommandHandler
	{
	private:
		class MyUIApplication* app;
		UINT32 cmd;
		UI_COMMANDTYPE ctype;
		int refNum;

	public:

		MyUICommandHandler(class MyUIApplication* uapp,UINT32 c,UI_COMMANDTYPE ty)
			{
			app = uapp;
			cmd = c;
			ctype = ty;
			}

		// IUnknown
		virtual ULONG __stdcall AddRef()
			{
			return ++refNum;
			}
		virtual ULONG __stdcall Release()
			{
			refNum--;
			return refNum;
			}
		virtual HRESULT __stdcall QueryInterface(REFIID iid,void ** ppvObject)
			{
			if (iid == IID_IUnknown || iid == __uuidof(IUICommandHandler))
				{
				*ppvObject = (void*)this;
				AddRef();
				return S_OK;
				}
			return E_NOINTERFACE;
			}

		// IUICommandHandler
		virtual HRESULT __stdcall UpdateProperty(UINT32 commandId,REFPROPERTYKEY key,const PROPVARIANT *currentValue,PROPVARIANT *newValue)
			{
			// By default, enable all contextual tabs
			if (key == UI_PKEY_ContextAvailable)
				{
				unsigned int gPR = (unsigned int)UI_CONTEXTAVAILABILITY_NOTAVAILABLE;
				gPR = (unsigned int)UI_CONTEXTAVAILABILITY_ACTIVE;
				UIInitPropertyFromUInt32(key, gPR, newValue);
				}
			return S_OK;
			}
		virtual HRESULT __stdcall Execute(UINT32 commandId,UI_EXECUTIONVERB verb,const PROPERTYKEY *key,const PROPVARIANT *currentValue,IUISimplePropertySet *commandExecutionProperties)
			{
			if (verb == UI_EXECUTIONVERB_EXECUTE)
				{
				TCHAR msg[1000];
				_stprintf_s(msg,1000,_T("Command Invoked: %u"),commandId);
				MessageBox(MainWindow,msg,_T("Command"),MB_OK);
				}
			return S_OK;
			}
	};

// IUIApplication Implementation for the Ribbon
class MyUIApplication : public IUIApplication
	{
	private:
		HWND hh; // Our Window
		int refNum;

	public:

		MyUIApplication(HWND hP)
			{
			hh = hP;
			}

		// IUnknown
		virtual ULONG __stdcall AddRef()
			{
			return ++refNum;
			}
		virtual ULONG __stdcall Release()
			{
			refNum--;
			if (!refNum)
				delete this;
			return refNum;
			}
		virtual HRESULT __stdcall QueryInterface(REFIID iid,void ** ppvObject)
			{
			if (iid == IID_IUnknown || iid == __uuidof(IUIApplication))
				{
				*ppvObject = (void*)this;
				AddRef();
				return S_OK;
				}
			return E_NOINTERFACE;
			}

		// IUIApplication callbacks
		// These have to implemented to handle notifications from a control
		virtual HRESULT __stdcall OnCreateUICommand(UINT32 commandId,UI_COMMANDTYPE typeID,IUICommandHandler **commandHandler)
			{
			// Create a command handler
			if (!commandHandler)
				return E_POINTER;
			MyUICommandHandler * C = new MyUICommandHandler(this,commandId,typeID);
			*commandHandler = (IUICommandHandler*)C;
			return S_OK;
			}
		virtual HRESULT __stdcall OnDestroyUICommand(UINT32 commandId,UI_COMMANDTYPE typeID,IUICommandHandler *commandHandler)
			{
			return S_OK;
			}
		virtual HRESULT __stdcall OnViewChanged(UINT32 viewId,UI_VIEWTYPE typeID,IUnknown *view,UI_VIEWVERB verb,INT32 uReasonCode)
			{
			return S_OK;
			}
	};


IUIFramework* u_f = 0; // Framework 
IUIRibbon* u_r = 0;   // Ribbon
MyUIApplication* u_app = 0; // IUIApplication


// WndProc
LRESULT CALLBACK Main_DP(HWND hh,UINT mm,WPARAM ww,LPARAM ll)
	{
	switch(mm)
		{
		case WM_CREATE:
			{
			// Create Ribbon
			HRESULT hr = 0;
			hr = CoCreateInstance(CLSID_UIRibbonFramework,0,CLSCTX_ALL,__uuidof(IUIFramework),(void**)&u_f);
			if (FAILED(hr))
				return 0; 

			// Initialize with the IUIApplication callback
			u_app = new MyUIApplication(hh);
			hr = u_f->Initialize(hh,u_app);
			if (FAILED(hr))
				{
				u_f->Release();
				u_f = 0;
				return 0;
				}

			// Load the Markup
			hr = u_f->LoadUI(hRibInstance,_T("APPLICATION_RIBBON"));
			if (FAILED(hr))
				{
				u_f->Release();
				u_f = 0;
				return 0;
				}

			break;
			}

		case WM_CLOSE:
			{
			// Release Interfaces
			if (u_r)
				u_r->Release();
			u_r = 0;
			if (u_f)
				u_f->Release();
			u_f = 0;

			DestroyWindow(hh);
			break;
			}

		case WM_DESTROY:
			{
			PostQuitMessage(0);
			break;
			}
		}
	
	return DefWindowProc(hh,mm,ww,ll);
	}



// WinMain
int __stdcall WinMain(HINSTANCE h,HINSTANCE,LPSTR tt,int)
	{
	// Initialize OLE
	OleInitialize(0);

	// Common Controls
	INITCOMMONCONTROLSEX icex = {0};
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES | ICC_WIN95_CLASSES;
	icex.dwSize = sizeof(icex);

	hAppInstance = h;

#ifdef _X64_
	hRibInstance = LoadLibrary(L"RIBDLL64.DLL");
#else
	hRibInstance = LoadLibrary(L"RIBDLL32.DLL");
#endif

	// Register Class
	WNDCLASSEX wClass = {0};
	wClass.cbSize = sizeof(wClass);

	wClass.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_PARENTDC;
	wClass.lpfnWndProc = (WNDPROC)Main_DP;
	wClass.hInstance = h;
	wClass.hIcon = LoadIcon(0,IDI_SHIELD);
	wClass.hCursor = LoadCursor(0, IDC_ARROW);
	wClass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wClass.lpszClassName = _T("CLASS");
	wClass.hIconSm = wClass.hIcon;
	RegisterClassEx(&wClass);


	// MainWindow
	MainWindow = CreateWindowEx(0,
		_T("CLASS"),
		_T("Ribbon Test"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,CW_USEDEFAULT,CW_USEDEFAULT,1024,500,
		0,0,h, 0);
	ShowWindow(MainWindow,SW_SHOW);


	// Message Loop
	MSG msg;
	while(GetMessage(&msg,0,0,0))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}
	return 0;
	}
