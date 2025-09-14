// (c) 2000-2007 Electronic Arts Inc.

/*********************************************************************************
ABSTRACT

showany_eg.cpp - GIMEX example, display a GIMEX supported file format

DESCRIPTION

This example shows how to display a GIMEX supported file format using
the WINDOWS API.

In order to build this example, you must include <HTML dib.htm>dib.cpp<HTML /a> and dib.h.

Please see the GIMEX <HTML a href="../sdk/gettingstarted/compile.htm">compiling and linking<HTML /a> for the files that
are required to build this example. You will also need to download
<HTML a href="../sdk/gettingstarted/download.htm">gimex.dll or gimex.lib<HTML /a> and the <HTML a href="http://tools.eac.ea.com/pc">REAL libraries<HTML /a>
in order to run this example.

SOURCE
@*********************************************************************************/

#if defined(_MSC_VER)
#pragma warning(push,1)
#include <windows.h>
#pragma warning(pop)
/* warning C4505: unreferenced inline function has been removed */
#pragma warning(disable: 4514)
#endif
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

#include "dib.h"
#include "gopen.h"
#if defined(_MSC_VER) && _MSC_VER>=1400
#define ENABLE_OMP 1
#include <omp.h>
#endif

DIB *g_pDIB = NULL;
int g_width;
int g_height;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

DIB *LoadANY(const char *pPathName)
{
	DIB* pDIB = NULL;
	GSTREAM* g;
	GINSTANCE* gx;

    // open source file and set best GIMEX module for file type:
    g = GIMEX_gopen(pPathName,0);

	if (g != NULL)
	{
        if (GIMEX_open(&gx, g, (char*)pPathName,0))
		{
			GINFO* ginfo = GIMEX_info(gx, 0);
			if (ginfo != NULL)
			{
                g_width = ginfo->width;
                g_height = ginfo->height;
				pDIB = DIB_create(ginfo->width, ginfo->height, ginfo->bpp);
				if (pDIB != NULL)
				{
					GIMEX_read(gx, ginfo, DIB_getbase(pDIB), DIB_getrowbytes(pDIB));
				}
				gfree(ginfo);
			}
			GIMEX_close(gx);
		}
		gclose(g);
	}
    else
    {
		MessageBox(NULL, TEXT("Could open file!"), "showany", MB_ICONERROR);
    }

	return pDIB;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC         hdc;
	PAINTSTRUCT ps;
	RECT        rect;

	switch (message)
	{
	case WM_CREATE:
		return 0;

	case WM_PAINT:
		hdc = BeginPaint (hwnd, &ps);

		GetClientRect (hwnd, &rect);

		if (g_pDIB != NULL)
		{
			DIB_draw(g_pDIB, hdc, 0,0);
//            TextOut(hdc,0,0,"Hello",5);
		}

		EndPaint (hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage (0);
		return 0;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT ("ShowAny");
	HWND         hwnd;
	MSG          msg={0,};
	WNDCLASS     wndclass;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT ("Could not register window class!"), szAppName, MB_ICONERROR);
		return 0;
	}

    if (GIMEX_init())       // load and initialize library
    {
		g_pDIB = LoadANY(szCmdLine);
        if (g_pDIB)
        {
        	hwnd = CreateWindow (szAppName, // window class name
        		TEXT ("ShowAny"),			// window caption
        		WS_OVERLAPPEDWINDOW,        // window style
        		CW_USEDEFAULT,              // initial x position
        		CW_USEDEFAULT,              // initial y position
        		g_width,                    // initial x size
        		g_height,                   // initial y size
        		NULL,                       // parent window handle
        		NULL,                       // window menu handle
        		hInstance,                  // program instance handle
        		NULL);                      // creation parameters

        	ShowWindow(hwnd, iCmdShow);
        	UpdateWindow(hwnd);

        	while (GetMessage(&msg, NULL, 0, 0))
        	{
        		TranslateMessage(&msg);
        		DispatchMessage(&msg);
        	}
        }
    }

	return msg.wParam;
}

// END ABSTRACT
