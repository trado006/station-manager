#pragma once
#include <fstream>
#include "data_type.h"

#pragma warning(disable : 4996)

/**
* get user list from account file
* @return 0 if the file is not available
* @return 1 if success
*/
int prepareUserList(const char filePath[], users &userList) {
	userList.clear();
	ifstream inData;
	inData.open(filePath, ios::in);
	if (inData.fail()) {
		return 0;
	}
	while (!inData.eof()) {
		user pUser;
		inData >> (pUser).username;
		inData >> (pUser).password;
		pUser.islogin = false;
		if ((pUser).username.empty()) continue;
		(userList).push_back((pUser));
	}
	inData.close();
	return 1;
}

/**
* The makeDirectory function create the last foulder of the path
* @param	dir		is the path of foulder
* @return	0		if create is okey
* @return	-1		if the foulder is exists or not done
*/
int makeDirectory(const char dir[]) {
	int n = strlen(dir);
	wchar_t *w_dir = new wchar_t[n+1];
	toWchar_t(dir, w_dir, n+1);
	int rt = _wmkdir(w_dir);
	delete(w_dir);
	return rt;
}

/**
* The createFile funtion creates file with file_path
* @return true if success
* @return false if fail
*/
bool createFile(const char file_path[]) {
	FILE *fp;
	fp = fopen(file_path, "ab");
	if (fp == NULL) return false;
	fclose(fp);
	return true;
}

/**
* The getFileNames function get all name of the files is the children in the foulder_path
* @param	foulder_path		the foulder path
* @param	file_names			contains file names after the function is called
* @return the size of file_names
*/
int getFileNames(const string foulder_path, string &file_names) {
	file_names.clear();
	WIN32_FIND_DATA data;
	memset(&data, -1, sizeof data);
	//find first file
	wchar_t wfouler_path[260];
	toWchar_t(foulder_path + "*.*", wfouler_path, sizeof wfouler_path);
	HANDLE hFile = FindFirstFile(wfouler_path, &data);
	do {
		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			char ch[260];
			char DefChar = ' ';
			WideCharToMultiByte(CP_ACP, 0, data.cFileName, -1, ch, 260, &DefChar, NULL);
			//A std:string  using the char* constructor.
			int n = strlen(ch);
			if (n > 4) {
				ch[n - 1 - 3] = 0;
				n -= 4;
				if (n == 2 && ch[0] == 'f' && ch[1] == 'a') continue;
			}
			if (n > 0) {
				file_names.append(ch);
				file_names.append("\n");
			}
		}
	} while (FindNextFile(hFile, &data));
	FindClose(hFile);
	return file_names.size();
}

/**
* The getFileSize function return the size of file
* @return	-1 if file not exists
* @return	else is the size of file
*/
int getFileSize(const char file_path[]) {
	FILE *fp;
	//check file is exists
	fp = fopen(file_path, "rb");
	if (fp == NULL) {
		//printf("File path not exists!\n");
		return -1;
	}
	//ckeck size of file
	fseek(fp, 0, SEEK_END);
	int file_size = (int)ftell(fp);
	fclose(fp);
	return file_size;
}
/**
* The function read a block in file_path from front possition and store in buff
* @param buff_size is the maximum number of bytes readable
* @return the number of bytes readable if success
* @return -1 if file_path is not exists
*/
int readFileBlock(const char file_path[], int front,char *buff, int buff_size) {
	FILE *fp;
	//check file is exists
	fp = fopen(file_path, "rb");
	if (fp == NULL) {
		//printf("File path not exists!\n");
		return -1;
	}
	fseek(fp, front, SEEK_SET);
	int readedBytes = fread(buff, sizeof(char), buff_size, fp);
	fclose(fp);
	if (readedBytes >= 0) return readedBytes;
	return -1;
}

/*delete file from file_path*/
bool deleteFile(const char* file_path)
{
	int ret = remove(file_path);
	bool is_ok = (ret == 0) ? true : false;
	return is_ok;
}

