
#include <windows.h>
#pragma comment(lib, "msimg32.lib")
#include <tlhelp32.h>

#include <tchar.h>
#include <sys/stat.h>

#include <map>
#include <regex>
#include <vector>
#include <string>

#include "UNDOCAPI.h"

namespace PPSHUAI{

typedef std::vector<std::string> STRINGVECTOR;
typedef std::vector<std::wstring> WSTRINGVECTOR;

typedef std::vector<STRINGVECTOR> STRINGVECTORVECTOR;
typedef std::vector<WSTRINGVECTOR> WSTRINGVECTORVECTOR;

typedef std::map<std::string, STRINGVECTOR> STRINGVECTORMAP;
typedef std::map<std::wstring, WSTRINGVECTOR> WSTRINGVECTORMAP;

typedef std::map<std::string, std::string> STRINGSTRINGMAP;
typedef std::map<std::wstring, std::wstring> WSTRINGWSTRINGMAP;

typedef std::map<std::string, STRINGSTRINGMAP> STRINGSTRINGMAPMAP;
typedef std::map<std::wstring, WSTRINGWSTRINGMAP> WSTRINGWSTRINGMAPMAP;


#define __MY_A(V)				#V
#define __MY_W(V)				L##V

#if !defined(_UNICODE) && !defined(UNICODE)
#define __MY_T(V)				#V
#define TSTRING					std::string
#define TSTRINGVECTOR			STRINGVECTOR
#define TSTRINGVECTORVECTOR		STRINGVECTORVECTOR
#define TSTRINGTSTRINGMAP		STRINGSTRINGMAP
#define TSTRINGTSTRINGMAPMAP	STRINGSTRINGMAPMAP
#define TSTRINGVECTORMAP		STRINGVECTORMAP
#else
#define __MY_T(V)				L###V
#define TSTRING					std::wstring
#define TSTRINGVECTOR			WSTRINGVECTOR
#define TSTRINGVECTORVECTOR		WSTRINGVECTORVECTOR
#define TSTRINGTSTRINGMAP		WSTRINGWSTRINGMAP
#define TSTRINGTSTRINGMAPMAP	WSTRINGWSTRINGMAPMAP
#define TSTRINGVECTORMAP		WSTRINGVECTORMAP
#endif // !defined(_UNICODE) && !defined(UNICODE)

#define _tstring TSTRING
#define tstring TSTRING

__inline static std::string STRING_FORMAT_A(const CHAR * paFormat, ...)
{
	INT nAS = 0;
	std::string A = ("");
	
	va_list valist = { 0 };

	va_start(valist, paFormat);

	nAS = _vscprintf_p(paFormat, valist);
	if (nAS > 0)
	{
		A.resize((nAS + sizeof(CHAR)) * sizeof(CHAR), ('\0'));
		_vsnprintf((CHAR *)A.c_str(), nAS * sizeof(CHAR), paFormat, valist);
	}

	va_end(valist);

	return A.c_str();
}

__inline static std::wstring STRING_FORMAT_W(const WCHAR * pwFormat, ...)
{
	INT nWS = 0;
	std::wstring W = (L"");

	va_list valist = { 0 };

	va_start(valist, pwFormat);

	nWS = _vscwprintf_p(pwFormat, valist);
	if (nWS > 0)
	{
		W.resize((nWS + sizeof(WCHAR)) * sizeof(WCHAR), (L'\0'));
		_vsnwprintf((WCHAR *)W.c_str(), nWS * sizeof(WCHAR), pwFormat, valist);
	}

	va_end(valist);

	return W.c_str();
}

//解析错误标识为字符串
__inline static std::string ParseErrorA(DWORD dwErrorCodes, HINSTANCE hInstance = NULL)
{
	BOOL bResult = FALSE;
	HLOCAL hLocal = NULL;
	std::string strErrorText = ("");

	bResult = ::FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		hInstance,
		dwErrorCodes,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPSTR)&hLocal,
		0,
		NULL);
	if (!bResult)
	{
		if (hInstance)
		{
			bResult = ::FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_HMODULE |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				hInstance,
				dwErrorCodes,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
				(LPSTR)&hLocal,
				0,
				NULL);
			if (!bResult)
			{
				// failed
				// Unknown error code %08x (%d)
				strErrorText = STRING_FORMAT_A(("Unknown error code 0x%08X"), dwErrorCodes);
			}
		}
	}

	if (bResult && hLocal)
	{
		// Success
		LPSTR pT = (LPSTR)strchr((LPCSTR)hLocal, ('\r'));
		if (pT != NULL)
		{
			//Lose CRLF
			*pT = ('\0');
		}
		strErrorText = (LPCSTR)hLocal;
	}

	if (hLocal)
	{
		::LocalFree(hLocal);
		hLocal = NULL;
	}

	return strErrorText;
}
//解析错误标识为字符串
__inline static std::wstring ParseErrorW(DWORD dwErrorCodes, HINSTANCE hInstance = NULL)
{
	BOOL bResult = FALSE;
	HLOCAL hLocal = NULL;
	std::wstring strErrorText = (L"");

	bResult = ::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		hInstance,
		dwErrorCodes,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPWSTR)&hLocal,
		0,
		NULL);
	if (!bResult)
	{
		if (hInstance)
		{
			bResult = ::FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_HMODULE |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				hInstance,
				dwErrorCodes,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
				(LPWSTR)&hLocal,
				0,
				NULL);
			if (!bResult)
			{
				// failed
				// Unknown error code %08x (%d)
				strErrorText = STRING_FORMAT_W((L"Unknown error code 0x%08X"), dwErrorCodes);
			}
		}
	}

	if (bResult && hLocal)
	{
		// Success
		LPWSTR pT = (LPWSTR)wcschr((LPCWSTR)hLocal, (L'\r'));
		if (pT != NULL)
		{
			//Lose CRLF
			*pT = (L'\0');
		}
		strErrorText = (LPCWSTR)hLocal;
	}

	if (hLocal)
	{
		::LocalFree(hLocal);
		hLocal = NULL;
	}

	return strErrorText;
}

__inline static std::string ToUpperCaseA(std::string & strA)
{
	return strupr((LPSTR)strA.c_str());
}
__inline static std::wstring ToUpperCaseW(std::wstring & strW)
{
	return _wcsupr((LPWSTR)strW.c_str());
}

__inline static std::string ToLowerCaseA(std::string & strA)
{
	return strlwr((LPSTR)strA.c_str());
}
__inline static std::wstring ToLowerCaseW(std::wstring & strW)
{
	return _wcsupr((LPWSTR)strW.c_str());
}

__inline static std::string GetFilePathDriveA(LPCSTR lpFileName)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szDrive;
}
__inline static std::wstring GetFilePathDriveW(LPCWSTR lpFileName)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szDrive;
}
__inline static std::string GetFilePathDirA(LPCSTR lpFileName)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szDir;
}
__inline static std::wstring GetFilePathDirW(LPCWSTR lpFileName)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szDir;
}
__inline static std::string GetFilePathExtA(LPCSTR lpFileName)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szExt;
}
__inline static std::wstring GetFilePathExtW(LPCWSTR lpFileName)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szExt;
}
__inline static std::string GetFilePathFnameA(LPCSTR lpFileName)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szFname;
}
__inline static std::wstring GetFilePathFnameW(LPCWSTR lpFileName)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szFname;
}
__inline static void SplitFilePathA(LPCSTR lpFileName, std::string & strDrive, 
	std::string & strDir, std::string & strFname, std::string & strExt)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);
	strDrive = szDrive;
	strDir = szDir;
	strFname = szFname;
	strExt = szExt;
}
__inline static void SplitFilePathW(LPCWSTR lpFileName, std::wstring & strDrive,
	std::wstring & strDir, std::wstring & strFname, std::wstring & strExt)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);
	strDrive = szDrive;
	strDir = szDir;
	strFname = szFname;
	strExt = szExt;
}

//初始化调试窗口显示
__inline static void InitDebugConsole()
{
	FILE *pStdOut = stdout;
	FILE *pStdIn = stdin;
	FILE *pStdErr = stderr;

	if (!AllocConsole())
	{
		_TCHAR tErrorInfos[16384] = { 0 };
		_sntprintf(tErrorInfos, sizeof(tErrorInfos) / sizeof(_TCHAR), _T("控制台生成失败! 错误代码:0x%X。"), GetLastError());
		MessageBox(NULL, tErrorInfos, _T("错误提示"), 0);
		return;
	}
	SetConsoleTitle(_T("TraceDebugWindow"));

	pStdOut = _tfreopen(_T("CONOUT$"), _T("w"), stdout);
	pStdIn = _tfreopen(_T("CONIN$"), _T("r"), stdin);
	pStdErr = _tfreopen(_T("CONERR$"), _T("w"), stderr);
	_tsetlocale(LC_ALL, _T("chs"));
}

//释放掉调试窗口显示
__inline static void ExitDebugConsole()
{
	FreeConsole();
}

