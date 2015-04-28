/*************************************************************************************
 *
 *  Copyright ?1998 Cam Upright
 *
 *  File:			blit.cpp
 *  Content:		Simple program to show how to use the Blt and BltFast methods
 *  Basic Design:   This program initializes DirectDraw, then goes into the main 
 *					message loop.  When PeekMessage reveals there's no messages,
 *					the program runs the UpdateStuff function.  This function does
 *					2 things.  #1 is it displays the face on the screen in one of
 *					4 ways, and #2 it flips the surfaces.  When the program 
 *					recieves a WM_KEYDOWN message, and f1 - f4 are pressed, the
 *					dwBlitExample changes. This value determines which of 4 
 *					different blitting effect are displayed.
 *
 ************************************************************************************/



#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include "ddutil.h"		     /* This would normally be in <> instead, but because 
					of problems with directx 6 using different arguments
					in the functions, I have to send have the ddutil files
					with the blit program */
#include <stdio.h>
#include "resource.h"

#define TOP_LEFT			1
#define BOTTOM_RIGHT			2
#define FULL_SCREEN			3
#define SHRINK_N_GROW			4

LPDIRECTDRAW				lpDD;           // DirectDraw object
LPDIRECTDRAWSURFACE			lpDDSPrimary;   // DirectDraw primary surface
LPDIRECTDRAWSURFACE			lpDDSBack;      // DirectDraw back surface
LPDIRECTDRAWSURFACE			lpDDSFace;
DWORD						dwBlitExamples = FULL_SCREEN;
BOOL						bActive;        // is application active?
HWND						hwnd;
double						size = 0;



void DoFlip (void);
void ClearScreen(void);
static void finiObjects( void );
void UpdateStuff(void);


void ClearScreen(void)		// Name says it all
{
	DDBLTFX				ddbltfx;
	HRESULT				ddrval;


	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor = 0;
	while (1)
	{
		ddrval = lpDDSBack->Blt(NULL,NULL,NULL,DDBLT_COLORFILL,&ddbltfx);

		if( ddrval == DD_OK )
		    break;
		if( ddrval == DDERR_SURFACELOST )
		{
		    ddrval = lpDDSBack->Restore();
		    if( ddrval != DD_OK )
				break;
		}
		if( ddrval != DDERR_WASSTILLDRAWING )
		    break;
	}

}

void DoBlitExample (void)
{
	HRESULT						ddrval;
	RECT						rcDest;


	ClearScreen();

	if (dwBlitExamples == TOP_LEFT)
	{
		ddrval = lpDDSBack->BltFast(0, 0, lpDDSFace, NULL, NULL);

		return;
	}


	if (dwBlitExamples == BOTTOM_RIGHT)
	{
		rcDest.left = 640 - 120;
		rcDest.top = 480 - 120;
		rcDest.right = rcDest.left + 120;
		rcDest.bottom = rcDest.top + 120;


		ddrval = lpDDSBack->Blt(&rcDest, lpDDSFace, NULL, NULL, NULL);

		return;
	}

	if (dwBlitExamples == FULL_SCREEN)
	{

		ddrval = lpDDSBack->Blt(NULL, lpDDSFace, NULL, DDBLT_WAIT, NULL);

		return;
	}


	/***************************************************************************/
	// Insert code here

	if (dwBlitExamples == SHRINK_N_GROW)
	{
		static bool	ShrinkOrGrow = 1;
		static int pos = 40;


		rcDest.left = (DWORD)(640/2- 120) - pos;
		rcDest.top = (DWORD)(480/2- 120) - pos;
		rcDest.right = (DWORD)(640/2 + 120) +pos;
		rcDest.bottom = (DWORD)(480/2 + 120) +pos;

		if (ShrinkOrGrow){
			pos++;
		}
		else{
			pos--;
		}
		if (pos > 50){
			ShrinkOrGrow = 0;
		}
		if (pos < 10){
			ShrinkOrGrow = 1;
		}
		ddrval = lpDDSBack->Blt(&rcDest, lpDDSFace, NULL, NULL, NULL);

		return;
	}

	/***************************************************************************/
}


static void finiObjects( void )
{
    if( lpDD != NULL )
    {
	if( lpDDSPrimary != NULL )
	{
	    lpDDSPrimary->Release();
	    lpDDSPrimary = NULL;
	}
	if ( lpDDSFace != NULL )
	{
		lpDDSFace->Release();
	    lpDDSFace = NULL;
	}
	lpDD->Release();
	lpDD = NULL;
    }
} /* finiObjects */


