// Main
#include "stdafx.h"

// Ribbon code 
IUIFramework* u_f = 0; // Framework 
IUIRibbon* u_r = 0;   // Ribbon
class MyUIApplication* u_app = 0; // IUIApplication


// Simple UI property set, this is for recent items
class MySUPS : public IUISimplePropertySet
	{
	private:

		wstring fil;
		int ref;

	public:


		// IUnknown
		virtual ULONG __stdcall AddRef()
			{
			return ++ref;
			}

		virtual ULONG __stdcall Release()
			{
			return --ref;
			}
		virtual HRESULT __stdcall QueryInterface(const IID& id,void**p)
			{
			if (id == IID_IUnknown || id == __uuidof(IUISimplePropertySet))
				{
				*p = this;
				AddRef();
				return S_OK;
				}
			return E_NOINTERFACE;
			}

		MySUPS()
			{
			ref = 1;
			fil.clear();
			}
		virtual void SetV(const TCHAR* f)
			{
			fil = f;
			}
		virtual HRESULT STDMETHODCALLTYPE GetValue(REFPROPERTYKEY key,PROPVARIANT *value)
			{
			HRESULT hr = E_NOTIMPL;
			if (key == UI_PKEY_Label)
				{
				hr = UIInitPropertyFromString(UI_PKEY_Label, fil.c_str(), value);
				}
			else if (key == UI_PKEY_LabelDescription)
				{
				hr = UIInitPropertyFromString(UI_PKEY_LabelDescription, fil.c_str(), value);
				}
			return hr;
			}

	};