//获取毫秒时间计数器(返回结果为100纳秒的时间, 1ns=1 000 000ms=1000 000 000s)
#define MILLI_100NANO (ULONGLONG)(1000000ULL / 100ULL)
__inline static std::string GetCurrentSystemTimeA()
{
	CHAR szTime[MAXCHAR] = {0};
	SYSTEMTIME st = { 0 };
	//FILETIME ft = { 0 };
	//::GetSystemTimeAsFileTime(&ft);
	::GetLocalTime(&st);
	//::GetSystemTime(&st);
	//::SystemTimeToFileTime(&st, &ft);
	//::SystemTimeToTzSpecificLocalTime(NULL, &st, &st);
	wsprintfA(szTime, ("%04d-%02d-%02d %02d:%02d:%02d.%03d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return std::string(szTime);
}
__inline static std::wstring GetCurrentSystemTimeW()
{
	WCHAR wzTime[MAXCHAR] = { 0 };
	SYSTEMTIME st = { 0 };
	//FILETIME ft = { 0 };
	//::GetSystemTimeAsFileTime(&ft);
	::GetLocalTime(&st);
	//::GetSystemTime(&st);
	//::SystemTimeToFileTime(&st, &ft);
	//::SystemTimeToTzSpecificLocalTime(NULL, &st, &st);
	wsprintfW(wzTime, (L"%04d-%02d-%02d %02d:%02d:%02d.%03d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return std::wstring(wzTime);
}

#if !defined(_UNICODE) && !defined(UNICODE)
#define ToUpperCase				ToUpperCaseA
#define ToLowerCase				ToLowerCaseA
#define STRING_FORMAT			STRING_FORMAT_A
#define GetCurrentSystemTime	GetCurrentSystemTimeA
#define ParseError				ParseErrorA
#define GetFilePathDrive		GetFilePathDriveA
#define GetFilePathDir			GetFilePathDirA
#define GetFilePathExt			GetFilePathExtA
#define GetFilePathFname		GetFilePathFnameA
#define SplitFilePath			SplitFilePathA

#else
#define ToUpperCase				ToUpperCaseW
#define ToLowerCase				ToLowerCaseW
#define STRING_FORMAT			STRING_FORMAT_W
#define GetCurrentSystemTime	GetCurrentSystemTimeW
#define ParseError				ParseErrorW
#define GetFilePathDrive		GetFilePathDriveW
#define GetFilePathDir			GetFilePathDirW
#define GetFilePathExt			GetFilePathExtW
#define GetFilePathFname		GetFilePathFnameW
#define SplitFilePath			SplitFilePathW
#endif // !defined(_UNICODE) && !defined(UNICODE)

__inline static LONGLONG GetCurrentTimerTicks()
{
	FILETIME ft = { 0 };
	SYSTEMTIME st = { 0 };
	ULARGE_INTEGER u = { 0, 0 };
	::GetSystemTime(&st);
	::SystemTimeToFileTime(&st, &ft);
	u.HighPart = ft.dwHighDateTime;
	u.LowPart = ft.dwLowDateTime;
	return u.QuadPart;
}
//获取运行时间间隔差值(输入参数单位为100纳秒)
__inline static LONGLONG GetIntervalTimerTicks(LONGLONG llTime)
{
	return (LONGLONG)((GetCurrentTimerTicks() - llTime) / MILLI_100NANO);
}
//时间间隔差值(输入参数单位为100纳秒)
__inline static LONGLONG SubtractTimerTicks(LONGLONG llTimeA, LONGLONG llTimeB)
{
	return (LONGLONG)((llTimeA - llTimeB) / MILLI_100NANO);
}

#if !defined(_DEBUG) && !defined(DEBUG)
#define START_TIMER_TICKS(x)
#define RESET_TIMER_TICKS(x)
#define CLOSE_TIMER_TICKS(x)
#else
#define START_TIMER_TICKS(x) ULONGLONG ull##x = PPSHUAI::GetCurrentTimerTicks();
#define RESET_TIMER_TICKS(x) ull##x = PPSHUAI::GetCurrentTimerTicks();
#define CLOSE_TIMER_TICKS(x) printf(("%s %s: %s() %llu ms\r\n"), PPSHUAI::GetCurrentSystemTimeA().c_str(), #x, __FUNCTION__, (PPSHUAI::GetCurrentTimerTicks() - ull##x) / MILLI_100NANO);
#endif

namespace Convert{
	//	ANSI to Unicode
	__inline static std::wstring ANSIToUnicode(const std::string str)
	{
		int len = 0;
		len = str.length();
		int unicodeLen = ::MultiByteToWideChar(CP_ACP,
			0,
			str.c_str(),
			-1,
			NULL,
			0);
		wchar_t * pUnicode;
		pUnicode = new  wchar_t[(unicodeLen + 1)];
		memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
		::MultiByteToWideChar(CP_ACP,
			0,
			str.c_str(),
			-1,
			(LPWSTR)pUnicode,
			unicodeLen);
		std::wstring rt;
		rt = (wchar_t*)pUnicode;
		delete pUnicode;
		return rt;
	}

	//Unicode to ANSI
	__inline static std::string UnicodeToANSI(const std::wstring str)
	{
		char* pElementText;
		int iTextLen;
		iTextLen = WideCharToMultiByte(CP_ACP,
			0,
			str.c_str(),
			-1,
			NULL,
			0,
			NULL,
			NULL);
		pElementText = new char[iTextLen + 1];
		memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
		::WideCharToMultiByte(CP_ACP,
			0,
			str.c_str(),
			-1,
			pElementText,
			iTextLen,
			NULL,
			NULL);
		std::string strText;
		strText = pElementText;
		delete[] pElementText;
		return strText;
	}
	//UTF - 8 to Unicode
	__inline static std::wstring UTF8ToUnicode(const std::string str)
	{
		int len = 0;
		len = str.length();
		int unicodeLen = ::MultiByteToWideChar(CP_UTF8,
			0,
			str.c_str(),
			-1,
			NULL,
			0);
		wchar_t * pUnicode;
		pUnicode = new wchar_t[unicodeLen + 1];
		memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
		::MultiByteToWideChar(CP_UTF8,
			0,
			str.c_str(),
			-1,
			(LPWSTR)pUnicode,
			unicodeLen);
		std::wstring rt;
		rt = (wchar_t*)pUnicode;
		delete pUnicode;
		return rt;
	}
	//Unicode to UTF - 8
	__inline static std::string UnicodeToUTF8(const std::wstring str)
	{
		char*   pElementText;
		int iTextLen;
		iTextLen = WideCharToMultiByte(CP_UTF8,
			0,
			str.c_str(),
			-1,
			NULL,
			0,
			NULL,
			NULL);
		pElementText = new char[iTextLen + 1];
		memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
		::WideCharToMultiByte(CP_UTF8,
			0,
			str.c_str(),
			-1,
			pElementText,
			iTextLen,
			NULL,
			NULL);
		std::string strText;
		strText = pElementText;
		delete[] pElementText;
		return strText;
	}

	__inline static std::string TToA(tstring tsT)
	{
		std::string str = "";

#if !defined(UNICODE) && !defined(_UNICODE)
		str = tsT;
#else
		str = UnicodeToANSI(tsT);
#endif

		return str;
	}

	__inline static std::wstring TToW(tstring tsT)
	{
		std::wstring wstr = L"";

#if !defined(UNICODE) && !defined(_UNICODE)
		wstr = ANSIToUnicode(tsT);
#else
		wstr = tsT;
#endif

		return wstr;
	}

	__inline static tstring AToT(std::string str)
	{
		tstring ts = _T("");

#if !defined(UNICODE) && !defined(_UNICODE)
		ts = str;
#else
		ts = ANSIToUnicode(str);
#endif

		return ts;
	}

	__inline static tstring WToT(std::wstring wstr)
	{
		tstring ts = _T("");

#if !defined(UNICODE) && !defined(_UNICODE)
		ts = UnicodeToANSI(wstr);
#else
		ts = wstr;
#endif

		return ts;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：LPWSTR字符串类型转换为LPSTR字符串类型
	// 参    数：LPWSTR参数类型
	// 返 回 值：返回LPSTR类型数据
	// 编 写 者: ppshuai 20141112
	//////////////////////////////////////////////////////////////////////////
	__inline static LPSTR UnicodeToAnsi(LPWSTR lpUnicode)
	{
		LPSTR pAnsi = NULL;
		int nAnsiLength = 0;
		if (lpUnicode)
		{
			nAnsiLength = WideCharToMultiByte(CP_ACP, NULL, lpUnicode, -1, NULL, 0, NULL, NULL);
			if (nAnsiLength)
			{
				pAnsi = (LPSTR)malloc(nAnsiLength * sizeof(CHAR));
				if (pAnsi)
				{
					WideCharToMultiByte(CP_ACP, NULL, lpUnicode, -1, pAnsi, nAnsiLength, NULL, NULL);
				}
			}
		}
		return pAnsi;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：LPWSTR字符串类型转换为LPSTR字符串类型
	// 参    数：LPWSTR参数类型
	// 返 回 值：返回LPSTR类型数据
	// 编 写 者: ppshuai 20141112
	//////////////////////////////////////////////////////////////////////////
	__inline static LPWSTR AnsiToUnicode(LPSTR lpAnsi)
	{
		LPWSTR pUnicode = NULL;
		int nUnicodeLength = 0;
		if (lpAnsi)
		{
			nUnicodeLength = MultiByteToWideChar(CP_ACP, NULL, lpAnsi, -1, NULL, 0);
			if (nUnicodeLength)
			{
				pUnicode = (LPWSTR)malloc(nUnicodeLength * sizeof(WCHAR));
				if (pUnicode)
				{
					MultiByteToWideChar(CP_ACP, NULL, lpAnsi, -1, pUnicode, nUnicodeLength);
				}
			}
		}
		return pUnicode;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：释放自分配的内存
	// 参    数：void *参数类型
	// 返 回 值：无返回值
	// 编 写 者: ppshuai 20141112
	//////////////////////////////////////////////////////////////////////////
	__inline static void LocalFreeMemory(LPVOID * lpPointer)
	{
		if (lpPointer && (*lpPointer))
		{
			free((*lpPointer));
			(*lpPointer) = NULL;
		}
	}
	//utf8 转 Unicode
	__inline static std::wstring Utf82Unicode(const std::string& utf8string)
	{
		int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
		if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
		{
			return std::wstring(L"");
		}

		std::vector<wchar_t> resultstring(widesize);

		int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);

		if (convresult != widesize)
		{
			return std::wstring(L"");
		}

		return std::wstring(&resultstring[0]);
	}

	//unicode 转为 ascii
	__inline static std::string WideByte2Acsi(std::wstring& wstrcode)
	{
		int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);
		if (asciisize == ERROR_NO_UNICODE_TRANSLATION || asciisize == 0)
		{
			return std::string("");
		}
		std::vector<char> resultstring(asciisize);
		int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);

		if (convresult != asciisize)
		{
			return std::string("");
		}

		return std::string(&resultstring[0]);
	}

	//utf-8 转 ascii
	__inline static std::string UTF_82ASCII(std::string& strUtf8Code)
	{
		std::string strRet("");
		//先把 utf8 转为 unicode
		std::wstring wstr = Utf82Unicode(strUtf8Code);
		//最后把 unicode 转为 ascii
		strRet = WideByte2Acsi(wstr);
		return strRet;
	}

	///////////////////////////////////////////////////////////////////////


	//ascii 转 Unicode
	__inline static std::wstring Acsi2WideByte(std::string& strascii)
	{
		int widesize = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, NULL, 0);
		if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
		{
			return std::wstring(L"");
		}
		std::vector<wchar_t> resultstring(widesize);
		int convresult = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, &resultstring[0], widesize);


		if (convresult != widesize)
		{
			return std::wstring(L"");
		}

		return std::wstring(&resultstring[0]);
	}
	
	//Unicode 转 Utf8
	__inline static std::string Unicode2Utf8(const std::wstring& widestring)
	{
		int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
		if (utf8size == 0)
		{
			return std::string("");
		}

		std::vector<char> resultstring(utf8size);

		int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);

		if (convresult != utf8size)
		{
			return std::string("");
		}

		return std::string(&resultstring[0]);
	}

	//ascii 转 Utf8
	__inline static std::string ASCII2UTF_8(std::string& strAsciiCode)
	{
		std::string strRet("");
		//先把 ascii 转为 unicode
		std::wstring wstr = Acsi2WideByte(strAsciiCode);
		//最后把 unicode 转为 utf8
		strRet = Unicode2Utf8(wstr);
		return strRet;
	}

	//ANSI转UTF8
	__inline static std::string ANSI2UTF8(std::string str)
	{
		return UnicodeToUTF8(ANSIToUnicode(str));
	}
	//UTF8转ANSI
	__inline static std::string UTF82ANSI(std::string str)
	{
		return UnicodeToANSI(UTF8ToUnicode(str));
	}

	__inline static TSTRING VarChangeToStr(VARIANT var)
	{
		TSTRING t = _T("");
		VARIANT varDest = { 0 };
		HRESULT hResult = S_FALSE;

		varDest.vt = VT_BSTR;
		varDest.bstrVal = BSTR(L"");

		hResult = ::VariantChangeType(&varDest, &var, VARIANT_NOUSEROVERRIDE | VARIANT_LOCALBOOL, VT_BSTR);
		if (SUCCEEDED(hResult))
		{
			t = WToT(varDest.bstrVal);
		}

		return t;
	}
	__inline static tstring VariantToTstring(VARIANT var)
	{
		tstring A = _T("");
		VARIANT varBSTR = { 0 };
		double dDecVal = 0.0f;
		SYSTEMTIME st = { 0 };
		//CString strFormat(_T(""));
		switch (var.vt)
		{
		case VT_EMPTY: // 0
			A = _T("");
			break;

		case VT_NULL: // 1
			A = _T("");
			break;

		case VT_I2: // 2
			A = STRING_FORMAT(_T("%d"), var.iVal);
			break;

		case VT_I4: // 3
			A = STRING_FORMAT(_T("%ld"), var.intVal);
			break;

		case VT_R4: // 4
			A = STRING_FORMAT(_T("%10.6f"), (double)var.fltVal);
			break;

		case VT_R8: // 5
			A = STRING_FORMAT(_T("%10.6f"), var.dblVal);
			break;

		case VT_CY: // 6
			varBSTR.vt = VT_BSTR;
			varBSTR.bstrVal = L"";
			VariantChangeType(&varBSTR, &var, VARIANT_NOVALUEPROP, varBSTR.vt);
			A = STRING_FORMAT(_T("%10.6f"), varBSTR.dblVal);
			break;

		case VT_DATE: // 7
			VariantTimeToSystemTime(var.date, &st);
			A = STRING_FORMAT(_T("%04d-%02d-%02d %02d:%02d:%02d.%d"),
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			break;

		case VT_BSTR: // 8
			A = WToT(var.bstrVal);
			break;

		case VT_DISPATCH: // 9
			A = STRING_FORMAT(_T("0x%lX"), var.pdispVal);
			break;

		case VT_ERROR: // 10
			A = STRING_FORMAT(_T("%ld"), var.scode);
			break;

		case VT_BOOL: // 11
			A = STRING_FORMAT(_T("%d"), varBSTR.boolVal);
			break;

		case VT_VARIANT: // 12
			A = STRING_FORMAT(_T("0x%lX"), var.pvarVal);
			break;

		case VT_UNKNOWN: // 13
			A = STRING_FORMAT(_T("Unknow Ptr:0x%lX"), var.pvarVal);
			break;

		case VT_DECIMAL: // 14
			dDecVal = var.decVal.Lo64;
			dDecVal *= (var.decVal.sign == 128) ? -1 : 1;
			dDecVal /= pow(10, var.decVal.scale);

			A = STRING_FORMAT(tstring(_T("%.") + STRING_FORMAT(_T("%d"), var.decVal.scale) + _T("f")).c_str(), dDecVal);
			break;

		case VT_I1: // 16
			A = STRING_FORMAT(_T("%C(0x%02X)"), var.cVal, var.cVal);
			break;

		case VT_UI1: // 17
			A = STRING_FORMAT(_T("%C(0x%02X)"), var.cVal, var.cVal);
			break;

		case VT_UI2: // 18
			A = STRING_FORMAT(_T("%u"), var.uiVal);
			break;

		case VT_UI4: // 19
			A = STRING_FORMAT(_T("%u"), var.uintVal);
			break;

		case VT_I8: // 20
			A = STRING_FORMAT(_T("%lld"), var.llVal);
			break;

		case VT_UI8: // 21
			A = STRING_FORMAT(_T("%llu"), var.ullVal);
			break;

		case VT_INT: // 22
			A = STRING_FORMAT(_T("%ld"), var.intVal);
			break;

		case VT_UINT: // 23
			A = STRING_FORMAT(_T("%lu"), var.uintVal);
			break;

		case VT_VOID: // 24
			A = _T("");
			break;

		case VT_HRESULT: // 25
			A = _T("");
			break;

		case VT_PTR: // 26
			A = _T("");
			break;

		case VT_SAFEARRAY: // 27
			//A = _T("");
		{
			HRESULT hResult = S_FALSE;
			LONG lIdx = 0;
			LONG lLbound = 0;
			LONG lUbound = 0;
			BSTR bsPropName = NULL;
			hResult = SafeArrayGetLBound(var.parray, 1, &lLbound);
			hResult = SafeArrayGetUBound(var.parray, 1, &lUbound);

			A += _T("[");
			for (lIdx = lLbound; lIdx <= lUbound; lIdx++) {
				// Get this property name.
				hResult = SafeArrayGetElement(var.parray, &lIdx, &bsPropName);
				if (lIdx != lLbound)
				{
					A += _T(",");
				}

				if (FAILED(hResult))
				{
					A += _T("");
				}
				else
				{
					A += STRING_FORMAT(_T("%s"), WToT(bsPropName));
				}
			}
			A += _T("]");
			//hResult = SafeArrayDestroy(var.parray);
			//var.parray = NULL;
		}
		break;

		case VT_CARRAY: // 28
			A = _T("");
			break;

		case VT_USERDEFINED: // 29
			A = _T("");
			break;

		case VT_LPSTR: // 30
			A = AToT(var.pcVal);
			break;

		case VT_LPWSTR: // 31
			A = WToT(var.bstrVal);
			break;

		case VT_RECORD: // 36
			A = _T("");
			break;

		case VT_INT_PTR: // 37
			A = _T("");
			break;

		case VT_UINT_PTR: // 38
			A = _T("");
			break;

		case VT_FILETIME: // 64
			A = _T("");
			break;

		case VT_BLOB: // 65
			A = _T("");
			break;

		case VT_STREAM: // 66
			A = _T("");
			break;

		case VT_STORAGE: // 67
			A = _T("");
			break;

		case VT_STREAMED_OBJECT: // 68
			A = _T("");
			break;

		case VT_STORED_OBJECT: // 69
			A = _T("");
			break;

		case VT_BLOB_OBJECT: // 70
			A = _T("");
			break;

		case VT_CF: // 71
			A = _T("");
			break;

		case VT_CLSID: // 72
			A = _T("");
			break;

		case VT_VERSIONED_STREAM: // 73
			A = _T("");
			break;

		case VT_BSTR_BLOB: // 0xfff
			A = _T("");
			break;

		case VT_VECTOR: // 0x1000
			A = _T("");
			break;
		case VT_ARRAY: // 0x2000
			A = _T("");
			break;
		case VT_ARRAY | VT_UI1: // 0x2011
		{
			HRESULT hResult = S_FALSE;
			LONG lIdx = 0;
			LONG lLbound = 0;
			LONG lUbound = 0;
			BYTE bValue = NULL;
			hResult = SafeArrayGetLBound(var.parray, 1, &lLbound);
			hResult = SafeArrayGetUBound(var.parray, 1, &lUbound);

			A += _T("[");
			for (lIdx = lLbound; lIdx <= lUbound; lIdx++) {
				// Get this property name.
				hResult = SafeArrayGetElement(var.parray, &lIdx, &bValue);
				if (lIdx != lLbound)
				{
					A += _T(",");
				}

				if (FAILED(hResult))
				{
					A += _T("");
				}
				else
				{
					A += STRING_FORMAT(_T("0x%02X"), bValue);
				}
			}
			A += _T("]");
			//hResult = SafeArrayDestroy(var.parray);
			//var.parray = NULL;
		}
		break;

		case VT_BYREF: // 0x4000
			A = _T("");
			break;

		case VT_RESERVED: // 0x8000
			A = _T("");
			break;

		case VT_ILLEGAL: // 0xffff
			A = _T("");
			break;

			//case VT_ILLEGALMASKED: // 0xfff
			//	break;
			//case VT_TYPEMASK: // 0xfff
			//	break;
		default:
			A = _T("");
			break;
		}
	}
}

namespace FilePath{

	__inline static BOOL SelectSaveFile(_TCHAR(&tFileName)[MAX_PATH], const _TCHAR * ptFilter = _T("Execute Files (*.EXE)\0*.EXE\0All Files (*.*)\0*.*\0\0"))
	{
		BOOL bResult = FALSE;
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = ptFilter;
		ofn.lpstrFile = tFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
		bResult = GetSaveFileName(&ofn);
		if (bResult == FALSE)
		{
			//dwError = CommDlgExtendedError();
			//return bResult;
		}
		return bResult;
	}
	__inline static BOOL SelectOpenFile(_TCHAR(&tFileName)[MAX_PATH], const _TCHAR * ptFilter = _T("Execute Files (*.EXE)\0*.EXE\0All Files (*.*)\0*.*\0\0"))
	{
		BOOL bResult = FALSE;
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFilter = ptFilter;
		ofn.lpstrFile = tFileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
		bResult = GetOpenFileName(&ofn);
		if (bResult == FALSE)
		{
			//dwError = CommDlgExtendedError();
			//return bResult;
		}
		return bResult;
	}
	//获取程序工作路径
	__inline static tstring GetWorkPath()
	{
		tstring tsWorkPath = _T("");
		_TCHAR tWorkPath[MAX_PATH] = { 0 };
		GetCurrentDirectory(MAX_PATH, tWorkPath);
		if (*tWorkPath)
		{
			tsWorkPath = tstring(tWorkPath) + _T("\\");
		}
		return tsWorkPath;
	}

	//获取系统临时路径
	__inline static tstring GetTempPath()
	{
		_TCHAR tTempPath[MAX_PATH] = { 0 };
		::GetTempPath(MAX_PATH, tTempPath);
		return tstring(tTempPath);
	}

	//获取程序文件路径
	__inline static tstring GetProgramPath()
	{
		tstring tsFilePath = _T("");
		_TCHAR * pFoundPosition = 0;
		_TCHAR tFilePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, tFilePath, MAX_PATH);
		if (*tFilePath)
		{
			pFoundPosition = _tcsrchr(tFilePath, _T('\\'));
			if (*(++pFoundPosition))
			{
				*pFoundPosition = _T('\0');
			}
			tsFilePath = tFilePath;
		}
		return tsFilePath;
	}

	__inline static //获取系统路径
		tstring GetSystemPath()
	{
		tstring tsSystemPath = _T("");
		_TCHAR tSystemPath[MAX_PATH] = { 0 };
		GetSystemDirectory(tSystemPath, MAX_PATH);
		if (*tSystemPath)
		{
			tsSystemPath = tstring(tSystemPath) + _T("\\");
		}
		return tsSystemPath;
	}

	__inline static //获取系统路径
		tstring GetSystemPathX64()
	{
		tstring tsSystemPath = _T("");
		_TCHAR tSystemPath[MAX_PATH] = { 0 };
		GetSystemWow64Directory(tSystemPath, MAX_PATH);
		if (*tSystemPath)
		{
			tsSystemPath = tstring(tSystemPath) + _T("\\");
		}
		return tsSystemPath;
	}

	__inline static
		BOOL IsFileExist(LPCTSTR fileName)
	{
		WIN32_FIND_DATA	findData;

		HANDLE hFile = FindFirstFile(fileName, &findData);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			FindClose(hFile);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	__inline static
		BOOL IsFileExistEx(LPCTSTR lpFileName)
	{
		BOOL bResult = FALSE;
		GET_FILEEX_INFO_LEVELS gfil = GetFileExInfoStandard;
		WIN32_FILE_ATTRIBUTE_DATA wfad = { 0 };
		if (GetFileAttributes(lpFileName) != INVALID_FILE_ATTRIBUTES)
		{
			bResult = TRUE;
		}
		else
		{
			if (GetFileAttributesEx(lpFileName, gfil, &wfad) &&
				wfad.dwFileAttributes != INVALID_FILE_ATTRIBUTES)
			{
				bResult = TRUE;
			}
		}
		return bResult;
	}

	//判断目录是否存在
	__inline static BOOL IsDirectoryExists(LPCTSTR lpDirectory)
	{
		BOOL bResult = TRUE;
		struct _stat st = { 0 };
		if ((_tstat(lpDirectory, &st) != 0) || (st.st_mode & S_IFDIR != S_IFDIR))
		{
			bResult = FALSE;
		}

		return bResult;
	}
	//判断目录是否存在，若不存在则创建
	__inline static BOOL CreateCascadeDirectory(LPCTSTR lpPathName,        //Directory name
		LPSECURITY_ATTRIBUTES lpSecurityAttributes/* = NULL*/  // Security attribute
		)
	{
		if (IsDirectoryExists(lpPathName))       //如果目录已存在，直接返回
		{
			return TRUE;
		}

		_TCHAR tPathSect[MAX_PATH] = { 0 };
		_TCHAR tPathName[MAX_PATH] = { 0 };
		_tcscpy(tPathName, lpPathName);
		_TCHAR *pToken = _tcstok(tPathName, _T("\\"));
		while (pToken)
		{
			_sntprintf(tPathSect, sizeof(tPathSect) / sizeof(_TCHAR), _T("%s%s\\"), tPathSect, pToken);
			if (!IsDirectoryExists(tPathSect))
			{
				//创建失败时还应删除已创建的上层目录，此次略
				if (!CreateDirectory(tPathSect, lpSecurityAttributes))
				{
					_tprintf(_T("CreateDirectory Failed: %d\n"), GetLastError());
					return FALSE;
				}
			}
			pToken = _tcstok(NULL, _T("\\"));
		}
		return TRUE;
	}

	__inline static LPVOID MapViewOfFileAgain(HANDLE hFileMapping, LPVOID * lppBaseAddress, ULARGE_INTEGER * pui, SIZE_T stNumberOfBytesToMap = 0, LPVOID lpBaseAddress = 0)
	{
		if (lppBaseAddress && (*lppBaseAddress))
		{
			::UnmapViewOfFile((*lppBaseAddress));
			(*lppBaseAddress) = NULL;
		}
		return ((*lppBaseAddress) = ::MapViewOfFileEx(hFileMapping, FILE_MAP_ALL_ACCESS, pui->HighPart, pui->LowPart, stNumberOfBytesToMap, lpBaseAddress));
	}

	__inline static void MapRelease(HANDLE * phFileMapping, LPVOID * lpBaseAddress)
	{
		if (lpBaseAddress && (*lpBaseAddress))
		{
			// 从进程的地址空间撤消文件数据映像
			::UnmapViewOfFile((*lpBaseAddress));
			(*lpBaseAddress) = NULL;
		}

		if (phFileMapping && (*phFileMapping))
		{
			// 关闭文件映射对象
			::CloseHandle((*phFileMapping));
			(*phFileMapping) = NULL;
		}
	}

	__inline static HANDLE MapFileCreate(LPVOID * lpFileData, LPCTSTR lpFileName)
	{
		DWORD dwResult = 0;
		PBYTE pbFile = NULL;
		BOOL bLoopFlag = FALSE;
		HANDLE hWaitEvent[] = { 0, 0 };
		DWORD dwWaitEventNum = sizeof(hWaitEvent) / sizeof(HANDLE);

		SYSTEM_INFO si = { 0 };
		HANDLE hFileMapping = NULL;
		LPVOID lpBaseAddress = NULL;
		ULONGLONG ullFileVolume = 0LL;
		SIZE_T stNumberOfBytesToMap = 0;
		ULARGE_INTEGER uiFileSize = { 0, 0 };

		// 创建文件内核对象，其句柄保存于hFile
		HANDLE hFile = ::CreateFile(lpFileName,
			GENERIC_WRITE | GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL);

		uiFileSize.LowPart = ::GetFileSize(hFile, &uiFileSize.HighPart);
		// 创建文件映射内核对象，句柄保存于hFileMapping
		hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE,
			uiFileSize.HighPart, uiFileSize.LowPart, NULL);
		if (hFile)
		{
			// 释放文件内核对象
			CloseHandle(hFile);
			hFile = NULL;
		}

		if (hFileMapping)
		{
			// 设定大小、偏移量等参数	
			//SystemKernel::GetNativeSystemInformation(&si);
			//ullFileVolume = si.dwAllocationGranularity;

			// 将文件数据映射到进程的地址空间
			(*lpFileData) = MapViewOfFileEx(hFileMapping, FILE_MAP_ALL_ACCESS,
				uiFileSize.HighPart, uiFileSize.LowPart, stNumberOfBytesToMap, lpBaseAddress);
		}

		return hFileMapping;
	}

	__inline static HANDLE MapCreate(LPVOID * lpData, LPCTSTR lpMapName, ULARGE_INTEGER * puiFileSize)
	{
		SYSTEM_INFO si = { 0 };
		HANDLE hFileMapping = NULL;
		LPVOID lpBaseAddress = NULL;
		ULONGLONG ullFileVolume = 0LL;
		SIZE_T stNumberOfBytesToMap = 0;

		// 创建文件映射内核对象，句柄保存于hFileMapping
		hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
			puiFileSize->HighPart, puiFileSize->LowPart, lpMapName);
		if (hFileMapping)
		{
			// 设定大小、偏移量等参数	
			//PPSHUAI::SystemKernel::GetNativeSystemInformation(&si);
			//ullFileVolume = si.dwAllocationGranularity;

			// 将文件数据映射到进程的地址空间
			(*lpData) = MapViewOfFileEx(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, puiFileSize->QuadPart, lpBaseAddress);
		}

		return hFileMapping;
	}

#define CMD_PATH_NAME				"CMD.EXE" //相对路径名称

	//获取cmd.exe文件路径
	__inline static tstring GetCmdPath()
	{
		return GetSystemPath() + _T(CMD_PATH_NAME);
	}

	//设定cmd.exe路径
	static const tstring CMD_FULL_PATH_NAME = GetCmdPath();
}

