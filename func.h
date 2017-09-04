//

void nop()
	{
	}


// Extract resources to file or memory
INT_PTR ExtractDefaultFile(TCHAR* Put,const TCHAR* Name,bool Mem,HINSTANCE hXX,const TCHAR* ty)
	{
	HRSRC R = FindResource(hXX,Name,ty);
	if (!R)
		return 0;
	HGLOBAL hG = LoadResource(hXX,R);
	if (!hG)
		return 0;
	int S = SizeofResource(hXX,R);
	char* p = (char*)LockResource(hG);
	if (!p)
		{
		FreeResource(R);
		return 0;
		}
	if (Mem)
		{
		char* d = new char[S + 10];
		memset(d,0,S + 10);
		memcpy(d,p,S);
		FreeResource(R);
		return (INT_PTR)d;
		}

	bool E = false;
	FILE* fp = _tfopen(Put,_T("wb"));
	if (fp)
		{
		fwrite(p,1,S,fp);
		fclose(fp);
		E = true;
		}
	FreeResource(R);
	return E;
	}


HBITMAP LoadTransparentToolbarImage(HINSTANCE hI,const TCHAR* Name,unsigned long Color)
	{
	UNREFERENCED_PARAMETER(hI);
	// Load the PNG
	TCHAR tp[1000] = {0};
	TCHAR t[1000] = {0};
	GetTempPath(1000,tp);
	GetTempFileName(tp,L"tmp",0,t);
	DeleteFile(t);
	if (!ExtractDefaultFile(t,(TCHAR*)Name,false,hAppInstance,_T("PNG")))
		return 0;

	// Load the PNG Image
	using namespace Gdiplus;
	Gdiplus::Bitmap* bi = new Gdiplus::Bitmap(t);
	if (bi->GetLastStatus() != Ok)
		{
		delete bi;
		return 0;
		}

	unsigned int A = GetAValue(Color);
	unsigned int R = GetRValue(Color);
	unsigned int G = GetGValue(Color);
	unsigned int B = GetBValue(Color);
	Gdiplus::Color co((BYTE)A,(BYTE)R,(BYTE)G,(BYTE)B);
	HBITMAP hb = 0;

	bi->GetHBITMAP(co,&hb);
	return hb;
	}

// W class
W::W(const char *a)
{
   MS = 0;
   WS = 0;
   OS = 0;
   strcpy(mempty, "");
   lstrcpyW(wempty, L"");
   if (!a)
      return;
   if (strcmp(a, "") == 0)
      return;
   s = strlen(a);
   MS = new char[s + 10];
   strcpy(MS, a);
   WS = new wchar_t[(s + 10) * 2];
   MultiByteToWideChar(CP_UTF8, 0, MS, -1, WS, (s + 10) * 2);
   OS = SysAllocString(WS);
}

W::W(const wchar_t * a)
{
   MS = 0;
   WS = 0;
   OS = 0;
   if (!a)
      return;
   if (lstrcmpW(a, L"") == 0)
      return;
   s = lstrlenW(a);
   WS = new wchar_t[s + 10];
   lstrcpyW(WS, a);
   MS = new char[(s + 10) * 2];
   WideCharToMultiByte(CP_UTF8, 0, WS, -1, MS, (s + 10) * 2, 0, 0);
   OS = SysAllocString(WS);
}

W::~W()
{
   if (MS)
      delete[]MS;
   if (WS)
      delete[]WS;
   if (OS)
      SysFreeString(OS);
}
const char *W::m()
{
   return (const char *)MS;
}
const wchar_t *W::w()
{
   return (const wchar_t *)WS;
}
const TCHAR *W::t()
{
#ifdef _UNICODE
   return (const wchar_t *)WS;
#else
   return (const char *)MS;
#endif
}
const char *W::me()
{
   if (!MS)
      return (const char *)mempty;
   return (const char *)MS;
}
const wchar_t *W::we()
{
   if (!WS)
      return (const wchar_t *)wempty;
   return (const wchar_t *)WS;
}
const BSTR W::o()
{
   return (const BSTR)OS;
}
W::operator  const char *()
{
   return me();
}
W::operator  const wchar_t *()
{
   return we();
}
W::operator  char *()
{
   return (char *)me();
}

W::operator  wchar_t * ()
{
   return (wchar_t *) we();
}


