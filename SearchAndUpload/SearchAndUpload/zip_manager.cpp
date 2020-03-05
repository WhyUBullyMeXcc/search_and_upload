#include "zip_manager.h"



zip_manager::zip_manager() {

}

zip_manager::~zip_manager() {
    std::cout << "Object is being deleted" << std::endl;

}

string get_file_name(string src) {
    string path_string = src;
    int index = path_string.find_last_of("\\");
    if (index != path_string.npos)
        path_string = path_string.substr(index + 1, path_string.length());
    return path_string;
}

void zip_manager::add_file_to_zip(HZIP& hz, vector <string>  aim_file) {

    for (int i = 0; i < aim_file.size(); i++) {
        cout << aim_file.at(i) << endl;
        cout << get_file_name(aim_file.at(i)) << endl;
        ZipAdd(hz, get_file_name(aim_file.at(i)).c_str(), aim_file.at(i).c_str());
    }
    //ZipAdd(hz, (TCHAR*)"W:\search_and_upload\SearchAndUpload\SearchAndUpload\\unzip.cpp", (TCHAR*)"W:\search_and_upload\SearchAndUpload\SearchAndUpload\\unzip.cpp");

}

void zip_manager::start(char* file_name, char* password, vector <string> aim_file) {
    //CreateFiles();
    HZIP hz;
    DWORD writ;
    //string pass = "dsa";
    // EXAMPLE 1 - create a zipfile from existing files
    hz = CreateZip(file_name, password);
    //ZipAdd(hz, _T("zip.cpp"), _T("zip.cpp"));
    add_file_to_zip(hz, aim_file);
    CloseZip(hz);
    cout << "Created" << file_name << "successful £¡"  << endl;

}
