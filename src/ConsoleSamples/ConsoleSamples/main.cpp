
#include "CommonHeader.h"
#include "FileMapHeader.h"
#include "HeapWalk.h"

void ReadListCtrlDataDemo(SIZE_T stMemoryAddressOffset, 
	const _TCHAR * ptProcessName = _T("terminal.exe"));

int memorysearchtest();

void PrintIconDirEntry(const PPSHUAI::PE::GROUPICONDIRENTRY * pGroupIconDirEntry) 
{
	_tprintf_p(_T("ID: %04d; width=%02d; height=%02d; bpp=%02d\r\n"),
		pGroupIconDirEntry->wID,
		pGroupIconDirEntry->bWidth, 
		pGroupIconDirEntry->bHeight, 
		pGroupIconDirEntry->wBitCount);
}

void PrintIconInfo(HICON hIcon)
{
	ICONINFO ii = { 0 };
	GetIconInfo(hIcon, &ii);
	_tprintf_p(_T("xHotspot=%02d; yHotspot=%02d\n"), ii.xHotspot, ii.yHotspot);
}

HICON LoadSpecificIcon(HMODULE hModule, WORD wId) 
{
	HRSRC hRsrc = FindResource(hModule, MAKEINTRESOURCE(wId), RT_ICON);
	HGLOBAL hGlobal = LoadResource(hModule, hRsrc);
	BYTE* lpData = (BYTE*)LockResource(hGlobal);
	DWORD dwSize = SizeofResource(hModule, hRsrc);

	HICON hIcon = CreateIconFromResourceEx(lpData, dwSize, TRUE, 0x00030000,
		0, 0, LR_DEFAULTCOLOR);
	return hIcon;
}

BOOL GetIconDirectory(HMODULE hMod, WORD wId) 
{
	std::map<WORD, PPSHUAI::PE::GROUPICONDIRENTRY> wgiemap;

	HRSRC hRsrc = ::FindResource(hMod, MAKEINTRESOURCE(wId), RT_GROUP_ICON);
	HGLOBAL hGlobal = ::LoadResource(hMod, hRsrc);
	PPSHUAI::PE::GROUPICONDIR * lpGroupIconDir = (PPSHUAI::PE::GROUPICONDIR *)::LockResource(hGlobal);

	for (WORD wIndex = 0; wIndex < lpGroupIconDir->wCount; ++wIndex) 
	{
		wgiemap.insert(std::map<WORD, PPSHUAI::PE::GROUPICONDIRENTRY>::value_type(wIndex, lpGroupIconDir->Entries[wIndex]));
	}
	return TRUE;
}

