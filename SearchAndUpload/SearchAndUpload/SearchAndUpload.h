#pragma once
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <direct.h>
#include <io.h>
#include <fstream>
#include <cstring>
#include <string.h>
#include <stack>
#include <thread>
#include <mutex>
#include <Windows.h>
#include <time.h>



using namespace std;

#pragma warning(disable : 4996)

mutex my_mutex;
int thread_test = 0;  //count用来统计文件数

//char* volName = (char*)"w:\\";
//HANDLE hVol = INVALID_HANDLE_VALUE;
//USN_JOURNAL_DATA UsnInfo; // 储存USN日志的基本信息
//#define BUF_LEN 4096

//ofstream fout("E:\\log.txt");
long int counts = 0;
