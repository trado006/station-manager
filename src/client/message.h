#pragma once
#include <iostream>
#include <string>
#include <WinSock2.h>
#define BUFFER_SIZE 2048
using namespace std;

/**
* The pasteLength function paste length param into header of message
*/
void pasteLength(unsigned short length, char mes[]) {
	unsigned short Net_num = htons(length);
	void *p = &Net_num;
	mes[0] = *((char*)p);
	mes[1] = *((char*)p + 1);
}

/**
* The getRG_message function get RG message contains register infomation
* @return length of message
*/
int getRG_message(char buff[], int buff_size) {
	int length;
	string username, password;
	cout << "Please input your username:";
	getline(cin, username);
	cout << "Please input your password:";
	getline(cin, password);
	length = username.size() + password.size() + 1 + 5;
	if (length > buff_size) {
		cout << "Error: input is longer than able\n";
		return 0;
	}
	pasteLength(length, buff);
	buff[2] = '\0';
	strcat_s(buff + 2, buff_size - 2, "RG ");
	strcat_s(buff + 2, buff_size - 2, username.c_str());
	strcat_s(buff + 2, buff_size - 2, "\n");
	strcat_s(buff + 2, buff_size - 2, password.c_str());
	return length;
}

/**
* The getLI_message function get login message
* @return length of message
*/
int getLI_message(char buff[], int buff_size) {
	int length = getRG_message(buff, buff_size);
	buff[2] = 'L'; buff[3] = 'I'; buff[4] = ' ';
	return length;
}

/**
* The getAA_message function get AA message to add new address
* @param	seq		is used if function have much message
* @param	press	is used to wait user press to continue
* @return	length	of message
*/
int getAA_message(char buff[], int buff_size, int &seq, bool &press) {
	static string category, address;
	if (seq == 0) {
		category.clear();
		address.clear();
		int length = 4;
		strcpy_s(buff + 2,BUFFER_SIZE-3,"SG");
		pasteLength(length, buff);
		seq = 1;
		return length;
	}
	if (seq == 1) {
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff);
		seq = 2;
		press = false;
		return length;
	}
	if (seq == 2) {
		int length;
		string category, address;
		printf("Please input your category:");
		getline(cin, category);
		printf("Note: input number to add friend address\n");
		printf("Please input your address or number:");
		getline(cin, address);
		length = category.size() + address.size() + 1 + 5;
		if (length > buff_size) {
			cout << "Error: input is longer than able\n";
			seq = 0;
			return 0;
		}
		pasteLength(length, buff);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "AA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		strcat_s(buff + 2, buff_size - 2, "\n");
		strcat_s(buff + 2, buff_size - 2, address.c_str());
		seq = 0;
		return length;
	}
	//default error
	seq = 0;
	return 0;
}

/**
* The getRA_message function get RM message to remove address
* @param	seq		is used if function have much message
* @param	press	is used to wait user press to continue
* @return	length	of message
*/
int getRA_message(char buff[], int buff_size, int &seq, bool &press) {
	static string category, num;
	if (seq == 0) {
		category.clear();
		num.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1) {
		int length;
		printf("Please input your category:");
		getline(cin, category);
		length = category.size() + 5;
		if (length > buff_size) {
			cout << "Error: input is longer than able\n";
			seq = 0;
			return 0;
		}
		pasteLength(length, buff);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "GA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 2;
		press = false;
		return length;
	}
	if (seq == 2) {
		int length;
		printf("Please input the number of address:");
		getline(cin, num);
		length = category.size() + num.size() + 1 + 5;
		if (length > buff_size) {
			cout << "Error: input is longer than able\n";
			seq = 0;
			return 0;
		}
		pasteLength(length, buff);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "RA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		strcat_s(buff + 2, buff_size - 2, "\n");
		strcat_s(buff + 2, buff_size - 2, num.c_str());
		seq = 0;
		return length;
	}
	//default
	seq = 0;
	return 0;
}