namespace String{

	__inline static size_t file_reader(std::string&data, std::string filename, std::string mode = "rb")
	{
#define DATA_BASE_SIZE	0xFFFF

		FILE * pF = 0;
		size_t size = 0;

		pF = fopen(filename.c_str(), mode.c_str());
		if (pF)
		{
			while (!feof(pF))
			{
				data.resize(data.size() + DATA_BASE_SIZE);
				size += fread((void *)(data.c_str() + data.size() - DATA_BASE_SIZE), sizeof(char), DATA_BASE_SIZE, pF);
			}
			fclose(pF);
			pF = 0;
		}

		return size;

#undef DATA_BASE_SIZE
	}

	__inline static size_t file_writer(std::string data, std::string filename, std::string mode = "wb")
	{
		FILE * pF = 0;
		size_t size = 0;

		pF = fopen(filename.c_str(), mode.c_str());
		if (pF)
		{
			size = fwrite((void *)(data.c_str()), sizeof(char), data.size(), pF);
			fclose(pF);
			pF = 0;
		}

		return size;
	}

	__inline static bool string_regex_valid(std::string data, std::string pattern)
	{
		return std::regex_match(data, std::regex(pattern));
	}

	__inline static int string_regex_replace_all(std::string & result, std::string & data, std::string replace, std::string pattern, std::regex_constants::match_flag_type matchflagtype = std::regex_constants::match_default)
	{
		int nresult = (-1);
		try
		{
			data = std::regex_replace(data, std::regex(pattern), replace, matchflagtype);
			nresult = data.length();
		}
		catch (const std::exception & e)
		{
			result = e.what();
		}
		return nresult;
	}

