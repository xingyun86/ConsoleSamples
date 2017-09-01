
#include <tchar.h>
#include <stdio.h>
#include <windows.h> 
#include <lm.h>

#include <iostream>
#pragma comment(lib, "netapi32.lib")

TCHAR* envVarStrings[] =
{
	TEXT("OS         = %OS%"),
	TEXT("PATH       = %PATH%"),
	TEXT("HOMEPATH   = %HOMEPATH%"),
	TEXT("TEMP       = %TEMP%")
};
#define  ENV_VAR_STRING_COUNT  (sizeof(envVarStrings)/sizeof(TCHAR*))
#define INFO_BUFFER_SIZE 32767

__inline static void GetComputerInfo()
{
	DWORD i;
	TCHAR  infoBuf[INFO_BUFFER_SIZE];
	DWORD  bufCharCount = INFO_BUFFER_SIZE;

	// Get and display the name of the computer. 
	bufCharCount = INFO_BUFFER_SIZE;
	if (!GetComputerName(infoBuf, &bufCharCount))
	{
		_tprintf(TEXT("GetComputerName(error=%s)\r\n"), PPSHUAI::ParseError(GetLastError()).c_str());
	}
	
	_tprintf(TEXT("\nComputer name:      %s"), infoBuf);

	// Get and display the user name. 
	bufCharCount = INFO_BUFFER_SIZE;
	if (!GetUserName(infoBuf, &bufCharCount))
	{
		_tprintf(TEXT("GetUserName(error=%s)\r\n"), PPSHUAI::ParseError(GetLastError()).c_str());
	}
	_tprintf(TEXT("\nUser name:          %s"), infoBuf);

	// Get and display the system directory. 
	if (!GetSystemDirectory(infoBuf, INFO_BUFFER_SIZE))
	{
		_tprintf(TEXT("GetSystemDirectory(error=%s)\r\n"), PPSHUAI::ParseError(GetLastError()).c_str());
	}
	_tprintf(TEXT("\nSystem Directory:   %s"), infoBuf);

	// Get and display the Windows directory. 
	if (!GetWindowsDirectory(infoBuf, INFO_BUFFER_SIZE))
	{
		_tprintf(TEXT("GetWindowsDirectory(error=%s)\r\n"), PPSHUAI::ParseError(GetLastError()).c_str());
	}
	_tprintf(TEXT("\nWindows Directory:  %s"), infoBuf);

	// Expand and display a few environment variables. 
	_tprintf(TEXT("\n\nSmall selection of Environment Variables:"));
	for (i = 0; i < ENV_VAR_STRING_COUNT; ++i)
	{
		bufCharCount = ExpandEnvironmentStrings(envVarStrings[i], infoBuf,
			INFO_BUFFER_SIZE);
		if (bufCharCount > INFO_BUFFER_SIZE)
			_tprintf(TEXT("\n\t(Buffer too small to expand: \"%s\")"),
			envVarStrings[i]);
		else if (!bufCharCount)
		{
			_tprintf(TEXT("ExpandEnvironmentStrings(error=%s)\r\n"), PPSHUAI::ParseError(GetLastError()).c_str());
		}
		else
			_tprintf(TEXT("\n   %s"), infoBuf);
	}
	_tprintf(TEXT("\n\n"));
}

__inline static void GetSystemInfo()
{
	SYSTEM_INFO siSysInfo;

	// Copy the hardware information to the SYSTEM_INFO structure. 

	GetSystemInfo(&siSysInfo);

	// Display the contents of the SYSTEM_INFO structure. 

	printf("Hardware information: \n");
	printf("  OEM ID: %u\n", siSysInfo.dwOemId);
	printf("  Number of processors: %u\n",
		siSysInfo.dwNumberOfProcessors);
	printf("  Page size: %u\n", siSysInfo.dwPageSize);
	printf("  Processor type: %u\n", siSysInfo.dwProcessorType);
	printf("  Minimum application address: 0x%lX\n",
		siSysInfo.lpMinimumApplicationAddress);
	printf("  Maximum application address: 0x%lX\n",
		siSysInfo.lpMaximumApplicationAddress);
	printf("  Active processor mask: %u\n",
		siSysInfo.dwActiveProcessorMask);

}

__inline static void NetGetSystemInfo(int argc, _TCHAR ** argv)
{
	DWORD dwLevel = 102;
	LPWKSTA_INFO_102 pBuf = NULL;
	NET_API_STATUS nStatus;
	WCHAR * pszServerName = NULL;
	//
	// Check command line arguments.
	//
	if (argc > 2)
	{
		fwprintf(stderr, L"Usage: %s [\\\\ServerName]\n", argv[0]);
		exit(1);
	}
	// The server is not the default local computer.
	//
	if (argc == 2)
		pszServerName = __wargv[1];
	//
	// Call the NetWkstaGetInfo function, specifying level 102.
	//
	nStatus = NetWkstaGetInfo(pszServerName,
		dwLevel,
		(LPBYTE *)&pBuf);
	//
	// If the call is successful,
	//  print the workstation data.
	//
	if (nStatus == NERR_Success)
	{
		printf("\n\tPlatform: %d\n", pBuf->wki102_platform_id);
		wprintf(L"\tName:     %s\n", pBuf->wki102_computername);
		printf("\tVersion:  %d.%d\n", pBuf->wki102_ver_major,
			pBuf->wki102_ver_minor);
		wprintf(L"\tDomain:   %s\n", pBuf->wki102_langroup);
		wprintf(L"\tLan Root: %s\n", pBuf->wki102_lanroot);
		wprintf(L"\t# Logged On Users: %d\n", pBuf->wki102_logged_on_users);
	}
	//
	// Otherwise, indicate the system error.
	//
	else
	{
		fprintf(stderr, "A system error has occurred: %d\n", nStatus);
	}
	//
	// Free the allocated memory.
	//
	if (pBuf != NULL)
	{
		NetApiBufferFree(pBuf);
		pBuf = NULL;
	}
}