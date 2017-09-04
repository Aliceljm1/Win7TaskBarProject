// Main
#include "stdafx.h"

void SelectFileLocation(HWND hh,wstring& fn)
	{
	HRESULT hr = S_OK;
	IShellItem* si = 0; 

	IFileSaveDialog* fs;
	hr = CoCreateInstance(CLSID_FileSaveDialog,NULL,CLSCTX_INPROC,__uuidof(IFileSaveDialog),(void**)&fs);
	if (SUCCEEDED(hr))
		{
		FILEOPENDIALOGOPTIONS fop;
		hr = fs->GetOptions(&fop);
		fop |= FOS_OVERWRITEPROMPT | FOS_PATHMUSTEXIST | FOS_FORCESHOWHIDDEN;
		fop &= ~FOS_FORCEFILESYSTEM;
		hr = fs->SetOptions(fop);
		hr = fs->SetFileName(fn.c_str());
		if (SUCCEEDED(hr))
			{
			hr = fs->Show(hh);
			if (SUCCEEDED(hr))
				{
				hr = fs->GetResult(&si);
				}
			}
		fs->Release();
		}

	if (!si)
		return;

	LPWSTR ff = 0;
	si->GetDisplayName(SIGDN_FILESYSPATH,&ff);
	if (ff)
		{
		fn = ff;
		CoTaskMemFree(ff);
		}
	si->Release();
	}


void SavePathInLibrary(const TCHAR* x)
	{
	// Creates a W7Downloader Library if not existing
	// Adds the target folder of the file to the library
	HRESULT hr = 0;

	// Get the default save as folder
	wstring SaveF;
	if (!GetSaveFolder(SaveF))
		return;

	// First, make sure the library exists
	IShellLibrary* sl = 0;
	SHCreateLibrary(__uuidof(IShellLibrary),(void**)&sl);
	if (!sl)
		return;
	IShellItem* si = 0;
	sl->SaveInKnownFolder(FOLDERID_Libraries,L"W7Downloads",LSF_FAILIFTHERE,&si);
	if (si)
		si->Release();
	si = 0;
	sl->Release();
	sl = 0;

	// Load the library
	TCHAR ln[10000] = {0};
	PWSTR LibraryFolderPath = 0;
	SHGetKnownFolderPath(FOLDERID_Libraries,0,0,&LibraryFolderPath);
	if (!LibraryFolderPath)
		return;
	swprintf_s(ln,10000,L"%s\\W7Downloads.library-ms",LibraryFolderPath);
	CoTaskMemFree(LibraryFolderPath);
	hr = SHLoadLibraryFromParsingName(ln,STGM_READWRITE,__uuidof(IShellLibrary),(void**)&sl);
	if (!sl)
		return;

	// Add the SaveF folder to this library
	hr = SHAddFolderPathToLibrary(sl,SaveF.c_str());
	sl->Commit();

	// Add the file directory in that path ?
	TCHAR fn[10000] = {0};
	wcscpy_s(fn,10000,x);
	wchar_t* a = wcsrchr(fn,'\\');
	if (a)
		{
		*a = 0;

		hr = SHAddFolderPathToLibrary(sl,fn);
		sl->Commit();
		}


	sl->Release();
	}

// 

