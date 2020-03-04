#pragma once
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <time.h>

#include <winbase.h>
#include <vector>
#include <algorithm>
#include <winternl.h>

using namespace std;


typedef struct _FILE_NAME_INFORMATION {
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;


#define BUF_LEN 4096
class usn_manager {

    private:


    public:
        void start();
        int file_type(char* patName, char* relName);
        HMODULE load_ntdll(HMODULE hmodule);
        void get_path_from_frn(HANDLE& volume_handle, DWORDLONG frn);
        void watch_usn(char* volName);
        usn_manager();
        ~usn_manager();

};

