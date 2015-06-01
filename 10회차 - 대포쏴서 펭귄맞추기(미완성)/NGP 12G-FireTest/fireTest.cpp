#include <windows.h>
#include <time.h>
#include <windowsx.h>
#include <math.h>
#include <ddraw.h>
#include <dsound.h>
#include <stdio.h>


#include "dsutil.h"
#include "ddutil.h"

long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

HWND MainHwnd;

LPDIRECTDRAW         DirectOBJ;
LPDIRECTDRAWSURFACE  RealScreen;
LPDIRECTDRAWSURFACE  BackScreen;
LPDIRECTDRAWCLIPPER	 ClipScreen;

LPDIRECTDRAWSURFACE  SpriteImage;
LPDIRECTDRAWSURFACE  BackImage;

LPDIRECTSOUND       SoundOBJ = NULL;
LPDIRECTSOUNDBUFFER SoundDSB = NULL;
DSBUFFERDESC        DSB_desc;
HSNDOBJ Sound[10];


int MouseX, MouseY;
bool space = false;
bool checkTime = false;

//////////////////////////////////////////////////////////////////////////////////


BOOL _InitDirectSound( void )
{
    if ( DirectSoundCreate(NULL,&SoundOBJ,NULL) == DS_OK )
    {
        if (SoundOBJ->SetCooperativeLevel(MainHwnd,DSSCL_PRIORITY)!=DS_OK) return FALSE;

        memset(&DSB_desc,0,sizeof(DSBUFFERDESC));
        DSB_desc.dwSize = sizeof(DSBUFFERDESC);
        DSB_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

        if (SoundOBJ->CreateSoundBuffer(&DSB_desc,&SoundDSB,NULL)!=DS_OK) return FALSE;
        SoundDSB -> SetVolume(0);
        SoundDSB -> SetPan(0);
        return TRUE;
    }
    return FALSE;
}


BOOL Fail( HWND hwnd, char *Output )
{
    ShowWindow( hwnd, SW_HIDE );
    MessageBox( hwnd,  Output, "Game Programming", MB_OK );
    DestroyWindow( hwnd );
    return FALSE;
}

void _ReleaseAll( void )
{
    if ( DirectOBJ != NULL )
    {

        if ( BackScreen != NULL )
        {
            BackScreen->Release();
            BackScreen = NULL;
        }
        if ( RealScreen != NULL )
        {
            RealScreen->Release();
            RealScreen = NULL;
        }
        if ( SpriteImage != NULL )
        {
            SpriteImage->Release();
            SpriteImage = NULL;
        }

        DirectOBJ->Release();
        DirectOBJ = NULL;
    }
}


