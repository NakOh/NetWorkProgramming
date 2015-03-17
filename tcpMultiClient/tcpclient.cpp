#include <stdio.h>
#include <WinSock2.h>

int  RecvBytes=0;

DWORD WINAPI NetReceive(LPVOID socketConnect)
{
	char recvBuffer[127];


	while(1){
		RecvBytes = ::recv((SOCKET)socketConnect, recvBuffer, 127, 0 );
		if(RecvBytes<1) 
			exit(0);
		printf( "< %d bytes Received: %s\n", RecvBytes, recvBuffer );
	}

	return NULL;
}


void main()
{
	WSADATA wsaData;
	SOCKET socketConnect;
	struct sockaddr_in serverAddr;

	::WSAStartup( 0x202, &wsaData );

	socketConnect = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( socketConnect == INVALID_SOCKET )
	{
		printf( "Cannot create socket !!\n" );
		return;
	}

	//  접속할 서버의 정보를 설정한다.
	::memset( &serverAddr, 0, sizeof( serverAddr ) );
	serverAddr.sin_family		= AF_INET;
	serverAddr.sin_addr.s_addr	= ::inet_addr( "127.0.0.1" ); //::htonl(INADDR_LOOPBACK); 
	serverAddr.sin_port			= ::htons( 8600 );

	if( ::connect( socketConnect, ( struct sockaddr* )&serverAddr, sizeof( serverAddr ) ) == SOCKET_ERROR )
	{
		printf( "Cannot connect to server !!\n" );
		return;
	}

	// create thread for receive
	CreateThread(NULL, 0, NetReceive, (void *) socketConnect, THREAD_PRIORITY_NORMAL, NULL);

	//  서버와 통신을 한다.
	while(1){
		char sendBuffer[127] = "Test client message...";
		int sentBytes;


		gets(sendBuffer);

		sentBytes = ::send( socketConnect, sendBuffer, ::strlen( sendBuffer ) + 1, 0 );
		printf( "> %d bytes sent.\n", sentBytes );

		if(RecvBytes<0) 
			break;
	}

	::shutdown( socketConnect, SD_BOTH );
	::closesocket( socketConnect );
	::WSACleanup();
}