bool GetSaveFolder(wstring& x)
	{
	x.clear();
	PWSTR FolderPath = 0;
	SHGetKnownFolderPath(FOLDERID_Documents,0,0,&FolderPath);
	if (!FolderPath)
		return false;

	wchar_t dst[10000] = {0};
	swprintf_s(dst,10000,L"%s\\W7DL",FolderPath);
	CreateDirectory(dst,0);

	x = dst;
	CoTaskMemFree(FolderPath);
	return true;
	}

void RegisterExtension(const TCHAR* ext)
	{
	Z<TCHAR> t1(1000);
	Z<TCHAR> t2(1000);

	// 1. .ext in HK_R
	_stprintf(t1,_T("Software\\Classes\\.%s"),ext);
	HKEY pK1 = 0;
	RegCreateKeyEx(HKEY_CURRENT_USER,t1,0,0,0,KEY_ALL_ACCESS | KEY_WOW64_64KEY,0,&pK1,0);
	if (!pK1)
		return;

	_stprintf(t1,_T("W7DLFile"));
	RegSetValue(pK1,0,REG_SZ,(LPWSTR)t1.operator TCHAR*(),_tcslen(t1)*2);
	RegCloseKey(pK1);

	// 2. R\W7DLFile\shell\open\command
	_stprintf(t1,_T("Software\\Classes\\W7DLFile\\shell\\open\\command"),ext);
	pK1 = 0;
	RegCreateKeyEx(HKEY_CURRENT_USER,t1,0,0,0,KEY_ALL_ACCESS | KEY_WOW64_64KEY,0,&pK1,0);
	if (!pK1)
		return;

	// cmd "%1"
	t1._clear();
	t1[0] = '\"';
	GetModuleFileName(hAppInstance,t1 + 1,1000);
	_stprintf(t1 + _tcslen(t1),L"\" \"%%1\"");
	RegSetValue(pK1,0,REG_SZ,(LPWSTR)t1.operator TCHAR*(),_tcslen(t1)*2);
	RegCloseKey(pK1);

	// 3. R\W7DLFile\DefaultIcon
	pK1 = 0;
	_stprintf(t1,_T("Software\\Classes\\W7DLFile\\DefautIcon"),ext);
	RegCreateKeyEx(HKEY_CURRENT_USER,t1,0,0,0,KEY_ALL_ACCESS | KEY_WOW64_64KEY,0,&pK1,0);
	if (!pK1)
		return;
	t1._clear();
	t1[0] = '\"';
	GetModuleFileName(hAppInstance,t1 + 1,1000);
	_stprintf(t1 + _tcslen(t1),L"\",0");
	RegSetValue(pK1,0,REG_SZ,(LPWSTR)t1.operator TCHAR*(),_tcslen(t1)*2);
	RegCloseKey(pK1);
	}



