#pragma once
#include "unbox_mess.h"
#include "io_file.h"
#define USER_FILE "account.txt"

// paste opcode and length into message
void packageHeader(char opcode,unsigned short length, char mes[]) {
	mes[0] = opcode;
	unsigned short Net_num = htons(length);
	void *p = &Net_num;
	mes[1] = *((char*)p);
	mes[2] = *((char*)p + 1);
}

//create error message with opcode = 0 and error_code != -1
//or end message has payload = 0 with error_code = -1
char* getNoticeMess(int error_code, int opcode = 0) {
	char *notice;
	if (error_code!=-1) {
		notice = new char[4];
		packageHeader(-1, 1, notice);
		notice[3] = error_code;
	}
	else {
		notice = new char[3];
		packageHeader(opcode, 0, notice);
	}
	return notice;
}

/**
*The getRG_response function creates response from register message and store in res queue with userList
*@return	-1		if have error message
*@return	0		if success
*/
int getRG_response(queue<char*> &res, char mess[], int mess_len, users &userList) {
	user usr;
	int rt = getAccount(mess, mess_len, usr);
	if (rt != -1) {
		res.push(getNoticeMess(rt));
		return -1;
	}
	string foulder_path;
	foulder_path.append("users/");
	foulder_path.append(usr.username.c_str());
	if (makeDirectory(foulder_path.c_str())) {
		res.push(getNoticeMess(1));
		return -1;
	}
	string fa_path = foulder_path + "/fa.txt";
	if (!createFile(fa_path.c_str())) {
		printf("create fa file error\n");
		exit(0);
	}
	foulder_path.append("/back_up");
	if (makeDirectory(foulder_path.c_str())) {
		printf("create back_up foulder error\n");
		exit(0);
	}
	//save account
	string s;
	s.append(usr.username.c_str());
	s.append(" ");
	s.append(usr.password.c_str());
	s.append("\n");
	if (!appendFile(USER_FILE, s.c_str(), s.size())) {
		printf("append user file error\n");
		exit(0);
	}
	//add user to userlist
	userList.push_back(usr);
	res.push(getNoticeMess(-1));
	return 0;
}

/**
*The getLI_response function creates response from log in message and store in res queue with userList and clientInfo
*@return	-1		if have error message
*@return	0		if success
*/
int getLI_response(queue<char*> &res, char mess[], int mess_len, users &userList, ClientInfo &clientInfo) {
	if (clientInfo.islogin) {
		res.push(getNoticeMess(6));
		return -1;
	}
	user usr;
	int rt = getAccount(mess, mess_len, usr);
	if (rt != -1) {
		res.push(getNoticeMess(rt));
		return -1;
	}
	int index = accountLocation(userList, usr);
	if (index == -1) {
		res.push(getNoticeMess(3));
		return -1;
	}
	if (userList[index].islogin) {
		res.push(getNoticeMess(4));
		return -1;
	}
	userList[index].islogin = true;
	clientInfo.islogin = true;
	clientInfo.login_place = index;
	strcpy_s(clientInfo.username, usr.username.c_str());
	printf("username:%s\n", clientInfo.username);
	string fa_path;
	fa_path.append("users/");
	fa_path.append(usr.username.c_str());
	fa_path.append("/fa.txt");
	
	int fa_bytes = getFileSize(fa_path.c_str());
	clientInfo.fa_bytes = fa_bytes;
	//read file fa.txt
	int beginPos = 0;
	while (beginPos < fa_bytes) {
		int buff_size = (fa_bytes - beginPos + 3) > BUFFSIZE ? BUFFSIZE : fa_bytes - beginPos + 3;
		char *buff = new char[buff_size];
		int length = readFileBlock(fa_path.c_str(), beginPos, buff + 3, buff_size - 3);
		if (length <= 0) {
			delete(buff);
			break;
		}
		packageHeader(2, length, buff);
		res.push(buff);
		beginPos += length;
	}
	res.push(getNoticeMess(-1,2));
	return 0;
}