#if !defined(_CONSOLE) && !defined(CONSOLE)
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, INT nCmdShow)
#else
int _tmain(int argc, _TCHAR *argv[])
#endif //#if !defined(_CONSOLE) && !defined(CONSOLE)
{
	PPSHUAI::SystemKernel::PromotingPrivilege(SE_DEBUG_NAME, TRUE);
	//PPSHUAI::SystemKernel::PromotingPrivilege(SE_LOCK_MEMORY_NAME, TRUE);
	//CoInitialize(NULL);
	//OleInitialize(NULL);
	//PPSHUAI::GUI::ImagesRenderDisplay(NULL, NULL, _T("D:\\1.jpg"));
	{
		/*BOOL bRestult = PathFileExists(_T("D:\\A\\"));
		LPCTSTR lpT = RT_CURSOR;

		//heap_walk_main();
		//return 0;
		HICON hIconLarge = NULL;
		HICON hIconSmall = NULL;
		ExtractIconEx(_T("D:\\Tencent\\WeChat\\WeChat.exe"), 0, &hIconLarge, &hIconSmall, 1);
		DWORD dwError = GetLastError();
		dwError = 0;
		PPSHUAI::PE::EnumPEResources(_T("D:\\ttss.exe"));

		return 0;*/

		HMODULE hModule = LoadLibraryEx(_T("D:\\ttss.exe"), NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
		if (hModule)
		{
			//GetIconDirectory(hModule, 101);
		}
	}
	{
		PPSHUAI::TSTRINGVECTOR tv;

		PPSHUAI::TSTRINGVECTORMAP tvmap = {
			{ _T("进程名称"), tv },
			{ _T("进程ID"), tv },
		};
		std::map<DWORD, PROCESSENTRY32> pemap;
		std::map<DWORD, PROCESSENTRY32>::iterator itEnd;
		std::map<DWORD, PROCESSENTRY32>::iterator itIdx;
		PPSHUAI::SystemKernel::EnumProcess_R3(&pemap);
		itEnd = pemap.end();
		itIdx = pemap.begin();

		for (; itIdx != itEnd; itIdx++)
		{
			tvmap.at(_T("进程名称")).push_back(itIdx->second.szExeFile);
			tvmap.at(_T("进程ID")).push_back(PPSHUAI::STRING_FORMAT(_T("%ld"), itIdx->second.th32ProcessID));
		}
		
		PPSHUAI::COMMONWINDOW::CAnimationWindow aw(_T("D:\\test-png"), _T(".bmp"));
		aw.RunApp();

		PPSHUAI::COMMONWINDOW::CCryptoWindow cw;
		cw.RunApp();

		PPSHUAI::COMMONWINDOW::CSelectProcessWindow spw(&tvmap);
		spw.RunApp();
		PPSHUAI::COMMONWINDOW::CFileIconWindow fiw;
		fiw.RunApp();

		PPSHUAI::COMMONWINDOW::CWindowsManager wm;
		wm.RunApp();
		
		return 0;
	}

	{
		//crypt_tmain(argc, argv);
		//return 0;
	}
	
	{
		memorysearchtest();

		START_TIMER_TICKS(MAIN);

		CHAR * pszText = ("34.09");
		SIZE_T stTextSize = strlen(pszText);
		
		_TCHAR tszProcessName[MAX_PATH] = _T("terminal.exe");

		BYTE * pbData = NULL;
		HANDLE hProcess = NULL;
		HANDLE hFileMapping = NULL;
		std::map<SIZE_T, SIZE_T> rssmap;
		std::map<SIZE_T, SIZE_T> sssmap;
		std::map<SIZE_T, SIZE_T> rsssmap;
		ULARGE_INTEGER uiDataSize = { 0 };		
		
		std::map<SIZE_T, MEMORY_BASIC_INFORMATION> mbimap;

		hProcess = PPSHUAI::SystemKernel::InitProcessHandle(tszProcessName);

		if (hProcess)
		{
			PPSHUAI::SystemKernel::GetProcessMemoryPageList(&mbimap, hProcess);
			
			hFileMapping = PPSHUAI::SystemKernel::MapMemoryInitialize(&pbData, &uiDataSize, &rsssmap, &mbimap, hProcess);
			if (hFileMapping)
			{
				PPSHUAI::SystemKernel::MapMemorySearchString(&rssmap, (LPVOID *)pbData, &uiDataSize, pszText, stTextSize);
				
				PPSHUAI::SystemKernel::MapMemoryRelativeTransferAbsolute(&sssmap, &rssmap, &rsssmap, &mbimap);
				
				PPSHUAI::SystemKernel::MapMemoryExitialize(&hFileMapping, (LPVOID *)&pbData);
			}

			//PPSHUAI::SystemKernel::SearchProcessMemoryPageList(&sssmap, &mbimap, pszText, stTextSize, hProcess);

			//PPSHUAI::SystemKernel::SearchProcessMemoryPageListEx(&sssmap, &mbimap, pszText, stTextSize, hProcess);

			//PPSHUAI::SystemKernel::PrintProcessMemoryPages(&mbimap);

			PPSHUAI::SystemKernel::ExitProcessHandle(&hProcess);
		}
		
		CLOSE_TIMER_TICKS(MAIN);

		//ReadListCtrlDataDemo(0x4EDD0A8, _T("terminal.exe"));

		//ReadListCtrlDataDemo(0x45712FA, _T("terminal.exe"));	
		//ReadListCtrlDataDemo(0x457172A, _T("terminal.exe"));
		//ReadListCtrlDataDemo(0x4571B5A, _T("terminal.exe"));
		//ReadListCtrlDataDemo(0x45DD438, _T("terminal.exe"));
		return 0;
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

void ReadListCtrlDataDemo(SIZE_T stMemoryAddressOffset, const _TCHAR * ptProcessName/* = _T("terminal.exe")*/)
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
	PPSHUAI::SystemKernel::EnumProcess_R3(&pemap);
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

int memorysearchtest()
{
	START_TIMER_TICKS(DEBUG);

	_TCHAR tszProcessName[MAX_PATH] = _T("terminal.exe");

	HANDLE hProcess = NULL;
	std::map<SIZE_T, MEMORY_BASIC_INFORMATION> mbimap;
	std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itEnd;
	std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itIdx;
	BYTE * pbData = NULL;
	SIZE_T stReadPos = 0;
	SIZE_T stNumberOfBytesRead = 0;
	ULARGE_INTEGER ui = { 0 };
	std::map<SIZE_T, SIZE_T> mssmap;
	SYSTEM_INFO si = { 0 };
	PPSHUAI::SystemKernel::GetNativeSystemInformation(&si);
	std::string strFindStr("34.09");
	std::map<SIZE_T, SIZE_T> ssmap;
	std::map<SIZE_T, SIZE_T>::iterator itE;
	std::map<SIZE_T, SIZE_T>::iterator itS;
	std::string::size_type stSearchPos = 0;
	std::string strMemoryBuffer;// ((CHAR *)pbData, ui.QuadPart);
	BOOL bFound = FALSE;

	hProcess = PPSHUAI::SystemKernel::InitProcessHandle(tszProcessName);

	if (hProcess)
	{
		PPSHUAI::SystemKernel::GetProcessMemoryPageList(&mbimap, hProcess);

		itEnd = mbimap.end();
		itIdx = mbimap.begin();
		for (; itIdx != itEnd; itIdx++)
		{
			if (((itIdx->second.AllocationProtect & PAGE_READONLY) == PAGE_READONLY) ||
				((itIdx->second.AllocationProtect & PAGE_READWRITE) == PAGE_READWRITE) ||
				((itIdx->second.AllocationProtect & PAGE_WRITECOPY) == PAGE_WRITECOPY) ||
				((itIdx->second.AllocationProtect & PAGE_EXECUTE_READ) == PAGE_EXECUTE_READ) ||
				((itIdx->second.AllocationProtect & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE) ||
				((itIdx->second.AllocationProtect & PAGE_EXECUTE_WRITECOPY) == PAGE_EXECUTE_WRITECOPY))
			{
				ui.QuadPart += itIdx->second.RegionSize;
			}
		}
		ui.QuadPart += ui.QuadPart % si.dwAllocationGranularity;

		START_TIMER_TICKS(AAA);

		HANDLE hFileMapping = PPSHUAI::FilePath::MapCreate((LPVOID *)&pbData, _T("__MMAP__"), &ui);
		if (hFileMapping)
		{
			itEnd = mbimap.end();
			itIdx = mbimap.begin();

			for (; itIdx != itEnd; itIdx++)
			{
				if (ReadProcessMemory(hProcess, (LPCVOID)itIdx->first, pbData + stReadPos, itIdx->second.RegionSize, &stNumberOfBytesRead))
				{
					mssmap.insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)stReadPos, itIdx->first));
					stReadPos += stNumberOfBytesRead;
				}
			}

			START_TIMER_TICKS(assign);
			strMemoryBuffer.assign((CHAR *)pbData, ui.QuadPart);
			CLOSE_TIMER_TICKS(assign);

			stSearchPos = 0;
			while ((stSearchPos = strMemoryBuffer.find(strFindStr, stSearchPos)) != std::string::npos)
			{
				bFound = FALSE;
				itE = mssmap.end();
				itS = mssmap.begin();
				for (; itS != itE; itS++)
				{
					if ((SIZE_T)stSearchPos >= itS->first &&
						(SIZE_T)stSearchPos < (itS->first + mbimap[itS->second].RegionSize))
					{
						bFound = TRUE;
						break;
					}
				}
				if (bFound)
				{
					ssmap.insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)mbimap[itS->second].BaseAddress + (stSearchPos - itS->first), (SIZE_T)mbimap[itS->second].RegionSize));
				}
				stSearchPos += strFindStr.length();
			}

			PPSHUAI::FilePath::MapRelease(&hFileMapping, (LPVOID *)&pbData);
		}
		CLOSE_TIMER_TICKS(AAA);

		//PPSHUAI::SystemKernel::SearchProcessMemoryPageList(&ssmap, &mbimap, pszText, stSize, hProcess);
		//PPSHUAI::SystemKernel::SearchProcessMemoryPageListEx(&ssmap, &mbimap, pszText, stSize, hProcess);
		//PPSHUAI::SystemKernel::PrintProcessMemoryPages(&mbimap);
		PPSHUAI::SystemKernel::ExitProcessHandle(&hProcess);
	}

	//ReadListCtrlDataDemo(0x4EDD0A8, _T("terminal.exe"));

	//ReadListCtrlDataDemo(0x45712FA, _T("terminal.exe"));	
	//ReadListCtrlDataDemo(0x457172A, _T("terminal.exe"));
	//ReadListCtrlDataDemo(0x4571B5A, _T("terminal.exe"));
	//ReadListCtrlDataDemo(0x45DD438, _T("terminal.exe"));

	CLOSE_TIMER_TICKS(DEBUG);

	return 0;
}
