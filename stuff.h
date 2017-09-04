// Function/Class declarations - Templates

#define XML_OPTIONAL_MIME
#include "xml.h"

// Class W is used to quickly switch between ANSI/UTF/OLE strings
class W
	{
	private:

		char mempty[5];
		wchar_t wempty[5];

		char * MS;
		wchar_t * WS;
		BSTR OS;
		int s;

	public:

		W(const char*);
		W(const wchar_t*);
		~W();
		const TCHAR* t();
		const char* m();
		const wchar_t* w();
		const char* me();
		const wchar_t* we();
		operator const char*();
		operator const wchar_t*();
		operator char*();
		operator wchar_t*();
		const BSTR o();
	};


struct A_FONT
	{
	wchar_t Family[1000];
	int Size;
	};

struct POINTF3
	{
	double x,z,y;
	};

struct A_DOWNLOAD
	{
	wstring Local;  // Local Target
	wstring Remote; // Remote URL
	int State; // 0 Idle, 1 Downloading , 2 finished OK , 3 finished error
	unsigned long long TotalBytes;
	unsigned long long DownloadedBytes;
	HANDLE Thr;  // Download thread
	wstring StatusReason;
	HANDLE PauseThis;
	bool CancelThis;
	string LocationIP;
	double lx,ly; // Coordinates

	// Drawing 
	int Percentage;
	D2D1_POINT_2F PrevPoint; // Last Animation Point
	RECT AnimRect; // To show animated download status
	RECT HitTest; // Drawing in the client area
	bool Selected;


	A_DOWNLOAD()
		{
		CancelThis = false;
		Percentage = 0;
		PrevPoint.x = PrevPoint.y = 0;
		Local.clear();
		Remote.clear();
		State = 0;
		Selected = false;
		TotalBytes = 0;
		DownloadedBytes = 0;
		memset(&HitTest,0,sizeof(HitTest));
		memset(&AnimRect,0,sizeof(AnimRect));
		StatusReason.clear();
		Thr = 0;
		PauseThis = 0;
		LocationIP.clear();
		lx = 0;
		ly = 0;
		}
	};

extern vector<A_DOWNLOAD*> Downloads;
extern vector<wstring> RecentURLs;


// Globals
extern HINSTANCE hAppInstance;
extern HICON hIcon1;
extern HWND MainWindow;
extern const TCHAR* ttitle;
extern bool IsItemSelected;
extern unsigned int FGColor,BGColor;
extern XML* cfg;
extern A_FONT CurrentFont;
extern unsigned int LastRibbonHeight;
extern const wchar_t* OurAppID;
extern HANDLE PauseAll;
extern POINTF3 Loc;


#define AXRGB(a,r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))
#define GetAValue(rgb) (rgb >> 24)

// Function Declarations
bool RibbonInit(HWND hh);
void RibbonEnd();
bool TBInit(HWND hh);
void TBEnd();
bool SensorsInit(HWND hh);
void SensorsEnd();
bool D2DInit(HWND hh);
bool D2DCreateFonts();
void D2DEnd();
void PaintWindow();
void ResizeWindow();
bool InitAnim(HWND hh);
void EndAnim();

void nop();
void __cdecl odprintf(HWND hL,const char *format, ...);
void GetDownloadLocationIP(void* xx);
LRESULT TouchHandler(HWND hh,UINT mm,WPARAM ww,LPARAM ll);
LRESULT GestureHandler(HWND hh,UINT mm,WPARAM ww,LPARAM ll);
void Update();
bool GetSaveFolder(wstring& x);
void SavePathInLibrary(const TCHAR* x);