	__inline static bool string_regex_find(std::string & result, STRINGVECTORVECTOR & svv, std::string & data, std::string pattern)
	{
		std::smatch smatch;
		bool bresult = false;

		result = ("");

		try
		{
			std::string::const_iterator itb = data.begin();
			std::string::const_iterator ite = data.end();
			while (std::regex_search(itb, ite, smatch, std::regex(pattern)))//如果匹配成功  
			{
				if (smatch.size() > 1)
				{
					for (size_t stidx = svv.size() + 1; stidx < smatch.size(); stidx++)
					{
						svv.push_back(STRINGVECTOR());
					}
					for (size_t stidx = 1; stidx < smatch.size(); stidx++)
					{
						svv.at(stidx - 1).push_back(std::string(smatch[stidx].first, smatch[stidx].second));
						itb = smatch[stidx].second;
					}
					bresult = true;
				}
			}
		}
		catch (const std::exception & e)
		{
			result = e.what();
		}

		return bresult;
	}

	//获取指定两个字符串之间的字符串数据
	__inline static std::string string_reader(std::string strData,
		std::string strStart, std::string strFinal,
		bool bTakeStart = false, bool bTakeFinal = false)
	{
		std::string strRet = ("");
		std::string::size_type stStartPos = std::string::npos;
		std::string::size_type stFinalPos = std::string::npos;
		stStartPos = strData.find(strStart);
		if (stStartPos != std::string::npos)
		{
			stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
			if (stFinalPos != std::string::npos)
			{
				if (!bTakeStart)
				{
					stStartPos += strStart.length();
				}
				if (bTakeFinal)
				{
					stFinalPos += strFinal.length();
				}
				strRet = strData.substr(stStartPos, stFinalPos - stStartPos);
			}
		}

		return strRet;
	}
	//获取指定两个字符串之间的字符串数据
	__inline static std::string::size_type string_reader(std::string &strRet, std::string strData,
		std::string strStart, std::string strFinal, std::string::size_type stPos = 0,
		bool bTakeStart = false, bool bTakeFinal = false)
	{
		std::string::size_type stRetPos = std::string::npos;
		std::string::size_type stStartPos = stPos;
		std::string::size_type stFinalPos = std::string::npos;

		strRet = ("");

		stStartPos = strData.find(strStart, stStartPos);
		if (stStartPos != std::string::npos)
		{
			stRetPos = stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
			if (stFinalPos != std::string::npos)
			{
				if (!bTakeStart)
				{
					stStartPos += strStart.length();
				}
				if (bTakeFinal)
				{
					stFinalPos += strFinal.length();
				}
				strRet = strData.substr(stStartPos, stFinalPos - stStartPos);
			}
		}

		return stRetPos;
	}

