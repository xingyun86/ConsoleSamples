
#include "MACROS.h"

namespace PPSHUAI{
	namespace SystemKernel{
		__inline static
			BOOL ReadProcessMemoryData(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpData, SIZE_T stSize, SIZE_T * pstNumberOfBytesRead)
		{
			return ReadProcessMemory(hProcess, lpBaseAddress, lpData, stSize, pstNumberOfBytesRead);
		}
		__inline static
			BOOL WriteProcessMemoryData(HANDLE hProcess, LPVOID lpBaseAddress, LPVOID lpData, SIZE_T stSize, SIZE_T * pstNumberOfBytesWtitten)
		{
			return WriteProcessMemory(hProcess, lpBaseAddress, lpData, stSize, pstNumberOfBytesWtitten);
		}
		__inline static
			BOOL ReadProcessMemoryData(HANDLE hProcess, std::map<SIZE_T, std::string> * pssmap, std::map<SIZE_T, BOOL> * psbmap = NULL)
		{
			BOOL bResult = FALSE;
			SIZE_T stNumberOfBytesRead = 0;
			CHAR czData[MAXWORD + 1] = { 0 };
			std::map<SIZE_T, std::string>::iterator itEnd;
			std::map<SIZE_T, std::string>::iterator itIdx;

			itEnd = pssmap->end();
			itIdx = pssmap->begin();
			for (; itIdx != itEnd; itIdx++)
			{
				if (psbmap)
				{
					psbmap->at(itIdx->first) = FALSE;
				}
				stNumberOfBytesRead = 0;
				bResult = ReadProcessMemory(hProcess, (LPCVOID)itIdx->first, czData, sizeof(czData) * sizeof(CHAR), &stNumberOfBytesRead);
				if (bResult || stNumberOfBytesRead)
				{
					itIdx->second = czData;
					memset(czData, 0, sizeof(czData) * sizeof(CHAR));
					if (psbmap)
					{
						psbmap->at(itIdx->first) = TRUE;
					}
				}
			}

			return bResult;
		}
		__inline static
			BOOL WriteProcessMemoryData(HANDLE hProcess, std::map<SIZE_T, std::string> * pssmap, std::map<SIZE_T, BOOL> * psbmap = NULL)
		{
			BOOL bResult = FALSE;
			SIZE_T stNumberOfBytesWritten = 0;
			std::map<SIZE_T, std::string>::iterator itEnd;
			std::map<SIZE_T, std::string>::iterator itIdx;

			itEnd = pssmap->end();
			itIdx = pssmap->begin();
			for (; itIdx != itEnd; itIdx++)
			{
				if (psbmap)
				{
					psbmap->at(itIdx->first) = FALSE;
				}
				stNumberOfBytesWritten = 0;
				bResult = WriteProcessMemory(hProcess, (LPVOID)itIdx->first, itIdx->second.c_str(), itIdx->second.length(), &stNumberOfBytesWritten);
				if (bResult || stNumberOfBytesWritten)
				{
					if (psbmap)
					{
						psbmap->at(itIdx->first) = TRUE;
					}
				}
			}

			return bResult;
		}

		__inline static
			BOOL ModifyProcessMemoryVirtualProtect(HANDLE hProcess, LPVOID lpBaseAddress, SIZE_T stRegionSize, DWORD dwNewProtect, DWORD * pdwOldProtect)
		{
			return VirtualProtectEx(hProcess, (LPVOID)lpBaseAddress, stRegionSize, dwNewProtect, pdwOldProtect);
		}

