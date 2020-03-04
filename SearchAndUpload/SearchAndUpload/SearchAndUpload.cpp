// SearchAndUpload.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//命令行工具实现搜索Windows系统,全磁盘如果有新产生的指定多钟后缀文件（比如docx、PDF、PPT等），立即通过打包压缩加密上传文件；
//可以上传第三方网盘（可以考虑dropbox网盘，需要挂VPn,百度网盘等），上传成功后，支持rar解压加密文件；

//支持搜索多国语言文件


#include "SearchAndUpload.h"
#include "usn_manager.h"
#include "global.h"

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




std::vector<std::string> get_all_files(std::string path, std::string suffix) {
    std::vector<std::string> files;
    //    files.clear();
    regex reg_obj(suffix, regex::icase);

    std::vector<std::string> paths;
    paths.push_back(path);
    return files;
}



void counters(int id, int numIter) {
    for (int i = 0; i < numIter; ++i) {
        cout << "counter id:" << id << endl;
        cout << "iteraion:" << i << endl;
        my_mutex.lock();
        thread_test += 1;
        cout << "counts:" << thread_test << endl;
        my_mutex.unlock();
    }
}


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
    char curPath[MAX_PATH], nextPath[MAX_PATH], curFileName[MAX_PATH];  //curPath为当前搜索路径，nextPath为其一子文件夹路径
    strcpy(curPath, path);
    strcat(curPath, "\\*.*");  //执行curPath=path+"\\*.*"
    handle = _findfirst(curPath, &findData);    // 查找目录中的第一个文件
    if (handle == -1) {
        cout << "Failed to find first file!\n";
        return;
    }

    //ofstream output;
    //output.open("files.txt");

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
        if (nType != DRIVE_NO_ROOT_DIR) {                // DRIVE_NO_ROOT_DIR: 路径无效
            cout << "detected " << rootPath << endl;
            exits_drives.push_back(rootPath);
        }
    }
}


