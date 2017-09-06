
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
	HWND hWndHeader = NULL; //listview�ؼ�����ͷ���
	DWORD dwRowIndex = 0;
	DWORD dwColIndex = 0;
	DWORD dwRowCount = 0; //listview�ؼ��е�����
	DWORD dwColCount = 0; //listview�ؼ��е�����
	DWORD ProcessID = NULL;
	HANDLE hProcess = NULL;
	LPTSTR lpItemData = NULL;
	DWORD dwItemSize = BASE_ITEM_SIZE;
	SIZE_T dwNumberOfBytesWritten = 0;
	_TCHAR tzItemData[BASE_ITEM_SIZE + 1] = { 0 };

	ProcessID = dwPID;

	hWndListView = (HWND)dwWndID;

	//listview����ͷ���
	hWndHeader = (HWND)::SendMessage(hWndListView, LVM_GETHEADER, 0, 0);

	//������:���̵�����
	dwRowCount = ::SendMessage(hWndListView, LVM_GETITEMCOUNT, 0, 0);
	//�б�����
	dwColCount = ::SendMessage(hWndHeader, HDM_GETITEMCOUNT, 0, 0);

	//�򿪲��������
	hProcess = ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, ProcessID);
	if (!hProcess)
	{
		_tprintf(_T("OpenProcess() failed! ErrorId=%d\r\n"), GetLastError());
		goto __LEAVE_CLEAN__;
	}

	//���������ڴ���
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
			//����ڴ�ռ�
			memset(tzItemData, 0, sizeof(tzItemData));
			//����ڴ�ռ�
			memset(&lvi, 0, sizeof(lvi));
			//�����ڴ�ռ�
			lpItemData = (LPTSTR)::VirtualAllocEx(hProcess, NULL, BASE_ITEM_SIZE * sizeof(_TCHAR), MEM_COMMIT, PAGE_READWRITE);
			if (lpItemData)
			{
				lvi.mask = LVIF_TEXT;    //˵��pszText����Ч��
				lvi.iItem = dwRowIndex;        //�к�
				lvi.iSubItem = dwColIndex;        //�к�
				lvi.cchTextMax = BASE_ITEM_SIZE;    //���ܴ洢�������ı�Ϊ512�ֽ�
				lvi.pszText = lpItemData;

				::WriteProcessMemory(hProcess, plvi, &lvi, sizeof(LVITEM), &dwNumberOfBytesWritten);
				::SendMessage(hWndListView, LVM_GETITEM, (WPARAM)dwRowIndex, (LPARAM)plvi);

				::ReadProcessMemory(hProcess, lpItemData, tzItemData, sizeof(tzItemData), &dwNumberOfBytesWritten);
				_tprintf(_T("==[%s]\r\n"), tzItemData);

				//�ͷ��ڴ�ռ�
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
			//�ͷ��ڴ�ռ�
			//�������������ͷ�����������ڴ�ռ�,MEM_RELEASE��ʽ�ܳ���,��ȫ����
			VirtualFreeEx(hProcess, plvi, 0, MEM_DECOMMIT | MEM_RELEASE);
			plvi = NULL;
		}

		//�رմ򿪵Ľ��̶���
		CloseHandle(hProcess);
		hProcess = NULL;
	}
}
__inline static void GetListCtrlData()
{
	DWORD dwProcessID = 0;
	//��ȡ���洰�ھ��
	HWND hWndDesktop = ::GetDesktopWindow();
	HWND hWndApp = (HWND)::FindWindowEx(hWndDesktop, 0, _T("MetaQuotes::MetaTrader::5.00"), NULL);
	HWND hWndStandard = (HWND)::FindWindowEx(hWndApp, 0, NULL, _T("����"));
	HWND hWndToolBox = (HWND)::FindWindowEx(hWndStandard, 0, NULL, _T("������"));
	_TCHAR tzClassName[MAX_PATH] = { 0 };
	GetClassName(hWndToolBox, tzClassName, sizeof(tzClassName) / sizeof(_TCHAR));
	//���̽��洰�ڵľ��,ͨ��SPY��ȡ
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