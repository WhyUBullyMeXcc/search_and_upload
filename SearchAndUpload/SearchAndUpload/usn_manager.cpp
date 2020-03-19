#include "usn_manager.h"
#pragma warning(disable : 4700)
#include "global.h"


typedef NTSTATUS(NTAPI* NTCREATEFILE)(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength);

typedef NTSTATUS(NTAPI* NTQUERYINFORMATIONFILE)(
    HANDLE handle,
    PIO_STATUS_BLOCK io,
    PVOID ptr,
    ULONG len,
    FILE_INFORMATION_CLASS FileInformationClass);

typedef NTSTATUS(NTAPI* NTCLOSE)(
    IN HANDLE Handle);



long counters = 0;


void usn_manager::start(vector <string> drives) {
    //vector <thread > tp;
    for (int i = 0; i < drives.size(); i++) {
        thread jt(&usn_manager::watch_usns, this, drives.at(i), i);
        jt.join();
        //tp.push_back(jt);
    }
    ////Sleep(20000);
    //for (int i = 0; i < tp.size(); i++) {
    //    if (tp.at(i).joinable())
    //        tp.at(i).join();
    //}
}

int usn_manager::file_type(char* patName, char* relName) {
    string pat;
    string allname;
    pat = patName;
    allname = relName;
    int flag = 0;

    //string temp[] = { ".exe",
    //                  ".cpp",
    //                  ".zip",
    //                  ".css"
    //                };

    //sizeof(temp) / sizeof(string)
    for (int i = 0; i < G_file_types.size() ; i++) {
        // cout << temp[i] << endl;
        int f_i = allname.find(G_file_types.at(i), allname.length() - G_file_types.at(i).length() - 1);
        if (f_i != allname.npos)
            flag = 1;
    }
    return flag;
}

HMODULE usn_manager::load_ntdll(HMODULE hmodule) {
    LPCSTR MYDLL = "ntdll.dll";
    hmodule = LoadLibrary(MYDLL);
    if (!hmodule)
        cout << "载入ntdll.dll失败" << endl
             << endl;
    return hmodule;
}


void usn_manager::get_path_from_frn(HANDLE& volume_handle, DWORDLONG frn, string volpath) {

    HMODULE hmodule = NULL;

    hmodule = load_ntdll(hmodule);

    NTCREATEFILE NtCreateFile = NULL;
    NtCreateFile = (NTCREATEFILE)GetProcAddress(hmodule, "NtCreateFile");
    if (!NtCreateFile)
        cout << "导出NtCreateFile函数失败" << endl
             << endl;

    NTQUERYINFORMATIONFILE NtQueryInformationFile = NULL;
    NtQueryInformationFile = (NTQUERYINFORMATIONFILE)GetProcAddress(hmodule, "NtQueryInformationFile");
    if (!NtQueryInformationFile)
        cout << "导出NtQueryInformationFile函数失败" << endl
             << endl;

    NTCLOSE NtClose = NULL;
    NtClose = (NTCLOSE)GetProcAddress(hmodule, "NtClose");
    if (!NtClose)
        cout << "导出NtClose函数失败" << endl
             << endl;

    UNICODE_STRING us_id;
    OBJECT_ATTRIBUTES oa;
    IO_STATUS_BLOCK isb;
    LARGE_INTEGER id;
    HANDLE file_handle = NULL;

    id.QuadPart = frn;

    us_id.Buffer = (WCHAR*)&id;
    us_id.Length = 8;
    us_id.MaximumLength = 8;

    oa.ObjectName = &us_id;
    oa.Length = sizeof(OBJECT_ATTRIBUTES);
    oa.RootDirectory = volume_handle;
    oa.Attributes = OBJ_CASE_INSENSITIVE;
    oa.SecurityDescriptor = NULL;
    oa.SecurityQualityOfService = NULL;

    //使用NtCreateFile函数通过文件ID获得目标文件的句柄

    ULONG status = NtCreateFile(
                       &file_handle,
                       FILE_GENERIC_READ,
                       &oa,
                       &isb,
                       NULL,
                       FILE_ATTRIBUTE_NORMAL,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       FILE_OPEN,
                       FILE_OPEN_BY_FILE_ID,
                       NULL,
                       0);

    if (0 == status) {
        FILE_NAME_INFORMATION* name_info = (FILE_NAME_INFORMATION*)malloc(512);

        //利用刚刚得到的句柄通过NtQueryInformationFile函数获得路径信息

        status = NtQueryInformationFile(
                     file_handle,
                     &isb,
                     name_info,
                     512,
                     FileNameInformation);

        if (0 == status) {
            char path_buf[260] = { 0 };
            int path_size = (*name_info).FileNameLength;
            //宽字符转为char型
            WideCharToMultiByte(CP_OEMCP, 0, (*name_info).FileName, path_size / 2, path_buf, path_size, NULL, NULL);
            //WideCharToMultiByte(1361, 0, (*name_info).FileName, path_size / 2, path_buf, path_size, NULL, NULL);
            //path_buf = (*name_info).FileName;

            wstring path_buffer;
            path_buffer = (*name_info).FileName;

            cout << "File Path：" << volpath.substr(0, 2) << path_buf << endl
                 << endl;

            G_change_files_path.push_back(volpath.substr(0, 2) + path_buf);
        }

        free(name_info);
        NtClose(file_handle); //关闭目标文件句柄
    }

}

