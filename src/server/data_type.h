#pragma once

#include <WinSock2.h>
#include <string.h>
#include <string>
#include <vector>
#include <queue>

using namespace std;

#define RECEIVE 0
#define SEND 1
#define BUFFSIZE 2048

/*The user structure save account data*/
typedef struct user {
	string username;
	string password;
	bool islogin;// init is false
} user;

/*The users is account list*/
typedef vector<user> users;

/**
* The ClientInfo structure contains infomation of client's connect session
* @variable login_place is place of login account in userList
* @variable fa_bytes is size of fa.txt file when loged in
* @variable response contains the reponses to client
*/
typedef struct ClientInfo {
	bool islogin; //init is false
	int login_place;
	char username[260];
	int fa_bytes; //the bytes of the fa.txt file is get at log in
	int recvBytes;
	char recvBuffer[BUFFSIZE];
	queue<char*> responses;
} ClientInfo;

/*The SocketInfo structure contains information of the socket communicating with client*/
typedef struct SocketInfo {
	WSAOVERLAPPED overlapped;
	SOCKET socket;
	int operation;//init is receive
	WSABUF dataBuf;
	int sendBytes;
	int sentBytes;
	ClientInfo clientInfo;
}SocketInfo;

/**
* The accountLocation function defines user location usr in userList
* @return -1 if usr not contains in userList
* @return else is location of usr in userList
*/
int accountLocation(users &userList, user &usr) {
	for (int i = 0; i < (int)userList.size(); i++) {
		if (usr.username == userList[i].username && usr.password == userList[i].password)
			return i;
	}
	return -1;
}

/**
* The initSockInfo function constructs a socketinfo and event and put connSock on n in array
* @param	socks		An array of pointers of socket information struct
* @param	events		An WSAEVENT array
* @param	n			Index of the construct socket
* @param	connSock	An socket
*/
void initSockInfo(SocketInfo *socks[],int n,SOCKET &connSock) {
	// Append connected socket to the array of SocketInfo
	socks[n] = new SocketInfo;
	socks[n]->socket = connSock;
	socks[n]->dataBuf.buf = socks[n]->clientInfo.recvBuffer;
	socks[n]->dataBuf.len = BUFFSIZE;
	socks[n]->operation = RECEIVE;
	socks[n]->clientInfo.islogin = false;
}

/**
* The removeSockInfo function remove a SocketInfo 'rm_socket' in socks
* @param	socks		An array of pointers of socket information struct
* @param	n_socks		Index of the construct socket
*/
void removeSockInfo(SocketInfo *socks[], int n_socks, SocketInfo *rm_sock) {
	//WSACloseEvent(events[n]);
	ClientInfo *clientInfo = &(rm_sock->clientInfo);
	while (!clientInfo->responses.empty()) {
		delete(clientInfo->responses.front());
		clientInfo->responses.pop();
	}
	int index;
	for (index = 0; index < n_socks; index++)
		if (socks[index]->socket == rm_sock->socket)
			break;
	closesocket(socks[index]->socket);
	delete(socks[index]);
	socks[index] = socks[n_socks - 1];
}

/*Convert string to wchar_t array*/
void toWchar_t(const string &src, wchar_t *des, int des_size) {
	for (int i = 0; i < (int)src.size(); i++) {
		if (des_size > i) {
			des[i] = (wchar_t)src[i];
		}
		else break;
	}
	if ((int)src.size() > des_size) {
		des[des_size - 1] = '\0';
	}
	else {
		des[src.size()] = '\0';
	}
}

/*Convert char array to wchar_t array*/
void toWchar_t(const char *src, wchar_t *des, int des_size) {
	int src_len = strlen(src);
	for (int i = 0; i < src_len; i++) {
		if (des_size > i) {
			des[i] = (wchar_t)src[i];
		}
		else break;
	}
	if (src_len > des_size) {
		des[des_size - 1] = '\0';
	}
	else {
		des[src_len] = '\0';
	}
}

/*return true if all the charactors in array is number*/
bool checkNum(const char s[]) {
	int len = strlen(s);
	if (len == 0) return false;
	for (int i = 0; i < len; i++) {
		if (s[i]<'0' || s[i]>'9') return false;
	}
	return true;
}