	__inline static std::string string_replace_all(std::string &strData, std::string strDst, std::string strSrc, std::string::size_type stPos = 0)
	{
		while ((stPos = strData.find(strSrc, stPos)) != std::string::npos)
		{
			strData.replace(stPos, strSrc.length(), strDst);
		}

		return strData;
	}

	__inline static void string_split_to_vector(STRINGVECTOR & sv, std::string strData, std::string strSplitter, std::string::size_type stPos = 0)
	{
		std::string strTmp = ("");
		std::string::size_type stIdx = 0;
		std::string::size_type stSize = strData.length();

		while ((stPos = strData.find(strSplitter, stIdx)) != std::string::npos)
		{
			strTmp = strData.substr(stIdx, stPos - stIdx);
			if (!strTmp.length())
			{
				strTmp = ("");
			}
			sv.push_back(strTmp);

			stIdx = stPos + strSplitter.length();
		}

		if (stIdx < stSize)
		{
			strTmp = strData.substr(stIdx, stSize - stIdx);
			if (!strTmp.length())
			{
				strTmp = ("");
			}
			sv.push_back(strTmp);
		}
	}

	//获取指定两个字符串之间的字符串数据
	__inline static std::wstring wstring_reader(std::wstring wstrData,
		std::wstring wstrStart, std::wstring wstrFinal,
		bool bTakeStart = false, bool bTakeFinal = false)
	{
		std::wstring wstrRet = (L"");
		std::wstring::size_type stStartPos = std::wstring::npos;
		std::wstring::size_type stFinalPos = std::wstring::npos;
		stStartPos = wstrData.find(wstrStart);
		if (stStartPos != std::wstring::npos)
		{
			stFinalPos = wstrData.find(wstrFinal, stStartPos + wstrStart.length());
			if (stFinalPos != std::wstring::npos)
			{
				if (!bTakeStart)
				{
					stStartPos += wstrStart.length();
				}
				if (bTakeFinal)
				{
					stFinalPos += wstrFinal.length();
				}
				wstrRet = wstrData.substr(stStartPos, stFinalPos - stStartPos);
			}
		}

		return wstrRet;
	}

