
#include <windows.h>
#include <commctrl.h>

#include <map>
#include "MACROS.h"


__inline static void Init_List_Data(DWORD dwPID, DWORD dwWndID)
{

#define BASE_ITEM_SIZE 0x200 //512

	LVITEM *plvi = NULL;
	LVITEM lvi = { 0 };
	HWND hWndListView = NULL;
	HWND hWndHeader = NULL; //listview控件的列头句柄
	DWORD dwRowIndex = 0;
	DWORD dwColIndex = 0;
	DWORD dwRowCount = 0; //listview控件中的行数
	DWORD dwColCount = 0; //listview控件中的列数
	DWORD ProcessID = NULL;
	HANDLE hProcess = NULL;
	LPTSTR lpItemData = NULL;
	DWORD dwItemSize = BASE_ITEM_SIZE;
	SIZE_T dwNumberOfBytesWritten = 0;
	_TCHAR tzItemData[BASE_ITEM_SIZE + 1] = { 0 };

	ProcessID = dwPID;

	hWndListView = (HWND)dwWndID;

	//listview的列头句柄
	hWndHeader = (HWND)::SendMessage(hWndListView, LVM_GETHEADER, 0, 0);

	//总行数:进程的数量
	dwRowCount = ::SendMessage(hWndListView, LVM_GETITEMCOUNT, 0, 0);
	//列表列数
	dwColCount = ::SendMessage(hWndHeader, HDM_GETITEMCOUNT, 0, 0);

	//打开并插入进程
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, ProcessID);
	if (!hProcess)
	{
		_tprintf(_T("OpenProcess() failed! ErrorId=%d\r\n"), GetLastError());
		goto __LEAVE_CLEAN__;
	}

	//申请代码的内存区
	plvi = (LVITEM *)::VirtualAllocEx(hProcess, NULL, sizeof(LVITEM), MEM_COMMIT, PAGE_READWRITE);
	if (!plvi)
	{
		_tprintf(_T("VirtualAllocEx() failed! ErrorId=%d\r\n"), GetLastError());
		goto __LEAVE_CLEAN__;
	}
	for (dwRowIndex = 0; dwRowIndex < dwRowCount; dwRowIndex++)
	{
		for (dwColIndex = 0; dwColIndex < dwColCount; dwColIndex++)
		{
			//清空内存空间
			memset(tzItemData, 0, sizeof(tzItemData));
			//清空内存空间
			memset(&lvi, 0, sizeof(lvi));
			//申请内存空间
			lpItemData = (LPTSTR)::VirtualAllocEx(hProcess, NULL, BASE_ITEM_SIZE * sizeof(_TCHAR), MEM_COMMIT, PAGE_READWRITE);
			if (lpItemData)
			{
				lvi.mask = LVIF_TEXT;    //说明pszText是有效的
				lvi.iItem = dwRowIndex;        //行号
				lvi.iSubItem = dwColIndex;        //列号
				lvi.cchTextMax = BASE_ITEM_SIZE;    //所能存储的最大的文本为512字节
				lvi.pszText = lpItemData;

				::WriteProcessMemory(hProcess, plvi, &lvi, sizeof(LVITEM), &dwNumberOfBytesWritten);
				::SendMessage(hWndListView, LVM_GETITEM, (WPARAM)dwRowIndex, (LPARAM)plvi);

				::ReadProcessMemory(hProcess, lpItemData, tzItemData, sizeof(tzItemData), &dwNumberOfBytesWritten);
				_tprintf(_T("==[%s]\r\n"), tzItemData);

				//释放内存空间
				::VirtualFreeEx(hProcess, lpItemData, 0, MEM_DECOMMIT | MEM_RELEASE);
				lpItemData = NULL;
			}
		}
	}

__LEAVE_CLEAN__:

	if (hProcess)
	{
		if (plvi)
		{
			//释放内存空间
			//在其它进程中释放申请的虚拟内存空间,MEM_RELEASE方式很彻底,完全回收
			VirtualFreeEx(hProcess, plvi, 0, MEM_DECOMMIT | MEM_RELEASE);
			plvi = NULL;
		}

		//关闭打开的进程对象
		CloseHandle(hProcess);
		hProcess = NULL;
	}
}
__inline static void GetListCtrlData()
{
	DWORD dwProcessID = 0;
	//获取桌面窗口句柄
	HWND hWndDesktop = ::GetDesktopWindow();
	HWND hWndApp = (HWND)::FindWindowEx(hWndDesktop, 0, _T("MetaQuotes::MetaTrader::5.00"), NULL);
	HWND hWndStandard = (HWND)::FindWindowEx(hWndApp, 0, NULL, _T("周期"));
	HWND hWndToolBox = (HWND)::FindWindowEx(hWndStandard, 0, NULL, _T("工具箱"));
	_TCHAR tzClassName[MAX_PATH] = { 0 };
	GetClassName(hWndToolBox, tzClassName, sizeof(tzClassName) / sizeof(_TCHAR));
	//进程界面窗口的句柄,通过SPY获取
	HWND hWndListview = (HWND)::FindWindowEx(hWndToolBox, 0, _T("SysListView32"), NULL);
	DWORD dwThreadID = GetWindowThreadProcessId(hWndListview, &dwProcessID);
	Init_List_Data(dwProcessID, (DWORD)hWndListview);
}
__inline static int GetListCtrlDataEx(HWND hWndListCtrl)
{
	{
		int i = 0;
		HWND hListWnd = (HWND)hWndListCtrl;
		HWND hHeaderWnd = ListView_GetHeader(hListWnd);
		int nColCount = Header_GetItemCount(hHeaderWnd);
		int nRowCount = ListView_GetItemCount(hListWnd);
		HD_ITEM hdi = { 0 };
		LV_ITEM lvi = { 0 };
		_TCHAR tText[MAX_PATH] = { 0 };
		for (i = 0; i < nColCount; i++)
		{
			memset(tText, 0, sizeof(tText));
			hdi.mask = HDI_TEXT;
			hdi.pszText = tText;
			hdi.cchTextMax = sizeof(tText);
			//Header_GetItem(hHeaderWnd, i, &hdi);
			PostMessage((hHeaderWnd), HDM_GETITEM, (WPARAM)(int)(i), (LPARAM)(HD_ITEM *)(&hdi));
			if (*tText)
			{
				_tprintf(_T("%d: [%s]\r\n"), i, tText);
			}
		}
		_tprintf(_T("==================================\r\n"));
		for (i = 0; i < nRowCount; i++)
		{
			memset(tText, 0, sizeof(tText));
			lvi.iItem = i;
			lvi.mask = LVIF_TEXT;
			lvi.pszText = tText;
			lvi.cchTextMax = sizeof(tText);
			ListView_GetItem(hListWnd, &lvi);
			if (*tText)
			{
				_tprintf(_T("%d: [%s]\r\n"), i, tText);
			}
		}
	}
	return 0;
}