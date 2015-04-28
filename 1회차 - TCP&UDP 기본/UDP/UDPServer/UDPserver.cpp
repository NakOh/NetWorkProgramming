#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <Winsock2.h>

#define  ECHOMAX	255

void main()
{
	WSADATA wsaData;
	SOCKET listenSocket;
	struct sockaddr_in echoServerAddr;
	struct sockaddr_in echoClientAddr;
	char echoBuffer[ECHOMAX];
	int receiveSize, clientAddrLen;

	//  ��Ʈ��ũ�� �ʱ�ȭ �Ѵ�.
	::WSAStartup( 0x202, &wsaData );

	//  UDP ������ �����Ѵ�.
//	::socket();
	listenSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//  sockaddr_in ����ü�� ������ �����Ѵ�.
	::memset( &echoServerAddr, 0, sizeof( echoServerAddr ) );
	echoServerAddr.sin_family = AF_INET;
	echoServerAddr.sin_addr.s_addr = ::inet_addr("127.0.0.1");
	echoServerAddr.sin_port = ::htons(8599);


//	::bind();
	::bind(listenSocket, (struct sockaddr*)&echoServerAddr, sizeof(echoServerAddr));
	while( 1 )
	{
		clientAddrLen = sizeof( echoClientAddr );

		//  Ŭ���̾�Ʈ�κ��� �޽����� �����ϱ⸦ ��ٸ���.
//		::recvfrom();
		receiveSize = ::recvfrom(listenSocket, echoBuffer, ECHOMAX, 0, (struct sockaddr*)&echoClientAddr, &clientAddrLen);
		if( receiveSize < 0 )
			continue;

		printf( "Handling client - %s\n>%d Bytes : %s\n", ::inet_ntoa( echoClientAddr.sin_addr ), receiveSize, echoBuffer );

		//  ���� �޽����� Ŭ���̾�Ʈ�� �ǵ��� ������.
//		::sendto();
		::sendto(listenSocket, echoBuffer, receiveSize, 0, (struct sockaddr*)&echoClientAddr, clientAddrLen);

	}

	//  ������ ��/��� ���۸� ���� ��Ȱ��ȭ ��Ų��.
	::shutdown( listenSocket, SD_BOTH );

	//  ���� �۾��� �����Ѵ�.
	::closesocket( listenSocket );

	::WSACleanup();
}