/**
*The getSG_response function creates response from suggest message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getSG_response(queue<char*> &res, char mess[], int mess_len, bool islogin) {
	if (mess_len != 4) {
		res.push(getNoticeMess(0));
		return -1;
	}
	if (!islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string sg_path = "suggest.txt";
	int sg_bytes = getFileSize(sg_path.c_str());
	//read file suggest.txt
	int beginPos = 0;
	while (beginPos < sg_bytes) {
		int buff_size = (sg_bytes - beginPos + 3) > BUFFSIZE ? BUFFSIZE : sg_bytes - beginPos + 3;
		char *buff = new char[buff_size];
		int length = readFileBlock(sg_path.c_str(), beginPos, buff + 3, buff_size - 3);
		if (length <= 0) {
			delete(buff);
			break;
		}
		packageHeader(1, length, buff);
		res.push(buff);
		beginPos += length;
	}
	res.push(getNoticeMess(-1,1));
	return 0;
}

/**
*The getGC_response function creates response from get_categary message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getGC_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (mess_len != 4) {
		res.push(getNoticeMess(0));
		return -1;
	}
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string foulder_path;
	foulder_path.append("users/");
	foulder_path.append(clientInfo.username);
	foulder_path.append("/");
	
	string file_names;
	int s_size = getFileNames(foulder_path, file_names);
	
	int beginPos = 0;
	while (beginPos < s_size) {
		int buff_size = (s_size - beginPos + 3) > BUFFSIZE ? BUFFSIZE : s_size - beginPos + 3;
		char *buff = new char[buff_size];
		memcpy_s(buff + 3, buff_size - 3, file_names.c_str() + beginPos, buff_size - 3);
		packageHeader(3, buff_size - 3, buff);
		res.push(buff);
		beginPos += (buff_size - 3);
	}
	res.push(getNoticeMess(-1,3));
	return 0;
}

/**
*The getAA_response function creates response from log in message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getAA_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string categary, address;
	int rt = getAddress(mess, mess_len, categary, address);
	if (rt != -1) {
		res.push(getNoticeMess(rt));
		return -1;
	}
	if (!categary.compare("fa")) {
		res.push(getNoticeMess(11));
		return -1;
	}
	string file_path, fa_path;
	file_path.append("users/");
	file_path.append(clientInfo.username);
	file_path.append("/"); 
	fa_path.append(file_path.c_str());
	fa_path.append("fa.txt");
	file_path.append(categary.c_str());
	file_path.append(".txt");
	
	//Check add friend
	if (checkNum(address.c_str())) {
		int separation, num = atoi(address.c_str());
		string line;
		int rt = getFileLine(fa_path.c_str(), num, line);
		if (rt == -1) {
			res.push(getNoticeMess(11));
			return -1;
		}
		if (rt == 0) {
			res.push(getNoticeMess(21));
			return -1;
		}
		separation = line.size() - 1;
		for (int i = 0; i < (int)line.size(); i++)
			if(line[i]==5) separation = i+1;
		address.clear();
		address.push_back('+');
		for (; separation < (int)line.size(); separation++) {
			address.push_back(line[separation]);
		}
		if (!appendFile(file_path.c_str(), address.c_str(), address.size())) {
			res.push(getNoticeMess(11));
			return -1;
		}
	}
	else {
		appendFile(file_path.c_str(), "-", 1);
		address.push_back('\n');
		if (!appendFile(file_path.c_str(), address.c_str(), address.size())) {
			res.push(getNoticeMess(11));
			return -1;
		}
	}
	res.push(getNoticeMess(-1));
	return 0;
}

/**
*The getRA_response function creates response from remove_address message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getRA_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string categary, address;
	int rt = getAddress(mess, mess_len, categary, address);
	if (rt != -1) {
		res.push(getNoticeMess(rt));
		return -1;
	}
	if (!categary.compare("fa")) {
		res.push(getNoticeMess(11));
		return -1;
	}
	int line_num = atoi(address.c_str());
	if (line_num <= 0) {
		res.push(getNoticeMess(21));
		return -1;
	}
	string file_path;
	file_path.append("users/");
	file_path.append(clientInfo.username);
	file_path.append("/"); file_path.append(categary.c_str());
	file_path.append(".txt");
	rt = deleteFileLine(file_path.c_str(), line_num);
	if (rt == -1) {
		res.push(getNoticeMess(11));
		return -1;
	}
	if (rt == 0) {
		res.push(getNoticeMess(21));
		return -1;
	}
	res.push(getNoticeMess(-1));
	return 0;
}

/**
*The getGA_response function creates response from get-address message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getGA_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (mess_len<6 || mess[4] != ' ') {
		res.push(getNoticeMess(0));
		return -1;
	}
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string categary;
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	categary.append(mess + 5);
	categary.push_back(tmp);

	string categary_path = "users/";
	categary_path.append(clientInfo.username);
	categary_path.push_back('/');
	categary_path.append(categary.c_str());
	categary_path.append(".txt");

	//read file categary_name.txt
	int cgBytes = getFileSize(categary_path.c_str());
	if (cgBytes < 0) {
		res.push(getNoticeMess(11));
		return -1;
	}
	int beginPos = 0;
	while (beginPos < cgBytes) {
		int buff_size = (cgBytes - beginPos + 3) > BUFFSIZE ? BUFFSIZE : cgBytes - beginPos + 3;
		char *buff = new char[buff_size];
		int length = readFileBlock(categary_path.c_str(), beginPos, buff + 3, buff_size - 3);
		if (length <= 0) {
			delete(buff);
			break;
		}
		packageHeader(4, length, buff);
		res.push(buff);
		beginPos += length;
	}
	res.push(getNoticeMess(-1, 4));
	return 0;
}

/**
*The getGA_response function creates response from share-friend message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getSF_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string username, categary, address;
	int rt = getSharedAddress(mess, mess_len, username, categary, address);
	if (rt != -1) {
		res.push(getNoticeMess(rt));
		return -1;
	}
	if (!categary.compare("fa")) {
		res.push(getNoticeMess(11));
		return -1;
	}
	int line_num = atoi(address.c_str());
	if (line_num <= 0) {
		res.push(getNoticeMess(21));
		return -1;
	}
	string file_path, addressContent;
	file_path.append("users/");
	file_path.append(clientInfo.username);
	file_path.append("/"); file_path.append(categary.c_str());
	file_path.append(".txt");
	rt = getFileLine(file_path.c_str(), line_num, addressContent);
	if (rt == -1) {
		res.push(getNoticeMess(11));
		return -1;
	}
	if (rt == 0) {
		res.push(getNoticeMess(21));
		return -1;
	}
	addressContent.erase(addressContent.begin());

	string fa_file_path;
	fa_file_path.append("users/");
	fa_file_path.append(username);
	fa_file_path.append("/fa.txt");
	printf("%s\n", address.c_str());

	string writeToFile;
	writeToFile.append(clientInfo.username);
	writeToFile.append(" ");
	writeToFile.append(categary);
	writeToFile.append("\5");
	writeToFile.append(addressContent);
	rt = appendFile(fa_file_path.c_str(), writeToFile.c_str(), writeToFile.size());

	if (rt == 0) {
		res.push(getNoticeMess(1));
		return -1;
	}
	res.push(getNoticeMess(-1));
	return 0;
}

/**
*The getGB_response function createss response from get-backup message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getGB_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo) {
	if (mess_len != 4) {
		res.push(getNoticeMess(0));
		return -1;
	}
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string foulder_path;
	foulder_path.append("users/");
	foulder_path.append(clientInfo.username);
	foulder_path.append("/back_up/");
	printf("foulder_path %s\n", foulder_path.c_str());
	string file_names;
	int len = getFileNames(foulder_path, file_names);
	printf("len %d\n", len);
	int begin = 0;
	while (begin < len) {
		char *buff = new char[BUFFSIZE];
		int length_param = BUFFSIZE - 3 >(len - begin) ? (len - begin) : BUFFSIZE - 3;
		memcpy_s(buff + 3, BUFFSIZE - 3, file_names.c_str() + begin, length_param);
		packageHeader(3, length_param, buff);
		res.push(buff);
		begin += length_param;
	}
	res.push(getNoticeMess(-1, 3));
	return 0;
}

/**/
/**
*The getBK_response function createss response from backup message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getBK_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo) {
	//length|BK_categary.
	if (mess_len < 6 || mess[4] != ' ') {
		res.push(getNoticeMess(0));
		return -1;
	}
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string categary;
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	categary.append(mess + 5);
	categary.push_back(tmp);

	string categary_path = "users/";
	categary_path.append(clientInfo.username);
	categary_path.push_back('/');
	categary_path.append(categary.c_str());
	categary_path.append(".txt");
	printf("%s file path\n", categary_path.c_str());

	string backupFilePath = "users/";
	backupFilePath.append(clientInfo.username);
	backupFilePath.append("/back_up/");
	backupFilePath.append(categary.c_str());
	backupFilePath.append(".txt");
	printf("%s is backup file path\n", backupFilePath.c_str());

	int rt = copyFile(categary_path.c_str(), backupFilePath.c_str());
	if (rt == -1) {
		res.push(getNoticeMess(11));
		return -1;
	}

	res.push(getNoticeMess(-1));
	return 0;
}

/**
*The getRS_response function creates response from restore message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getRS_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo) {
	//length|RS_categary.
	if (mess_len < 6 || mess[4] != ' ') {
		res.push(getNoticeMess(0));
		return -1;
	}
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	string categary;
	char tmp = mess[mess_len - 1];
	mess[mess_len - 1] = '\0';
	categary.append(mess + 5);
	categary.push_back(tmp);

	string categary_path = "users/";
	categary_path.append(clientInfo.username);
	categary_path.push_back('/');
	categary_path.append(categary.c_str());
	categary_path.append(".txt");
	printf("%s file path\n", categary_path.c_str());

	string backupFilePath = "users/";
	backupFilePath.append(clientInfo.username);
	backupFilePath.append("/back_up/");
	backupFilePath.append(categary.c_str());
	backupFilePath.append(".txt");
	printf("%s is backup file path\n", backupFilePath.c_str());

	int rt = copyFile(backupFilePath.c_str(), categary_path.c_str());
	if (rt == -1) {
		res.push(getNoticeMess(11));
		return -1;
	}
	res.push(getNoticeMess(-1));
	return 0;
}

/**
*The getLO_response function creates response from log out message and store in res queue
*@return	-1		if have error message
*@return	0		if success
*/
int getLO_response(queue<char*> &res, char mess[], int mess_len, ClientInfo &clientInfo,users &userList) {
	if (mess_len != 4) {
		res.push(getNoticeMess(0));
		return -1;
	}
	if (!clientInfo.islogin) {
		res.push(getNoticeMess(5));
		return -1;
	}
	clientInfo.islogin = false;
	userList[clientInfo.login_place].islogin = false;
	int fa_bytes = clientInfo.fa_bytes;

	string fa_path;
	fa_path.append("users/");
	fa_path.append(clientInfo.username);
	fa_path.append("/fa.txt");

	int file_size = getFileSize(fa_path.c_str());
	if (file_size <= 0 || fa_bytes == 0) {
		res.push(getNoticeMess(-1));
		return 0;
	}
	char *buff = new char[file_size-fa_bytes];
	int block_size = readFileBlock(fa_path.c_str(), fa_bytes, buff, file_size - fa_bytes);
	deleteFile(fa_path.c_str());
	appendFile(fa_path.c_str(), buff, file_size - fa_bytes);
	delete(buff);
	res.push(getNoticeMess(-1));
	return 0;
}

