#pragma once
#include <stdio.h>
#include <WinSock2.h>
#define BUFFER_SIZE 2048

//show error notice from error code
void getErrorNotice(int error_code) {
	switch (error_code) {
	case 0:
		printf("Error: unspecified message\n");
		break;
	case 1:
		printf("Error: username is not valid\n");
		break;
	case 2:
		printf("Error: password is not valid\n");
		break;
	case 3:
		printf("Error: username or password is wrong\n");
		break;
	case 4:
		printf("Error: account haved login at another\n");
		break;
	case 5:
		printf("Error: didn't log in yet\n");
		break;
	case 6:
		printf("Error: haven't log out\n");
		break;
	case 11:
		printf("Error: categary name is not valid\n");
		break;
	case 21:
		printf("Error: address number is not exists\n");
		break;
	default:
		printf("common error\n");
		break;
	}
};

//get length from place possition of message
unsigned short getLength(char place[]) {
	void *p = place;
	unsigned short Host_num = ntohs(*((unsigned short*)p));
	return Host_num;
}

//show suggest categary from the responses 
void showSuggestCategory(bool &first_res, char res[], int res_len) {
	static bool enter;
	if (first_res) {
		first_res = false;
		enter = false;
		printf("===Show suggested categary===\n");
		if (res_len != 0) printf("\t");
	}
	for (int i = 3; i < res_len + 3; i++) {
		if (res[i] == '\n') {
			enter = true;
			printf("\n");
		}
		else if (enter) {
			enter = false;
			printf("\t%c", res[i]);
		}
		else {
			printf("%c", res[i]);
		}
	}
}

//show friend address from the responses
void showFriendAddress(bool &first_res, char res[], int res_len) {
	static bool enter;
	static int num;
	if (first_res) {
		enter = false;
		num = 1;
		first_res = false;
		printf("+++Log in is okey+++\n");
		if (res_len != 0) {
			printf("===Show friend address===\n");
			printf("format:num friend categary |^| address\n");
			printf("%d\t", num++);
		}
	}
	for (int i = 3; i < res_len + 3; i++) {
		switch (res[i]) {
		case 5:
			printf(" |^| ");
			enter = false;
			break;
		case '\n':
			printf("\n");
			enter = true;
			break;
		default:
			if (enter) printf("%d\t", num++);
			printf("%c", res[i]);
			enter = false;
			break;
		}
	}
}

//show personal categary from the responses
void showCategory(bool &first_res, char res[], int res_len) {
	static bool enter;
	if (first_res) {
		enter = false;
		first_res = false;
		printf("===Show categary===\n");
		if (res_len != 0) {
			printf("\t");
		}
	}
	for (int i = 3; i < res_len + 3; i++) {
		if (res[i] == '\n') {
			enter = true;
			printf("\n");
		}
		else if (enter) {
			enter = false;
			printf("\t%c", res[i]);
		}
		else {
			printf("%c", res[i]);
		}
	}
}

//show address content in responses
void showAddress(bool &first_res, char res[], int res_len) {
	static bool enter;
	static int num;
	if (first_res) {
		enter = false;
		num = 1;
		first_res = false;
		printf("===Show address===\n");
		if (res_len != 0) {
			printf("prefix - is myself\n");
			printf("%d\t", num++);
		}
	}
	for (int i = 3; i < res_len + 3; i++) {
		switch (res[i]) {
		case '\n':
			printf("\n");
			enter = true;
			break;
		case '+':
		case '-':
			if (enter) {
				enter = false;
				printf("%d\t%c", num++, res[i]);
			}
			else {
				printf("%c", res[i]);
			}
			break;
		default:
			printf("%c", res[i]);
			break;
		}
	}
}

//show backed up category in response
void showBackedupCategory(bool &first_res, char res[], int res_len) {
	static bool enter;
	if (first_res) {
		enter = false;
		first_res = false;
		printf("===Show back up===\n");
		if (res_len != 0) printf("\t");
	}
	for (int i = 3; i < res_len + 3; i++) {
		if (res[i] == '\n') {
			enter = true;
			printf("\n");
		}
		else if (enter) {
			printf("\t%c", res[i]);
		}
		else {
			printf("%c", res[i]);
		}
	}
}


//receives the responses from server and notifies them to user
//return the opcode of the responses
int receive(SOCKET client) {
	static char res[BUFFER_SIZE];
	bool first_res = true;
	int ret, res_len, opcode;
	while (true) {
		// receive response from server
		ret = recv(client, res, 3, 0);
		if (ret != 3) {
			printf("ERROR: length of response at lease 3\n");
			exit(0);
		}
		opcode = res[0];
		res_len = getLength(res + 1);
		if (res_len != 0) {
			if (recv(client, res + 3, res_len, 0) != res_len) {
				printf("ERROR: receive length byte\n");
				exit(0);
			}
		}
		switch (opcode) {
		case -1:
			if (res_len != 1) {
				printf("Erorr opcode=1 length != 1\n");
				return -1;
			}
			getErrorNotice(res[3]);
			return -1;

		case 0:
			if (res_len != 0) {
				printf("Erorr opcode=0 length != 0\n");
				return -1;
			}
			printf("+++The command is done+++\n");
			return 0;

		case 1:
			showSuggestCategory(first_res, res, res_len);
			break;
		case 2:
			showFriendAddress(first_res, res, res_len);
			break;
		case 3:
			showCategory(first_res, res, res_len);
			break;
		case 4:
			showAddress(first_res, res, res_len);
			break;
		case 5:
			showBackedupCategory(first_res, res, res_len);
			break;
		}
		if (res_len == 0) return opcode;
	}
}

//show original response
int getOriginalResponse(SOCKET &client) {
	printf("original\n");
	static char recvBuff[BUFFER_SIZE];
	int ret;
	int point = 0;
	while (true) {
		int i;
		// receive response from server
		for (i = 0; i < 5; i++) {
			ret = recv(client, recvBuff, 3, 0);
			int length = getLength(recvBuff + 1);
			if (length != 0)
				ret += recv(client, recvBuff + 3, length, 0);
			//ret = recv(client, recvBuff, BUFFER_SIZE, 0);
			if (ret == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAETIMEDOUT) {
					if (i == 4)printf("Time out!\n");
					continue;
				}
				printf("Error %d: Can't receive message.\n", WSAGetLastError());
				exit(0);
			}
			break;
		}
		printf("receive message\n");
		if (ret < 3) {
			printf("response length is at lease 3\n");
			return -1;
		}
		int opcode = recvBuff[0];
		int length = (recvBuff[1] << 8) + recvBuff[2];
		printf("receive response from server:\n");
		printf("length of reponse: %d\n", ret);
		printf("opcode: %d\n", opcode);
		printf("length param: %d\n", length);

		if (length + 3 > ret) {
			printf("lenth: %d > recv_len: %d\n", length, ret);
			return -1;
		}
		if (opcode == -1 && length == 1) printf("error code %d\n", recvBuff[3]);
		else {
			for (int i = 3; i < length + 3; i++) {
				printf("%d ", recvBuff[i]);
			}
		}

		if (opcode == -1 || opcode == 0 || length == 0) {
			printf("\ndone\n");
			return opcode;
		}
	}
}