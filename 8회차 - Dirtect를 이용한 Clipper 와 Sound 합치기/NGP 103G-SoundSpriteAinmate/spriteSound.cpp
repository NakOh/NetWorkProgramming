#include <windows.h>
#include <windowsx.h>
#include <ddraw.h>
#include <dsound.h>

#include "ddutil.h"
#include "dsutil.h"

#define _GetKeyState( vkey ) HIBYTE(GetAsyncKeyState( vkey ))
#define _GetKeyPush( vkey )  LOBYTE(GetAsyncKeyState( vkey ))

HWND MainHwnd;
HSNDOBJ Sound[10];

LPDIRECTDRAW         DirectOBJ;
LPDIRECTDRAWSURFACE  RealScreen;
LPDIRECTDRAWSURFACE  BackScreen;
LPDIRECTDRAWSURFACE  SpriteImage;
LPDIRECTDRAWSURFACE  BackGround;

LPDIRECTDRAWCLIPPER	ClipScreen;


LPDIRECTSOUND       SoundOBJ = NULL;
LPDIRECTSOUNDBUFFER SoundDSB = NULL;
DSBUFFERDESC        DSB_desc;

int gFullScreen=0;
int gWidth, gHeight;
int MouseX, MouseY;
static bool click = false;



BOOL Fail( HWND hwnd )
{
    ShowWindow( hwnd, SW_HIDE );
    MessageBox( hwnd, "DIRECT X 초기화에 실패했습니다.", "프로그래밍", MB_OK );
    DestroyWindow( hwnd );
    return FALSE;
}

void _Play(int num)
{
	SndObjPlay(Sound[num], NULL);
}

BOOL _InitDirectSound(void)
{
	if (DirectSoundCreate(NULL, &SoundOBJ, NULL) == DS_OK)
	{
		if (SoundOBJ->SetCooperativeLevel(MainHwnd, DSSCL_PRIORITY) != DS_OK) return FALSE;

		memset(&DSB_desc, 0, sizeof(DSBUFFERDESC));
		DSB_desc.dwSize = sizeof(DSBUFFERDESC);
		DSB_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

		if (SoundOBJ->CreateSoundBuffer(&DSB_desc, &SoundDSB, NULL) != DS_OK) return FALSE;
		SoundDSB->SetVolume(0);
		SoundDSB->SetPan(0);
		return TRUE;
	}
	return FALSE;
}

void _ReleaseAll( void )
{
    if ( DirectOBJ != NULL )
    {
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
        if ( BackGround != NULL )
        {
            BackGround->Release();
            BackGround = NULL;
        }
        DirectOBJ->Release();
        DirectOBJ = NULL;
    }
}

long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        case    WM_MOUSEMOVE    :   MouseX = LOWORD(lParam);
                                    MouseY = HIWORD(lParam);
                                    break;
		
        case    WM_DESTROY      :  _ReleaseAll();
                                    PostQuitMessage( 0 );
                                    break;

		case  WM_LBUTTONDOWN:{ click = true; }
								  break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}

