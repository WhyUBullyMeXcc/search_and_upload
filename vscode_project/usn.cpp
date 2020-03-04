/**
* 对Ntfs下USN操作的示例程序
*/
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <time.h>
#include <winternl.h>
#include <winbase.h>
#include <vector>
#include <algorithm>

using namespace std;

#define BUF_LEN 4096

ofstream fout("E:\\log.txt");
long counter = 0;

vector<DWORDLONG> G_element_node;

//首先进行函数类型的定义

typedef NTSTATUS(NTAPI *NTCREATEFILE)(
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

typedef NTSTATUS(NTAPI *NTQUERYINFORMATIONFILE)(
	HANDLE handle,
	PIO_STATUS_BLOCK io,
	PVOID ptr,
	ULONG len,
	FILE_INFORMATION_CLASS FileInformationClass);

typedef NTSTATUS(NTAPI *NTCLOSE)(

	IN HANDLE Handle);



int file_type(char *patName, char *relName)
{
	string pat;
	string allname;
	pat = patName;
	allname = relName;
	int flag = 0;
	string temp[] = {".exe",
					 ".cpp",
					 ".zip",
					 ".css"};
	for (int i = 0; i < sizeof(temp) / sizeof(string); i++)
	{
		// cout << temp[i] << endl;
		int f_i = allname.find(temp[i], allname.length() - temp[i].length() - 1);
		if (f_i != allname.npos)
		{
			flag = 1;
		}
	}

	// cout << flag <<endl;
	// int index = allname.find(pat, allname.length() - pat.length() - 1);
	// if (index != allname.npos)
	// 	flag = 1;
	// else
	// 	return 0;
	return flag;
}

HMODULE load_ntdll(HMODULE hmodule)
{
	LPCSTR MYDLL = "ntdll.dll";
	hmodule = LoadLibrary(MYDLL);
	if (!hmodule)
		cout << "载入ntdll.dll失败" << endl
			 << endl;
	return hmodule;
}

//volume_handle为使用CreateFile获得的卷句柄
void get_path_from_frn(HANDLE &volume_handle, DWORDLONG frn)
{
	//Nt函数的导出

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

	//各种所需变量的定义

	UNICODE_STRING us_id; //UNICODE_STRING型的文件ID
	OBJECT_ATTRIBUTES oa;
	IO_STATUS_BLOCK isb;
	LARGE_INTEGER id;		   //文件ID
	HANDLE file_handle = NULL; //frn所指示的文件的句柄

	id.QuadPart = frn; //文件ID赋值为frn

	us_id.Buffer = (WCHAR *)&id; //建立UNICODE型的文件ID
	us_id.Length = 8;
	us_id.MaximumLength = 8;

	oa.ObjectName = &us_id; //赋值OBJECT_ATTRIBUTES结构体
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

	if (0 == status)
	{
		FILE_NAME_INFORMATION *name_info = (FILE_NAME_INFORMATION *)malloc(512);

		//利用刚刚得到的句柄通过NtQueryInformationFile函数获得路径信息

		status = NtQueryInformationFile(
			file_handle,
			&isb,
			name_info,
			512,
			FileNameInformation);

		if (0 == status)
		{
			char path_buf[260] = {0};
			int path_size = (*name_info).FileNameLength;
			//宽字符转为char型
			WideCharToMultiByte(CP_OEMCP, 0, (*name_info).FileName, path_size / 2, path_buf, path_size, NULL, NULL);
			cout << "路径："
				 << "w:" << path_buf << endl
				 << endl;
		}

		free(name_info);
		NtClose(file_handle); //关闭目标文件句柄
	}

	// CloseHandle(volume_handle);//关闭卷句柄
}

void watch_usn(char *volName, HANDLE hVol, USN_JOURNAL_DATA UsnInfo)
{
	BOOL status;
	BOOL isNTFS = false;
	BOOL getHandleSuccess = false;
	BOOL initUsnJournalSuccess = false;

	//判断驱动盘是否NTFS格式
	// cout << "step 01. nstf 判断驱动盘是否NTFS格式\n";
	cout << "step 01. nstf \n";
	char sysNameBuf[MAX_PATH] = {0};
	status = GetVolumeInformationA(volName,
								   NULL,
								   0,
								   NULL,
								   NULL,
								   NULL,
								   sysNameBuf, // 驱动盘的系统名
								   MAX_PATH);
	cout << status << endl;
	if (0 != status)
	{
		cout << "file system:" << sysNameBuf << "\n";
		// 比较字符串
		if (0 == strcmp(sysNameBuf, "NTFS"))
		{
			// cout << "此驱动盘是NTFS格式！转向step-02.\n";
			cout << "step-02.\n";
			isNTFS = true;
		}
		else
		{
			cout << "该驱动盘非NTFS格式\n";
		}
	}

	if (isNTFS)
	{
		//step 02. 获取驱动盘句柄
		cout << "step 02. get handel \n";
		char fileName[MAX_PATH];
		fileName[0] = '\0';
		strcpy_s(fileName, "\\\\.\\"); //传入的文件名
		strcat_s(fileName, volName);
		string fileNameStr = (string)fileName;
		fileNameStr.erase(fileNameStr.find_last_of(":") + 1);
		cout << "驱动盘地址:" << fileNameStr.data() << "\n";

		hVol = CreateFileA(fileNameStr.data(), //可打开或创建以下对象，并返回可访问的句柄：控制台，通信资源，目录（只读打开），磁盘驱动器，文件
						   GENERIC_READ | GENERIC_WRITE,
						   FILE_SHARE_READ | FILE_SHARE_WRITE,
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_READONLY,
						   NULL);
		cout << hVol << endl;

		if (INVALID_HANDLE_VALUE != hVol)
		{
			cout << "获取驱动盘句柄成功！转向step-03.\n";
			getHandleSuccess = true;
		}
		else
		{
			cout << "获取驱动盘句柄失败 —— handle:" << hVol << " error:" << GetLastError() << "\n";
		}
	}

	if (getHandleSuccess)
	{
		//step 03. 初始化USN日志文件
		cout << "step 03. 初始化USN日志文件\n";
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

		if (0 != status)
		{
			cout << "初始化USN日志文件成功！转向step-04.\n";
			initUsnJournalSuccess = true;
		}
		else
		{
			cout << "初始化USN日志文件失败 —— status:" << status << " error:" << GetLastError() << "\n";
		}
	}

	if (initUsnJournalSuccess)
	{

		BOOL getBasicInfoSuccess = false;

		//step 04. 获取USN日志基本信息(用于后续操作)
		cout << "step 04. 获取USN日志基本信息(用于后续操作)\n";
		DWORD br;
		status = DeviceIoControl(hVol,
								 FSCTL_QUERY_USN_JOURNAL,
								 NULL,
								 0,
								 &UsnInfo,
								 sizeof(USN_JOURNAL_DATA),
								 &br,
								 NULL);

		if (0 != status)
		{
			cout << "获取USN日志基本信息成功！转向step-05.\n";
			getBasicInfoSuccess = true;
		}
		else
		{
			cout << "获取USN日志基本信息失败 —— status:" << status << " error:" << GetLastError() << "\n";
		}
		if (getBasicInfoSuccess)
		{
			cout << "UsnJournalID: " << UsnInfo.UsnJournalID << "\n";
			cout << "lowUsn: " << UsnInfo.FirstUsn << "\n";
			cout << "highUsn: " << UsnInfo.NextUsn << "\n";

			//step 05. 枚举USN日志文件中的所有记录
			cout << "step 05. 枚举USN日志文件中的所有记录\n";
			MFT_ENUM_DATA med;
			med.StartFileReferenceNumber = 0;
			med.LowUsn = 0;
			med.HighUsn = UsnInfo.NextUsn;

			CHAR buffer[BUF_LEN]; //储存记录的缓冲,尽量足够地大 buf_len = 4096
			DWORD usnDataSize;
			PUSN_RECORD UsnRecord;
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
										NULL))
			{
				DWORD dwRetBytes = usnDataSize - sizeof(USN);

				UsnRecord = (PUSN_RECORD)(((PCHAR)buffer) + sizeof(USN)); // 找到第一个USN记录
				while (dwRetBytes > 0)
				{
					const int strLen = UsnRecord->FileNameLength;
					char fileName[MAX_PATH] = {0};
					//char filePath[MAX_PATH] = {0};
					WideCharToMultiByte(CP_OEMCP, NULL, UsnRecord->FileName, strLen / 2, fileName, strLen, NULL, FALSE);

					//cout << "FileName: " << fileName << "\n";
					//cout << "FileReferenceNumber: " << UsnRecord->FileReferenceNumber << "\n";
					//cout << "ParentFileReferenceNumber: " << UsnRecord->ParentFileReferenceNumber << "\n";
					////cout<< "FilePath: " << filePath << "\n\n";
					if (file_type(".cpp", fileName))
					{

						element_node.push_back(UsnRecord->FileReferenceNumber);

						// 获取文件路径
						// get_path_from_frn(hVol, UsnRecord->FileReferenceNumber);

						fout << "FileName:" << fileName << endl;
						fout << "file type" << UsnRecord->FileAttributes << endl;
						fout << "FileReferenceNumber:" << UsnRecord->FileReferenceNumber << endl;
						fout << "ParentFileReferenceNumber:" << UsnRecord->ParentFileReferenceNumber << endl;
						fout << endl;
						counter++;
					}

					//fout << "FilePath:" << filePath << endl;

					// 获取下一个记录
					DWORD recordLen = UsnRecord->RecordLength;
					dwRetBytes -= recordLen;
					UsnRecord = (PUSN_RECORD)(((PCHAR)UsnRecord) + recordLen);
				}
				med.StartFileReferenceNumber = *(USN *)&buffer;
			}

			std::sort(element_node.begin(), element_node.end());
			tmp_vector = element_node;
			cout << "共" << counter << "个文件\n";

			if (G_element_node.size() == 0)
			{
				G_element_node = element_node;
				G_element_node.erase(G_element_node.begin() + 2345);
			}
			int gn = G_element_node.size();
			int sn = element_node.size();
			if (gn != sn)
			{
				cout << "文件有变更 变更数为:" << sn - gn << endl;
				vector<DWORDLONG> change_file_node;

				for (int index = 0; index < G_element_node.size(); index++)
				{
					if (G_element_node.at(index) != tmp_vector.at(index))
					{
						change_file_node.push_back(tmp_vector.at(index));
						tmp_vector.erase(tmp_vector.begin() + index);
					}
				}
				while (G_element_node.size() != tmp_vector.size())
				{

					change_file_node.push_back(tmp_vector.at(tmp_vector.size() - 1));
					tmp_vector.erase(tmp_vector.end() - 1);
				}
				cout << "num" << change_file_node.size() << endl;

				for (int i = 0; i < change_file_node.size(); i++)
				{
					cout << change_file_node.at(i) << endl;
					get_path_from_frn(hVol, change_file_node.at(i));
				}
			}
			G_element_node = element_node;
			long clock_end = clock();
			cout << "花费" << clock_end - clock_start << "毫秒" << endl;
			fout << "共" << counter << "个文件" << endl;
			fout << flush;
			fout.close();
		}

		//step 06. 删除USN日志文件(当然也可以不删除)
		cout << "step 06. 删除USN日志文件(当然也可以不删除)\n";
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
		{
			cout << "成功删除USN日志文件!\n";
		}
		else
		{
			cout << "删除USN日志文件失败 —— status:" << status << " error:" << GetLastError() << "\n";
		}
	}
	if (getHandleSuccess)
	{
		CloseHandle(hVol);
	} //释放资源
}

int main()
{
	char *volName = "w:\\";
	// memset(volName, 0, sizeof(volName)/sizeof(char *));
	HANDLE hVol;
	USN_JOURNAL_DATA UsnInfo; // 储存USN日志的基本信息
	watch_usn(volName, hVol, UsnInfo);
	// Sleep(10000);
	// char *dv = "d:\\";
	// watch_usn(dv, hVol, UsnInfo);

	// while (1)
	// {

	// 	// watch_usn();
	// }
	//MessageBox(0, L"按确定退出", L"结束", MB_OK);
	// getchar();
	return 0;
}