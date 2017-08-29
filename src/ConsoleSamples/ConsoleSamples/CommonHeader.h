
#include <windows.h>
#include <tlhelp32.h>

#include <tchar.h>
#include <sys/stat.h>

#include <map>
#include <regex>
#include <vector>
#include <string>

#include "UNDOCAPI.h"

#if !defined(_UNICODE) && !defined(UNICODE)
#define TSTRING std::string
#else
#define TSTRING std::wstring
#endif

#define _tstring TSTRING
#define tstring TSTRING

typedef std::vector<std::string> STRINGVECTOR;
typedef std::vector<std::wstring> WSTRINGVECTOR;

typedef std::vector<STRINGVECTOR> STRINGVECTORVECTOR;
typedef std::vector<WSTRINGVECTOR> WSTRINGVECTORVECTOR;

__inline static std::string STRING_FORMAT_A(const CHAR * paFormat, ...)
{
	INT nAS = 0;
	std::string A = 0;
	
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
	std::wstring W = 0;

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

#if !defined(_UNICODE) && !defined(UNICODE)
#define STRING_FORMAT STRING_FORMAT_A
#else
#define STRING_FORMAT STRING_FORMAT_W
#endif

//��ʼ�����Դ�����ʾ
__inline static void InitDebugConsole()
{
	FILE *pStdOut = stdout;
	FILE *pStdIn = stdin;
	FILE *pStdErr = stderr;

	if (!AllocConsole())
	{
		_TCHAR tErrorInfos[16384] = { 0 };
		_sntprintf(tErrorInfos, sizeof(tErrorInfos) / sizeof(_TCHAR), _T("����̨����ʧ��! �������:0x%X��"), GetLastError());
		MessageBox(NULL, tErrorInfos, _T("������ʾ"), 0);
		return;
	}
	SetConsoleTitle(_T("TraceDebugWindow"));

	pStdOut = _tfreopen(_T("CONOUT$"), _T("w"), stdout);
	pStdIn = _tfreopen(_T("CONIN$"), _T("r"), stdin);
	pStdErr = _tfreopen(_T("CONERR$"), _T("w"), stderr);
	_tsetlocale(LC_ALL, _T("chs"));
}

//�ͷŵ����Դ�����ʾ
__inline static void ExitDebugConsole()
{
	FreeConsole();
}

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

//utf8 ת Unicode
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

//unicode תΪ ascii
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

//utf-8 ת ascii
__inline static std::string UTF_82ASCII(std::string& strUtf8Code)
{
	std::string strRet("");
	//�Ȱ� utf8 תΪ unicode
	std::wstring wstr = Utf82Unicode(strUtf8Code);
	//���� unicode תΪ ascii
	strRet = WideByte2Acsi(wstr);
	return strRet;
}

///////////////////////////////////////////////////////////////////////


//ascii ת Unicode
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


//Unicode ת Utf8
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

//ascii ת Utf8
__inline static std::string ASCII2UTF_8(std::string& strAsciiCode)
{
	std::string strRet("");
	//�Ȱ� ascii תΪ unicode
	std::wstring wstr = Acsi2WideByte(strAsciiCode);
	//���� unicode תΪ utf8
	strRet = Unicode2Utf8(wstr);
	return strRet;
}


//��ʾ����Ļ����
__inline static void CenterWindowInScreen(HWND hWnd)
{
	RECT rcWindow = { 0 };
	RECT rcScreen = { 0 };
	SIZE szAppWnd = { 300, 160 };
	POINT ptAppWnd = { 0, 0 };

	// Get workarea rect.
	BOOL fResult = SystemParametersInfo(SPI_GETWORKAREA,   // Get workarea information
		0,              // Not used
		&rcScreen,    // Screen rect information
		0);             // Not used

	GetWindowRect(hWnd, &rcWindow);
	szAppWnd.cx = rcWindow.right - rcWindow.left;
	szAppWnd.cy = rcWindow.bottom - rcWindow.top;

	//������ʾ
	ptAppWnd.x = (rcScreen.right - rcScreen.left - szAppWnd.cx) / 2;
	ptAppWnd.y = (rcScreen.bottom - rcScreen.top - szAppWnd.cy) / 2;
	MoveWindow(hWnd, ptAppWnd.x, ptAppWnd.y, szAppWnd.cx, szAppWnd.cy, TRUE);
}

//��ʾ�ڸ���������
__inline static void CenterWindowInParent(HWND hWnd, HWND hParentWnd)
{
	RECT rcWindow = { 0 };
	RECT rcParent = { 0 };
	SIZE szAppWnd = { 300, 160 };
	POINT ptAppWnd = { 0, 0 };

	GetWindowRect(hParentWnd, &rcParent);
	GetWindowRect(hWnd, &rcWindow);
	szAppWnd.cx = rcWindow.right - rcWindow.left;
	szAppWnd.cy = rcWindow.bottom - rcWindow.top;

	//������ʾ
	ptAppWnd.x = (rcParent.right - rcParent.left - szAppWnd.cx) / 2;
	ptAppWnd.y = (rcParent.bottom - rcParent.top - szAppWnd.cy) / 2;
	MoveWindow(hWnd, ptAppWnd.x, ptAppWnd.y, szAppWnd.cx, szAppWnd.cy, TRUE);
}

//���ݽ���ID��ֹ����
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

//����Ӧ�ó����ļ����ơ��������������ͼ��ȴ�ʱ����������
typedef enum LaunchType {
	LTYPE_0 = 0, //����
	LTYPE_1 = 1, //ֱ��
	LTYPE_2 = 2, //�ӳ�(�趨�ȴ�ʱ��)
}LAUNCHTYPE;

//����Ӧ�ó����ļ����ơ��������������ͼ��ȴ�ʱ����������
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

//ϵͳ��Ȩ����
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

//���ϵͳ�汾�Ƿ���Vista����ߵİ汾
__inline static bool IsOsVersionVistaOrGreater()
{
	OSVERSIONINFOEX ovex = { 0 };
	_TCHAR tzVersionInfo[MAX_PATH] = { 0 };

	//���ò����Ĵ�С�����ò��ж��Ƿ�ɹ�
	ovex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!GetVersionEx((OSVERSIONINFO *)(&ovex)))
	{
		return false;
	}
	//ͨ���汾�ţ��ж��Ƿ���vista��֮��汾
	if (ovex.dwMajorVersion > 5)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//��鲢����ϵͳ�汾ѡ��򿪳���ʽ
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
				//DEBUG_TRACE(_T("����Ȩ�ޱ��û��ܾ�\n"));
			}
			else if (dwStatus == ERROR_FILE_NOT_FOUND)
			{
				//DEBUG_TRACE(_T("��Ҫִ�е��ļ�û���ҵ�\n"));
			}
			else
			{
				//DEBUG_TRACE(_T("ʧ��ԭ��δ�ҵ�\n"));
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

//����ʵ��ֻ����һ��
__inline static BOOL RunAppOnce(tstring tsName)
{
	HANDLE hMutexInstance = ::CreateMutex(NULL, FALSE, tsName.c_str());  //��������
	if (hMutexInstance)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			//OutputDebugString(_T("�����ⷵ�أ�"));
			CloseHandle(hMutexInstance);
			return FALSE;
		}
	}
	return TRUE;
}

