#include "upload_manager.h"


void upload_manager::start() {
	
}

string upload_manager::prepare_string( string file_name, string file_path) {
    string res_command = "curl.exe -X POST https://content.dropboxapi.com/2/files/upload"
		"--header \"Authorization: Bearer SK\""
		" --header \"Dropbox - API - Arg : {\"path\": \"/willbereplace_1\",\"mode\": \"add\",\"autorename\": true,"
		"\"mute\": false,\"strict_conflict\": false}\" --header \"Content-Type: application/octet-stream\" --data - binary @willbereplace_2";

	//string file_name = "2s3523zxcvxczvzxc5.exe";
	//string file_path = "E:/www.dropbox.com/developers/documentation.EXE";
	res_command = res_command.replace(res_command.find("willbereplace_1"), 15, file_name);
	res_command = res_command.replace(res_command.find("willbereplace_2"), 15, file_name);


	return res_command;	
}
