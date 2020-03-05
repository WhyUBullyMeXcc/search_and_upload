#pragma once
#include <vector>
#include <string>
#include <iostream>
using namespace std;

class upload_manager {
    private:
		string prepare_string(string file_name, string file_path);
    public:
        void start();
};



