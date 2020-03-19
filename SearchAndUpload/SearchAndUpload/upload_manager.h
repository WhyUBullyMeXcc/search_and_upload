#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <time.h>
#include <Windows.h>
#include <direct.h>
#include <fstream>
using namespace std;

class upload_manager {
    private:
		
		string get_file_name(string src);
        string prepare_string(string file_name, string file_path);

    public:
        void start(string file_path);
		void up(string file_path);
};