BOOL _GameMode( HINSTANCE hInstance, int nCmdShow, DWORD  x, DWORD  y, DWORD  bpp, int FullScreen)
{
    WNDCLASS wc;
    DDSURFACEDESC ddsd;
    DDSCAPS ddscaps;
    LPDIRECTDRAW pdd;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = GetStockBrush(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "GameProg";
    RegisterClass( &wc );

	if(FullScreen){
		MainHwnd = CreateWindowEx (
			    0, "GameProg", NULL, WS_POPUP, 0, 0,
				GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
				NULL, NULL, hInstance, NULL );
	}
	else{
		MainHwnd = CreateWindow( "GameProg", "WindowMode", 
			WS_OVERLAPPEDWINDOW, 0, 0, x, y, NULL, NULL, hInstance, NULL );
	}
    if ( !MainHwnd ) return FALSE;


	// 다이렉트 드로우(DD) 생성
    if(FAILED(DirectDrawCreate( NULL, &pdd, NULL )))
		return Fail( MainHwnd, "DirectDrawCreate" );
	// DD에 연결
    if(FAILED(pdd->QueryInterface(IID_IDirectDraw, (LPVOID *) &DirectOBJ)))
		return Fail( MainHwnd, "QueryInterface" );

	// 윈도우 핸들의 협력 단계를 설정한다.
	if(FullScreen){
		if(FAILED(DirectOBJ->SetCooperativeLevel( MainHwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN )))
			return Fail( MainHwnd, "SetCooperativeLevel" );
		// Set full screen display mode
		if(FAILED(DirectOBJ->SetDisplayMode( x, y, bpp)))
			return Fail( MainHwnd, "SetDisplayMode" );

		memset( &ddsd, 0, sizeof(ddsd) );
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
		if(FAILED(DirectOBJ -> CreateSurface( &ddsd, &RealScreen, NULL )))
			return Fail( MainHwnd, "CreateSurface" );

	   memset( &ddscaps, 0, sizeof(ddscaps) );
	   ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
	   if(FAILED(RealScreen -> GetAttachedSurface( &ddscaps, &BackScreen )))
			return Fail( MainHwnd, "GetAttachedSurface" );
	}
	else{
		if(FAILED(DirectOBJ->SetCooperativeLevel(MainHwnd, DDSCL_NORMAL)))
			return Fail( MainHwnd, "SetCooperativeLevel" );

		memset( &ddsd, 0, sizeof(ddsd) );
	    ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwBackBufferCount = 0;
	    if(FAILED(DirectOBJ -> CreateSurface( &ddsd, &RealScreen, NULL )))
			return Fail( MainHwnd,  "CreateSurface" );

		memset( &ddsd, 0, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = x;
		ddsd.dwHeight = y;
		if(FAILED(DirectOBJ->CreateSurface( &ddsd, &BackScreen, NULL )))
			return Fail( MainHwnd, "CreateSurface" );

		if(FAILED(DirectOBJ->CreateClipper( 0, &ClipScreen, NULL)))
			return Fail( MainHwnd, "CreateClipper" );

		if(FAILED(ClipScreen->SetHWnd( 0, MainHwnd )))
			return Fail( MainHwnd, "SetHWnd" );

		if(FAILED(RealScreen->SetClipper( ClipScreen )))
			return Fail( MainHwnd, "SetClipper" );

		SetWindowPos(MainHwnd, NULL, 0, 0, x, y, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	SetFocus( MainHwnd );
	ShowWindow( MainHwnd, nCmdShow );
	UpdateWindow( MainHwnd );
	ShowCursor( TRUE );

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// Implmentation Here

#define MAX_OBJ	20

struct _OBJECT
{
    int		Type;	// 1,2,3
	int		srcWidth, srcHeight;
	int		srcX, srcY;
	int		TargetX, TagretY;
	int		TargetSize;
    int		X1, Y1, X2, Y2;

    int		Attack, Crash;
    int		Location, Move, Show;
    int		Demage, Death;
    int		Weapon, Hit;
	double	Level, HP, MP, EXP;
};
_OBJECT Object[MAX_OBJ];


void _GameProc( int FullScreen )
{
	RECT	BackRect={0,0,640,480};
	RECT	srcRect, dstRect[2];
	int		BaseY=350;

	//Object로 옮길 것
	int MouseX1, MouseY1;
	double  static distant;
	double  static cosangle, sinangle;

	double static t1;
	double static t2;
	double  increaseX = 0;
	double	increaseY = 0;
	int static speed = 0;

	bool static goRight = true;
	
	
	// Clear Back Ground
	BackScreen->BltFast(0, 0, BackImage, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);


	//////////////////////////////

	// Enter splite animation here
	if (space == true){
		if (checkTime == true){
			//1번 계산만 필요한 것들
			t1 = clock()/100;
			distant = sqrt(pow((double)(MouseX - 50),2) + pow((double)(MouseY - 350), 2));
			cosangle = (MouseX - 50) / distant;
			sinangle = (360 - MouseY) / distant;
			checkTime = false;
		}
		t2 = clock()/100;
	}
	
	// Canon x= 10, 85, 150-220,   y=350, 410
	srcRect.left = 10; 
    srcRect.top = 350; 
    srcRect.right = 85; 
    srcRect.bottom = 410; 
	BackScreen->BltFast(50, BaseY, SpriteImage, &srcRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	

	// Ball x=235, 255    y=365,385
	srcRect.left = 235; 
    srcRect.top = 365; 
    srcRect.right = 255; 
    srcRect.bottom = 385; 
	

	dstRect[0].left = 120 + (int)(60 * cosangle * (t2 - t1))%640;
	dstRect[0].top = 350 - (int)(60 * sinangle * (t2 - t1) - 0.5*9.8 * (t2 - t1)*(t2 - t1))%480;
	dstRect[0].right = dstRect[0].left + 20;
	dstRect[0].bottom = dstRect[0].top + 20;

	if (dstRect[0].right >= 650 || dstRect[0].bottom >= 500)
		space = false;
	BackScreen->Blt(&dstRect[0], SpriteImage, &srcRect,  DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	
	//충돌체 구현
	if ((dstRect[1].left <= dstRect[0].right <= dstRect[1].right && dstRect[1].top <= dstRect[0].bottom <= dstRect[1].bottom) ||
		(dstRect[1].left <= dstRect[0].left <= dstRect[1].right && dstRect[1].top <= dstRect[0].bottom <= dstRect[1].bottom)){

		srcRect.left = 255;
		srcRect.top = 350;
		srcRect.right = 340;
		srcRect.bottom = 410;
	
			
	}
	else{
		// Hero x=0, 60    y=0,50
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = 60;
		srcRect.bottom = 50;
	}

		dstRect[1].left = 500 + speed;
		dstRect[1].top = 350;
		dstRect[1].right = dstRect[1].left + 60;
		dstRect[1].bottom = dstRect[1].top + 50;

		if (dstRect[1].left + speed > 550){ goRight = false; }

		if (dstRect[1].left + speed < 300){ goRight = true; }

		if (goRight){ speed += 2; }
		else { speed += -2; }



		BackScreen->Blt(&dstRect[1], SpriteImage, &srcRect, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	
	/////////////////////////////////

	if(FullScreen)
		RealScreen -> Flip( NULL, DDFLIP_WAIT );
	else{
		RECT WinRect;
		RECT Rect={0,0,640,480};

		GetWindowRect(MainHwnd, &WinRect);
		RealScreen -> Blt( &WinRect, BackScreen, &Rect, DDBLT_WAIT, NULL );
	}
}



long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
		case WM_MOUSEMOVE:  MouseX = LOWORD(lParam);
				     		MouseY = HIWORD(lParam);
				            break;

		case WM_LBUTTONDOWN  :  
			break;

        case WM_DESTROY      :  _ReleaseAll();
			PostQuitMessage( 0 );
            break;

		case WM_TIMER:
			_GameProc(0);
			break;

        case WM_KEYDOWN:            
            switch (wParam)
            {
                case VK_ESCAPE:
                case VK_F12: 
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    return 0;            


				case VK_LEFT: 
					return 0;

                case VK_RIGHT: 
					return 0;

                case VK_UP:  
					return 0;

                case VK_DOWN: 
					return 0;


				case VK_SPACE:
								  space = true;
								  checkTime = true;
								  break; 

				case VK_CONTROL:
					break;
            }
            break;

    }

    return DefWindowProc( hWnd, message, wParam, lParam );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    MSG msg;

    if ( !_GameMode(hInstance, nCmdShow, 640, 480, 32, 0) ) return FALSE;


    BackImage = DDLoadBitmap( DirectOBJ, "Back.BMP", 0, 0 );
    DDSetColorKey( BackImage, RGB(0,0,0) );

    SpriteImage = DDLoadBitmap( DirectOBJ, "Char.BMP", 0, 0 );
    DDSetColorKey( SpriteImage, RGB(0,0,0) );

    if ( _InitDirectSound() )
    {
        Sound[0] = SndObjCreate(SoundOBJ,"MUSIC.WAV",1);
		Sound[1] = SndObjCreate(SoundOBJ,"land.WAV",2);
		Sound[2] = SndObjCreate(SoundOBJ,"gun.WAV",2);
//        SndObjPlay( Sound[0], DSBPLAY_LOOPING );
    }

	SetTimer(MainHwnd, 1, 10, NULL);


	// Main message loop
	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


    DestroyWindow( MainHwnd );

    return TRUE;
}

///////////////////// End of Game Program
