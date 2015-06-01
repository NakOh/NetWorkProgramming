#include <windows.h>
#include <windowsx.h>
#include <dsound.h>
#include <ddraw.h>
#include "ddutil.h"
#include "dsutil.h"

#define _GetKeyState( vkey ) HIBYTE(GetAsyncKeyState( vkey ))
#define _GetKeyPush( vkey )  LOBYTE(GetAsyncKeyState( vkey ))



LPDIRECTSOUND       SoundOBJ = NULL;
LPDIRECTSOUNDBUFFER SoundDSB = NULL;
DSBUFFERDESC        DSB_desc;


HWND MainHwnd;
HSNDOBJ Sound[10];


long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        case    WM_DESTROY      :   PostQuitMessage( 0 );
                                    break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}


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

void _Play( int num )
{
    SndObjPlay( Sound[num], NULL );
}


int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    MSG msg;
    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = GetStockBrush(GRAY_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "dSOUND";
    RegisterClass( &wc );

    int xx, yy;

    xx = (GetSystemMetrics(SM_CXSCREEN) - 640) / 2;
    yy = (GetSystemMetrics(SM_CYSCREEN) - 480) / 2;

    MainHwnd = CreateWindow( "dSOUND", "게임 사운드", 
		WS_OVERLAPPEDWINDOW, xx, yy, 640, 480, NULL, NULL, hInstance, NULL );

    if ( !MainHwnd ) return FALSE;

    SetFocus( MainHwnd );
    ShowWindow( MainHwnd, nCmdShow );
    UpdateWindow( MainHwnd );


    if ( _InitDirectSound() )
    {
        Sound[0] = SndObjCreate(SoundOBJ,"MUSIC.WAV",1);
        Sound[1] = SndObjCreate(SoundOBJ,"LAND.WAV",2);
        Sound[2] = SndObjCreate(SoundOBJ,"GUN1.WAV",2);
        Sound[3] = SndObjCreate(SoundOBJ,"KNIFE1.WAV",2);
        Sound[4] = SndObjCreate(SoundOBJ,"DAMAGE1.WAV",2);
        Sound[5] = SndObjCreate(SoundOBJ,"DAMAGE2.WAV",2);
        SndObjPlay( Sound[0], DSBPLAY_LOOPING );
    }

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
            if ( _GetKeyPush('1') ) _Play( 1 );
            if ( _GetKeyPush('2') ) _Play( 2 );
            if ( _GetKeyPush('3') ) _Play( 3 );
            if ( _GetKeyPush('4') ) _Play( 4 );
            if ( _GetKeyPush('5') ) _Play( 5 );
        }
    }
    DestroyWindow( MainHwnd );

    return TRUE;
}
