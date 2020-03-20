#pragma once
#include <iostream>
#include <windows.h>
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

class MyWindowsNotify {
    public:
        MyWindowsNotify() {
            std::cout << "The MyWindowsNotify is create" << std::endl;
        }
        ~MyWindowsNotify() {
            std::cout << "The MyWindowsNotify was delete" << std::endl;
        }

        void start(vector <string> drives);

        int file_type(char* relName);

        void add_file_to_G_N(string fileName);

        void watch_drives(string paths);
};