		__inline static
			void GetProcessMemoryPageList(std::map<SIZE_T, MEMORY_BASIC_INFORMATION> * pmbimap, HANDLE hProcess)
		{
			DWORD dwPID = 0;
			SIZE_T stResultSize = 0;
			LPVOID lpMemoryAddress = NULL;
			LPVOID lpMemoryAddressMIN = NULL;
			LPVOID lpMemoryAddressMAX = NULL;
			SYSTEM_INFO siSystemInfo = { 0 };
			MEMORY_BASIC_INFORMATION mbiMemoryBuffer = { 0 };

			// Get maximum address range from system info.
			GetNativeSystemInformation(&siSystemInfo);
			lpMemoryAddressMIN = siSystemInfo.lpMinimumApplicationAddress;
			lpMemoryAddressMAX = siSystemInfo.lpMaximumApplicationAddress;
			lpMemoryAddress = lpMemoryAddressMIN;
			// Walk process addresses.
			while (lpMemoryAddress < lpMemoryAddressMAX)
			{
				// Query next region of memory in the process.
				stResultSize = VirtualQueryEx(hProcess, lpMemoryAddress, &mbiMemoryBuffer, sizeof(MEMORY_BASIC_INFORMATION));
				if (stResultSize != sizeof(MEMORY_BASIC_INFORMATION))
				{
					break;
				}

				pmbimap->insert(std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::value_type((SIZE_T)mbiMemoryBuffer.BaseAddress, mbiMemoryBuffer));
				// increment lpMemoryAddress to next region of memory.
				lpMemoryAddress = (LPVOID)((SIZE_T)mbiMemoryBuffer.BaseAddress + (SIZE_T)mbiMemoryBuffer.RegionSize);
			}
		}

		__inline static
			void SearchProcessMemoryPageList(std::map<SIZE_T, SIZE_T> * pssmap,
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION> * pmbimap,
			const VOID * pvData, SIZE_T stSize, HANDLE hProcess)
		{
#ifndef _PART_DATA_SIZE_
#define _PART_DATA_SIZE_ 0x800000 // 8M
#endif //_PART_DATA_SIZE_

			BYTE * pbData = NULL;
			CHAR * pcbData = NULL;
			SIZE_T stDataHead = 0;
			SIZE_T stDataSize = 0;
			SIZE_T stReadSize = 0;
			SIZE_T stSearchPos = 0;
			SIZE_T stNumberOfBytesRead = 0;
			std::string strMemoryBuffer = ("");
			SIZE_T stUnitSize = _PART_DATA_SIZE_;
			std::string strFindData((CHAR *)pvData, stSize);
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itEnd;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itIdx;

			itEnd = pmbimap->end();
			itIdx = pmbimap->begin();

			//���������ڴ���
			pbData = (BYTE *)malloc(stUnitSize * sizeof(BYTE));
			if (pbData)
			{
				strMemoryBuffer.assign((CHAR *)pbData, stUnitSize * sizeof(BYTE));
				pcbData = (CHAR *)strMemoryBuffer.c_str();
				for (; itIdx != itEnd; itIdx++)
				{
					stDataHead = 0;
					stDataSize = itIdx->second.RegionSize;

					while (stDataHead < stDataSize)
					{
						stNumberOfBytesRead = 0;
						stReadSize = (((itIdx->second.RegionSize - stDataHead) > stUnitSize) ? stUnitSize : (itIdx->second.RegionSize - stDataHead));
						if (ReadProcessMemory(hProcess, (BYTE *)itIdx->second.BaseAddress + stDataHead, (LPVOID)pcbData, stReadSize, &stNumberOfBytesRead))
						{
							// Method One
							// String find and search
							stSearchPos = 0;
							while ((stSearchPos = strMemoryBuffer.find(strFindData, stSearchPos)) != std::string::npos)
							{
								pssmap->insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)itIdx->second.BaseAddress + stSearchPos, (SIZE_T)itIdx->second.BaseAddress));
								stSearchPos += stSize;
							}

							stDataHead += ((stNumberOfBytesRead <= stDataSize) ? stNumberOfBytesRead : (stNumberOfBytesRead - stSize < 0) ? stSize : (stNumberOfBytesRead - stSize));
							memset(pcbData, 0, stUnitSize * sizeof(BYTE));
						}
						else
						{
							// Read failure
							break;
						}
					}
				}
				free(pbData);
				pbData = NULL;
			}