//��ȡ������·��
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

//��ȡϵͳ��ʱ·��
__inline static tstring GetTempPath()
{
	_TCHAR tTempPath[MAX_PATH] = { 0 };
	GetTempPath(MAX_PATH, tTempPath);
	return tstring(tTempPath);
}

//��ȡ�����ļ�·��
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

//��ȡϵͳ·��
__inline static tstring GetSystemPath()
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

//�ж�Ŀ¼�Ƿ����
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
//�ж�Ŀ¼�Ƿ���ڣ����������򴴽�
__inline static BOOL CreateCascadeDirectory(LPCTSTR lpPathName,        //Directory name
	LPSECURITY_ATTRIBUTES lpSecurityAttributes/* = NULL*/  // Security attribute
	)
{
	if (IsDirectoryExists(lpPathName))       //���Ŀ¼�Ѵ��ڣ�ֱ�ӷ���
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
			//����ʧ��ʱ��Ӧɾ���Ѵ������ϲ�Ŀ¼���˴���
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

#define CMD_PATH_NAME				"CMD.EXE" //���·������

//��ȡcmd.exe�ļ�·��
__inline static tstring GetCmdPath()
{
	return GetSystemPath() + _T(CMD_PATH_NAME);
}

//�趨cmd.exe·��
static const tstring CMD_FULL_PATH_NAME = GetCmdPath();

#define ANSI2UTF8(x) UnicodeToUTF8(ANSIToUnicode(x))
#define UTF82ANSI(x) UnicodeToANSI(UTF8ToUnicode(x))

__inline static size_t file_reader(std::string&data, std::string filename, std::string mode = "rb")
{
#define DATA_BASE_SIZE	0x200

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
		while (std::regex_search(itb, ite, smatch, std::regex(pattern)))//���ƥ��ɹ�  
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

//��ȡָ�������ַ���֮����ַ�������
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
//��ȡָ�������ַ���֮����ַ�������
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

//��ȡָ�������ַ���֮����ַ�������
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

//��ȡָ�������ַ���֮����ַ�������
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

//��ȡ����ʱ�������(���ؽ��Ϊ100�����ʱ��, 1ns=1 000 000ms=1000 000 000s)
CONST ULONGLONG MILLI_100NANO = 1000000ULL / 100ULL;
__inline static LONGLONG GetCurrentMillisecons()
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

__inline static TSTRING ParseError(DWORD dwErrorCodes, HINSTANCE hInstance = NULL)
{
	BOOL bResult = FALSE;
	HLOCAL hLocal = NULL;
	TSTRING strErrorText = _T("");
	
	bResult = ::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		hInstance,
		dwErrorCodes,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPTSTR)&hLocal,
		0,
		NULL);
	if (!bResult)
	{
		if (hInstance)
		{
			bResult = ::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_HMODULE |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				hInstance,
				dwErrorCodes,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
				(LPTSTR)&hLocal,
				0,
				NULL);
			if (!bResult)
			{
				// failed
				// Unknown error code %08x (%d)
				strErrorText = STRING_FORMAT(_T("Unknown error code 0x%08X"), dwErrorCodes);
			}
		}
	}

	if (bResult && hLocal)
	{
		// Success
		LPTSTR pT = (LPTSTR)_tcschr((LPCTSTR)hLocal, _T('\r'));
		if (pT != NULL)
		{
			//Lose CRLF
			*pT = _T('\0');
		}
		strErrorText = (LPCTSTR)hLocal;
	}

	if (hLocal)
	{
		::LocalFree(hLocal);
		hLocal = NULL;
	}

	return strErrorText;
}
__inline static void printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	_tprintf(TEXT("\n\t%s failed with error %d (%s)"), msg, eNum, sysMsg);
}

