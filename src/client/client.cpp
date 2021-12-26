// client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "message.h"
#include "response.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 2048
#pragma comment(lib, "ws2_32.lib")

int showMenu();

int main(int argc, char *argv[])
{
	//Step 01: check argv
	struct in_addr ipAddr;
	if (argc != 3 || atoi(argv[2]) < 1 || inet_pton(AF_INET, argv[1], &ipAddr) != 1) {
		printf("Input not correcly!\n");
		return 0;
	}

	//Step 02: start up winsock
	WORD wVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(wVersion, &wsaData) == SOCKET_ERROR) {
		printf("Version is not supported!\n");
		return 0;
	};

	//Step 03: construct socket 
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Don't construct socket with error code: %d\n", WSAGetLastError());
		return 0;
	}
	// Set time-out for receive
	//int tv = 800;
	//setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char *)(&tv), sizeof(int));

	//Step 04: Specify server address
	sockaddr_in serverAddr;
	int serverAddrLen = sizeof(serverAddr);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! Can not connect server.\n");
		return 0;
	}

	//Step 6: Communicate with server
	char buff[BUFFER_SIZE];
	int opt = -1, tmp, rt, seq = 0;
	bool press;
	while (1) {
		if(seq == 0) opt = showMenu();
		press = true;
		switch (opt)
		{
		case -1:
			closesocket(client);
			exit(0);
		case 1:
			//register message
			rt = getRG_message(buff, sizeof buff);
			if (rt <= 0) continue;
			break;
		case 2://login message
			rt = getLI_message(buff, sizeof buff);
			if (rt <= 0) continue;
			break;
		case 3://add address
			rt = getAA_message(buff, sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 4:
			rt = getRA_message(buff,sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 5://show address
			rt = getGA_message(buff, sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 6://share address
			rt = getSF_message(buff, sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 7://backup categary
			rt = getBK_message(buff, sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 8://restore categary
			rt = getRS_message(buff, sizeof buff, seq, press);
			if (rt <= 0) continue;
			break;
		case 9:
			printf("WARNING: friend address can be lost(y/n):");
			tmp = _getch();
			printf("\n");
			if (tmp != 'y') {
				rt = 0;
				press = false;
				break;
			}
			rt = getLO_message(buff, sizeof buff);
			if (rt <= 0) continue;
			break;
		default:
			printf("option is error\n");
			continue;
		}
		int ret = send(client, buff, rt, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error %d: Can not send message.\n", WSAGetLastError());
			exit(0);
		}
		//receive response
		if(ret>0)
		rt = receive(client);
		if (rt == -1 && seq != 0) {
			seq = 0;
			press = true;
		}
		if (press) {
			printf("Press to continue:");
			_getch();
			printf("\n\n");
		}
	}
	// Step 00: Close socket
	closesocket(client);
	
	WSACleanup();
	return 0;
}

//show menu and return option number
int showMenu() {
	int check;
	cout << "********Menu********" << endl;
	cout << "1. Register" << endl;
	cout << "2. Log in" << endl;
	cout << "3. Add address" << endl;
	cout << "4. Remove address" << endl;
	cout << "5. Show address" << endl;
	cout << "6. Share address" << endl;
	cout << "7. Backup categary" << endl;
	cout << "8. Restore categary" << endl;
	cout << "9. Log out" << endl;
	cout << "-1.Exit" << endl;
	cout << "Please input the number:";
	string s;
	getline(cin, s);
	check = atoi(s.c_str());
	if (check<-1 || check>9 || check == 0) {
		cout << "The number is not correctly" << endl;
		cout << "Please press to continue:";
		_getch();
		cout << "\n\n";
		return showMenu();
	}
	return check;
}