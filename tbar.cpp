// Main
#include "stdafx.h"

ITaskbarList3* tb = 0;

bool TBInit(HWND hh)
	{
	UNREFERENCED_PARAMETER(hh);
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList,0,CLSCTX_INPROC_SERVER,__uuidof(ITaskbarList3),(void**)&tb);
	if (!tb)
		return false; // ITaskBarList3 not available


	// Put the MRU
	SHAddToRecentDocs(SHARD_PATHW,0);
	for(unsigned int i = 0 ; i < RecentURLs.size() ; i++)
		{
		SHAddToRecentDocs(SHARD_PATHW,(void*)RecentURLs[i].c_str());
		}

	// Use the Destination List
	ICustomDestinationList* cdl = 0;
	hr = CoCreateInstance(CLSID_DestinationList,NULL,CLSCTX_INPROC_SERVER,__uuidof(ICustomDestinationList),(void**)&cdl);
	if (!cdl)
		{
		tb->Release();
		tb = 0;
		return false;
		}

	// Load AppID
	hr = cdl->SetAppID(OurAppID);
	if (FAILED(hr))
		{
		cdl->Release();
		cdl = 0;
		tb->Release();
		tb = 0;
		return false;
		}

	// Recent Items
	UINT MaxCount = 0;
	IObjectArray* oa = 0;
	hr = cdl->BeginList(&MaxCount,__uuidof(IObjectArray),(void**)&oa);
	if (FAILED(hr))
		{
		cdl->Release();
		cdl = 0;
		tb->Release();
		tb = 0;
		return false;
		}
	hr = cdl->AppendKnownCategory(KDC_RECENT);
	oa->Release();
	oa = 0;
	cdl->Release();
	cdl = 0;


	// Load the toolbar
	HBITMAP LoadTransparentToolbarImage(HINSTANCE hI,const TCHAR* Name,unsigned long Color);
	HBITMAP hB = LoadTransparentToolbarImage(hAppInstance,_T("WM7_TOOLBAR"),0xFFFFFFFF);
	if (!hB)
		{
		tb->Release();
		tb = 0;
		return false;
		}

	// Check dimensions
	BITMAP bi = {0};
	GetObject((HANDLE)hB,sizeof(bi),&bi);
	if (bi.bmHeight == 0)
		{
		DeleteObject(hB);
		tb->Release();
		tb = 0;
		return false;
		}
	int nI = bi.bmWidth/bi.bmHeight;
	HIMAGELIST tlbi = ImageList_Create(bi.bmHeight,bi.bmHeight,ILC_COLOR32,nI,0);

	// Add the bitmap
	ImageList_Add(tlbi,hB,0);
	hr = tb->ThumbBarSetImageList(MainWindow,tlbi);
	DeleteObject(hB);
	if (FAILED(hr))
		{
		tb->Release();
		tb = 0;
		return false;
		}

	// Add 2 buttons:
	THUMBBUTTON tup[2];
	int ids[] = {701,702};
	wchar_t* txts[] = {L"Stop all",L"Start all"};
	for(int i = 0 ; i < 2 ; i++)
		{
		THUMBBUTTON& tu = tup[i];
		tu.dwMask = THB_FLAGS | THB_BITMAP | THB_TOOLTIP;
		tu.iBitmap = i;
		tu.iId = ids[i];
		_tcscpy(tu.szTip,txts[i]);
		tu.dwFlags = THBF_ENABLED | THBF_DISMISSONCLICK;
		}
	hr = tb->ThumbBarAddButtons(MainWindow,2,tup);
	if (FAILED(hr))
		{
		tb->Release();
		tb = 0;
		return false;
		}

	tb->SetProgressState(MainWindow,TBPF_NORMAL);
	return true;
	}

void TBEnd()
	{
	if (tb)
		tb->Release();
	tb = 0;
	}

void UpdateTaskBar()
	{
	if (!tb)
		return;

	unsigned long long ts = 0;
	unsigned long long tr = 0;
	for(unsigned int i = 0 ; i < Downloads.size() ; i++)
		{
		A_DOWNLOAD& A = *Downloads[i];

		ts += A.TotalBytes;
		tr += A.DownloadedBytes;
		}

	tb->SetProgressValue(MainWindow,tr,ts);
	}

