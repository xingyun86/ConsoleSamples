
#include "CommonHeader.h"

void ReadListCtrlDataDemo(SIZE_T stMemoryAddressOffset, const _TCHAR * ptProcessName = _T("terminal.exe"))
{
	DWORD dwPID = 0;
	BOOL bFound = FALSE;
	HANDLE hProcess = NULL;
	LPVOID lpBaseAddress = 0;
	SIZE_T stNumberOfBytesWritten = 0;
	_TCHAR tzTextData[0x10000] = { 0 };
	_TCHAR tzProcessName[MAX_PATH] = { 0 };
	std::map<DWORD, PROCESSENTRY32>::iterator itEnd;
	std::map<DWORD, PROCESSENTRY32>::iterator itIdx;
	std::map<DWORD, PROCESSENTRY32> pemap;

	_tsetlocale(LC_ALL, _T("chs"));

	lstrcpy(tzProcessName, ptProcessName);
	PPSHUAI::SystemKernel::EnumProcess_R3(pemap);
	itEnd = pemap.end();
	itIdx = pemap.begin();
	for (; itIdx != itEnd; itIdx++)
	{
		if (!_tcsicmp(itIdx->second.szExeFile, tzProcessName))
		{
			dwPID = itIdx->second.th32ProcessID;
			break;
		}
	}
	if (dwPID > 0)
	{
		lpBaseAddress = (LPVOID)(stMemoryAddressOffset);
		//打开并插入进程
		hProcess = ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, dwPID);
		if (hProcess)
		{
			while (1)
			{
				::ReadProcessMemory(hProcess, lpBaseAddress, tzTextData, sizeof(tzTextData), &stNumberOfBytesWritten);
				_tprintf(_T("[%s]"), tzTextData);
				_tprintf(_T("\r\n"));
				Sleep(1000);
				break;
			}
			CloseHandle(hProcess);
			hProcess = NULL;
		}
	}
}


int _tmain(int argc, _TCHAR *argv[])
{
	PPSHUAI::SystemKernel::PromotingPrivilege(SE_DEBUG_NAME, TRUE);

	{
		PPSHUAI::GUI::CWindowsManager wm;
		wm.RunApp();
		
		return 0;
	}

	{

		crypt_tmain(argc, argv);
		return 0;
	}
	{
		std::map<SIZE_T, SIZE_T> ssmap;
		CHAR * pText = ("34.09");
		SIZE_T stSize = strlen(pText);
		VOID * pvData = (VOID *)(pText);
		PPSHUAI::SystemKernel::SearchProcessMemoryPageListEx(ssmap, pvData, stSize, _T("terminal.exe"));

		//ShowProcessMemoryPages(_T("terminal.exe"));

		//ReadListCtrlDataDemo(0x4EDD0A8, _T("terminal.exe"));

		//ReadListCtrlDataDemo(0x45712FA, _T("terminal.exe"));	
		//ReadListCtrlDataDemo(0x457172A, _T("terminal.exe"));
		//ReadListCtrlDataDemo(0x4571B5A, _T("terminal.exe"));
		//ReadListCtrlDataDemo(0x45DD438, _T("terminal.exe"));

	}
	
	//STRING_FORMAT_W(L"%d", 123);
	//STRING_FORMAT_A("%d", 123);

	const char * pANSI = "结余:";
	const wchar_t * pUNICODE = PPSHUAI::Convert::ANSIToUnicode(pANSI).c_str();
	const char * pUTF8 = PPSHUAI::Convert::ANSI2UTF8(pANSI).c_str();
	//GetListCtrlData();
	
	//HWND hWndListView = 0x0000;
	//ShowListCtrlDataEx(hWndListView);

	//get_system_password(0, 0);
	
	//GetSystemInfo();
	//GetComputerInfo();

	//GetWindowsVersion();
	
	return 0;
}