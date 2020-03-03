// SearchAndUpload.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
//命令行工具实现搜索Windows系统全磁盘指定多钟后缀文件（比如docx、PDF、PPT等），通过打包压缩加密上传文件；
//可以上传第三方网盘（可以考虑dropbox网盘，需要挂VPn,百度网盘等），上传成功后，支持rar解压加密文件；

//支持搜索多国语言文件

#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <direct.h>
#include <iostream>
#include <io.h>
#include <fstream>
#include <cstring>
#include <string.h>
#include <windows.h>
#include <stack>

using namespace std;

#pragma warning(disable : 4996)

std::vector<std::string> get_all_files(std::string path, std::string suffix) {
    std::vector<std::string> files;
    //    files.clear();
    regex reg_obj(suffix, regex::icase);

    std::vector<std::string> paths;
    paths.push_back(path);



    return files;
}

int counts = 0;  //count用来统计文件数



int file_type(char* patName, char* relName) {
    string pat;
    string allname;
    pat = patName;
    allname = relName;
    int index = allname.find(pat, allname.length() - pat.length());
    if (index != allname.npos) {
        //cout << "ok" << endl;
        return 1;
    } else
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

int main() {
    std::cout << "Hello World!\n";

    //char dir[100];
    string dir = "W:/*.txt";
    listFiles((char*)"D:", (char*)".cpp", true);

    //Search((char*)"W:/LeetCode", (char*)"*.*", true);

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