void DoFlip (void)
{
	while( 1 )
	{	
		HRESULT ddrval;
		
		ddrval = lpDDSPrimary->Flip( NULL, 0 );
		if (ddrval == DD_OK)
			
		    break;

		if( ddrval == DDERR_SURFACELOST )
		{
		    ddrval = lpDDSPrimary->Restore();
		    if( ddrval != DD_OK )
				break;
		}
		if( ddrval != DDERR_WASSTILLDRAWING )
		    break;
	}
}


void UpdateStuff(void)
{
	DoBlitExample();
	DoFlip();
}



long FAR PASCAL WindowProc( HWND hWnd, UINT message, 
			    WPARAM wParam, LPARAM lParam )
{
    static BYTE			phase = 0;


    switch( message )
    {
    case WM_ACTIVATEAPP:
	bActive = wParam;
	break;

    case WM_CREATE:
	break;

    case WM_SETCURSOR:
	SetCursor(NULL);
	return TRUE;
 
    case WM_KEYDOWN:
	switch( wParam )
	{
	case VK_ESCAPE:
	case VK_F12:
	    PostMessage(hWnd, WM_CLOSE, 0, 0);
	    break;

	case VK_F1:
		dwBlitExamples = TOP_LEFT;
		break;

	case VK_F2:
		dwBlitExamples = BOTTOM_RIGHT;
		break;

	case VK_F3:
		dwBlitExamples = FULL_SCREEN;
		break;

	case VK_F4:
		dwBlitExamples = SHRINK_N_GROW;
		break;
	}
	break;					// Break from WM_KEYDOWN switch

    case WM_DESTROY:
	finiObjects();
	PostQuitMessage( 0 );
	break;
    }
		
    return DefWindowProc(hWnd, message, wParam, lParam);

} /* WindowProc */

/*
 * doInit - do work required for every instance of the application:
 *                create the window, initialize data
 */
static BOOL doInit( HINSTANCE hInstance, int nCmdShow )
{
    WNDCLASS            wc;
    DDSURFACEDESC       ddsd;
    DDSCAPS             ddscaps;
    HRESULT             ddrval;

    /*
     * set up and register window class
     */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( hInstance, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = NULL;
    wc.lpszMenuName = "BlitExample";
    wc.lpszClassName = "BlitExample";
    RegisterClass( &wc );
    
    /*
     * create a window
     */
    hwnd = CreateWindowEx(
	WS_EX_TOPMOST,
	"BlitExample",
	"Blit Example",
	WS_POPUP,
	0, 0,
	GetSystemMetrics( SM_CXSCREEN ),
	GetSystemMetrics( SM_CYSCREEN ),
	NULL,
	NULL,
	hInstance,
	NULL );

    if( !hwnd )
    {
	return FALSE;
    }

    ShowWindow( hwnd, nCmdShow );
    UpdateWindow( hwnd );

    /*
     * create the main DirectDraw object
     */
    ddrval = DirectDrawCreate( NULL, &lpDD, NULL );
    if( ddrval != DD_OK )
		return FALSE;
	// Get exclusive mode
	ddrval = lpDD->SetCooperativeLevel( hwnd,
				DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
	if(ddrval != DD_OK )
		return FALSE;

    ddrval = lpDD->SetDisplayMode( 640, 480, 16 );
    if( ddrval != DD_OK )
		return FALSE;

	// Create the primary surface with 1 back buffer
	ddsd.dwSize = sizeof( ddsd );
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
			      DDSCAPS_FLIP | 
			      DDSCAPS_COMPLEX;

	ddsd.dwBackBufferCount = 1;
	ddrval = lpDD->CreateSurface( &ddsd, &lpDDSPrimary, NULL );
	if( ddrval != DD_OK )
		return FALSE;
    // Get a pointer to the back buffer
    ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
    ddrval = lpDDSPrimary->GetAttachedSurface(&ddscaps, 
					  &lpDDSBack);
 
	lpDDSFace = DDLoadBitmap(lpDD, "SICK", 0, 0);

	ClearScreen();
	DoFlip();
	ClearScreen();

	return TRUE;
        
} /* doInit */

/*
 * WinMain - initialization, message loop
 */
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
			LPSTR lpCmdLine, int nCmdShow)
{
    MSG         msg;

    lpCmdLine = lpCmdLine;
    hPrevInstance = hPrevInstance;

    if( !doInit( hInstance, nCmdShow ) )
    {
		finiObjects();
		DestroyWindow( hwnd );
		return FALSE;
    }

	while( 1 )
	{
   		if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
   		{
			if( !GetMessage( &msg, NULL, 0, 0 ) )
			{
			   	return msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			UpdateStuff();
	}
    return msg.wParam;

}
