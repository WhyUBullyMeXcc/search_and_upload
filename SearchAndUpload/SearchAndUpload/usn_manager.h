#pragma once

#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <time.h>
#include <mutex>
#include <winbase.h>
#include <vector>
#include <algorithm>
#include <winternl.h>
#include <stack>
#include <thread>
#include <algorithm>
#include <iterator>



using namespace std;


typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;


#define BUF_LEN 4096
class usn_manager {

    private:
        
        mutex Mutex;//Ïß³ÌËø


    public:
        void start(vector <string> drives);
        int file_type(char* patName, char* relName);
        HMODULE load_ntdll(HMODULE hmodule);
        void get_path_from_frn(HANDLE& volume_handle, DWORDLONG frn, string volpath);
        void watch_usn(string path);
        void watch_usns(string path,int oper);
        usn_manager();
        ~usn_manager();

};