#ifdef _PART_DATA_SIZE_
#undef _PART_DATA_SIZE_
#endif //_PART_DATA_SIZE_
		}

		__inline static
			void SearchProcessMemoryPageList(std::map<SIZE_T, SIZE_T> * pssmap, const VOID * pvData, SIZE_T stSize, const _TCHAR * ptProcessName)
		{
			HANDLE hProcess = NULL;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION> mbimap;

			hProcess = InitProcessHandle(ptProcessName);

			if (hProcess)
			{
				GetProcessMemoryPageList(&mbimap, hProcess);

				SearchProcessMemoryPageList(pssmap, &mbimap, pvData, stSize, hProcess);

				ExitProcessHandle(&hProcess);
			}
		}

		__inline static
			void SearchProcessMemoryPageListEx(std::map<SIZE_T, SIZE_T> * pssmap,
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION> * pmbimap,
			const VOID * pvData, SIZE_T stSize, HANDLE hProcess)
		{
#ifndef _PART_DATA_SIZE_
#define _PART_DATA_SIZE_ 0x800000 // 8M
#endif //_PART_DATA_SIZE_
			DWORD dwPID = 0;
			BYTE * pbData = NULL;
			LPVOID lpData = NULL;
			SIZE_T stDataHead = 0;
			SIZE_T stDataSize = 0;
			SIZE_T stReadSize = 0;
			SIZE_T stSearchPos = 0;
			SIZE_T stNumberOfBytesRead = 0;
			SIZE_T stUnitSize = _PART_DATA_SIZE_;
			std::string strFindData((CHAR *)pvData, stSize);
			std::string strMemoryBuffer(stUnitSize, ('\x00'));
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itEnd;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itIdx;

			itEnd = pmbimap->end();
			itIdx = pmbimap->begin();

			if (strMemoryBuffer.size() > 0)
			{
				pbData = (BYTE *)strMemoryBuffer.c_str();

				//���������ڴ���
				for (; itIdx != itEnd; itIdx++)
				{
					stDataHead = 0;
					stDataSize = itIdx->second.RegionSize;

					while (stDataHead < stDataSize)
					{
						stNumberOfBytesRead = 0;
						stReadSize = (((itIdx->second.RegionSize - stDataHead) > stUnitSize) ? stUnitSize : (itIdx->second.RegionSize - stDataHead));
						if (ReadProcessMemory(hProcess, (BYTE *)itIdx->second.BaseAddress + stDataHead, (LPVOID)pbData, stReadSize, &stNumberOfBytesRead))
						{
							stSearchPos = 0;
							while ((stSearchPos = strMemoryBuffer.find(strFindData, stSearchPos)) != std::string::npos)
							{
								pssmap->insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)itIdx->second.BaseAddress + stSearchPos, (SIZE_T)itIdx->second.BaseAddress));
								stSearchPos += stSize;
							}
							stDataHead += ((stNumberOfBytesRead <= stDataSize) ? stNumberOfBytesRead : (stNumberOfBytesRead - stSize < 0) ? stNumberOfBytesRead : (stNumberOfBytesRead - stSize));
							memset(pbData, 0, stUnitSize * sizeof(BYTE));
						}
						else
						{
							// Read failure
							break;
						}
					}
				}
			}