int main() {
    std::cout << "Hello World!\n";

    string dir = "W:/*.txt";
    //listFiles((char*)"D:", (char*)".cpp", true);
    //thread t1(counters, 1, 6);
    //thread t2(counters, 2, 4);

    ////如果没有join，main函数加载两个线程后立即结束，导致线程也中止
    ////可以确保主线程一直运行，直到两个线程都执行完毕
    //t1.join();
    //t2.join();

    //获取所有盘符
    scan_all_drives();



    for (int i = 0; i < exits_drives.size(); i++) {
        //
        cout << exits_drives.at(i) << endl;
    }

    usn_manager usn;

    while (true) {
        if (change_files_path.size()) {
            for (int i = 0 ; i < change_files_path.size(); i++)
                cout << change_files_path.at(i) << endl;

            change_files_path.erase(change_files_path.begin(), change_files_path.end());
        } else
            usn.start(exits_drives);
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




//if (!get_handle(volName, hVol)) {
//  cout << "error handle" << endl;
//}



//BOOL status;
//BOOL isNTFS = false;
//BOOL getHandleSuccess = false;
//BOOL initUsnJournalSuccess = false;

//判断驱动盘是否NTFS格式
//cout << "step 01. 判断驱动盘是否NTFS格式\n";
//char sysNameBuf[MAX_PATH] = { 0 };
//status = GetVolumeInformationA(volName,
//                               NULL,
//                               0,
//                               NULL,
//                               NULL,
//                               NULL,
//                               sysNameBuf, // 驱动盘的系统名
//                               MAX_PATH);
//cout << status << endl;
//if (0 != status) {
//    cout << "文件系统名:" << sysNameBuf << "\n";
//     比较字符串
//    if (0 == strcmp(sysNameBuf, "NTFS")) {
//        cout << "此驱动盘是NTFS格式！转向step-02.\n";
//        isNTFS = true;
//    } else
//        cout << "该驱动盘非NTFS格式\n";

//}

//if (isNTFS) {
//    step 02. 获取驱动盘句柄
//    cout << "step 02. 获取驱动盘句柄\n";
//    char fileName[MAX_PATH];
//    fileName[0] = '\0';
//    strcpy_s(fileName, "\\\\.\\");//传入的文件名
//    strcat_s(fileName, volName);
//    string fileNameStr = (string)fileName;
//    fileNameStr.erase(fileNameStr.find_last_of(":") + 1);
//    cout << "驱动盘地址:" << fileNameStr.data() << "\n";
//    hVol = CreateFile(fileNameStr.data(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
//    hVol = CreateFileA(fileNameStr.data(),//可打开或创建以下对象，并返回可访问的句柄：控制台，通信资源，目录（只读打开），磁盘驱动器，文件
//                       GENERIC_READ | GENERIC_WRITE,
//                       FILE_SHARE_READ | FILE_SHARE_WRITE,
//                       NULL,
//                       OPEN_EXISTING,
//                       FILE_ATTRIBUTE_READONLY,
//                       NULL);
//    cout << hVol << endl;

//    if (INVALID_HANDLE_VALUE != hVol) {
//        cout << "获取驱动盘句柄成功！转向step-03.\n";
//        getHandleSuccess = true;
//    } else
//        cout << "获取驱动盘句柄失败 —— handle:" << hVol << " error:" << GetLastError() << "\n";
//}

//if (getHandleSuccess) {
//    step 03. 初始化USN日志文件
//    cout << "step 03. 初始化USN日志文件\n";
//    DWORD br;
//    CREATE_USN_JOURNAL_DATA cujd;
//    cujd.MaximumSize = 0;
//    cujd.AllocationDelta = 0;
//    status = DeviceIoControl(hVol,
//                             FSCTL_CREATE_USN_JOURNAL,
//                             &cujd,
//                             sizeof(cujd),
//                             NULL,
//                             0,
//                             &br,
//                             NULL);

//    if (0 != status) {
//        cout << "初始化USN日志文件成功！转向step-04.\n";
//        initUsnJournalSuccess = true;
//    } else
//        cout << "初始化USN日志文件失败 —— status:" << status << " error:" << GetLastError() << "\n";
//}
//JOURNAL_INFO journal_info;
//if (initUsnJournalSuccess) {

//    BOOL getBasicInfoSuccess = false;


//    step 04. 获取USN日志基本信息(用于后续操作)
//    cout << "step 04. 获取USN日志基本信息(用于后续操作)\n";
//    DWORD br;
//    status = DeviceIoControl(hVol,
//                             FSCTL_QUERY_USN_JOURNAL,
//                             NULL,
//                             0,
//                             &UsnInfo,
//                             sizeof(UsnInfo),
//                             &br,
//                             NULL);

//    DWORD bytes_returned;
//    USN_JOURNAL_DATA ujd;

//    if (DeviceIoControl(hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &ujd, sizeof(ujd), &bytes_returned, NULL)) {
//        journal_info.journal_id = ujd.UsnJournalID;
//        journal_info.high_usn = ujd.NextUsn;
//        cout << "  safasf 获取USN日志基本信息成功！转向step-05.\n";
//    } else {
//        CloseHandle(hVol);
//        return false;
//    }


//    if (0 != status) {
//        cout << "获取USN日志基本信息成功！转向step-05.\n";
//        getBasicInfoSuccess = true;
//    } else
//        cout << "获取USN日志基本信息失败 —— status:" << status << " error:" << GetLastError() << "\n";
//    if (getBasicInfoSuccess) {
//        cout << "UsnJournalID: " << UsnInfo.UsnJournalID << "\n";
//        cout << "lowUsn: " << UsnInfo.FirstUsn << "\n";
//        cout << "highUsn: " << UsnInfo.NextUsn << "\n";

//        step 05. 枚举USN日志文件中的所有记录
//        cout << "step 05. 枚举USN日志文件中的所有记录\n";
//        MFT_ENUM_DATA_V0 med;
//        med.StartFileReferenceNumber = 0;
//        med.LowUsn = 0;
//        med.HighUsn = journal_info.high_usn;

//        CHAR buffer[BUF_LEN]; //储存记录的缓冲,尽量足够地大 buf_len = 4096
//        DWORD usnDataSize;
//        PUSN_RECORD UsnRecord;
//        long clock_start = clock();

//        USN_JOURNAL_DATA ujd;

//        while (0 != DeviceIoControl(hVol,
//                                    FSCTL_ENUM_USN_DATA,
//                                    &med,
//                                    sizeof(med),
//                                    buffer,
//                                    BUF_LEN,
//                                    &usnDataSize,
//                                    NULL)) {
//            DWORD dwRetBytes = usnDataSize - sizeof(USN);

//            UsnRecord = (PUSN_RECORD)(((PCHAR)buffer) + sizeof(USN));// 找到第一个USN记录
//            while (dwRetBytes > 0) {
//                const int strLen = UsnRecord->FileNameLength;
//                char fileName[MAX_PATH] = { 0 };
//                char filePath[MAX_PATH] = {0};
//                WideCharToMultiByte(CP_OEMCP, NULL, UsnRecord->FileName, strLen / 2, fileName, strLen, NULL, FALSE);

//                cout << "FileName: " << fileName << "\n";
//                cout << "FileReferenceNumber: " << UsnRecord->FileReferenceNumber << "\n";
//                cout << "ParentFileReferenceNumber: " << UsnRecord->ParentFileReferenceNumber << "\n";
//                cout<< "FilePath: " << filePath << "\n\n";

//                fout << "FileName:" << fileName << endl;
//                fout << "FileReferenceNumber:" << UsnRecord->FileReferenceNumber << endl;
//                fout << "ParentFileReferenceNumber:" << UsnRecord->ParentFileReferenceNumber << endl;
//                fout << "FilePath:" << filePath << endl;
//                fout << endl;
//                counter++;

//                 获取下一个记录
//                DWORD recordLen = UsnRecord->RecordLength;
//                dwRetBytes -= recordLen;
//                UsnRecord = (PUSN_RECORD)(((PCHAR)UsnRecord) + recordLen);
//            }
//            med.StartFileReferenceNumber = *(USN*)&buffer;

//        }
//        cout << "共" << counter << "个文件\n";
//        long clock_end = clock();
//        cout << "花费" << clock_end - clock_start << "毫秒" << endl;
//        fout << "共" << counter << "个文件" << endl;
//        fout << flush;
//        fout.close();
//    }


//    step 06. 删除USN日志文件(当然也可以不删除)
//    cout << "step 06. 删除USN日志文件(当然也可以不删除)\n";
//    DELETE_USN_JOURNAL_DATA dujd;
//    dujd.UsnJournalID = UsnInfo.UsnJournalID;
//    dujd.DeleteFlags = USN_DELETE_FLAG_DELETE;

//    status = DeviceIoControl(hVol,
//                             FSCTL_DELETE_USN_JOURNAL,
//                             &dujd,
//                             sizeof(dujd),
//                             NULL,
//                             0,
//                             &br,
//                             NULL);

//    if (0 != status)
//        cout << "成功删除USN日志文件!\n";
//    else
//        cout << "删除USN日志文件失败 —— status:" << status << " error:" << GetLastError() << "\n";
//}
//if (getHandleSuccess)
//    CloseHandle(hVol);
////释放资源




//usn_manager usn;
//usn.start();


//   char* volName = (char*)"w:\\";
//   // memset(volName, 0, sizeof(volName)/sizeof(char *));
//   HANDLE hVol = new HANDLE;
//USN_JOURNAL_DATA UsnInfo = {}; // 储存USN日志的基本信息

//   usn.watch_usn(volName, hVol, UsnInfo);