//��ȡWindowsϵͳ��Ϣ
__inline static VOID GetNativeSystemInformation(SYSTEM_INFO & system_info)
{
	typedef void (WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO);

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress(GetModuleHandle(_T("KERNEL32.DLL")), "GetNativeSystemInfo");
	if (fnGetNativeSystemInfo)
	{
		fnGetNativeSystemInfo(&system_info);
	}
	else
	{
		GetSystemInfo(&system_info);
	}
}
//��ȡ����ϵͳ�汾��Ϣ
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

__inline static TSTRING VarToStr(VARIANT var)
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

//////////////////////////////////////////////////////////////////////////////
//�жϽ�����64λ����32λ(bIsWow64ΪTRUEΪ64λ������FALSEΪ32λ)
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

//��ȡ�����û�������
__inline static BOOL GetProcessUserName(TSTRING & strUser, DWORD dwID) // ����ID 
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
//��ȡ����ӳ������
//Windows 2000		= GetModuleFileName()
//Windows XP x32	= GetProcessImageFileName()
//Windows XP x64	= GetProcessImageFileName()
//Windows Vista		= QueryFullProcessImageName()
//Windows 7			= QueryFullProcessImageName()
//Windows 8			= QueryFullProcessImageName()
//Windows 8.1		= QueryFullProcessImageName()
//Windows 10		= QueryFullProcessImageName()
///////////////////////////////////////////////////////////////////////

__inline static BOOL EnumModules_R3(std::map<DWORD, MODULEENTRY32> & memap, DWORD dwPID)
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
		memap.insert(std::map<DWORD, MODULEENTRY32>::value_type(me.th32ModuleID, me));
	} while (Module32Next(hSnapModule, &me));