void usn_manager::watch_usns(string path, int oper) {
    //cout << "current oper" << oper << endl;
    vector<DWORDLONG> G_element_node;
    if (G_drives_scan_result.size() > oper)
        G_element_node = G_drives_scan_result.at(oper);

    char* volName = (char*)path.c_str();
    HANDLE hVol = INVALID_HANDLE_VALUE;
    USN_JOURNAL_DATA UsnInfo; // 储存USN日志的基本信息
    BOOL status;
    BOOL isNTFS = false;
    BOOL getHandleSuccess = false;
    BOOL initUsnJournalSuccess = false;

    //判断驱动盘是否NTFS格式
    // cout << "step 01. nstf 判断驱动盘是否NTFS格式\n";
    //cout << "step 01. nstf \n";
    char sysNameBuf[MAX_PATH] = { 0 };
    status = GetVolumeInformationA(volName,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   sysNameBuf, // 驱动盘的系统名
                                   MAX_PATH);
    //cout << status << endl;
    if (0 != status) {
        //cout << "file system:" << sysNameBuf << "\n";

        // 比较字符串
        if (0 == strcmp(sysNameBuf, "NTFS")) {
            //cout << "此驱动盘是NTFS格式！转向step-02.\n";
            isNTFS = true;
        } else
            cout << "该驱动盘非NTFS格式\n";
    }

    if (isNTFS) {
        //step 02. 获取驱动盘句柄
        //cout << "step 02. get handel \n";
        char fileName[MAX_PATH];
        fileName[0] = '\0';
        strcpy_s(fileName, "\\\\.\\"); //传入的文件名
        strcat_s(fileName, volName);
        string fileNameStr = (string)fileName;
        fileNameStr.erase(fileNameStr.find_last_of(":") + 1);
        cout << "正在扫描:" << volName << "\n";

        hVol = CreateFileA(fileNameStr.data(), //可打开或创建以下对象，并返回可访问的句柄：控制台，通信资源，目录（只读打开），磁盘驱动器，文件
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_READONLY,
                           NULL);
        //cout << hVol << endl;

        if (INVALID_HANDLE_VALUE != hVol) {
            //cout << "获取驱动盘句柄成功！转向step-03.\n";
            getHandleSuccess = true;
        } else
            cout << "获取驱动盘句柄失败 —— handle:" << hVol << " error:" << GetLastError() << "\n";
    }

    if (getHandleSuccess) {
        //step 03. 初始化USN日志文件
        //cout << "step 03. 初始化USN日志文件\n";
        DWORD br;
        CREATE_USN_JOURNAL_DATA cujd;
        cujd.MaximumSize = 0;
        cujd.AllocationDelta = 0;
        status = DeviceIoControl(hVol,
                                 FSCTL_CREATE_USN_JOURNAL,
                                 &cujd,
                                 sizeof(cujd),
                                 NULL,
                                 0,
                                 &br,
                                 NULL);

        if (0 != status) {
            //cout << "初始化USN日志文件成功！转向step-04.\n";
            initUsnJournalSuccess = true;
        } else
            cout << "初始化USN日志文件失败 —— status:" << status << " error:" << GetLastError() << "\n";
    }

    if (initUsnJournalSuccess) {

        BOOL getBasicInfoSuccess = false;

        //step 04. 获取USN日志基本信息(用于后续操作)
        //cout << "step 04. 获取USN日志基本信息(用于后续操作)\n";
        DWORD br;
        status = DeviceIoControl(hVol,
                                 FSCTL_QUERY_USN_JOURNAL,
                                 NULL,
                                 0,
                                 &UsnInfo,
                                 sizeof(USN_JOURNAL_DATA),
                                 &br,
                                 NULL);

        if (0 != status) {
            //cout << "获取USN日志基本信息成功！转向step-05.\n";
            getBasicInfoSuccess = true;
        } else
            cout << "获取USN日志基本信息失败 —— status:" << status << " error:" << GetLastError() << "\n";
        if (getBasicInfoSuccess) {
            //cout << "UsnJournalID: " << UsnInfo.UsnJournalID << "\n";
            //cout << "lowUsn: " << UsnInfo.FirstUsn << "\n";
            //cout << "highUsn: " << UsnInfo.NextUsn << "\n";

            //step 05. 枚举USN日志文件中的所有记录
            //cout << "step 05. 枚举USN日志文件中的所有记录\n";
            MFT_ENUM_DATA_V0 med;
            med.StartFileReferenceNumber = 0;
            med.LowUsn = 0;
            med.HighUsn = UsnInfo.NextUsn;

            CHAR buffer[BUF_LEN]; //储存记录的缓冲,尽量足够地大 buf_len = 4096
            DWORD usnDataSize;
            PUSN_RECORD UsnRecord;
            PUSN_RECORD* sUsnRecord = new PUSN_RECORD;
            long clock_start = clock();

            vector<DWORDLONG> element_node;
            vector<DWORDLONG> tmp_vector;



            while (0 != DeviceIoControl(hVol,
                                        FSCTL_ENUM_USN_DATA,
                                        &med,
                                        sizeof(med),
                                        buffer,
                                        BUF_LEN,
                                        &usnDataSize,
                                        NULL)) {
                DWORD dwRetBytes = usnDataSize - sizeof(USN);

                UsnRecord = (PUSN_RECORD)(((PCHAR)buffer) + sizeof(USN)); // 找到第一个USN记录
                while (dwRetBytes > 0) {
                    const int strLen = UsnRecord->FileNameLength;
                    char fileName[MAX_PATH] = { 0 };
                    //char filePath[MAX_PATH] = {0};
                    WideCharToMultiByte(CP_OEMCP, NULL, UsnRecord->FileName, strLen / 2, fileName, strLen, NULL, FALSE);

                    //cout << "FileName: " << fileName << "\n";
                    //cout << "FileReferenceNumber: " << UsnRecord->FileReferenceNumber << "\n";
                    //cout << "ParentFileReferenceNumber: " << UsnRecord->ParentFileReferenceNumber << "\n";
                    ////cout<< "FilePath: " << filePath << "\n\n";
                    if (file_type((char*)".cpp", fileName)) {

                        element_node.push_back(UsnRecord->FileReferenceNumber);

                        // 获取文件路径
                        // get_path_from_frn(hVol, UsnRecord->FileReferenceNumber);

                        //ofstream fout("E:\\log.txt", ios::app);
                        //fout << "FileName:" << fileName << endl;
                        //fout << "file type" << UsnRecord->FileAttributes << endl;
                        //fout << "FileReferenceNumber:" << UsnRecord->FileReferenceNumber << endl;
                        //fout << "ParentFileReferenceNumber:" << UsnRecord->ParentFileReferenceNumber << endl;
                        //fout << endl;
                        //fout << flush;
                        //fout.close();


                        counters++;

                    }

                    //fout << "FilePath:" << filePath << endl;

                    // 获取下一个记录
                    DWORD recordLen = UsnRecord->RecordLength;
                    dwRetBytes -= recordLen;
                    UsnRecord = (PUSN_RECORD)(((PCHAR)UsnRecord) + recordLen);
                }
                med.StartFileReferenceNumber = *(USN*)&buffer;
            }

            cout << "共监控" << counters << "个文件\n";


            //cout << "error" << GetLastError();
            std::sort(element_node.begin(), element_node.end());
            std::sort(G_element_node.begin(), G_element_node.end());


            if (G_element_node.size() == 0) {
                G_element_node = element_node;
                //G_element_node.erase(G_element_node.begin() + 2345);
            }

            tmp_vector = element_node;

            vector<DWORDLONG> diff_vce;

            set_symmetric_difference(G_element_node.begin(), G_element_node.end(), element_node.begin(), element_node.end(), inserter(diff_vce, diff_vce.begin())); ///取 对称差集运算

            if (diff_vce.size()) {
                cout << "文件有变更 变更数为:" << diff_vce.size() << endl;


                cout << "差异文件 FileReferenceNumber ID= {";
                vector<DWORDLONG>::iterator pos;
                for (pos = diff_vce.begin(); pos != diff_vce.end(); pos++) {
                    if (pos != diff_vce.begin())
                        cout << ", ";
                    cout << *pos;
                }

                cout << "}" << endl;
            }

            for (int i = 0; i < diff_vce.size(); i++) {
                cout << diff_vce.at(i) << endl;
                get_path_from_frn(hVol, diff_vce.at(i), path);
            }

            G_element_node = element_node;

            myMutex.lock();
            if (G_drives_scan_result.size() > oper)
                G_drives_scan_result[oper] = G_element_node;
            else
                G_drives_scan_result.push_back(G_element_node);
            myMutex.unlock();
            //vector<DWORDLONG> G_element_node = .at(oper);

            long clock_end = clock();
            cout << "花费" << clock_end - clock_start << "毫秒" << endl;
            //ofstream fout("E:\\log.txt", ios::app);
            //fout << "共" << counters << "个文件" << endl;
            //fout << flush;
            //fout.close();
        }

        //step 06. 删除USN日志文件(当然也可以不删除)
        //cout << "step 06. 删除USN日志文件(当然也可以不删除)\n";
        DELETE_USN_JOURNAL_DATA dujd;
        dujd.UsnJournalID = UsnInfo.UsnJournalID;
        dujd.DeleteFlags = USN_DELETE_FLAG_DELETE;

        status = 0;
        status = DeviceIoControl(hVol,
                                 FSCTL_DELETE_USN_JOURNAL,
                                 &dujd,
                                 sizeof(dujd),
                                 NULL,
                                 0,
                                 &br,
                                 NULL);

        if (0 != status)
            status = status;
        //cout << "成功删除USN日志文件!\n";
        else
            cout << "删除USN日志文件失败 —— status:" << status << " error:" << GetLastError() << "\n";
    }
    if (getHandleSuccess)
        CloseHandle(hVol);
    //释放资源
}

usn_manager::usn_manager() {
}

usn_manager::~usn_manager() {
    cout << "Usn_manager Object is being deleted" << endl;
}
