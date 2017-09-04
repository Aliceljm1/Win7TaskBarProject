// Main
#include "stdafx.h"


IUIAnimationManager* am = 0;
IUIAnimationTransitionLibrary* amtr = 0;
IUIAnimationStoryboard* amb = 0;
IUIAnimationTimer* amt = 0;
IUIAnimationTimerUpdateHandler* amth = 0;
IUIAnimationVariable* amv[2] = {0,0};


class MyAnimationManagerEventHandler : public IUIAnimationManagerEventHandler
	{
	private:
		unsigned long ref;

	public:

		MyAnimationManagerEventHandler()
			{
			ref = 1;
			}

		// IUnknown
		ULONG __stdcall AddRef()
			{
			return ++ref;
			}

		ULONG __stdcall Release()
			{
			return --ref;
			}
		HRESULT __stdcall QueryInterface(const IID& id,void**p)
			{
			if (id == IID_IUnknown || id == __uuidof(IUIAnimationManagerEventHandler))
				{
				*p = this;
				AddRef();
				return S_OK;
				}
			return E_NOINTERFACE;
			}

	    // IUIAnimationManagerEventHandler
		HRESULT __stdcall OnManagerStatusChanged(
			UI_ANIMATION_MANAGER_STATUS newStatus,
			UI_ANIMATION_MANAGER_STATUS previousStatus
			)
			{
			UNREFERENCED_PARAMETER(previousStatus);
			if (newStatus == UI_ANIMATION_MANAGER_IDLE)
				{
				// Schedule the animation again
				PostMessage(MainWindow,WM_USER + 1,0,0);
				}
			return S_OK;
			}

};


class MyAnimationTimeEventHandler : public IUIAnimationTimerEventHandler
	{
	private:
		unsigned long ref;

	public:

		MyAnimationTimeEventHandler()
			{
			ref = 1;
			}

		// IUnknown
		ULONG __stdcall AddRef()
			{
			return ++ref;
			}

		ULONG __stdcall Release()
			{
			return --ref;
			}
		HRESULT __stdcall QueryInterface(const IID& id,void**p)
			{
			if (id == IID_IUnknown || id == __uuidof(IUIAnimationTimerEventHandler))
				{
				*p = this;
				AddRef();
				return S_OK;
				}
			return E_NOINTERFACE;
			}

	    // IUIAnimationTimerEventHandler
		HRESULT __stdcall OnPostUpdate()
			{
			return S_OK;
			}
		HRESULT __stdcall OnPreUpdate()
			{
			// Request the y variable value
			DOUBLE v1 = 0,v0 = 0;
			if (amv[0])
				amv[0]->GetValue(&v0);
			if (amv[1])
				amv[1]->GetValue(&v1);

			void D2DrawAnimation(double,double);
			D2DrawAnimation(v0,v1);
			return S_OK;
			}
		HRESULT __stdcall OnRenderingTooSlow(UINT32 framesPerSecond)
			{
			UNREFERENCED_PARAMETER(framesPerSecond);
			return E_NOTIMPL;
			}

};

MyAnimationTimeEventHandler matH;
MyAnimationManagerEventHandler maEH;


void StartAnim()
	{
	extern bool NextClearD2D;
	NextClearD2D = true;

	HRESULT hr = S_OK;
	if (amb)
		amb->Release();
	amb = 0;

	if (amv[1])
		amv[1]->Release();
	amv[1] = 0;
	if (amv[0])
		amv[0]->Release();
	amv[0] = 0;



	// The Variables to animate (x,y of the point)
	am->CreateAnimationVariable(0.0f,&amv[0]);
	if (!amv[0])
		{
		EndAnim();
		return;
		}
	am->CreateAnimationVariable(0.0f,&amv[1]);
	if (!amv[1])
		{
		EndAnim();
		return;
		}



	// Create the story board
	hr = am->CreateStoryboard(&amb);
	if (!amb)
		{
		EndAnim();
		return;
		}

	// Create a simple sin transition
	IUIAnimationTransition* trs = 0;
	amtr->CreateSinusoidalTransitionFromRange(5,0.0f,1.0f,0.5f,UI_ANIMATION_SLOPE_INCREASING,&trs);
	if (trs)
		{
		hr = amb->AddTransition(amv[1],trs);
		trs->Release();
		trs = 0;
		}
	amtr->CreateLinearTransition(5,100,&trs);
	if (trs)
		{
		hr = amb->AddTransition(amv[0],trs);
		trs->Release();
		trs = 0;
		}


	UI_ANIMATION_SECONDS se = 0;
	if (SUCCEEDED(amt->GetTime(&se)))
		hr = amb->Schedule(se);
	hr = amt->Enable();
	nop();
	}

bool InitAnim(HWND hh)
	{
	UNREFERENCED_PARAMETER(hh);
	// Manager, Timer and Translation
	HRESULT hr = 0;
	hr = CoCreateInstance(CLSID_UIAnimationManager,0,CLSCTX_INPROC_SERVER,__uuidof(IUIAnimationManager),(void**)&am);
	if (!am)
		return false;
	hr = am->SetManagerEventHandler(&maEH);

	hr = CoCreateInstance(CLSID_UIAnimationTimer,0,CLSCTX_INPROC_SERVER,__uuidof(IUIAnimationTimer),(void**)&amt);
	if (!amt)
		{
		EndAnim();
		return false;
		}

	hr = CoCreateInstance(CLSID_UIAnimationTransitionLibrary,0,CLSCTX_INPROC_SERVER,__uuidof(IUIAnimationTransitionLibrary),(void**)&amtr);
	if (!amtr)
		{
		EndAnim();
		return false;
		}

	// Animation Timer
	hr = am->QueryInterface(__uuidof(IUIAnimationTimerUpdateHandler),(void**)&amth);
	if (!amth)
		{
		EndAnim();
		return 0;
		}

	hr = amt->SetTimerUpdateHandler(amth,UI_ANIMATION_IDLE_BEHAVIOR_DISABLE);
	amth->Release();
	amth = 0;

	// Set the timer event handler
	hr = amt->SetTimerEventHandler(&matH);


	StartAnim();
	return true;
	}




void EndAnim()
	{
	if (amv[1])
		amv[1]->Release();
	amv[1] = 0;
	if (amv[0])
		amv[0]->Release();
	amv[0] = 0;
	if (amtr)
		amtr->Release();
	amtr = 0;
	if (amb)
		amb->Release();
	amb = 0;
	if (amt)
		amt->Release();
	amt = 0;
	if (am)
		am->Release();
	am = 0;
	}



