#include <stdio.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#include "wtpipv6.h"
#include "wspiapi.h"

#include "Packet.h"

#define MAX_CLIENT 5
#define MAX_MESSAGE 5

//**********************************************************************
void usage(char *progname)
{
    fprintf(stderr, "usage: %s [-n name] [-p port] \n", progname);
    fprintf(stderr, "   -n name    Host name to resolve, [127.0.0.1] \n");
    fprintf(stderr, "   -p port    Port number to resolve, [8600] \n");
  
    ExitProcess(-1);
}


int resolveAddr(int argc, char **argv, char *serverName, char *serverPort)
{
	int count, rc, i;

	serverName[0]=0;
	serverPort[0]=0;

    for(i=1; i < argc ;i++)
    {
        if ((argv[i][0] != '-') && (argv[i][0] != '/') && (strlen(argv[i]) < 2))
            usage(argv[0]);

        switch (tolower(argv[i][1]))
        {

            case 'n':       // name to resolve
                if (i+1 >= argc)
                    usage(argv[0]);
                strcpy(serverName, argv[++i]);
                break;

            case 'p':       // port/service to resolve
                if (i+1 >= argc)
                    usage(argv[0]);
                strcpy(serverPort, argv[++i]);
                break;


            default:
                usage(argv[0]);
                break;
        }
    }
	if(serverName[0]==0)
		strcpy(serverName, "127.0.0.1");
	if(serverPort[0]==0)
		strcpy(serverPort, "8600");

	printf("** Resolve Address %s:%s \n", serverName, serverPort);




	return 1;
}
//************************************************************************


DWORD WINAPI NetReceive(LPVOID socketConnect)
{
	char recvBuffer[127];
	int  recvBytes;

	Packet	recvPacket;
	char	receiveBuffer[PACKETBUFFERSIZE];
	int		receivedPacketSize=0;
	int     bufSize;

	while(1){

		///////////////////////////////////////////////////////
		// replace codes below into accurately reveiced routine using "Packet"
		bufSize = PACKETBUFFERSIZE - receivedPacketSize;
		recvBytes = recv((SOCKET)socketConnect, &(receiveBuffer[receivedPacketSize]), bufSize, 0);
		receivedPacketSize += recvBytes;
		while (receivedPacketSize > 0){
			recvPacket.copyToBuffer(receiveBuffer, receivedPacketSize);
			int packetlength = (int)recvPacket.getPacketSize();
			if (receivedPacketSize >= packetlength){
				//  Parsing, main routine 
				recvPacket.readData(recvBuffer, recvPacket.getDataFieldSize());
				printf("(%d Bytes, ID=%d) %s\n", recvPacket.getDataFieldSize(), recvPacket.id(), recvBuffer);
				
				receivedPacketSize -= packetlength;
				if (receivedPacketSize > 0)
				{
					::CopyMemory(recvBuffer, (receiveBuffer + recvPacket.getPacketSize()), receivedPacketSize);
					::CopyMemory(receiveBuffer, recvBuffer, receivedPacketSize);
				}
			}
		
		else{
			break;
		}
		}

		///////////////////////////////////////////////////////

	}
	
	return NULL;
}
	


void main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET socketConnect[MAX_CLIENT];
	struct sockaddr_in serverAddr;
	int  k;
	HANDLE handleThread[MAX_CLIENT];


	::WSAStartup( 0x202, &wsaData );


	for(k=0;k<MAX_CLIENT;k++){
		socketConnect[k] = INVALID_SOCKET;
		socketConnect[k] = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
		if( socketConnect[k] == INVALID_SOCKET )
		{
			printf( "Cannot create socket %d !!\n", k);
			continue;
		}
	}
	


	//******************************* Address and port resolve
	char serverName[120], serverPort[120];

	if(resolveAddr(argc, argv, serverName, serverPort)<1){
		printf("*** Unable to resolve server name !\n");
	    ExitProcess(-1);
	}

	//  접속할 서버의 정보를 설정한다.
	::memset( &serverAddr, 0, sizeof( serverAddr ) );
	serverAddr.sin_family		= AF_INET;
	serverAddr.sin_addr.s_addr	= ::inet_addr( serverName );
	serverAddr.sin_port			= ::htons( atoi(serverPort) );

	//********************************************************

	for(k=0;k<MAX_CLIENT;k++){
		if( socketConnect[k] != INVALID_SOCKET){
			if(::connect( socketConnect[k], ( struct sockaddr* )&serverAddr, sizeof( serverAddr ) ) == SOCKET_ERROR )
			{
				printf( "Cannot connect to server %d !!\n", k);
				socketConnect[k] = INVALID_SOCKET;
				continue;
			}
			else{
				// create thread for receive
				handleThread[k]=CreateThread(NULL, 0, NetReceive, (void *) socketConnect[k], THREAD_PRIORITY_NORMAL, NULL);
			}
		}
	}

	Packet sendPacket;

	//  서버와 통신을 한다.
	int count=0;
	while(count++ < MAX_MESSAGE){
		char sendBuffer[127];
		int sentBytes;

		for(k=0;k<MAX_CLIENT;k++){

			if(socketConnect[k] != INVALID_SOCKET){
				sprintf(sendBuffer, "%d> Test Message to server", k);
				if(count==MAX_MESSAGE)
					strcat(sendBuffer, ".");

				///////////////////////////////////////////////////////////

				sendPacket.clear();
				sendPacket.id(1000+count);
				sendPacket.writeData( sendBuffer, strlen(sendBuffer)+1 );

				sentBytes = ::send( socketConnect[k], sendPacket.getPacketBuffer(), sendPacket.getPacketSize(), 0 );
//				printf(">> send %d %d %s \n", sendPacket.id(), sendPacket.getPacketSize(), sendPacket.getPacketBuffer()+4 );

//				sentBytes = ::send( socketConnect[k], sendBuffer, ::strlen( sendBuffer ) + 1, 0 );

				///////////////////////////////////////////////////////////

				if(sentBytes<0){
					::shutdown( socketConnect[k], SD_BOTH );
					::closesocket( socketConnect[k] );
					socketConnect[k]=INVALID_SOCKET;
				}
			}
		}

//		Sleep(200);
	}


	::WaitForMultipleObjects( MAX_CLIENT, handleThread, TRUE, INFINITE );


	for(k=0;k<MAX_CLIENT;k++){
		::shutdown( socketConnect[k], SD_BOTH );
		::closesocket( socketConnect[k] );
	}
	::WSACleanup();

	printf("Server Connection Closed !\n");
	char temp[120];
	gets(temp);
}
