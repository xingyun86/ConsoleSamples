#include "CommonHeader.h"

#include "SystemDataInfo.h"
#include "ListCtrlData.h"

int _tmain(int argc, _TCHAR *argv[])
{
	STRING_FORMAT_W(L"%d", 123);
	STRING_FORMAT_A("%d", 123);

	//ShowListCtrlData();
	
	//HWND hWndListView = 0x0000;
	//ShowListCtrlDataEx(hWndListView);

	//get_system_password(0, 0);
	
	//GetSystemInfo();
	//GetComputerInfo();

	//GetWindowsVersion();
	
	return 0;
}