#pragma once
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "zip.h"
#include "unzip.h"
#include <iostream>

using namespace std;

class zip_manager {
    public:
        zip_manager();
        ~zip_manager();
		void add_file_to_zip(HZIP & hz, vector <string> aim_file);
        void start(char* file_name, char* password, vector <string> aim_file);
};