__LEAVE_CLEAN__:

	//�رվ��
	CloseHandle(hSnapModule);
	hSnapModule = NULL;

	return bRet;
}

__inline static BOOL EnumModules32_R3(std::map<DWORD, MODULEENTRY32> & memap, DWORD dwPID)
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
		memap.insert(std::map<DWORD, MODULEENTRY32>::value_type(me.th32ModuleID, me));
	} while (Module32Next(hSnapModule, &me));

__LEAVE_CLEAN__:

	//�رվ��
	CloseHandle(hSnapModule);
	hSnapModule = NULL;

	return bRet;
}

__inline static BOOL EnumProcess_R3(std::map<DWORD, PROCESSENTRY32> & pemap)
{
	BOOL bRet = FALSE;
	PROCESSENTRY32 pe = { 0 };
	HANDLE hSnapProcess = INVALID_HANDLE_VALUE;
	
	//��������
	hSnapProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapProcess == INVALID_HANDLE_VALUE)
	{
		goto __LEAVE_CLEAN__;
	}

	
	pe.dwSize = sizeof(PROCESSENTRY32);

	//��������	
	bRet = Process32First(hSnapProcess, &pe);
	if (!bRet)
	{
		goto __LEAVE_CLEAN__;
	}

	do
	{
		pemap.insert(std::map<DWORD, PROCESSENTRY32>::value_type(pe.th32ProcessID, pe));
	} while (Process32Next(hSnapProcess, &pe));
	
__LEAVE_CLEAN__:

	//�رվ��
	CloseHandle(hSnapProcess);
	hSnapProcess = NULL;

	return bRet;
}

__inline static BOOL EnumThread_R3(std::map<DWORD, THREADENTRY32> & temap, DWORD dwPID)
{
	BOOL bRet = FALSE;
	THREADENTRY32 te = { 0 };
	HANDLE hSnapThread = INVALID_HANDLE_VALUE;

	//��������
	hSnapThread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPID);
	if (hSnapThread == INVALID_HANDLE_VALUE)
	{
		goto __LEAVE_CLEAN__;
	}

	te.dwSize = sizeof(THREADENTRY32);

	//��������	
	bRet = Thread32First(hSnapThread, &te);
	if (!bRet)
	{
		goto __LEAVE_CLEAN__;
	}

	do
	{
		temap.insert(std::map<DWORD, THREADENTRY32>::value_type(te.th32ThreadID, te));
	} while (Thread32Next(hSnapThread, &te));

__LEAVE_CLEAN__:

	//�رվ��
	CloseHandle(hSnapThread);
	hSnapThread = NULL;

	return bRet;
}

__inline static BOOL EnumHeapList_R3(std::map<DWORD, HEAPLIST32> & hlmap, DWORD dwPID)
{
	BOOL bRet = FALSE;
	HEAPLIST32 hl = { 0 };
	HANDLE hSnapHeapList = INVALID_HANDLE_VALUE;

	//��������
	hSnapHeapList = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, dwPID);
	if (hSnapHeapList == INVALID_HANDLE_VALUE)
	{
		goto __LEAVE_CLEAN__;
	}

	hl.dwSize = sizeof(HEAPLIST32);

	//��������	
	bRet = Heap32ListFirst(hSnapHeapList, &hl);
	if (!bRet)
	{
		goto __LEAVE_CLEAN__;
	}

	do
	{
		hlmap.insert(std::map<DWORD, HEAPLIST32>::value_type(hl.th32HeapID, hl));
	} while (Heap32ListNext(hSnapHeapList, &hl));

__LEAVE_CLEAN__:

	//�رվ��
	CloseHandle(hSnapHeapList);
	hSnapHeapList = NULL;

	return bRet;
}

