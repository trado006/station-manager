#pragma once
#include "data_type.h"

/*get length from place possition of message*/
unsigned short getLength(char place[]) {
	void *p = place;
	unsigned short Host_num = ntohs(*((unsigned short*)p));
	return Host_num;
}

/*get mes_type from mes_type possition of message*/
int getMes_type(char mes_type[]) {
	return ((unsigned char)mes_type[0] << 8) + (unsigned char)mes_type[1];
}

/**
* The getAccount function get username and password in LI, LO message and save into user structure
* @return -1 success
* @return  0 message not valid
* @return  1 username not valid
* @return  2 password not valid
*/
int getAccount(char mess[], int mess_len, user &usr) {
	if (mess_len<6 || mess[4] != ' ') {
		return 0;
	}
	int separation = mess_len;
	for (int i = 5; i < mess_len; i++) {
		if (mess[i] == '\n') {
			separation = i;
			break;
		}
		if (mess[i] < 33) {
			return 1;
		}
	}
	if (separation == 5 || separation >= mess_len - 1) {
		return 0;
	}
	//check password syntax
	for (int i = separation + 1; i < mess_len; i++) {
		if (mess[i] < 33) {
			return 2;
		}
	}
	usr.islogin = false;
	usr.username.clear();
	usr.password.clear();
	char *username = mess + 5;
	mess[separation] = '\0';
	usr.username.append(username);
	char *password = mess + separation + 1;
	char tmp[2] = { mess[mess_len - 1],'\0' };
	mess[mess_len - 1] = '\0';
	usr.password.append(password);
	usr.password.append(tmp);
	return -1;
}
/**
* The getAddress get categary and address in AA, RA message
* @return -1 success
* @return  0 message not valid
*/
int getAddress(char *mess, int mess_len, string &categary, string &address) {
	if (mess_len<6 || mess[4] != ' ') {
		return 0;
	}
	int separation = mess_len;
	for (int i = 5; i < mess_len; i++) {
		if (mess[i] == '\n') {
			separation = i;
			break;
		}
	}
	if (separation < 6 || separation >= mess_len - 1) {
		return 0;
	}
	mess[separation] = '\0';
	categary.empty();
	categary.append(mess + 5);
	address.empty();
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	address.append(mess + separation + 1);
	address.push_back(tmp);
	return -1;
}

/**
* The getSharedAddress get friend name, categary and address in SF message
* @return -1 success
* @return  0 message not valid
*/
int getSharedAddress(char *mess, int mess_len, string &username, string &categary, string &address) {
	if (mess_len < 6 || mess[4] != ' ') {
		return 0;
	}
	int separation1 = mess_len, separation2 = mess_len;
	for (int i = 5; i < mess_len; i++) {
		if (mess[i] == '\n') {
			separation1 = i;
			break;
		}
	}
	for (int i = separation1 + 1; i < mess_len; i++) {
		if (mess[i] == '\n') {
			separation2 = i;
			break;
		}
	}
	mess[separation1] = '\0';
	username.empty();
	username.append(mess + 5);
	mess[separation2] = '\0';
	categary.empty();
	categary.append(mess + separation1 + 1);
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	address.append(mess + separation2 + 1);
	address.push_back(tmp);
	return -1;
}