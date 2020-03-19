// SearchAndUpload.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//命令行工具实现搜索Windows系统,全磁盘如果有新产生的指定多钟后缀文件（比如docx、PDF、PPT等），立即通过打包压缩加密上传文件；
//可以上传第三方网盘（可以考虑dropbox网盘，需要挂VPn,百度网盘等），上传成功后，支持rar解压加密文件；

//支持搜索多国语言文件


#include "SearchAndUpload.h"
#include "usn_manager.h"
#include "global.h"
#include "zip_manager.h"
#include "upload_manager.h"

//vector <string> change_files_path;
typedef struct {
    DWORDLONG journal_id;
    USN high_usn;
} JOURNAL_INFO;

typedef struct {
    string name;
    DWORDLONG PFRN;
} FILE_NODE;

typedef struct {
    string name;
    long long int file_cnt;
} VOLUME_INFO;


//多线程测试
//void counters(int id, int numIter) {
//    for (int i = 0; i < numIter; ++i) {
//        cout << "counter id:" << id << endl;
//        cout << "iteraion:" << i << endl;
//        my_mutex.lock();
//        thread_test += 1;
//        cout << "counts:" << thread_test << endl;
//        my_mutex.unlock();
//    }
//}

//匹配后缀简化测试版
int file_type(char* patName, char* relName) {
    string pat;
    string allname;
    pat = patName;
    allname = relName;
    int index = allname.find(pat, allname.length() - pat.length());
    if (index != allname.npos)
        return 1;
    else
        return 0;

}


void listFiles(char* path, char* name, bool children = false) {
    intptr_t handle;
    _finddata_t findData;
    char curPath[MAX_PATH], nextPath[MAX_PATH], curFileName[MAX_PATH];
    strcpy(curPath, path);
    strcat(curPath, "\\*.*");  //执行curPath=path+"\\*.*"
    handle = _findfirst(curPath, &findData);    // 查找目录中的第一个文件
    if (handle == -1) {
        cout << "Failed to find first file!\n";
        return;
    }
    do {
        if ((findData.attrib == _A_SUBDIR) && (findData.name[0] != '.')) { // 是否是子目录并且不为"."或".."
            strcpy(curFileName, findData.name);
            strcpy(nextPath, path);
            strcat(nextPath, "\\");
            strcat(nextPath, curFileName);  //执行nextPath=path+"\\"+findData.name，形成子文件夹路径
            listFiles(nextPath, name, children);  //递归搜索子文件夹
        } else if (findData.name[0] != '.') {
            if (file_type(name, findData.name)) { //比较当前文件是否与搜索字符串匹配
                DWORD sf = GetFileType((HANDLE*)handle);
                cout << path << "/" << findData.name << "\t" << findData.size << endl;
                counts++;
            }

        }
    } while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件
    _findclose(handle);    // 关闭搜索句柄
}

BOOL get_handle(char* volume_name, HANDLE&  volume_handle) {
    volume_handle = CreateFile(volume_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (volume_handle == INVALID_HANDLE_VALUE) return false;
    return true;

}

void scan_all_drives() {

    UINT nType;

    for (char i = 'A'; i <= 'Z'; i++) {
        char rootPath[10] = { 0 }, driveType[21] = { 0 };
        sprintf(rootPath, "%c:\\\\", i);
        nType = GetDriveType(rootPath);
        // DRIVE_NO_ROOT_DIR: 路径无效
        if (nType != DRIVE_NO_ROOT_DIR) {
            cout << "detected " << rootPath << endl;
            G_exits_drives.push_back(rootPath);
        }
    }
}

//传入文件类型分隔
vector<string> file_type_split(std::string& str, const std::string& delims = ",") {
    vector<std::string> output;

    while (str.find_first_of(delims) != str.npos) {
        int index = str.find_last_of(delims);
        string tmp = str.substr(index + 1, str.length());
        str = str.substr(0, index);
        //cout << tmp << endl;
        output.emplace_back(tmp);
    }
    //cout << str << endl;
    output.emplace_back(str);
    return output;
}

int main(int argc, const char** argv) {
    std::cout << endl << "Welcome to use this system file change watcher system !\n" << endl ;
    cout << "本软件 采用Dropbox进行第三方云存储，需要配置access token , access token申请网站" << endl
         << "https://www.dropbox.com/developers/apps" << endl << endl;

    cout << "使用方法如下：" << endl;
    cout << "sau.exe 保存本地zip位置 密码 定时上传时间间隔(s) \"后缀列表\" \"DropboxToken\"" << endl;
    cout << "例子：" << endl;
    cout << "sau.exe D:\\watcher.zip CBR 60 \".docx,.pptx,.pdf,.png\" \"wO-OKXR2pZAAAAAAAAAA\"" << endl;

    if (argc < 6) {
        cout << endl << "参数输入错误：" << endl << endl;
        cout << "使用方法如下：" << endl;
        cout << "sau.exe 保存本地zip位置 密码 定时上传时间间隔(s) \"后缀列表\" \"DropboxToken\"" << endl;
        cout << "例子：" << endl;
        cout << "sau.exe D:\\watcher.zip CBR 60 \".docx,.pptx,.pdf,.png\" \"wO-OKXR2pZAAAAAAAAAA\"" << endl;
        return 0;
    }

    const char* exce_path = argv[0];
    const char* zip_path = argv[1];
    const char* zip_password = argv[2];
    int timing = atoi(argv[3]) * 1000;
    const char* file_types_input = argv[4];
    const char* dropbox_token = argv[5];

    string ft = ".docx,.pptx,.pdf,.png";
    ft = file_types_input;
    std::vector<std::string> ftv;
    ftv = file_type_split(ft, ",");
    //cout << ftv.size() << endl;
    G_file_types = ftv;
    G_dropbox_token = dropbox_token;

    //调试libcurl用，出现 Couldn't resolve host 'content.dropboxapi.com'
    upload_manager um;
    um.up((char*)zip_path);

    return 0;

    //listFiles((char*)"D:", (char*)".cpp", true);

    //获取所有盘符
    scan_all_drives();

    //列出所有盘符
    //for (int i = 0; i < exits_drives.size(); i++) {
    //    //
    //    cout << exits_drives.at(i) << endl;
    //}

    usn_manager usn;
    DWORD timingTimeStamp = GetTickCount();
    DWORD startTimeStamp = GetTickCount();
    while (true) {
        DWORD endTimeStamp = GetTickCount();
        // 规定 秒循环清理一次
        if (G_change_files_path.size() && (endTimeStamp - timingTimeStamp > timing)) {
            for (int i = 0 ; i < G_change_files_path.size(); i++)
                cout << G_change_files_path.at(i) << endl;
            zip_manager zip_packer;


            zip_packer.start((char*)zip_path, (char*)zip_password, G_change_files_path);

            upload_manager uploader;
            uploader.start((char*)zip_path);

            G_change_files_path.erase(G_change_files_path.begin(), G_change_files_path.end());
            timingTimeStamp = GetTickCount();

        } else {
            cout << "准备全盘扫描" << endl;

            //启动扫描
            usn.start(G_exits_drives);

            DWORD tmpTimeStamp = GetTickCount();
            cout << endl << "全盘扫描完毕 花费" << (tmpTimeStamp - startTimeStamp) / 1000 << "秒" << endl << endl;
            startTimeStamp = tmpTimeStamp;
        }
    }


}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