/*append string at the end of file if file not exists will create
return true if seccess and false is fail*/
bool appendFile(const char file_path[],const char src[], int src_len) {
	FILE *fp;
	fp = fopen(file_path, "ab");
	if (fp == NULL) return false;
	fwrite(src, sizeof(char), src_len, fp);
	fclose(fp);
	return true;
}
/**
* the funtion get front, last possition and content of the required line in the file_path at line_num
* @Note: lpcontent or front, last param can be NULL
* @return -2 file_path is error
* @return -1 file is empty
* @return 0  success
* @return 1  file line not exists
*/
int findFileLine(const char file_path[], const int line_num, int *front, int *last, string *lpcontent) {
	if (line_num <= 0) return 1;
	int beginOfLine, endOfLine;
	int file_size = (int)getFileSize(file_path);
	if (file_size <= 0) return file_size - 1;
	char *buff = new char[file_size];
	int readedBytes = (int)readFileBlock(file_path, 0, buff, file_size);
	if (readedBytes <= 0) {
		delete(buff);
		return readedBytes-1;
	}
	int i, cnt = 0;
	for (i = 0; i<readedBytes; i++) {
		if (cnt == line_num - 1) {
			beginOfLine = i;
			break;
		}
		if (buff[i] == '\n') cnt++;
	}
	for (; i < readedBytes; i++) {
		if (buff[i] == '\n') cnt++;
		if (cnt == line_num) {
			endOfLine = i;
			break;
		}
	}
	if (i >= readedBytes) {
		delete(buff);
		return 1;
	}
	if (lpcontent != NULL) {
		(*lpcontent).empty();
		char tmp[2] = { buff[endOfLine],'\0' };
		buff[endOfLine] = '\0';
		(*lpcontent).append(buff + beginOfLine);
		(*lpcontent).append(tmp);
	}
	delete(buff);
	if (front != NULL && last != NULL) {
		*front = beginOfLine;
		*last = endOfLine;
	}
	return 0;
}
/**
* The deleteFileLine funtion delete a line in file at line_num
* @return -1 if file not exits
* @return  0 if file-line not exits
* @return  1 if it's key
*/
int deleteFileLine(const char file_path[],int line_num) {
	int beginOfLine, endOfLine;
	int rt = findFileLine(file_path, line_num, &beginOfLine, &endOfLine, NULL);
	if (rt < -1) return -1;
	if (rt == -1) {
		deleteFile(file_path);
		return -1;
	}
	if (rt == 1) return 0;
	int file_size = getFileSize(file_path);
	if (file_size <= 0) return -1;
	char *buff = new char[file_size];
	int readedBytes = (int)readFileBlock(file_path, 0, buff, file_size);
	if (readedBytes <= 0) {
		delete(buff);
		return -1;
	}
	deleteFile(file_path);
	//file contains data after delete line
	if (beginOfLine != 0 || endOfLine != readedBytes-1) {
		appendFile(file_path, buff, beginOfLine);
		appendFile(file_path, buff + endOfLine + 1, readedBytes - (endOfLine + 1));
	}
	delete(buff);
	return 1;
}
/**
* the getFileLine function get line content in file at line_num
* @return -1 if file not exits
* @return  0 if file-line not exits
* @return  1 if it's key
*/
int getFileLine(const char file_path[], int line_num, string &content) {
	int rt = findFileLine(file_path, line_num, NULL, NULL, &content);
	if (rt < -1) return -1;
	if (rt == -1 || rt == 1) return 0;
	return 1;
}

/**
* the copyFile function append from source file to destination file
* @return -1 if file not exits
* @return  1 if it's okey
*/
int copyFile(const char sourceFilePath[], const char destinationFilePath[]) {
	FILE *sp, *dp;
	sp = fopen(sourceFilePath, "rb");
	if (sp == NULL) return -1;
	dp = fopen(destinationFilePath, "wb+");
	if (dp == NULL) return -1;

	char c;
	while ((c = fgetc(sp)) != EOF) {
		fputc(c, dp);
	}
	
	fclose(sp);
	fclose(dp);

	return 1;
}