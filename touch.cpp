// Main
#include "stdafx.h"

bool InRect(int xx,int yy,RECT& r);
LRESULT TouchHandler(HWND hh,UINT mm,WPARAM ww,LPARAM ll)
	{
	// Get the touching
	int ni = LOWORD(ww);
	TOUCHINPUT* ti = new TOUCHINPUT[ni + 1];
	if (!GetTouchInputInfo((HTOUCHINPUT)ll,ni + 1,ti,sizeof(TOUCHINPUT)))
		{
		delete[] ti;
		return DefWindowProc(hh,mm,ww,ll);
		}

	// Process Messages
	for(int i = 0 ; i < ni ; i++)
		{
		LONG x = ti[i].x;
		LONG y = ti[i].y;

		// Convert these /100
		x /= 100;
		y /= 100;

		// Convert to client
		POINT p = {x,y};
		ScreenToClient(hh,&p);

		// Select downloads
		for(unsigned int i = 0 ; i < Downloads.size() ; i++)
			{
			if (InRect(p.x,p.y,Downloads[i]->HitTest))
				Downloads[i]->Selected = !Downloads[i]->Selected;
			}
		}




	delete[] ti;
	CloseTouchInputHandle((HTOUCHINPUT)ll);
	return 0;
	}