INT_PTR CALLBACK SINGLEADD(HWND hh,UINT mm,WPARAM ww,LPARAM ll)
	{
	static A_DOWNLOAD* A = 0;
	switch(mm)
		{
		case WM_INITDIALOG:
			{
			A = (A_DOWNLOAD*)ll;
			if (!A)
				{
				EndDialog(hh,0);
				return 0;
				}

			SetDlgItemText(hh,101,A->Remote.c_str());
			SetDlgItemText(hh,102,A->Local.c_str());
			SendMessage(hh,WM_COMMAND,MAKELPARAM(101,EN_UPDATE),(LPARAM)GetDlgItem(hh,101));
			CheckDlgButton(hh,301,BST_CHECKED);
		
			return true;
			}
		case WM_COMMAND:
			{
			int HW = HIWORD(ww);
			int LW = LOWORD(ww);

			if (LW == 201)
				{
				// Select target
				wchar_t txt[10000] = {0};
				GetDlgItemText(hh,102,txt,10000);
				wstring fn = txt;
				void SelectFileLocation(HWND hh,wstring& fn);
				SelectFileLocation(hh,fn);
				SetDlgItemText(hh,102,fn.c_str());
				return 0;
				}

			if (LW == IDCANCEL)
				{
				SendMessage(hh,WM_CLOSE,0,0);
				return 0;
				}

			if (LW == IDOK)
				{
				wchar_t txt[10000] = {0};
				GetDlgItemText(hh,101,txt,10000);
				if (!wcslen(txt))
					return 0;
				A->Remote = txt;
				GetDlgItemText(hh,102,txt,10000);
				if (!wcslen(txt))
					return 0;
				A->Local = txt;
				if (IsDlgButtonChecked(hh,301) == BST_CHECKED)
					SavePathInLibrary(txt);
				EndDialog(hh,1);
				return 0;
				}

			if (HW == EN_UPDATE && (HWND)ll == GetDlgItem(hh,101))
				{
				wstring SaveF;
				if (!GetSaveFolder(SaveF))
					return 0;

				// URL Update
				// Say ftp://examples:examples@test.com/file.zip
				wchar_t url[10000] = {0};
				GetDlgItemText(hh,101,url,1000);
				wchar_t* a1 = wcsrchr(url,'/');
				wchar_t* a2 = wcsrchr(url,'\\');
				wchar_t* a = 0;
				if (!a1)
					a = a2;
				if (!a2)
					a = a1;
				if (a1 && a2)
					{
					if (a1 > a2)
						a = a1;
					else
						a = a2;
					}

				wchar_t dst[10000] = {0};


				if (a)
					swprintf_s(dst,10000,L"%s\\%s",SaveF.c_str(),a + 1);
				else
					swprintf_s(dst,10000,L"%s\\",SaveF.c_str());
				SetDlgItemText(hh,102,dst);
				return 0;
				}
			
			return 0;
			}
		case WM_CLOSE:
			{
			EndDialog(hh,0);
			return 0;
			}
		}
	return 0;
	}

// Quick Query Fetch Wininet
std::string Fetch(TCHAR* TheLink)
	{
	// Create thread that will show download progress
	string R;
	R.clear();

	DWORD Size;
	unsigned long bfs = 1000;
	TCHAR ss[1000];
	DWORD TotalTransferred = 0;

	int err = 1;

	HINTERNET hI = 0,hRead = 0;

	hI = InternetOpen(_T("W7DL"),INTERNET_OPEN_TYPE_DIRECT,0,0,0);
	if (!hI)
		goto finish;

	BOOL ld = TRUE;
	InternetSetOption(hI,INTERNET_OPTION_HTTP_DECODING,(void*)&ld,sizeof(BOOL));

	hRead = InternetOpenUrl(hI,TheLink,0,0,INTERNET_FLAG_NO_CACHE_WRITE,0);
	if (!hRead)
		goto finish;

	if (!HttpQueryInfo(hRead,HTTP_QUERY_CONTENT_LENGTH,ss,&bfs,0))
		Size = (DWORD)-1;
	else
		{
		Size = _ttoi(ss);
		}

	for (;;)
		{
		DWORD n;
		char Buff[10010] = {0};

		BOOL F = FALSE;
GoE:
		F = InternetReadFile(hRead,Buff,10000,&n);
		if (F == false && ld == TRUE)
			{
			ld = FALSE;
			InternetSetOption(hI,INTERNET_OPTION_HTTP_DECODING,(void*)&ld,sizeof(BOOL));
			goto GoE;
			}
		if (F == false)
		 {
		 err = 2;
		 break;
		 }
		if (n == 0)
			{
			// End of file !
			err = 0;
			break;
			}
		TotalTransferred += n;

		R.append(Buff);
		}


finish:
	if (hRead)
		InternetCloseHandle(hRead);
	if (hI)
		InternetCloseHandle(hI);
	return R;
	}

