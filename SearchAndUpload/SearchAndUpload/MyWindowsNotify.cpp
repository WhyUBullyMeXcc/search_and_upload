#include "MyWindowsNotify.h"
#include "global.h"

void MyWindowsNotify::start(vector <string> drives) {
    for (int i = 0; i < drives.size(); i++) {
        thread jt(&MyWindowsNotify::watch_drives, this, drives.at(i));
        jt.detach();
        //tp.push_back(jt);
    }
    //Sleep(60000);
}

int MyWindowsNotify::file_type(char* relName) {
    string allname;
    allname = relName;
    int flag = 0;
    for (int i = 0; i < G_file_types.size(); i++) {
        // cout << temp[i] << endl;
        int f_i = allname.find(G_file_types.at(i), allname.length() - G_file_types.at(i).length() - 1);
        if (f_i != allname.npos)
            flag = 1;
    }
    return flag;
}

void MyWindowsNotify::add_file_to_G_N(string fileName) {
    if (file_type((char*)fileName.c_str())) {
        G_N_change_files_path.push_back(fileName);
        cout << fileName << endl;
    } else {
    }
}

void MyWindowsNotify::watch_drives(string paths) {
    HANDLE hDir;
    char notify[1024];
    DWORD cbBytes, i;
    char AnsiChar[3];
    wchar_t UnicodeChar[2];
    LPCSTR path = paths.c_str();

    char* pnotify = new char[1024];
    PFILE_NOTIFY_INFORMATION tmp;

    // GetCurrentDirectory(MAX_PATH, path.GetBuffer(MAX_PATH + 1));

    hDir = CreateFile(path, FILE_LIST_DIRECTORY,
                      FILE_SHARE_READ |
                      FILE_SHARE_WRITE |
                      FILE_SHARE_DELETE,
                      NULL,
                      OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
    if (hDir == INVALID_HANDLE_VALUE) {
        cout << "hDir:INVALID_HANDLE_VALUE\r\n" << endl;
        return;
    }

    while (TRUE) {
        if (ReadDirectoryChangesW(hDir, pnotify, sizeof(notify),
                                  true, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                                  &cbBytes, NULL, NULL)) {
            tmp = (PFILE_NOTIFY_INFORMATION)pnotify;
            char fileBuffer[1024];
            switch (tmp->Action) {
            case FILE_ACTION_ADDED:

                memset(&fileBuffer, 0, sizeof(fileBuffer));
                WideCharToMultiByte(CP_ACP, 0, tmp->FileName, tmp->FileNameLength / 2, fileBuffer, 1024, NULL, NULL);
                //cout << "Directory/File added - \r\n" << paths + fileBuffer << endl;
                add_file_to_G_N(paths + fileBuffer);
                break;
            case FILE_ACTION_REMOVED:
                memset(&fileBuffer, 0, sizeof(fileBuffer));
                WideCharToMultiByte(CP_ACP, 0, tmp->FileName, tmp->FileNameLength / 2, fileBuffer, 1024, NULL, NULL);
                //cout << "Directory/File removed - \r\n" << paths + fileBuffer << endl;
                add_file_to_G_N(paths + fileBuffer);

                break;
            case FILE_ACTION_MODIFIED:
                // cout << "Directory/File modified （修改文件内容）- \r\n"
                //  << tmp->FileName << endl;
                break;
            case FILE_ACTION_RENAMED_OLD_NAME:
                memset(&fileBuffer, 0, sizeof(fileBuffer));
                WideCharToMultiByte(CP_ACP, 0, tmp->FileName, tmp->FileNameLength / 2, fileBuffer, 1024, NULL, NULL);
                //cout << "Directory/File old name （修改文件名字）- \r\n" << paths + fileBuffer << endl;
                add_file_to_G_N(paths + fileBuffer);

                break;
            case FILE_ACTION_RENAMED_NEW_NAME:
                memset(&fileBuffer, 0, sizeof(fileBuffer));
                WideCharToMultiByte(CP_ACP, 0, tmp->FileName, tmp->FileNameLength / 2, fileBuffer, 1024, NULL, NULL);
                //cout << "Directory/File new name - \r\n" << paths + fileBuffer << endl;
                add_file_to_G_N(paths + fileBuffer);

                break;
            default:
                break;
            }
        }
    }
}