vector<MySUPS> LastProjectsRibbonArray;


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
			refNum = 1;
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
			if (refNum == 0)
				delete this;
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
			UNREFERENCED_PARAMETER(currentValue);
			// This is called when a property should be updated


			if (key == UI_PKEY_Enabled)
				{
				// Enable or Disable stuff
				bool CH = false;
				bool AnySelected();
				if (!AnySelected())
					{
					if (commandId == 10025 || commandId == 10026 || commandId == 10027)
						{
						UIInitPropertyFromBoolean(key,FALSE,newValue);
						CH = true;
						}
					}
				else
					{

					}

				if (!CH)
					UIInitPropertyFromBoolean(key,TRUE,newValue);
				}


			if (key == UI_PKEY_RecentItems)
				{
				HRESULT hry = S_OK;
				if (RecentURLs.empty())
					return S_OK;

				SAFEARRAY* sa = SafeArrayCreateVector(VT_UNKNOWN, 0, RecentURLs.size());
				if (!sa)
					return 0;

				LastProjectsRibbonArray.resize(RecentURLs.size());

				// Each one holds a IUISimplePropertySet
				for(long i = 0 ; i < (signed)RecentURLs.size() ; i++)
					{
					MySUPS& pu = LastProjectsRibbonArray[i];
					pu.SetV(W(RecentURLs[i].c_str()));
					IUnknown* pUnk = 0;
					hry = pu.QueryInterface(__uuidof(IUnknown), reinterpret_cast<void**>(&pUnk));
					hry = SafeArrayPutElement(sa,&i,pUnk);
					pUnk->Release();
					}

				// Pass it to the ribbon
				UIInitPropertyFromIUnknownArray(UI_PKEY_RecentItems, sa,newValue);
				SafeArrayDestroy(sa);

				return 0;
				}
			if (key == UI_PKEY_ContextAvailable) // Contextual Tabs update
				{
				unsigned int gPR = (unsigned int)UI_CONTEXTAVAILABILITY_NOTAVAILABLE;
				if (IsItemSelected)
					gPR = (unsigned int)UI_CONTEXTAVAILABILITY_ACTIVE;
				UIInitPropertyFromUInt32(key, gPR, newValue);
				}

			return S_OK;
			}
		virtual HRESULT __stdcall Execute(UINT32 commandId,UI_EXECUTIONVERB verb,const PROPERTYKEY *key,const PROPVARIANT *currentValue,IUISimplePropertySet *commandExecutionProperties)
			{
			UNREFERENCED_PARAMETER(commandExecutionProperties);
			UNREFERENCED_PARAMETER(key);
			if (verb == UI_EXECUTIONVERB_EXECUTE)
				{
				if (ctype == UI_COMMANDTYPE_ACTION)
					{
					// Button Pressed
					SendMessage(MainWindow,WM_COMMAND,commandId,0);
					}
				if (ctype == UI_COMMANDTYPE_RECENTITEMS)
					{
					if (!currentValue)
						return 0;

					// Recent Items
					SendMessage(MainWindow,WM_COMMAND,15000 + currentValue->intVal,0);
					}
				// This is called on a command
				if (ctype == UI_COMMANDTYPE_FONT)
					{
					// Set the font for writing 
					HRESULT hr = 0;
					const PROPVARIANT* pv = currentValue;
					if (!pv)
						return 0;
					IUnknown* u = (IUnknown*)pv->punkVal;
					if (!u)
						return 0;

					IPropertyStore* st = 0;
					u->QueryInterface(__uuidof(IPropertyStore),(void**)&st);
					if (!st)
						return 0;

					PROPVARIANT var;

					hr = st->GetValue(UI_PKEY_FontProperties_Family, &var);
					if (SUCCEEDED(hr) && (var.vt == VT_LPWSTR))
						{
						wcscpy_s(CurrentFont.Family,var.pwszVal);
						}

					hr = st->GetValue(UI_PKEY_FontProperties_Size,&var);
					if (SUCCEEDED(hr) && (var.vt == VT_I4 || var.vt == VT_DECIMAL))
						{
						CurrentFont.Size = var.uintVal;
						}
					if (CurrentFont.Size == 0)
						CurrentFont.Size = 12;

					// Save font to XML
					cfg->GetRootElement()->FindElementZ("cfg",true)->FindVariableZ("font",true)->SetBinaryValue((char*)&CurrentFont,sizeof(CurrentFont));

					// Create fonts again
					D2DCreateFonts();

					// Redraw
					Update();

					st->Release();
					}

				if (ctype == UI_COMMANDTYPE_COLORANCHOR)
					{
					if (commandId == 10017 || commandId == 10018)
						{
						// Get The FG/BG Color
						PROPVARIANT val;
						PropVariantInit(&val);
						HRESULT hr = u_f->GetUICommandProperty(commandId,UI_PKEY_Color,&val);
						if (SUCCEEDED(hr))
							{
							// Change the colors
							if (commandId == 10017)
								FGColor = val.uintVal;
							else
								BGColor = val.uintVal;

							// If Same colors, reset
							if (BGColor == FGColor)
								{
								FGColor = 0;
								BGColor = 0xFFFFFFFF;
								}

							// Save colors to XML
							cfg->GetRootElement()->FindElementZ("cfg",true)->FindVariableZ("bgc",true)->SetValueInt(BGColor);
							cfg->GetRootElement()->FindElementZ("cfg",true)->FindVariableZ("fgc",true)->SetValueInt(FGColor);

							// Redraw
							Update();
							}
						}
					}
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
			refNum = 1;
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
		// These have to be implemented to handle notifications from a control
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
			UNREFERENCED_PARAMETER(typeID);
			UNREFERENCED_PARAMETER(commandId);
			UNREFERENCED_PARAMETER(commandHandler);
			return S_OK;
			}
		virtual HRESULT __stdcall OnViewChanged(UINT32 viewId,UI_VIEWTYPE typeID,IUnknown *view,UI_VIEWVERB verb,INT32 uReasonCode)
			{
			UNREFERENCED_PARAMETER(uReasonCode);
			UNREFERENCED_PARAMETER(viewId);
			if (typeID == UI_VIEWTYPE_RIBBON && verb == UI_VIEWVERB_CREATE)
				{
				// Ribbon Created, get a pointer
				if (!u_r)
					{
					if (view)
						view->QueryInterface(__uuidof(IUIRibbon),(void**)&u_r);
					}
				}
			if (typeID == UI_VIEWTYPE_RIBBON &&  verb == UI_VIEWVERB_SIZE)
				{
				// Ribbon Resized, Update Window
				UINT32 cy = 0;
				if (u_r)
					{
					u_r->GetHeight(&cy);
					LastRibbonHeight = cy;
					Update();
					}
				return S_OK;
				}
			return S_OK;
			}
	};



bool RibbonInit(HWND hh)
	{
	// CoCreate the ribbon
	HRESULT hr = 0;
	hr = CoCreateInstance(CLSID_UIRibbonFramework,0,CLSCTX_ALL,__uuidof(IUIFramework),(void**)&u_f);
	if (FAILED(hr))
		return false; 

	// Initialize with the IUIApplication callback
	u_app = new MyUIApplication(hh);
	hr = u_f->Initialize(hh,u_app);
	if (FAILED(hr))
		{
		u_f->Release();
		u_f = 0;
		return false; 
		}

	// Get the IUIRibbon


	// Load the Markup
	hr = u_f->LoadUI(hAppInstance,_T("APPLICATION_RIBBON"));
	if (FAILED(hr))
		{
		u_f->Release();
		u_f = 0;
		return false; 
		}

	return true;
	}

void RibbonEnd()
	{
	// Release Interfaces
	if (u_r)
		u_r->Release();
	u_r = 0;
	if (u_f)
		u_f->Release();
	u_f = 0;
	if (u_app)
		u_app->Release();
	u_app = 0;
	}


void InvalidateRibbonCommand(int cmd)
	{
	if (!u_f)
		return;
	u_f->InvalidateUICommand(cmd,UI_INVALIDATIONS_STATE,0);
	}