/*process recvBuff in clientInfo to get responses from client messages*/
void getResponses(ClientInfo &clientInfo, users &userList) {
	int buff_len = clientInfo.recvBytes;
	char *recvBuff = clientInfo.recvBuffer;
	queue<char*> &responses = clientInfo.responses;

	int mes_beginPos = 0, mes_endPos = 0;
	while (mes_endPos<buff_len - 1) {
		if (buff_len - mes_beginPos < 4) {
			//receive error message
			printf("receive error message\n");
			responses.push(getNoticeMess(0));
			break;
		}
		int length = getLength(recvBuff + mes_beginPos);
		mes_endPos = mes_beginPos + (length - 1);
		//ckeck mes_endPos
		if (mes_endPos<mes_beginPos + 3 || mes_endPos>buff_len - 1) {
			//receive error message
			printf("receive error message\n");
			responses.push(getNoticeMess(0));
			break;
		}
		char * mes_begin = recvBuff + mes_beginPos;
		int mes_len = mes_endPos - mes_beginPos + 1;
		int mess_type = getMes_type(recvBuff + mes_beginPos + 2);
		switch (mess_type) {
		case (('R' << 8) + 'G'):
		{
			printf("register\n");
			getRG_response(responses, mes_begin, mes_len, userList);
			break;
		}
		case (('L' << 8) + 'I'): 
		{
			printf("login\n");
			getLI_response(responses, mes_begin, mes_len, userList, clientInfo);
			break;
		}
		case (('S' << 8) + 'G'):
		{
			printf("suggest\n");
			getSG_response(responses, mes_begin, mes_len, clientInfo.islogin);
			break;
		}
		case (('G' << 8) + 'C'):
		{
			printf("getcategary\n");
			getGC_response(responses, mes_begin, mes_len, clientInfo);
			break;
		}
		case (('A' << 8) + 'A'): 
		{
			printf("add address\n");
			getAA_response(responses, mes_begin, mes_len, clientInfo);
			break;
		}
		case (('R' << 8) + 'A'): 
		{
			printf("remove address\n");
			getRA_response(responses, mes_begin, mes_len, clientInfo);
			break;
		}
		case (('G' << 8) + 'A'): 
		{
			printf("get address\n");
			getGA_response(responses, mes_begin, mes_len, clientInfo);
			break;
		}
		case (('S' << 8) + 'F'):
		{
			printf("share address\n");
			getSF_response(responses, mes_begin, mes_len, clientInfo);
			break;
		}
		case (('G' << 8) + 'B'):
		{
			printf("get backup categary\n");
			getGB_response(responses, mes_begin, mes_len, clientInfo);
			break;
		}
		case (('B' << 8) + 'K'):
		{
			printf("get address\n");
			getBK_response(responses, mes_begin, mes_len, clientInfo);
			break;
		}
		case (('R' << 8) + 'S'):
		{
			printf("get address\n");
			getRS_response(responses, mes_begin, mes_len, clientInfo);
			break;
		}
		case (('L' << 8) + 'O'): 
		{
			printf("logout\n");
			getLO_response(responses, mes_begin, mes_len, clientInfo, userList);
			break;
		}
		default:
		{
			printf("default response\n");
			responses.push(getNoticeMess(0));
			break;
		}
		}
		printf("mes_beginPos %d mes_endPos %d\n",mes_beginPos, mes_endPos);
		mes_beginPos = ++mes_endPos;
	}
	
}