BOOL _GameMode( HINSTANCE hInstance, int nCmdShow, int x, int y, int bpp )
{
    HRESULT result;
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
    wc.lpszClassName = "EXAM3";
    RegisterClass( &wc );

    MainHwnd = CreateWindowEx (
            0, "EXAM3", NULL, WS_POPUP, 0, 0,
            GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
            NULL, NULL, hInstance, NULL );


    if ( !MainHwnd ) return FALSE;

    SetFocus( MainHwnd );
    ShowWindow( MainHwnd, nCmdShow );
    UpdateWindow( MainHwnd );
    ShowCursor( FALSE );

    result = DirectDrawCreate( NULL, &pdd, NULL );
    if ( result != DD_OK ) return Fail( MainHwnd );

    result = pdd->QueryInterface(IID_IDirectDraw, (LPVOID *) &DirectOBJ);
    if ( result != DD_OK ) return Fail( MainHwnd );

	gWidth=x;
	gHeight=y;

	// 윈도우 핸들의 협력 단계를 설정한다.
	if(gFullScreen){
	    result = DirectOBJ->SetCooperativeLevel( MainHwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
		if ( result != DD_OK ) return Fail( MainHwnd );

		result = DirectOBJ->SetDisplayMode( x, y, bpp);
		if ( result != DD_OK ) return Fail( MainHwnd );

		memset( &ddsd, 0, sizeof(ddsd) );
		ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;

	    result = DirectOBJ -> CreateSurface( &ddsd, &RealScreen, NULL );
	   if ( result != DD_OK ) return Fail( MainHwnd );

		memset( &ddscaps, 0, sizeof(ddscaps) );
		ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		result = RealScreen -> GetAttachedSurface( &ddscaps, &BackScreen );
		if ( result != DD_OK ) return Fail( MainHwnd );

	}
	else{
	    result = DirectOBJ->SetCooperativeLevel( MainHwnd, DDSCL_NORMAL );
		if ( result != DD_OK ) return Fail( MainHwnd );

		memset( &ddsd, 0, sizeof(ddsd) );
	    ddsd.dwSize = sizeof( ddsd );
		ddsd.dwFlags = DDSD_CAPS;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddsd.dwBackBufferCount = 0;

		result = DirectOBJ -> CreateSurface( &ddsd, &RealScreen, NULL );
	    if(result != DD_OK) return Fail(MainHwnd);

		memset( &ddsd, 0, sizeof(ddsd) );
		ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		ddsd.dwWidth = x; 
		ddsd.dwHeight = y;
		result = DirectOBJ->CreateSurface( &ddsd, &BackScreen, NULL );
		if ( result != DD_OK ) return Fail( MainHwnd );

		result = DirectOBJ->CreateClipper( 0, &ClipScreen, NULL);
		if ( result != DD_OK ) return Fail( MainHwnd );

		result = ClipScreen->SetHWnd( 0, MainHwnd );
		if ( result != DD_OK ) return Fail( MainHwnd );

		result = RealScreen->SetClipper( ClipScreen );
		if ( result != DD_OK ) return Fail( MainHwnd );

		SetWindowPos(MainHwnd, NULL, 100, 100, x, y, SWP_NOZORDER | SWP_NOACTIVATE); 
	}


    return TRUE;
}


void CALLBACK _GameProc(HWND hWnd, UINT message, UINT wParam, DWORD lParam)
{
	
		static int Frame = 0, Count = 0;
		RECT BackRect = { 0, 0, 640, 480 }, SpriteRect;

		BackScreen->BltFast(0, 0, BackGround, &BackRect, DDBLTFAST_WAIT | DDBLTFAST_NOCOLORKEY);

			SpriteRect.left = Frame * 100;
			SpriteRect.top = 0;
			SpriteRect.right = SpriteRect.left + 100;
			SpriteRect.bottom = 70;

			BackScreen->BltFast(MouseX - 50, MouseY - 35, SpriteImage, &SpriteRect, DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);

			if (click == true){
			// enter animation code here!
			Count = Count + 1;
			if (Count < 5){
				Frame = Count;
			}
			else{
				Count = 0;
				Frame = Count;
				click = false;
			}
		}
		if (gFullScreen)
			RealScreen->Flip(NULL, DDFLIP_WAIT);
		else{
			RECT WinRect, SrcRect = { 0, 0, gWidth, gHeight };

			GetWindowRect(MainHwnd, &WinRect);
			RealScreen->Blt(&WinRect, BackScreen, &SrcRect, DDBLT_WAIT, NULL);
		}	
}

int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    MSG msg;

    if ( !_GameMode(hInstance, nCmdShow, 640, 480, 32) ) return FALSE;

    SpriteImage = DDLoadBitmap( DirectOBJ, "sprite_char.BMP", 0, 0 );
    BackGround  = DDLoadBitmap( DirectOBJ, "sprite_back.BMP", 0, 0 );

	if (_InitDirectSound())
	{
		Sound[0] = SndObjCreate(SoundOBJ, "MUSIC.WAV", 1);
		Sound[1] = SndObjCreate(SoundOBJ, "LAND.WAV", 2);
		Sound[2] = SndObjCreate(SoundOBJ, "GUN1.WAV", 2);
		Sound[3] = SndObjCreate(SoundOBJ, "KNIFE1.WAV", 2);
		Sound[4] = SndObjCreate(SoundOBJ, "DAMAGE1.WAV", 2);
		Sound[5] = SndObjCreate(SoundOBJ, "DAMAGE2.WAV", 2);
		SndObjPlay(Sound[0], DSBPLAY_LOOPING);
	}

    DDSetColorKey( SpriteImage, RGB(0,0,0) );
	
		SetTimer(MainHwnd, 1, 10, _GameProc);
	

    while ( !_GetKeyState(VK_ESCAPE) )
    {
        if ( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
        {
            if ( !GetMessage(&msg, NULL, 0, 0) ) return msg.wParam;

            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
		else
		{
			if (_GetKeyPush('1')) _Play(1);
			if (_GetKeyPush('2')) _Play(2);
			if (_GetKeyPush('3')) _Play(3);
			if (_GetKeyPush('4')) _Play(4);
			if (_GetKeyPush('5')) _Play(5);
		}
    }
    DestroyWindow( MainHwnd );

    return TRUE;
}