void DownloadData(void* xxx)
	{
	A_DOWNLOAD* A = (A_DOWNLOAD*)xxx;
	if (!A)
		return;

	// Download
	unsigned long bfs = 1000;
	wchar_t ss[1000] = {0};
	A->State = 1;
	A->DownloadedBytes = 0;
	A->StatusReason = L"Downloading...";


	HINTERNET hI = 0,hRead = 0;

	hI = InternetOpen(_T("Windows 7 Downloader"),INTERNET_OPEN_TYPE_PRECONFIG,0,0,0);
	if (hI)
		{
		DWORD flg = 0;
		flg = INTERNET_FLAG_NO_CACHE_WRITE;
		hRead = InternetOpenUrl(hI,A->Remote.c_str(),0,0,flg,0);
		if (hRead)
			{
			if (!HttpQueryInfo(hRead,HTTP_QUERY_CONTENT_LENGTH,ss,&bfs,0))
				A->TotalBytes = (unsigned long long)-1;
			else
				A->TotalBytes = _ttoi64(ss);

			// Existing ?
			HANDLE hX = CreateFile(A->Local.c_str(),GENERIC_READ,0,0,OPEN_EXISTING,0,0);
			if (hX != INVALID_HANDLE_VALUE)
				{
				// Resume ?
				LARGE_INTEGER pu = {0};
				GetFileSizeEx(hX,&pu);
				CloseHandle(hX);
				if ((unsigned)pu.QuadPart >= A->TotalBytes || A->TotalBytes == -1 || pu.QuadPart > 0xFFFFFFFF)
					{
					// Either bigger or no size info, either case delete it
					DeleteFile(A->Local.c_str());
					}
				if (InternetSetFilePointer(hRead,pu.LowPart,0,FILE_BEGIN,0) == -1)
					{
					// Failed
					DeleteFile(A->Local.c_str());
					}
				}

			A->PauseThis = CreateEvent(0,TRUE,TRUE,0);

			int PrevPercentage = 0;
			for (; A->TotalBytes ;)
				{
				DWORD n;
				char Buff[10010] = {0};

				// Cancel this ?
				if (A->CancelThis)
					{
					A->StatusReason = L"Download failed";
					A->Percentage = 0;
					A->State = 3;
					break;
					}

				// Paused this?
				WaitForSingleObject(A->PauseThis,INFINITE);

				// Paused all?
				WaitForSingleObject(PauseAll,INFINITE);

				BOOL F = FALSE;

				F = InternetReadFile(hRead,Buff,10000,&n);
				if (F == false)
					{
					A->StatusReason = L"Download failed";
					A->Percentage = 0;
					A->State = 3;
					break;
					}
				if (n == 0)
					{
					// End of file
					A->StatusReason = L"Download finished";
					A->Percentage = 100;
					A->State = 2;
					break;
					}
				A->DownloadedBytes += n;

				// Write to File
				HANDLE hF = CreateFile(A->Local.c_str(),GENERIC_WRITE,0,0,OPEN_ALWAYS,0,0);
				SetFilePointer(hF,0,0,FILE_END);
				DWORD Actual = 0;
				WriteFile(hF,Buff,n,&Actual,0);
				FlushFileBuffers(hF);
				CloseHandle(hF);

				// Update Screen if Percentage Change
				A->Percentage = (int)((float)A->DownloadedBytes*100.0f/(float)A->TotalBytes);
				PrevPercentage = A->Percentage;

				void UpdateTaskBar();
				UpdateTaskBar();
				}
			InternetCloseHandle(hRead);
			}
		else
			{
			A->StatusReason = L"URL failed";
			A->State = 3;
			}
		InternetCloseHandle(hI);
		}
	else
		{
		A->StatusReason = L"URL failed";
		A->State = 3;
		}

	// Close event
	if (A->PauseThis)
		CloseHandle(A->PauseThis);
	A->PauseThis = 0;

	// Update Screen
	Update();
	}