	//获取指定两个字符串之间的字符串数据
	__inline static std::wstring::size_type wstring_reader(std::wstring &wstrRet, std::wstring wstrData,
		std::wstring wstrStart, std::wstring wstrFinal, std::wstring::size_type stPos = std::wstring::npos,
		bool bTakeStart = false, bool bTakeFinal = false)
	{
		std::wstring::size_type stRetPos = std::wstring::npos;
		std::wstring::size_type stStartPos = stPos;
		std::wstring::size_type stFinalPos = std::wstring::npos;

		wstrRet = (L"");

		stStartPos = wstrData.find(wstrStart);
		if (stStartPos != std::wstring::npos)
		{
			stRetPos = stFinalPos = wstrData.find(wstrFinal, stStartPos + wstrStart.length());
			if (stFinalPos != std::wstring::npos)
			{
				if (!bTakeStart)
				{
					stStartPos += wstrStart.length();
				}
				if (bTakeFinal)
				{
					stFinalPos += wstrFinal.length();
				}
				wstrRet = wstrData.substr(stStartPos, stFinalPos - stStartPos);
			}
		}

		return stRetPos;
	}
	__inline static std::wstring wstring_replace_all(std::wstring &wstrData, std::wstring wstrDst, std::wstring wstrSrc, std::wstring::size_type stPos = 0)
	{
		while ((stPos = wstrData.find(wstrSrc, stPos)) != std::wstring::npos)
		{
			wstrData.replace(stPos, wstrSrc.length(), wstrDst);
		}

		return wstrData;
	}
	__inline static void wstring_split_to_vector(WSTRINGVECTOR & wsv, std::wstring wstrData, std::wstring wstrSplitter, std::wstring::size_type stPos = 0)
	{
		std::wstring wstrTemp = (L"");
		std::wstring::size_type stIdx = 0;
		std::wstring::size_type stSize = wstrData.length();

		while ((stPos = wstrData.find(wstrSplitter, stIdx)) != std::wstring::npos)
		{
			wstrTemp = wstrData.substr(stIdx, stPos - stIdx);
			if (!wstrTemp.length())
			{
				wstrTemp = (L"");
			}
			wsv.push_back(wstrTemp);

			stIdx = stPos + wstrSplitter.length();
		}

		if (stIdx < stSize)
		{
			wstrTemp = wstrData.substr(stIdx, stSize - stIdx);
			if (!wstrTemp.length())
			{
				wstrTemp = (L"");
			}
			wsv.push_back(wstrTemp);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：选择读取数据行从文件中
	// 参    数：输出的文件行内容数据、过滤后缀名、过滤的前缀字符
	// 返 回 值：bool返回类型，成功返回true；失败返回false
	// 编 写 者: ppshuai 20141112
	//////////////////////////////////////////////////////////////////////////
	__inline bool ReadLinesFromFile(TSTRINGVECTOR * pStringVector, LPTSTR lpFilter = TEXT(""), TCHAR tNote = TEXT(';'), HWND hWnd = NULL)
	{
		bool result = false;
		FILE * pFile = NULL;
		TCHAR tFileName[MAX_PATH + 1] = { 0 };
		TCHAR tLineData[MAX_PATH * 4 + 1] = { 0 };
		pFile = _tfopen(tFileName, TEXT("rb"));
		if (pFile)
		{
			while (!feof(pFile))
			{
				_fgetts(tLineData, sizeof(tLineData), pFile);
				if (*tLineData != tNote && lstrlen(tLineData))
				{
					pStringVector->push_back(tLineData);
				}
			}
			fclose(pFile);
			pFile = NULL;
			result = true;
		}
		
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：遍历目录获取指定文件列表
	// 参    数：输出的文件行内容数据、过滤后缀名、过滤的前缀字符
	// 返 回 值：bool返回类型，成功返回true；失败返回false
	// 编 写 者: ppshuai 20141112
	//////////////////////////////////////////////////////////////////////////
	__inline static bool DirectoryTraversal(TSTRINGVECTOR * pStringVector, LPTSTR lpDirectory, LPTSTR lpFormat)
	{
		bool result = false;
		HANDLE hFileHandle = NULL;
		WIN32_FIND_DATA wfd = { 0 };
		_TCHAR tPathFile[MAX_PATH + 1] = { 0 };

		if (pStringVector)
		{
			pStringVector->clear();

			wsprintf(tPathFile, TEXT("%s\\*.%s"), lpDirectory, lpFormat);

			hFileHandle = FindFirstFile(tPathFile, &wfd);
			if (hFileHandle != INVALID_HANDLE_VALUE)
			{
				pStringVector->push_back(wfd.cFileName);
				while (FindNextFile(hFileHandle, &wfd))
				{
					pStringVector->push_back(wfd.cFileName);
				}
				result = true;
			}
		}
		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：执行程序命令并获取执行打印信息
	// 参    数：输出的文件行内容数据、要执行的命令
	// 返 回 值：bool返回类型，成功返回true；失败返回false
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static bool ExecuteCommand(TSTRINGVECTOR * pStringVector, tstring tCommandLine)
	{
		bool result = false;
		FILE * ppipe = NULL;
		tstring tItemText = TEXT("");
		_TCHAR tItemChar[2] = { TEXT('\0') };

		if (pStringVector)
		{
			// Open pipe to execute command line
			ppipe = _tpopen(tCommandLine.c_str(), TEXT("rb"));
			if (ppipe)
			{
				/* Read pipe until end of file, or an error occurs. */
				while (fread(&tItemChar, sizeof(_TCHAR), 1, ppipe))
				{
					if ((*tItemChar != TEXT('\n'))
						&& (*tItemChar != TEXT('\0')))
					{
						*(tItemChar + 1) = TEXT('\0');
						tItemText.append(tItemChar);
					}
					else
					{
						pStringVector->push_back(tItemText);
						tItemText.empty();
						tItemText = TEXT("");
					}
				}

				pStringVector->push_back(tItemText);

				/* Close pipe and print return value of pPipe. */
				if (feof(ppipe))
				{
					result = true;
					_pclose(ppipe);
					ppipe = NULL;
				}
			}
		}

		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：执行程序命令并获取执行打印信息
	// 参    数：输出的文件行内容数据、要执行的命令
	// 返 回 值：bool返回类型，成功返回true；失败返回false
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static bool ExecuteCommandEx(TSTRINGVECTOR * pStdOutputStringVector,
		TSTRINGVECTOR * pStdErrorStringVector,
		tstring tExecuteFile,
		tstring tCommandLine)
	{
		bool result = false;
		STARTUPINFO si = { 0 };
		HANDLE hStdErrorRead = NULL;
		HANDLE hStdOutputRead = NULL;
		HANDLE hStdErrorWrite = NULL;
		HANDLE hStdOutputWrite = NULL;
		SECURITY_ATTRIBUTES sa = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		tstring tItemText = TEXT("");
		_TCHAR tItemChar[2] = { TEXT('\0') };
		unsigned long ulBytesOfRead = 0;
		unsigned long ulBytesOfWritten = 0;

		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;



		// Set the bInheritHandle flag so pipe handles are inherited.
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;

		// Create a pipe for the child process's STDOUT.
		if (CreatePipe(&hStdOutputRead, &hStdOutputWrite, &sa, 0) &&
			CreatePipe(&hStdErrorRead, &hStdErrorWrite, &sa, 0))
		{
			// Ensure the read handle to the pipe for STDOUT is not inherited.
			if (SetHandleInformation(hStdOutputRead, HANDLE_FLAG_INHERIT, 0) &&
				SetHandleInformation(hStdErrorRead, HANDLE_FLAG_INHERIT, 0))
			{
				si.hStdOutput = hStdOutputWrite;
				si.hStdError = hStdErrorWrite;
				si.wShowWindow = SW_HIDE;
				si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

				// Start the child process.
				if (CreateProcess(tExecuteFile.size() ? tExecuteFile.c_str() : NULL,   // No module name (use command line)
					(TCHAR *)tCommandLine.c_str(),        // Command line
					NULL,           // Process handle not inheritable
					NULL,           // Thread handle not inheritable
					TRUE,          // Set handle inheritance to FALSE
					0,              // No creation flags
					NULL,           // Use parent's environment block
					NULL,           // Use parent's starting directory
					&si,            // Pointer to STARTUPINFO structure
					&pi))           // Pointer to PROCESS_INFORMATION structure
				{
					// Wait until child process exits.
					//WaitForSingleObject( pi.hProcess, INFINITE );

					// Close process and thread handles.
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);

					if (hStdOutputWrite)
					{
						CloseHandle(hStdOutputWrite);
						hStdOutputWrite = NULL;
					}
					if (hStdErrorWrite)
					{
						CloseHandle(hStdErrorWrite);
						hStdErrorWrite = NULL;
					}

					if (pStdOutputStringVector)
					{
						while (ReadFile(hStdOutputRead, tItemChar, sizeof(_TCHAR),
							&ulBytesOfRead, NULL) && ulBytesOfRead)
						{
							if ((*tItemChar != TEXT('\n')) &&
								(*tItemChar != TEXT('\0')))
							{
								*(tItemChar + 1) = TEXT('\0');
								tItemText.append(tItemChar);
							}
							else
							{
								pStdOutputStringVector->push_back(tItemText);
								tItemText.empty();
								tItemText = TEXT("");
							}
						}
						pStdOutputStringVector->push_back(tItemText);
					}

					if (pStdErrorStringVector)
					{
						while (ReadFile(hStdErrorRead, tItemChar, sizeof(_TCHAR),
							&ulBytesOfRead, NULL) && ulBytesOfRead)
						{
							if ((*tItemChar != TEXT('\n')) &&
								(*tItemChar != TEXT('\0')))
							{
								*(tItemChar + 1) = TEXT('\0');
								tItemText.append(tItemChar);
							}
							else
							{
								pStdErrorStringVector->push_back(tItemText);
								tItemText.empty();
								tItemText = TEXT("");
							}
						}
						pStdErrorStringVector->push_back(tItemText);
					}
					result = true;
				}
			}
		}

		if (hStdErrorWrite)
		{
			CloseHandle(hStdErrorWrite);
			hStdErrorWrite = NULL;
		}
		if (hStdOutputWrite)
		{
			CloseHandle(hStdOutputWrite);
			hStdOutputWrite = NULL;
		}
		if (hStdErrorRead)
		{
			CloseHandle(hStdErrorRead);
			hStdErrorRead = NULL;
		}
		if (hStdOutputRead)
		{
			CloseHandle(hStdOutputRead);
			hStdOutputRead = NULL;
		}

		return result;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：执行程序命令并获取执行打印信息
	// 参    数：输出的文件行内容数据、要执行的命令
	// 返 回 值：BOOL返回类型，成功返回TRUE；失败返回FALSE
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static BOOL ExecuteProcess(TCHAR * pProcName, TCHAR * pCmdLine,
		BOOL bShowFlag = FALSE, DWORD dwMilliseconds = INFINITE)
	{
		BOOL bResult = FALSE;
		DWORD dwShowFlag = 0;
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		DWORD dwCmdLineSizeLength = 0;
		TCHAR szCmdLine[MAX_PATH * 4 + 1] = { 0 };

		si.cb = sizeof(si);

		dwShowFlag = bShowFlag ? NULL : CREATE_NO_WINDOW;
		if (pCmdLine != NULL)
		{
			dwCmdLineSizeLength = (lstrlen(pCmdLine) < sizeof(szCmdLine)) ?
				lstrlen(pCmdLine) : sizeof(szCmdLine);
			lstrcpyn(szCmdLine, pCmdLine, dwCmdLineSizeLength);
			bResult = ::CreateProcess(pProcName, szCmdLine, NULL, NULL, FALSE,
				NORMAL_PRIORITY_CLASS | dwShowFlag, NULL, NULL, &si, &pi);
		}
		else
		{
			bResult = ::CreateProcess(pProcName, szCmdLine, NULL, NULL, FALSE,
				NORMAL_PRIORITY_CLASS | dwShowFlag, NULL, NULL, &si, &pi);
		}

		if (bResult)
		{
			WaitForSingleObject(pi.hProcess, dwMilliseconds);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}

		return bResult;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：执行程序命令并获取执行打印信息
	// 参    数：要删除的文件名
	// 返 回 值：bool返回类型，成功返回true；失败返回false
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static BOOL ForceDeleteFile(TCHAR * pszFileName)
	{
		BOOL bResult = FALSE;
		TCHAR tSystemPath[MAX_PATH + 1] = { 0 };
		TCHAR tCmdLine[MAX_PATH * 4 + 1] = { 0 };

		if (pszFileName && (*pszFileName) &&
			GetSystemDirectory(tSystemPath, sizeof(tSystemPath)))
		{
			wsprintf(tCmdLine, TEXT("%s\\CMD.EXE /c DEL /F /S /Q \"%s\""), tSystemPath, pszFileName);
			bResult = ExecuteProcess(NULL, tCmdLine);
		}

		return bResult;
	}
}

namespace SystemKernel{
	__inline static BOOL CreateAdvanceProcess(
		LPCTSTR lpApplicationName,
		LPTSTR lpCommandLine,
		DWORD dwCreateFlags,
		LPCTSTR lpCurrentDirectory = NULL,
		LPSTARTUPINFO lpStartupInfo = NULL,
		LPPROCESS_INFORMATION lpProcessInformation = NULL,
		LPVOID lpEnvironment = NULL,
		LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL,
		LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL,
		BOOL bInheritHandles = FALSE)
	{
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		LPSTARTUPINFO lpsi = &si;
		LPPROCESS_INFORMATION lppi = &pi;

		if (lpStartupInfo)
		{
			lpsi = lpStartupInfo;
		}
		if (lpProcessInformation)
		{
			lppi = lpProcessInformation;
		}

		return CreateProcess(
			lpApplicationName,
			lpCommandLine,
			lpProcessAttributes,
			lpThreadAttributes,
			bInheritHandles,
			dwCreateFlags,
			lpEnvironment,
			lpCurrentDirectory,
			lpsi,
			lppi
			);
	}
	//获取Windows系统信息
	__inline static VOID GetNativeSystemInformation(SYSTEM_INFO * psi)
	{
		typedef void (WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO);

		// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
		LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress(GetModuleHandle(_T("KERNEL32.DLL")), "GetNativeSystemInfo");
		if (fnGetNativeSystemInfo)
		{
			fnGetNativeSystemInfo(psi);
		}
		else
		{
			GetSystemInfo(psi);
		}
	}

	//获取操作系统版本信息
	__inline static void GetWindowsVersion(DWORD * dwMajor, DWORD * dwMinor, DWORD * dwBuildNumber)
	{
		typedef void (WINAPI * LPFN_RtlGetNtVersionNumbers)(DWORD * dwMajor, DWORD * dwMinor, DWORD * dwBuildNumber);

		LPFN_RtlGetNtVersionNumbers pfnRtlGetNtVersionNumbers = NULL;
		pfnRtlGetNtVersionNumbers = (LPFN_RtlGetNtVersionNumbers)GetProcAddress(GetModuleHandle(_T("NTDLL.DLL")), "RtlGetNtVersionNumbers");
		if (pfnRtlGetNtVersionNumbers)
		{
			pfnRtlGetNtVersionNumbers(dwMajor, dwMinor, dwBuildNumber);
			(*dwBuildNumber) &= 0xFFFF;
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	//判断进程是64位还是32位(bIsWow64为TRUE为64位，返回FALSE为32位)
	__inline static BOOL IsWow64Process(BOOL & bIsWow64, HANDLE hProcess)
	{
		typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
		BOOL bResult = FALSE;
		LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;

		if (hProcess)
		{
			fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("KERNEL32.DLL")), "IsWow64Process");
			if (fnIsWow64Process)
			{
				bResult = fnIsWow64Process(hProcess, &bIsWow64);
			}
		}

		return bResult;
	}
	__inline static BOOL IsWow64Process(BOOL & bIsWow64, DWORD dwProcessId)
	{
		BOOL bResult = FALSE;
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
		if (hProcess)
		{
			bResult = IsWow64Process(bIsWow64, hProcess);
			CloseHandle(hProcess);
			hProcess = NULL;
		}

		return bResult;
	}

	__inline static BOOL IsWow64System()
	{
		typedef VOID(__stdcall*LPFN_GETNATIVESYSTEMINFO)(LPSYSTEM_INFO lpSystemInfo);

		BOOL bResult = FALSE;
		SYSTEM_INFO si = { 0 };
		LPFN_GETNATIVESYSTEMINFO fnGetNativeSystemInfo = NULL;

		fnGetNativeSystemInfo = (LPFN_GETNATIVESYSTEMINFO)GetProcAddress(GetModuleHandle(_T("KERNEL32.DLL")), "GetNativeSystemInfo");
		if (fnGetNativeSystemInfo)
		{
			fnGetNativeSystemInfo(&si);

			if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ||
				si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			{
				bResult = TRUE;
			}
		}

		return bResult;
	}

	//获取进程用户函数：
	__inline static BOOL GetProcessUserName(TSTRING & strUser, DWORD dwID) // 进程ID 
	{
		HANDLE hToken = NULL;
		BOOL bResult = FALSE;
		DWORD dwSize = 0;

		TCHAR szUserName[256] = { 0 };
		TCHAR szDomain[256] = { 0 };
		DWORD dwDomainSize = 256;
		DWORD dwNameSize = 256;

		SID_NAME_USE    SNU;
		PTOKEN_USER pTokenUser = NULL;

		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwID);

		strUser = _T("SYSTEM");
		if (hProcess == NULL)
		{
			switch (GetLastError())
			{
			case ERROR_ACCESS_DENIED:
				strUser = _T("SYSTEM");
				break;
			default:
				strUser = _T("SYSTEM");
				break;
			}
			return bResult;
		}
		__try
		{
			if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
			{
				bResult = FALSE;
				__leave;
			}

			if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize))
			{
				if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				{
					bResult = FALSE;
					__leave;
				}
			}

			pTokenUser = NULL;
			pTokenUser = (PTOKEN_USER)malloc(dwSize);
			if (pTokenUser == NULL)
			{
				bResult = FALSE;
				__leave;
			}

			if (!GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize))
			{
				bResult = FALSE;
				__leave;
			}

			if (LookupAccountSid(NULL, pTokenUser->User.Sid, szUserName, &dwNameSize, szDomain, &dwDomainSize, &SNU) != 0)
			{
				strUser = szUserName;
				return TRUE;
			}
		}
		__finally
		{
			if (pTokenUser != NULL)
				free(pTokenUser);
		}

		return FALSE;
	}

	///////////////////////////////////////////////////////////////////////
	//获取进程映像名称
	//Windows 2000		= GetModuleFileName()
	//Windows XP x32	= GetProcessImageFileName()
	//Windows XP x64	= GetProcessImageFileName()
	//Windows Vista		= QueryFullProcessImageName()
	//Windows 7			= QueryFullProcessImageName()
	//Windows 8			= QueryFullProcessImageName()
	//Windows 8.1		= QueryFullProcessImageName()
	//Windows 10		= QueryFullProcessImageName()
	///////////////////////////////////////////////////////////////////////

	__inline static BOOL EnumModules_R3(std::map<DWORD, MODULEENTRY32> * pmemap, DWORD dwPID)
	{
		BOOL bRet = FALSE;
		MODULEENTRY32 me = { 0 };
		HANDLE hSnapModule = INVALID_HANDLE_VALUE;

		// Take a snapshot of all modules in the specified process.
		hSnapModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
		if (hSnapModule == INVALID_HANDLE_VALUE)
		{
			goto __LEAVE_CLEAN__;
		}

		// Set the size of the structure before using it.
		me.dwSize = sizeof(MODULEENTRY32);

		// Retrieve information about the first module,
		// and exit if unsuccessful
		if (!Module32First(hSnapModule, &me))
		{
			goto __LEAVE_CLEAN__;
		}

		// Now walk the module list of the process,
		// and display information about each module
		do
		{
			pmemap->insert(std::map<DWORD, MODULEENTRY32>::value_type(me.th32ModuleID, me));
		} while (Module32Next(hSnapModule, &me));

	__LEAVE_CLEAN__:

		//关闭句柄
		CloseHandle(hSnapModule);
		hSnapModule = NULL;

		return bRet;
	}

	__inline static BOOL EnumModules32_R3(std::map<DWORD, MODULEENTRY32> * pmemap, DWORD dwPID)
	{
		BOOL bRet = FALSE;
		MODULEENTRY32 me = { 0 };
		HANDLE hSnapModule = INVALID_HANDLE_VALUE;

		// Take a snapshot of all modules in the specified process.
		hSnapModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE32, dwPID);
		if (hSnapModule == INVALID_HANDLE_VALUE)
		{
			goto __LEAVE_CLEAN__;
		}

		// Set the size of the structure before using it.
		me.dwSize = sizeof(MODULEENTRY32);

		// Retrieve information about the first module,
		// and exit if unsuccessful
		if (!Module32First(hSnapModule, &me))
		{
			goto __LEAVE_CLEAN__;
		}

		// Now walk the module list of the process,
		// and display information about each module
		do
		{
			pmemap->insert(std::map<DWORD, MODULEENTRY32>::value_type(me.th32ModuleID, me));
		} while (Module32Next(hSnapModule, &me));

	__LEAVE_CLEAN__:

		//关闭句柄
		CloseHandle(hSnapModule);
		hSnapModule = NULL;

		return bRet;
	}

	__inline static BOOL EnumProcess_R3(std::map<DWORD, PROCESSENTRY32> * ppemap)
	{
		BOOL bRet = FALSE;
		PROCESSENTRY32 pe = { 0 };
		HANDLE hSnapProcess = INVALID_HANDLE_VALUE;

		//创建快照
		hSnapProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapProcess == INVALID_HANDLE_VALUE)
		{
			goto __LEAVE_CLEAN__;
		}


		pe.dwSize = sizeof(PROCESSENTRY32);

		//遍历进程	
		bRet = Process32First(hSnapProcess, &pe);
		if (!bRet)
		{
			goto __LEAVE_CLEAN__;
		}

		do
		{
			ppemap->insert(std::map<DWORD, PROCESSENTRY32>::value_type(pe.th32ProcessID, pe));
		} while (Process32Next(hSnapProcess, &pe));

	__LEAVE_CLEAN__:

		//关闭句柄
		CloseHandle(hSnapProcess);
		hSnapProcess = NULL;

		return bRet;
	}

