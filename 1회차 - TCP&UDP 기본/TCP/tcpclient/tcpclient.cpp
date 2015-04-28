#include <stdio.h>
#include <WinSock2.h>
void main()
{
	WSADATA wsaData;
	SOCKET socketConnect;
	struct sockaddr_in serverAddr;

	::WSAStartup( 0x202, &wsaData );

//	::socket();
	socketConnect= ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//  접속할 서버의 정보를 설정한다.
	::memset( &serverAddr, 0, sizeof( serverAddr ) );
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::inet_addr( "127.0.0.1" ); //::htonl( INADDR_LOOPBACK  );
	serverAddr.sin_port = ::htons(8600);

	::connect(socketConnect, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	//  서버와 통신을 한다.
	while(1){
		char sendBuffer[127] = "Test client message...";
		char recvBuffer[127];
		int sentBytes, recvBytes;

		printf(">");
		gets(sendBuffer);

		sentBytes = ::send(socketConnect, sendBuffer, ::strlen(sendBuffer)+1, 0);
		
		printf( "%d bytes sent.\n", sentBytes );

		recvBytes = ::recv(socketConnect, recvBuffer, 100, 0);
		printf( "%d bytes Received: %s\n", recvBytes, recvBuffer );
	}

	::shutdown( socketConnect, SD_BOTH );
	::closesocket( socketConnect );
	::WSACleanup();
}