void AddDownload(const wchar_t* url,const wchar_t* LocalFile)
	{
	A_DOWNLOAD* A = new A_DOWNLOAD;
	if (url)
		A->Remote = url;
	if (LocalFile)
		A->Local = LocalFile;
	if (!url || !LocalFile)
		{
		if (DialogBoxParam(hAppInstance,L"DIALOG_SINGLE",MainWindow,SINGLEADD,(LPARAM)A) == 0)
			{
			delete A;
			return;
			}	
		}



	Downloads.push_back(A);
	A_DOWNLOAD& rA = *A;
	rA.Thr = (HANDLE)_beginthread(DownloadData,0,(void*)&rA);

	// Also location if available
	_beginthread(GetDownloadLocationIP,0,(void*)&rA);

	// Invalidate Ribbon Commands to enable them 
	void InvalidateRibbonCommand(int cmd);
	InvalidateRibbonCommand(10025);
	InvalidateRibbonCommand(10026);


	// Add URL to recent items list
	wstring SaveF;
	if (!GetSaveFolder(SaveF))
		return;

	const wchar_t* a1 = wcsrchr(rA.Remote.c_str(),'/');
	const wchar_t* a2 = wcsrchr(rA.Remote.c_str(),'\\');
	const wchar_t* a = 0;
	if (!a1)
		a = a2;
	if (!a2)
		a = a1;
	if (a1 && a2)
		{
		if (a1 > a2)
			a = a1;
		else
			a = a2;
		}
	if (!a)
		return;
	a++;


	wchar_t ff[10000] = {0};
	swprintf_s(ff,10000,L"%s\\%s.W7DL",SaveF.c_str(),a);

	// Create this dummy file
	FILE* fw = _wfopen(ff,L"wb");
	if (!fw)
		return;
	fwprintf(fw,L"%s",rA.Remote.c_str());
	fclose(fw);

	for(unsigned int i = 0 ; i < RecentURLs.size() ; i++)
		{
		if (wcsicmp(ff,RecentURLs[i].c_str()) == 0)
			{
			return; // existing already
			}
		}
	RecentURLs.push_back(ff);
	if (RecentURLs.size() > 10)
		RecentURLs.erase(RecentURLs.begin());

	// Save Recent URLs to XML
	XMLElement* r = cfg->GetRootElement()->FindElementZ("cfg",true)->FindElementZ("recent",true);
	r->RemoveAllElements();
	char d[10000] = {0};
	for(unsigned int i = 0 ; i < RecentURLs.size() ; i++)
		{
		sprintf_s(d,10000,"<u n=\"%s\" />",W(RecentURLs[i].c_str()).operator char *());
		r->AddElement(d);
		}
	}




INT_PTR CALLBACK MULTIPLEADD(HWND hh,UINT mm,WPARAM ww,LPARAM ll)
	{
	static vector<wstring>* U = 0;
	switch(mm)
		{
		case WM_INITDIALOG:
			{
			U = (vector<wstring>*)ll;
			if (!U)
				{
				EndDialog(hh,0);
				return 0;
				}
			return true;
			}
		case WM_COMMAND:
			{
			int HW = HIWORD(ww);
			int LW = LOWORD(ww);

			if (LW == IDCANCEL)
				{
				SendMessage(hh,WM_CLOSE,0,0);
				return 0;
				}

			if (LW == IDOK)
				{
				// Add all urls
				U->clear();
				HWND hE = GetDlgItem(hh,101);
				TCHAR bu[10000] = {0};
				for(unsigned int i = 0 ; ; i++)
					{
					WORD* wo = (WORD*)bu;
					*wo = 10000;
					if (!SendMessage(hE,EM_GETLINE,i,(LPARAM)bu))
						break;
					U->push_back(bu);
					}

				EndDialog(hh,1);
				return 0;
				}

			if (HW == EN_UPDATE && (HWND)ll == GetDlgItem(hh,101))
				{
				wstring SaveF;
				if (!GetSaveFolder(SaveF))
					return 0;

				// URL Update
				// Say ftp://examples:examples@test.com/file.zip
				wchar_t url[10000] = {0};
				GetDlgItemText(hh,101,url,1000);
				wchar_t* a1 = wcsrchr(url,'/');
				wchar_t* a2 = wcsrchr(url,'\\');
				wchar_t* a = 0;
				if (!a1)
					a = a2;
				if (!a2)
					a = a1;
				if (a1 && a2)
					{
					if (a1 > a2)
						a = a1;
					else
						a = a2;
					}

				wchar_t dst[10000] = {0};


				if (a)
					swprintf_s(dst,10000,L"%s\\%s",SaveF.c_str(),a + 1);
				else
					swprintf_s(dst,10000,L"%s\\",SaveF.c_str());
				SetDlgItemText(hh,102,dst);
				return 0;
				}

			return 0;
			}
		case WM_CLOSE:
			{
			EndDialog(hh,0);
			return 0;
			}
		}
	return 0;
	}

