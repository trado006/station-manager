// OverlappedCompletionRoutineServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WS2tcpip.h>
#include "data_type.h"
#include "response.h"

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define PORT 5500
#define MAX_CLIENT 1024
#define USER_FILE "account.txt"

void CALLBACK workerRoutine(DWORD error, DWORD transferredBytes, LPWSAOVERLAPPED overlapped, DWORD inFlags);

SocketInfo* sockInfos[MAX_CLIENT];
users userList;

SOCKET acceptSocket;
int nClients = 0;
CRITICAL_SECTION criticalSection;

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKET listenSocket;
	SOCKADDR_IN serverAddr, clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	INT ret;
	//WSAEVENT acceptEvent;

	InitializeCriticalSection(&criticalSection);

	if ((ret = WSAStartup((2, 2), &wsaData)) != 0) {
		printf("WSAStartup() failed with error %d\n", ret);
		WSACleanup();
		return 1;
	}

	if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	serverAddr.sin_port = htons(PORT);
	if (bind(listenSocket, (PSOCKADDR)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	if (listen(listenSocket, 20)) {
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	printf("Server started!\n");

	if (!prepareUserList(USER_FILE, userList)) {
		printf("read user list error\n");
		exit(0);
	}
	printf("userlist %d\n", userList.size());
	
	while (TRUE) {
		if ((acceptSocket = accept(listenSocket, (PSOCKADDR)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
			printf("accept() failed with error %d\n", WSAGetLastError());
			return 1;
		}
		
		EnterCriticalSection(&criticalSection);

		if (nClients == MAX_CLIENT) {
			printf("Too many clients.\n");
			closesocket(acceptSocket);
			LeaveCriticalSection(&criticalSection);
			continue;
		}

		// Create a socket information structure to associate with the accepted socket
		initSockInfo(sockInfos, nClients, acceptSocket);

		DWORD flags = 0, recvBytes;
		SocketInfo* sockInfo = sockInfos[nClients];

		if (WSARecv(sockInfo->socket, &(sockInfo->dataBuf), 1, &recvBytes, &flags, &(sockInfo->overlapped), workerRoutine) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return 1;
			}
		}

		printf("Socket %d got connected...\n", acceptSocket);
		nClients++;
		LeaveCriticalSection(&criticalSection);
	}
	return 0;
}

void CALLBACK workerRoutine(DWORD error, DWORD transferredBytes, LPWSAOVERLAPPED overlapped, DWORD inFlags)
{
	DWORD sendBytes, recvBytes;
	DWORD flags;

	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	SocketInfo *sockInfo = (SocketInfo *)overlapped;
	ClientInfo *clientInfo = &(sockInfo->clientInfo);

	if (error != 0)
		printf("I/O operation failed with error %d\n", error);

	else if (transferredBytes == 0)
		printf("Closing socket %d\n\n", sockInfo->socket);

	if (error != 0 || transferredBytes == 0) {
		//Find and remove socket
		EnterCriticalSection(&criticalSection);

		if (clientInfo->islogin) {
			userList[clientInfo->login_place].islogin = false;
		}
		removeSockInfo(sockInfos, nClients, sockInfo);
		nClients--;

		LeaveCriticalSection(&criticalSection);

		return;
	}

	// Check to see if the recvBytes field equals zero. If this is so, then
	// this means a WSARecv call just completed so update the recvBytes field
	// with the transferredBytes value from the completed WSARecv() call	
	if (sockInfo->operation == RECEIVE) {
		clientInfo->recvBytes = transferredBytes;	//the number of bytes which is received from client
													//process recvbuff
		getResponses(*clientInfo, userList);
		//config parameter
		queue<char*> &responses = clientInfo->responses;
		unsigned int length = getLength(responses.front() + 1);
		sockInfo->sendBytes = length + 3;
		sockInfo->dataBuf.buf = responses.front();
		sockInfo->dataBuf.len = length + 3;
		sockInfo->sentBytes = 0;					//the number of bytes which is sent to client
		sockInfo->operation = SEND;				//set operation to send reply message
	}
	else {
		sockInfo->sentBytes += transferredBytes;
	}

	if (sockInfo->sendBytes > sockInfo->sentBytes) {
		sockInfo->dataBuf.buf = clientInfo->responses.front() + sockInfo->sentBytes;
		sockInfo->dataBuf.len = sockInfo->sendBytes - sockInfo->sentBytes;
		sockInfo->operation = SEND;
		if (WSASend(sockInfo->socket, &(sockInfo->dataBuf), 1, &sendBytes, 0, &(sockInfo->overlapped), workerRoutine) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSASend() failed with error %d\n", WSAGetLastError());
				if (clientInfo->islogin)
					userList[clientInfo->login_place].islogin = false;
				removeSockInfo(sockInfos, nClients, sockInfo);
				nClients--;
			}
		}
	}
	else {
		delete(clientInfo->responses.front());
		clientInfo->responses.pop();
		if (clientInfo->responses.empty()) {
			sockInfo->operation = RECEIVE;
			sockInfo->dataBuf.buf = clientInfo->recvBuffer;
			sockInfo->dataBuf.len = BUFFSIZE;
			flags = 0;

			if (WSARecv(sockInfo->socket, &(sockInfo->dataBuf), 1, &recvBytes, &flags, &(sockInfo->overlapped), workerRoutine) == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					printf("WSARecv() failed with error %d\n", WSAGetLastError());
					if (clientInfo->islogin)
						userList[clientInfo->login_place].islogin = false;
					removeSockInfo(sockInfos, nClients, sockInfo);
					nClients--;
				}
			}
		}
		else {
			queue<char*> &responses = clientInfo->responses;
			int length = getLength(responses.front() + 1);
			sockInfo->sendBytes = length + 3;
			sockInfo->dataBuf.buf = clientInfo->responses.front();
			sockInfo->dataBuf.len = length + 3;
			sockInfo->sentBytes = 0;					//the number of bytes which is sent to client
			sockInfo->operation = SEND;
			// Post an overlpped I/O request to begin receiving data on the socket
			if (WSASend(sockInfo->socket, &(sockInfo->dataBuf), 1, &sendBytes, 0, &(sockInfo->overlapped), workerRoutine) == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					printf("WSASend() failed with error %d\n", WSAGetLastError());
					if (clientInfo->islogin)
						userList[clientInfo->login_place].islogin = false;
					removeSockInfo(sockInfos, nClients, sockInfo);
					nClients--;
				}
			}
		}
	}
}