#ifdef _PART_DATA_SIZE_
#undef _PART_DATA_SIZE_
#endif //_PART_DATA_SIZE_
		}

		__inline static
			void SearchProcessMemoryPageListEx(std::map<SIZE_T, SIZE_T> * pssmap, const VOID * pvData, SIZE_T stSize, const _TCHAR * ptProcessName)
		{
			HANDLE hProcess = NULL;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION> mbimap;

			hProcess = InitProcessHandle(ptProcessName);

			if (hProcess)
			{
				GetProcessMemoryPageList(&mbimap, hProcess);

				SearchProcessMemoryPageListEx(pssmap, &mbimap, pvData, stSize, hProcess);

				ExitProcessHandle(&hProcess);
			}
		}
		__inline static
			void PrintMemoryBasicInformation(MEMORY_BASIC_INFORMATION * pmbi)
		{
			if (pmbi)
			{
				if ((pmbi->AllocationBase != pmbi->BaseAddress)
					&& (pmbi->State != MEM_FREE))
				{
#if !defined(_WIN64) && !defined(WIN64)
					_tprintf(_T("  0x%08lX  0x%08lX  "),
#else
					_tprintf(_T("  0x%016llX  0x%016llX  "),
#endif
						pmbi->BaseAddress,
						pmbi->RegionSize);
				}
				else
				{
#if !defined(_WIN64) && !defined(WIN64)
					_tprintf(_T("  0x%08lX  0x%08lX  "),
#else
					_tprintf(_T("  0x%016llX  0x%016llX  "),
#endif
						pmbi->BaseAddress,
						pmbi->RegionSize);
				}
				_tprintf(_T("\t"));

				switch (pmbi->State)
				{
				case MEM_COMMIT:
					_tprintf(_T("MEM_COMMIT "));
					break;
				case MEM_RESERVE:
					_tprintf(_T("MEM_FREE   "));
					break;
				case MEM_RELEASE:
					_tprintf(_T("MEM_RESERVE"));
					break;
				default:
					_tprintf(_T("-----------"));
					break;
				}
				_tprintf(_T("\t"));

				switch (pmbi->Type)
				{
				case MEM_IMAGE:
					_tprintf(_T("MEM_IMAGE  "));
					break;
				case MEM_MAPPED:
					_tprintf(_T("MEM_MAPPED "));
					break;
				case MEM_PRIVATE:
					_tprintf(_T("MEM_PRIVATE"));
					break;
				default:
					_tprintf(_T("-----------"));
					break;
				}
				_tprintf(_T("\t"));

				switch (pmbi->AllocationProtect)
				{
				case PAGE_READONLY:
					_tprintf(_T("PAGE_READONLY         "));
					break;
				case PAGE_READWRITE:
					_tprintf(_T("PAGE_READWRITE        "));
					break;
				case PAGE_WRITECOPY:
					_tprintf(_T("PAGE_WRITECOPY        "));
					break;
				case PAGE_EXECUTE:
					_tprintf(_T("PAGE_EXECUTE          "));
					break;
				case PAGE_EXECUTE_READ:
					_tprintf(_T("PAGE_EXECUTE_READ     "));
					break;
				case PAGE_EXECUTE_READWRITE:
					_tprintf(_T("PAGE_EXECUTE_READWRITE"));
					break;
				case PAGE_EXECUTE_WRITECOPY:
					_tprintf(_T("PAGE_EXECUTE_WRITECOPY"));
					break;
				case PAGE_GUARD:
					_tprintf(_T("PAGE_GUARD            "));
					break;
				case PAGE_NOACCESS:
					_tprintf(_T("PAGE_NOACCESS         "));
					break;
				case PAGE_NOCACHE:
					_tprintf(_T("PAGE_NOCACHE          "));
					break;
				default:
					_tprintf(_T("----------------------"));
					break;
				}

				_tprintf(_T("\r\n"));
			}
		}
		__inline static
			void PrintProcessMemoryPages(std::map<SIZE_T, MEMORY_BASIC_INFORMATION> * pmbimap)
		{
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itEnd;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itIdx;

			if (pmbimap)
			{
				itEnd = pmbimap->end();
				itIdx = pmbimap->begin();

				for (; itIdx != itEnd; itIdx++)
				{
					PrintMemoryBasicInformation(&itIdx->second);
				}
			}
		}

		__inline static
			void PrintProcessMemoryPages(const _TCHAR * ptProcessName)
		{
			HANDLE hProcess = NULL;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION> mbimap;

			hProcess = InitProcessHandle(ptProcessName);
			if (hProcess)
			{
				GetProcessMemoryPageList(&mbimap, hProcess);

				PrintProcessMemoryPages(&mbimap);

				ExitProcessHandle(&hProcess);
			}
		}

		// �ڴ�����ӳ���ʼ��
		__inline static
			HANDLE MapMemoryInitialize(BYTE ** ppbData, ULARGE_INTEGER * puiSize, std::map<SIZE_T, SIZE_T> * prsssmap, std::map<SIZE_T, MEMORY_BASIC_INFORMATION> * psmbimap, HANDLE hProcess, LPCTSTR lpMapName = _T("__MAP_MAP__"))
		{
			START_TIMER_TICKS(DEBUG);

			SYSTEM_INFO si = { 0 };
			SIZE_T stPlaceIndex = 0;
			HANDLE hFileMapping = NULL;
			SIZE_T stNumberOfBytesRead = 0;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itEnd;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itIdx;
			
			// Get memory map assign size
			PPSHUAI::SystemKernel::GetNativeSystemInformation(&si);

			// Calculate map size
			itEnd = psmbimap->end();
			itIdx = psmbimap->begin();
			for (; itIdx != itEnd; itIdx++)
			{
				if (((itIdx->second.AllocationProtect & PAGE_READONLY) == PAGE_READONLY) ||
					((itIdx->second.AllocationProtect & PAGE_READWRITE) == PAGE_READWRITE) ||
					((itIdx->second.AllocationProtect & PAGE_WRITECOPY) == PAGE_WRITECOPY) ||
					((itIdx->second.AllocationProtect & PAGE_EXECUTE_READ) == PAGE_EXECUTE_READ) ||
					((itIdx->second.AllocationProtect & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE) ||
					((itIdx->second.AllocationProtect & PAGE_EXECUTE_WRITECOPY) == PAGE_EXECUTE_WRITECOPY))
				{
					puiSize->QuadPart += itIdx->second.RegionSize;
				}
			}

			// Assign address size
			puiSize->QuadPart += puiSize->QuadPart % si.dwAllocationGranularity;
			
			// Create memory map
			hFileMapping = PPSHUAI::FilePath::MapCreate((LPVOID *)ppbData, lpMapName, puiSize);
			if (hFileMapping)
			{
				itEnd = psmbimap->end();
				itIdx = psmbimap->begin();
				for (; itIdx != itEnd; itIdx++)
				{
					if (ReadProcessMemory(hProcess, (LPCVOID)itIdx->first, (*ppbData) + stPlaceIndex, itIdx->second.RegionSize, &stNumberOfBytesRead))
					{
						if (prsssmap)
						{
							prsssmap->insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)stPlaceIndex, itIdx->first));
						}
						stPlaceIndex += stNumberOfBytesRead;
					}
				}
			}

			CLOSE_TIMER_TICKS(DEBUG);

			return hFileMapping;
		}

		// �ڴ���������
		__inline static
			SIZE_T MapMemorySearchString(std::map<SIZE_T, SIZE_T> * pssmap, LPVOID lpData, ULARGE_INTEGER * puiSize, LPVOID lpFindData, SIZE_T stFindSize)
		{
			START_TIMER_TICKS(DEBUG);

			std::string::size_type stPlaceIndex = 0;
			std::string strFindData((CONST CHAR *)lpFindData, stFindSize);
			std::string strMemoryBuffer((CHAR *)(lpData), puiSize->QuadPart);		
			
			while ((stPlaceIndex = strMemoryBuffer.find(strFindData, stPlaceIndex)) != std::string::npos)
			{
				pssmap->insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)stPlaceIndex, (SIZE_T)stPlaceIndex));
				stPlaceIndex += strFindData.length();
			}

			CLOSE_TIMER_TICKS(DEBUG);

			return 0;
		}

		//����ڴ�ӳ���ַ=>>���Խ����ڴ��ַ
		__inline static
			SIZE_T MapMemoryRelativeTransferAbsolute(std::map<SIZE_T, SIZE_T> * psssmap, std::map<SIZE_T, SIZE_T> * prssmap, std::map<SIZE_T, SIZE_T> * prvassmap, std::map<SIZE_T, MEMORY_BASIC_INFORMATION> * psmbimap)
		{
			std::map<SIZE_T, SIZE_T>::iterator itEnd;
			std::map<SIZE_T, SIZE_T>::iterator itIdx;
			std::map<SIZE_T, SIZE_T>::iterator itRSEnd;
			std::map<SIZE_T, SIZE_T>::iterator itRSIdx;
						
			// ����ڴ�ӳ���ַ�б�
			itEnd = prssmap->end();
			itIdx = prssmap->begin();
			for (; itIdx != itEnd; itIdx++)
			{
				// �����ڴ�����ӳ���ַ�б�
				itRSEnd = prvassmap->end();
				itRSIdx = prvassmap->begin();
				for (; itRSIdx != itRSEnd; itRSIdx++)
				{
					if ((SIZE_T)itIdx->first >= itRSIdx->first &&
						(SIZE_T)itIdx->first < (itRSIdx->first + psmbimap->at(itRSIdx->second).RegionSize))
					{
						break;
					}
				}
				if (itRSIdx != itRSEnd)
				{
					psssmap->insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)psmbimap->at(itRSIdx->second).BaseAddress + (itIdx->first - itRSIdx->first), (SIZE_T)psmbimap->at(itRSIdx->second).RegionSize));
				}
			}

			return 0;
		}

		// �ڴ�ӳ����Դ�ͷ�
		__inline static 
			void MapMemoryExitialize(HANDLE * phFileMapping, LPVOID * pbData)
		{
			PPSHUAI::FilePath::MapRelease(phFileMapping, pbData);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


		//ע��DLL��Զ�̽���(ʹ��CreateRemoteThread)
		__inline BOOL InjectDllToRemoteProcess(const _TCHAR* lpDllName, const _TCHAR* lpPid, const _TCHAR* lpProcName)
		{
			PROCESS_INFORMATION pi = { 0 };

			if ((ElevatePrivileges() == FALSE) || (UNDOCAPI::InitUnDocumentApis() == FALSE))
			{
				return FALSE;
			}

			if (_tcsstr(lpProcName, _T("\\")) || _tcsstr(lpProcName, _T("/")))
			{
				StartupProgram(lpProcName, _T(""), NULL, &pi);
			}

			if (pi.dwProcessId <= 0)
			{
				if (NULL == lpPid || 0 == _tcslen(lpPid))
				{
					if (NULL != lpProcName && 0 != _tcslen(lpProcName))
					{
						if (pi.dwProcessId = GetProcessIdByProcessName(lpProcName))
						{
							return FALSE;
						}
					}
					else
					{
						return FALSE;
					}
				}
				else
				{
					pi.dwProcessId = _ttoi(lpPid);
				}
			}

			//����Pid�õ����̾��(ע�����Ȩ��)
			HANDLE hRemoteProcess = NULL;
			hRemoteProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
			//hRemoteProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_SUSPEND_RESUME, FALSE, stPid);
			if (!hRemoteProcess && INVALID_HANDLE_VALUE == hRemoteProcess)
			{
				return FALSE;
			}

			//�������
			FUNC_PROC(ZwSuspendProcess)(hRemoteProcess);

			//����DLL·������Ҫ���ڴ�ռ�
			SIZE_T stSize = (1 + _tcslen(lpDllName)) * sizeof(_TCHAR);

			//ʹ��VirtualAllocEx������Զ�̽��̵��ڴ��ַ�ռ����DLL�ļ���������,�ɹ����ط����ڴ���׵�ַ.
			LPVOID lpRemoteBuff = VirtualAllocEx(hRemoteProcess, NULL, stSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (NULL == lpRemoteBuff)
			{
				CloseHandle(hRemoteProcess);
				return FALSE;
			}

			//ʹ��WriteProcessMemory������DLL��·�������Ƶ�Զ�̽��̵��ڴ�ռ�,�ɹ�����TRUE.
			SIZE_T stHasWrite = 0;
			BOOL bRet = WriteProcessMemory(hRemoteProcess, lpRemoteBuff, lpDllName, stSize, &stHasWrite);
			if (!bRet || stHasWrite != stSize)
			{
				VirtualFreeEx(hRemoteProcess, lpRemoteBuff, stSize, MEM_DECOMMIT | MEM_RELEASE);
				CloseHandle(hRemoteProcess);
				return FALSE;
			}

			//����һ�����������̵�ַ�ռ������е��߳�(Ҳ��:����Զ���߳�),�ɹ��������߳̾��.
			//ע��:���̾������߱�PROCESS_CREATE_THREAD, PROCESS_QUERY_INFORMATION, PROCESS_VM_OPERATION, PROCESS_VM_WRITE,��PROCESS_VM_READ����Ȩ��
			DWORD dwRemoteThread = 0;
			LPTHREAD_START_ROUTINE pfnLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
			HANDLE hRemoteThread = CreateRemoteThread(hRemoteProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pfnLoadLibrary, lpRemoteBuff, 0, &dwRemoteThread);
			if (INVALID_HANDLE_VALUE == hRemoteThread)
			{
				VirtualFreeEx(hRemoteProcess, lpRemoteBuff, stSize, MEM_DECOMMIT | MEM_RELEASE);
				CloseHandle(hRemoteProcess);
				return FALSE;
			}

			//ע��ɹ��ͷž��
			WaitForSingleObject(hRemoteThread, INFINITE);

			FUNC_PROC(ZwResumeProcess)(hRemoteProcess);

			CloseHandle(hRemoteThread);
			CloseHandle(hRemoteProcess);

			return TRUE;
		}

		//ʹ�ô����ʵ��Զ�̽���ע��
		__inline BOOL InjectDll(DWORD dwProcessId, DWORD dwThreadId, const _TCHAR * ptzDllName)
		{
			BOOL bResult = FALSE;
			FARPROC farproc = NULL;
			CONTEXT context = { 0 };
			LPVOID lpCodeBase = NULL;
			SIZE_T stCodeSize = USN_PAGE_SIZE;
			SIZE_T stNumberOfBytesWritten = 0;
			DWORD dwCurrentEipAddress = 0;
			DWORD dwIndexOffsetPosition = 0;
			CHAR szCodeData[USN_PAGE_SIZE] = { 0 };
			CHAR szCode0[] = ("\x60\xE8\x00\x00\x00\x00\x58\x83\xC0\x13\x50\xB8");
			CHAR szCode1[] = ("\xFF\xD0\x61\x68");
			CHAR szCode2[] = ("\xC3");

			//����Pid�õ����̾��(ע�����Ȩ��)
			HANDLE hRemoteProcess = NULL;
			HANDLE hRemoteThread = NULL;
			ULONG ulPreviousSuspendCount = 0;
			if ((ElevatePrivileges() == FALSE) || (UNDOCAPI::InitUnDocumentApis() == FALSE))
			{
				return FALSE;
			}

			hRemoteProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			//hRemoteProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_SUSPEND_RESUME, FALSE, stPid);
			if (!hRemoteProcess && INVALID_HANDLE_VALUE == hRemoteProcess)
			{
				return FALSE;
			}

			hRemoteThread = OpenThread(THREAD_ALL_ACCESS, FALSE, dwThreadId);
			//hRemoteProcess = OpenThread(THREAD_TERMINATE | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SET_INFORMATION |THREAD_SET_THREAD_TOKEN | THREAD_IMPERSONATE | THREAD_DIRECT_IMPERSONATION | THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME, FALSE, stPid);
			if (!hRemoteThread && INVALID_HANDLE_VALUE == hRemoteThread)
			{
				return FALSE;
			}

			//�������
			FUNC_PROC(ZwSuspendProcess)(hRemoteProcess);

			//�����߳�
			FUNC_PROC(ZwSuspendThread)(hRemoteThread, &ulPreviousSuspendCount);

			//��Զ�̽��̷����ִ�ж�дģ��
			lpCodeBase = VirtualAllocEx(hRemoteProcess, lpCodeBase, stCodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

			//�����߳������ĵı�ʶ
			context.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_SEGMENTS;

			//��ȡ�߳�������
			FUNC_PROC(ZwGetContextThread)(hRemoteThread, &context);

			//��ȡԶ�̽��̵ĵ�ǰִ�е�ַ
			dwCurrentEipAddress = context.Eip;

			//����Զ�̽��̵���һִ�е�ַ
			context.Eip = (DWORD)lpCodeBase;

			//��ȡLoadLibraryA�ĺ�����ַ
			farproc = GetProcAddress(GetModuleHandle(_T("KERNEL32.DLL")), ("LoadLibraryA"));

			///////////////////////////////////////////////////////////////////////////////////
			// ���ݿ�
			// CodeData �� 
			//		{ 96, 232, 0, 0, 0, 0, 88, 131, 192, 19, 80, 184 } / "\x60\xE8\x00\x00\x00\x00\x58\x83\xC0\x13\x50\xB8" 
			//		{ LoadLibraryA������ַ }
			//		{ 255, 208, 97, 104 } / "\xFF\xD0\x61\x68"
			//		{ Eip��ַ }
			//		{ 195 } / "\xC3"
			//		{ Dll·�� };

			memcpy(szCodeData + dwIndexOffsetPosition, szCode0, sizeof(szCode0) - 1);
			dwIndexOffsetPosition += sizeof(szCode0) - 1;
			memcpy(szCodeData + dwIndexOffsetPosition, &farproc, sizeof(farproc));
			dwIndexOffsetPosition += sizeof(farproc);
			memcpy(szCodeData + dwIndexOffsetPosition, szCode1, sizeof(szCode1) - 1);
			dwIndexOffsetPosition += sizeof(szCode1) - 1;
			memcpy(szCodeData + dwIndexOffsetPosition, &dwCurrentEipAddress, sizeof(dwCurrentEipAddress));
			dwIndexOffsetPosition += sizeof(dwCurrentEipAddress);
			memcpy(szCodeData + dwIndexOffsetPosition, szCode2, sizeof(szCode2) - 1);
			dwIndexOffsetPosition += sizeof(szCode2) - 1;
			memcpy(szCodeData + dwIndexOffsetPosition, Convert::TToA(ptzDllName).c_str(), Convert::TToA(ptzDllName).length());
			dwIndexOffsetPosition += Convert::TToA(ptzDllName).length();

			//д��Զ�̽��̿�ִ�ж�дģ��
			WriteProcessMemory(hRemoteProcess, lpCodeBase, szCodeData, dwIndexOffsetPosition, &stNumberOfBytesWritten);

			//�����߳�������
			FUNC_PROC(ZwSetContextThread)(hRemoteThread, &context);

			//�ָ��߳�
			FUNC_PROC(ZwResumeThread)(hRemoteThread, &ulPreviousSuspendCount);

			//�ָ�����
			FUNC_PROC(ZwResumeProcess)(hRemoteProcess);

			//�ͷ���Զ�̽��̷���Ŀ�ִ�ж�дģ��
			//VirtualFreeEx(hRemoteProcess, lpCodeBase, stCodeSize, MEM_DECOMMIT | MEM_RELEASE);

			return bResult;
		}
	}
}