#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>
#include <winnt.h>

using namespace std;

extern char* G_password;
extern string G_dropbox_token;
extern vector <string> G_exits_drives;
extern vector <string> G_file_types;
extern vector <string>  G_change_files_path;
extern vector<vector<DWORDLONG>> G_drives_scan_result;