__inline static NTSTATUS EnumProcessObject(
	std::vector<OBJECT_BASIC_INFORMATION> * pobiv, 
	std::vector<OBJECT_NAME_INFORMATION> * poniv,
	std::vector<OBJECT_TYPE_INFORMATION> * potiv,
	DWORD dwProcessID)
{
	BOOL bResult = FALSE;
	NTSTATUS ntStatus = 0;
	HANDLE hTargetHandle = 0;
	HANDLE hSourceProcess = 0;
	HANDLE hTargetProcess = 0;
	DWORD dwProcessHandleCount = 0;
	DWORD dwIndexX = 0;
	DWORD dwIndexY = 0;
	DWORD dwSourceHandleNumber = 0;
	SYSTEM_HANDLE* pSystemHandle = 0;
	OBJECT_BASIC_INFORMATION * pObjectBasicInformation = NULL;
	OBJECT_NAME_INFORMATION * pObjectNameInformation = NULL;
	OBJECT_TYPE_INFORMATION * pObjectTypeInformation = NULL;
	DWORD dwSystemHandleInfomationSize = sizeof(SYSTEM_HANDLE_INFORMATION);
	DWORD dwObjectBasicInformationSize = sizeof(OBJECT_BASIC_INFORMATION) + USN_PAGE_SIZE;
	DWORD dwObjectNameInformationSize = sizeof(OBJECT_NAME_INFORMATION) + USN_PAGE_SIZE;
	DWORD dwObjectTypeInformationSize = sizeof(OBJECT_TYPE_INFORMATION) + USN_PAGE_SIZE;

	hTargetProcess = GetCurrentProcess();
	pObjectBasicInformation = (OBJECT_BASIC_INFORMATION*)malloc(dwObjectBasicInformationSize);
	pObjectNameInformation = (OBJECT_NAME_INFORMATION*)malloc(dwObjectNameInformationSize);
	pObjectTypeInformation = (OBJECT_TYPE_INFORMATION*)malloc(dwObjectTypeInformationSize);

	hSourceProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_DUP_HANDLE | PROCESS_SUSPEND_RESUME, FALSE, dwProcessID);
	if (!hSourceProcess)
	{
		ntStatus = STATUS_UNSUCCESSFUL;
		return ntStatus;
	}
	(NTSTATUS)FUNC_PROC(ZwSuspendProcess)(hSourceProcess);
	(NTSTATUS)FUNC_PROC(ZwQueryInformationProcess)(hSourceProcess, ProcessHandleInformation, &dwProcessHandleCount, sizeof(dwProcessHandleCount), NULL);

	//������Ч�����4��ʼ,ÿ����4����
	dwSourceHandleNumber = sizeof(DWORD);

	for (dwIndexY = 0; dwIndexY < dwProcessHandleCount; dwIndexY++, dwSourceHandleNumber += sizeof(dwSourceHandleNumber))
	{
		//�ж��Ƿ�Ϊ��Ч���������TRUE��������Ч���
		bResult = DuplicateHandle(hSourceProcess, (HANDLE)dwSourceHandleNumber, hTargetProcess, &hTargetHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
		if (!bResult)
		{
			continue;
		}
		else
		{
			memset(pObjectBasicInformation, 0, dwObjectBasicInformationSize);
			memset(pObjectNameInformation, 0, dwObjectNameInformationSize);
			memset(pObjectTypeInformation, 0, dwObjectTypeInformationSize);

			(NTSTATUS)FUNC_PROC(ZwQueryObject)(hTargetHandle, ObjectBasicInformation, pObjectBasicInformation, dwObjectBasicInformationSize, NULL);
			(NTSTATUS)FUNC_PROC(ZwQueryObject)(hTargetHandle, ObjectNameInformation, pObjectNameInformation, dwObjectNameInformationSize, NULL);
			(NTSTATUS)FUNC_PROC(ZwQueryObject)(hTargetHandle, ObjectTypeInformation, pObjectTypeInformation, dwObjectTypeInformationSize, NULL);
			if (pObjectNameInformation->Name.Length && pObjectTypeInformation)
			{
				if (pobiv)
				{
					pobiv->push_back(*pObjectBasicInformation);
				}
				if (poniv)
				{
					poniv->push_back(*pObjectNameInformation);
				}
				if (potiv)
				{
					potiv->push_back(*pObjectTypeInformation);
				}
			}
			CloseHandle(hTargetHandle);
			hTargetHandle = NULL;
		}
	}
	(NTSTATUS)FUNC_PROC(ZwResumeProcess)(hSourceProcess);
	CloseHandle(hSourceProcess);
	hSourceProcess = NULL;

	ntStatus = STATUS_SUCCESS;
	return ntStatus;
}