void AddDownloads()
	{
	wstring SaveF;
	if (!GetSaveFolder(SaveF))
		return;

	vector<wstring> URLs;
	if (DialogBoxParam(hAppInstance,L"DIALOG_MULTIPLE",MainWindow,MULTIPLEADD,(LPARAM)&URLs) == 0)
		return;
	wchar_t dst[10000] = {0};
	wchar_t url[10000] = {0};

	for(unsigned int i = 0 ; i < URLs.size() ; i++)
		{
		if (URLs[i].length() == 0)
			continue;

		wcscpy_s(url,10000,URLs[i].c_str());


		wchar_t* a1 = wcsrchr(url,'/');
		wchar_t* a2 = wcsrchr(url,'\\');
		wchar_t* a = 0;
		if (!a1)
			a = a2;
		if (!a2)
			a = a1;
		if (a1 && a2)
			{
			if (a1 > a2)
				a = a1;
			else
				a = a2;
			}

		if (!a)
			continue;
		swprintf_s(dst,10000,L"%s\\%s",SaveF.c_str(),a + 1);
		AddDownload(URLs[i].c_str(),dst);
		}
	}


// Called when opening a .W7DL file
void LoadDownload(const wchar_t* fff)
	{
	if (!fff)
		return;
	if (!wcslen(fff))
		return;
	wchar_t ff[10000] = {0};
	wcscpy(ff,fff);
	FILE* fp = _wfopen(ff,L"rb");
	if (!fp)
		{
		// Check starting \"s 
		if (ff[0] == '\"')
			{
			ff[wcslen(ff) - 1] = 0;
			fp = _wfopen(ff + 1,L"rb");
			}
		}
	if (!fp)
		return;

	wchar_t fil[10000] = {0};
	fread(fil,1,10000,fp);
	fclose(fp);

	AddDownload(fil,0);
	}

bool InRect(int xx,int yy,RECT& r)
	{
	if (xx >= r.left && xx <= r.right && yy >= r.top && yy <= r.bottom)
		return true;
	return false;
	}

void GetDownloadLocationIP(void* xxx)
	{
	A_DOWNLOAD* A = (A_DOWNLOAD*)xxx;
	if (!A)
		return;


	A->LocationIP.clear();

	TCHAR se[1000] = {0};
	URL_COMPONENTS uc = {0};
	uc.dwStructSize = sizeof(uc);
	uc.lpszHostName = se;
	uc.dwHostNameLength = 1000;
	InternetCrackUrl(A->Remote.c_str(),0,0,&uc);

	unsigned int ia = inet_addr(W(se));
	if (ia == INADDR_NONE)
		{
		hostent*hp = gethostbyname(W(se));
		if (!hp)
			return;
		memcpy(&ia,hp->h_addr,4);
		}
	if (ia == INADDR_NONE)
		return;

	char d[1000] = {0};
	in_addr* nia = (in_addr*)&ia;
	sprintf_s(d,1000,"%s",inet_ntoa(*nia));
	A->LocationIP = d;

	// Get the http://api.hostip.info/get_html.php?ip=XXx&position=true
	swprintf_s(se,1000,L"http://api.hostip.info/get_html.php?ip=%S&position=true",d);
	string rr = Fetch(se);
	if (rr.empty())
		return;

	// rr is text data
	// Sample
	/*

	Country: UNITED STATES (US)
	City: Alma, AR
	Latitude: 35.4899
	Longitude: -94.2229
	*/

	const char* lat = strstr(rr.c_str(),"atitude");
	if (lat)
		{
		const char* sp = strchr(lat,' ');
		double YY = atof(sp);
		A->ly = YY;
		}

	const char* lon = strstr(rr.c_str(),"ongitude");
	if (lon)
		{
		const char* sp = strchr(lon,' ');
		double XX = atof(sp);
		A->lx = XX;
		}
	}

void Update()
	{
	InvalidateRect(MainWindow,0,TRUE);
	UpdateWindow(MainWindow);
	}



bool AnySelected()
	{
	if (Downloads.empty())
		return false;
	for(unsigned int i = 0 ; i < Downloads.size() ; i++)
		{
		if (Downloads[i]->Selected)
			return true;
		}
	return false;
	}


// Debug Printf
void __cdecl odprintf(const char *format, ...)
	{
	char    buf[4096], *p = buf;
	va_list args;
	int     n;

	va_start(args, format);
	n = _vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
	va_end(args);

	p += (n < 0) ? sizeof buf - 3 : n;

	while ( p > buf  &&  isspace((unsigned char)p[-1]))
		*--p = '\0';

	*p++ = '\r';
	*p++ = '\n';
	*p   = '\0';

	OutputDebugString(W(buf));
	}