	__inline static BOOL EnumThread_R3(std::map<DWORD, THREADENTRY32> * ptemap, DWORD dwPID)
	{
		BOOL bRet = FALSE;
		THREADENTRY32 te = { 0 };
		HANDLE hSnapThread = INVALID_HANDLE_VALUE;

		//创建快照
		hSnapThread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPID);
		if (hSnapThread == INVALID_HANDLE_VALUE)
		{
			goto __LEAVE_CLEAN__;
		}

		te.dwSize = sizeof(THREADENTRY32);

		//遍历进程	
		bRet = Thread32First(hSnapThread, &te);
		if (!bRet)
		{
			goto __LEAVE_CLEAN__;
		}

		do
		{
			ptemap->insert(std::map<DWORD, THREADENTRY32>::value_type(te.th32ThreadID, te));
		} while (Thread32Next(hSnapThread, &te));

	__LEAVE_CLEAN__:

		//关闭句柄
		CloseHandle(hSnapThread);
		hSnapThread = NULL;

		return bRet;
	}

	__inline static BOOL EnumHeapList_R3(std::map<ULONG_PTR, HEAPLIST32> * phlmap, DWORD dwPID)
	{
		BOOL bRet = FALSE;
		HEAPLIST32 hl = { 0 };
		HANDLE hSnapHeapList = INVALID_HANDLE_VALUE;

		//创建快照
		hSnapHeapList = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, dwPID);
		if (hSnapHeapList == INVALID_HANDLE_VALUE)
		{
			goto __LEAVE_CLEAN__;
		}

		hl.dwSize = sizeof(HEAPLIST32);

		//遍历进程	
		bRet = Heap32ListFirst(hSnapHeapList, &hl);
		if (!bRet)
		{
			goto __LEAVE_CLEAN__;
		}

		do
		{
			phlmap->insert(std::map<ULONG_PTR, HEAPLIST32>::value_type(hl.th32HeapID, hl));
		} while (Heap32ListNext(hSnapHeapList, &hl));

	__LEAVE_CLEAN__:

		//关闭句柄
		CloseHandle(hSnapHeapList);
		hSnapHeapList = NULL;

		return bRet;
	}

	__inline static	DWORD GetProcessIdByProcessName(const _TCHAR * ptProcessName)
	{
		DWORD dwPID = 0;
		_TCHAR tzProcessName[MAX_PATH] = { 0 };
		std::map<DWORD, PROCESSENTRY32> pemap;
		std::map<DWORD, PROCESSENTRY32>::iterator itEnd;
		std::map<DWORD, PROCESSENTRY32>::iterator itIdx;
		
		lstrcpy(tzProcessName, ptProcessName);

		EnumProcess_R3(&pemap);

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
		return dwPID;
	}

	__inline static HANDLE InitProcessHandle(DWORD dwPID)
	{
		// Open process
		return ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, dwPID);
	}
	__inline static HANDLE InitProcessHandle(const _TCHAR * ptProcessName)
	{
		DWORD dwPID = 0;

		dwPID = GetProcessIdByProcessName(ptProcessName);

		// Open process
		return ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, dwPID);
	}
	__inline static void ExitProcessHandle(HANDLE * pHandle)
	{
		if (pHandle && (*pHandle))
		{
			CloseHandle((*pHandle));
			(*pHandle) = NULL;
		}
	}
	//根据进程ID终止进程
	__inline static void TerminateProcessByProcessId(DWORD dwProcessId)
	{
		DWORD dwExitCode = 0;
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
		if (hProcess)
		{
			GetExitCodeProcess(hProcess, &dwExitCode);
			TerminateProcess(hProcess, dwExitCode);
			CloseHandle(hProcess);
			hProcess = 0;
		}
	}

	//传入应用程序文件名称、参数、启动类型及等待时间启动程序
	typedef enum LaunchType {
		LTYPE_0 = 0, //立即
		LTYPE_1 = 1, //直等
		LTYPE_2 = 2, //延迟(设定等待时间)
	}LAUNCHTYPE;

	//传入应用程序文件名称、参数、启动类型及等待时间启动程序
	__inline static BOOL LaunchAppProg(tstring tsAppProgName, tstring tsArguments = _T(""), bool bNoUI = true, LAUNCHTYPE type = LTYPE_0, DWORD dwWaitTime = WAIT_TIMEOUT)
	{
		BOOL bRet = FALSE;
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		DWORD dwCreateFlags = CREATE_NO_WINDOW;
		LPTSTR lpArguments = NULL;

		if (tsArguments.length())
		{
			lpArguments = (LPTSTR)tsArguments.c_str();
		}

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		if (!bNoUI)
		{
			dwCreateFlags = 0;
		}

		// Start the child process.
		bRet = CreateProcess(tsAppProgName.c_str(),   // No module name (use command line)
			lpArguments,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			dwCreateFlags,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory
			&si,            // Pointer to STARTUPINFO structure
			&pi);           // Pointer to PROCESS_INFORMATION structure
		if (bRet)
		{
			switch (type)
			{
			case LTYPE_0:
			{
				// No wait until child process exits.
			}
			break;
			case LTYPE_1:
			{
				// Wait until child process exits.
				WaitForSingleObject(pi.hProcess, INFINITE);
			}
			break;
			case LTYPE_2:
			{
				// Wait until child process exits.
				WaitForSingleObject(pi.hProcess, dwWaitTime);
			}
			break;
			default:
				break;
			}

			// Close process and thread handles.
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			// Exit process.
			TerminateProcessByProcessId(pi.dwProcessId);
		}
		else
		{
			//DEBUG_TRACE(_T("CreateProcess failed (%d).\n"), GetLastError());
		}
		return bRet;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：阻塞式启动程序或脚本
	// 参    数：文件名、参数、运行目录
	// 返 回 值：成功返回true，否则返回false
	// 编 写 者: ppshuai 20141112
	//////////////////////////////////////////////////////////////////////////
	__inline static bool RunAppUnBlock(LPCTSTR lpFileName,
		LPCTSTR lpParameters = NULL,
		int nShowCmd = SW_HIDE,
		LPCTSTR lpDirectory = NULL)
	{
		bool bResult = false;
		SHELLEXECUTEINFO sei = { 0 };
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_ASYNCOK;
		sei.hwnd = NULL;
		sei.lpVerb = TEXT("OPEN");
		sei.lpFile = lpFileName;
		sei.lpParameters = lpParameters;
		sei.lpDirectory = lpDirectory;
		sei.nShow = nShowCmd;
		sei.hInstApp = NULL;
		if (ShellExecuteEx(&sei))
		{
			// 等待脚本返回
			WaitForSingleObject(sei.hProcess, INFINITE);
			bResult = true;
		}

		return bResult;
	}
	//系统提权函数
	__inline static BOOL PromotingPrivilege(LPCTSTR lpszPrivilegeName, BOOL bEnable)
	{
		BOOL bRet = FALSE;
		LUID luid = { 0 };
		HANDLE hToken = NULL;
		TOKEN_PRIVILEGES tp = { 0 };

		if (OpenProcessToken(GetCurrentProcess(),
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken) &&
			LookupPrivilegeValue(NULL, lpszPrivilegeName, &luid))
		{
			tp.PrivilegeCount = 0x01L;
			tp.Privileges[0].Luid = luid;
			tp.Privileges[0].Attributes = (bEnable) ? SE_PRIVILEGE_ENABLED : 0;
			bRet = AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, NULL, NULL);
			CloseHandle(hToken);
			hToken = NULL;
		}

		return bRet;
	}

	//检查系统版本是否是Vista或更高的版本
	__inline static bool IsOsVersionVistaOrGreater()
	{
		OSVERSIONINFOEX ovex = { 0 };
		_TCHAR tzVersionInfo[MAX_PATH] = { 0 };

		//设置参数的大小，调用并判断是否成功
		ovex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		if (!GetVersionEx((OSVERSIONINFO *)(&ovex)))
		{
			return false;
		}
		//通过版本号，判断是否是vista及之后版本
		if (ovex.dwMajorVersion > 5)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	//检查并根据系统版本选择打开程序方式
	__inline static void ShellExecuteExOpen(tstring tsAppName, tstring tsArguments, tstring tsWorkPath)
	{
		if (IsOsVersionVistaOrGreater())
		{
			SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			sei.lpVerb = _T("runas");
			sei.lpFile = tsAppName.c_str();
			sei.lpParameters = tsArguments.c_str();
			sei.lpDirectory = tsWorkPath.c_str();
			sei.nShow = SW_SHOWNORMAL;
			if (!ShellExecuteEx(&sei))
			{
				DWORD dwStatus = GetLastError();
				if (dwStatus == ERROR_CANCELLED)
				{
					//DEBUG_TRACE(_T("提升权限被用户拒绝\n"));
				}
				else if (dwStatus == ERROR_FILE_NOT_FOUND)
				{
					//DEBUG_TRACE(_T("所要执行的文件没有找到\n"));
				}
				else
				{
					//DEBUG_TRACE(_T("失败原因未找到\n"));
				}
			}
		}
		else
		{
			//appPath.Replace(L"\\", L"\\\\");
			ShellExecute(NULL, _T("open"), tsAppName.c_str(), NULL, tsWorkPath.c_str(), SW_SHOWNORMAL);
		}

	}
	__inline static void LaunchAppProgByAdmin(tstring tsAppProgName, tstring tsArguments, bool bNoUI/* = true*/)
	{
		ShellExecuteExOpen(tsAppProgName, tsArguments, _T(""));
		/*const HWND hWnd = 0;
		const _TCHAR * pCmd = _T("runas");
		const _TCHAR * pWorkPath = _T("");
		int nShowType = bNoUI ? SW_HIDE : SW_SHOW;
		::ShellExecute(hWnd, pCmd, tsAppProgName.c_str(), tsArguments.c_str(), pWorkPath, nShowType);*/
	}

	//程序实例只允许一个
	__inline static BOOL RunAppOnce(LPCTSTR ptName)
	{
		HANDLE hMutexInstance = ::CreateMutex(NULL, FALSE, ptName);  //创建互斥
		if (hMutexInstance)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				//OutputDebugString(_T("互斥检测返回！"));
				CloseHandle(hMutexInstance);
				ExitProcess(0L);
				return FALSE;
			}
		}
		return TRUE;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：注册DLL库到Windows系统
	// 参    数：新文件名（可选）、资源ID、资源类型
	// 返 回 值：成功返回TRUE，否则返回FALSE
	// 编 写 者: ppshuai 20141112
	//////////////////////////////////////////////////////////////////////////
	__inline static void RegisterDynamicLibrary(LPCTSTR lpDllName, BOOL bEnable = FALSE)
	{
#define REG_TOOL_NAME TEXT("REGSVR32.EXE")

		TCHAR	l_tSystemPath[MAX_PATH + 1] = { 0 };//系统路径定义
		TCHAR	l_tToolCommand[MAX_PATH * 2 + 1] = { 0 };///工具命令路径定义


		if (GetSystemDirectory(l_tSystemPath, MAX_PATH) > 0)
		{
			if (bEnable)
			{
				wsprintf(l_tToolCommand, TEXT("%s\\%s /s /i \"%s\""),
					l_tSystemPath, REG_TOOL_NAME, lpDllName);
			}
			else
			{
				wsprintf(l_tToolCommand, TEXT("%s\\%s /s /u \"%s\""),
					l_tSystemPath, REG_TOOL_NAME, lpDllName);
			}
			RunAppUnBlock(l_tToolCommand);
		}
	}
}
}

#include "PEFileInfo.h"
#include "WindowHeader.h"
#include "MemoryHeader.h"
#include "ListCtrlData.h"
#include "SystemDataInfo.h"
#include "CryptyHeader.h"