/**
* The getGA_message function get GA message to get all address in the one category
* @param	seq		is used if function have much message
* @param	press	is used to wait user press to continue
* @return	length	of message
*/
int getGA_message(char buff[], int buff_size, int &seq, bool &press) {
	static string category;
	if (seq == 0) {
		category.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1) {
		string category;
		cout << "Please input your category:";
		getline(cin, category);
		int length = 4 + 1 + category.size();
		if (length > buff_size) {
			seq = 0;
			return 0;
		}
		pasteLength(length, buff);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "GA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 0;
		return length;
	}
	seq = 0;
	return 0;
}

/**
* The getSF_message function get SF message to share address to friend
* @param	seq		is used if function have much message
* @param	press	is used to wait user press to continue
* @return	length	of message
*/
int getSF_message(char buff[], int buff_size, int &seq, bool &press) {
	string username;
	static string category, num;
	if (seq == 0) {
		category.clear();
		num.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1) {
		int length;
		printf("Please input your category:");
		getline(cin, category);
		length = category.size() + 5;
		if (length > buff_size) {
			cout << "Error: input is longer than able\n";
			seq = 0;
			return 0;
		}
		pasteLength(length, buff);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "GA ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 2;
		press = false;
		return length;
	}
	if (seq == 2) {
		int length;
		printf("Please input the number of address:");
		getline(cin, num);
		printf("Please input your friend's username:");
		getline(cin, username);
		length = 4 + 1 + username.size() + 1 + category.size() + 1 + num.size();
		if (length > buff_size) {
			cout << "Error: input is longer than able\n";
			cout << "Please press to continue:";
			getchar();
			cout << "\n\n";
			seq = 0;
			return 0;
		}
		pasteLength(length, buff);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "SF ");
		strcat_s(buff + 2, buff_size - 2, username.c_str());
		strcat_s(buff + 2, buff_size - 2, "\n");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		strcat_s(buff + 2, buff_size - 2, "\n");
		strcat_s(buff + 2, buff_size - 2, num.c_str());
		seq = 0;
		return length;
	}
	//default
	seq = 0;
	return 0;
}

/**
* The getBK_message function get BK message to get backed up category
* @param	seq		is used if function have much message
* @param	press	is used to wait user press to continue
* @return	length	of message
*/
int getBK_message(char buff[], int buff_size, int &seq, bool &press) {
	static string category;
	if (seq == 0) {
		category.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GC");
		pasteLength(length, buff);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1) {
		string category;
		cout << "Please input your category:";
		getline(cin, category);
		int length = 4 + 1 + category.size();
		if (length > buff_size) {
			seq = 0;
			return 0;
		}
		pasteLength(length, buff);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "BK ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 0;
		return length;
	}
	seq = 0;
	return 0;
}

/**
* The getRS_message function get RS message to restore the category
* @param	seq		is used if function have much message
* @param	press	is used to wait user press to continue
* @return	length	of message
*/
int getRS_message(char buff[], int buff_size, int &seq, bool &press) {
	static string category;
	if (seq == 0) {
		category.clear();
		int length = 4;
		strcpy_s(buff + 2, BUFFER_SIZE - 3, "GB");
		pasteLength(length, buff);
		seq = 1;
		press = false;
		return length;
	}
	if (seq == 1) {
		string category;
		cout << "Please input your category:";
		getline(cin, category);
		int length = 4 + 1 + category.size();
		if (length > buff_size) {
			seq = 0;
			return 0;
		}
		pasteLength(length, buff);
		buff[2] = '\0';
		strcat_s(buff + 2, buff_size - 2, "RS ");
		strcat_s(buff + 2, buff_size - 2, category.c_str());
		seq = 0;
		return length;
	}
	seq = 0;
	return 0;
}

/**
* The getLO_message fuction get log out message
*/
int getLO_message(char buff[], int buff_size) {
	int length = 4;
	strcpy_s(buff + 2, BUFFER_SIZE - 3, "LO");
	pasteLength(length, buff);
	return length;
}