
namespace PPSHUAI{
	namespace SystemKernel{
		
		__inline static
			void GetProcessMemoryPageList(std::map<SIZE_T, MEMORY_BASIC_INFORMATION> &mbimap, HANDLE hProcess)
		{
			DWORD dwPID = 0;
			SIZE_T stResultSize = 0;
			LPVOID lpMemoryAddress = NULL;
			LPVOID lpMemoryAddressMIN = NULL;
			LPVOID lpMemoryAddressMAX = NULL;
			SYSTEM_INFO siSystemInfo = { 0 };
			MEMORY_BASIC_INFORMATION mbiMemoryBuffer = { 0 };

			// Get maximum address range from system info.
			GetNativeSystemInformation(siSystemInfo);
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

				mbimap.insert(std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::value_type((SIZE_T)mbiMemoryBuffer.BaseAddress, mbiMemoryBuffer));
				// increment lpMemoryAddress to next region of memory.
				lpMemoryAddress = (LPVOID)((SIZE_T)mbiMemoryBuffer.BaseAddress + (SIZE_T)mbiMemoryBuffer.RegionSize);
			}
		}

		__inline static
			void SearchProcessMemoryPageList(std::map<SIZE_T, SIZE_T> & ssmap, const VOID * pvData, SIZE_T stSize, HANDLE hProcess)
		{
			LPVOID lpData = NULL;
			//HANDLE hCurrentProcess = NULL;
			SIZE_T stNumberOfBytesWritten = 0;

			//hCurrentProcess = ::OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());

			std::map<SIZE_T, MEMORY_BASIC_INFORMATION> mbimap;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itEnd;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itIdx;
			GetProcessMemoryPageList(mbimap, hProcess);

			itEnd = mbimap.end();
			itIdx = mbimap.begin();

			//申请代码的内存区
			for (; itIdx != itEnd; itIdx++)
			{
				lpData = malloc(itIdx->second.RegionSize);
				//lpData = ::VirtualAllocEx(hCurrentProcess, NULL, itIdx->second.RegionSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				DWORD dwResult = GetLastError();
				if (lpData)
				{
					if (ReadProcessMemory(hProcess, itIdx->second.BaseAddress, lpData, itIdx->second.RegionSize, &stNumberOfBytesWritten))
					{
						for (SIZE_T stCount = 0; stCount < itIdx->second.RegionSize; stCount++)
						{
							if (!memcmp(pvData, (BYTE *)lpData + stCount, stSize))
							{
								ssmap.insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)itIdx->second.BaseAddress + stCount, (SIZE_T)itIdx->second.BaseAddress));
							}
						}
					}

					free(lpData);
					//::VirtualFreeEx(hCurrentProcess, lpData, itIdx->second.RegionSize, MEM_DECOMMIT | MEM_RELEASE);
					lpData = NULL;
				}
			}
		}

		__inline static
			void SearchProcessMemoryPageList(std::map<SIZE_T, SIZE_T> & ssmap, const VOID * pvData, SIZE_T stSize, const _TCHAR * ptProcessName)
		{
			HANDLE hProcess = NULL;

			hProcess = InitProcessHandle(ptProcessName);

			if (hProcess)
			{
				SearchProcessMemoryPageList(ssmap, pvData, stSize, hProcess);

				ExitProcessHandle(&hProcess);
			}
		}

		__inline static
			void SearchProcessMemoryPageListEx(std::map<SIZE_T, SIZE_T> & ssmap, const VOID * pvData, SIZE_T stSize, HANDLE hProcess)
		{
			DWORD dwPID = 0;
			LPVOID lpData = NULL;
			SIZE_T stNumberOfBytesWritten = 0;
			std::string strMemoryBuffer = ("");

			std::map<SIZE_T, MEMORY_BASIC_INFORMATION> mbimap;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itEnd;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itIdx;
			GetProcessMemoryPageList(mbimap, hProcess);

			itEnd = mbimap.end();
			itIdx = mbimap.begin();

			//申请代码的内存区
			for (; itIdx != itEnd; itIdx++)
			{
				if (itIdx->second.RegionSize < MINLONG / 2)
				{
					strMemoryBuffer.resize(itIdx->second.RegionSize);
					if (strMemoryBuffer.size() > 0 && ReadProcessMemory(hProcess, itIdx->second.BaseAddress, (LPVOID)strMemoryBuffer.c_str(), itIdx->second.RegionSize, &stNumberOfBytesWritten))
					{
						size_t stPos = 0;
						while (1)
						{
							stPos = strMemoryBuffer.find((const char *)pvData, stPos);
							if (stPos == std::string::npos)
							{
								break;
							}
							else
							{
								ssmap.insert(std::map<SIZE_T, SIZE_T>::value_type((SIZE_T)itIdx->second.BaseAddress + stPos, (SIZE_T)itIdx->second.BaseAddress));
								stPos += stSize;
							}
						}
					}
				}
			}
		}

		__inline static
			void SearchProcessMemoryPageListEx(std::map<SIZE_T, SIZE_T> & ssmap, const VOID * pvData, SIZE_T stSize, const _TCHAR * ptProcessName)
		{
			HANDLE hProcess = NULL;

			hProcess = InitProcessHandle(ptProcessName);

			if (hProcess)
			{
				SearchProcessMemoryPageListEx(ssmap, pvData, stSize, hProcess);

				ExitProcessHandle(&hProcess);
			}
		}
		__inline static
			void ShowProcessMemoryPages(HANDLE hProcess)
		{
			SIZE_T stResultSize = 0;

			std::map<SIZE_T, MEMORY_BASIC_INFORMATION> mbimap;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itEnd;
			std::map<SIZE_T, MEMORY_BASIC_INFORMATION>::iterator itIdx;
			GetProcessMemoryPageList(mbimap, hProcess);

			itEnd = mbimap.end();
			itIdx = mbimap.begin();

			for (; itIdx != itEnd; itIdx++)
			{
				if ((itIdx->second.AllocationBase != itIdx->second.BaseAddress)
					&& (itIdx->second.State != MEM_FREE))
				{
#if !defined(_WIN64) && !defined(WIN64)
					_tprintf(_T("  0x%08lX  0x%08lX  "),
#else
					_tprintf(_T("  0x%016llX  0x%016llX  "),
#endif
						itIdx->second.BaseAddress,
						itIdx->second.RegionSize);
				}
				else
				{
#if !defined(_WIN64) && !defined(WIN64)
					_tprintf(_T("  0x%08lX  0x%08lX  "),
#else
					_tprintf(_T("  0x%016llX  0x%016llX  "),
#endif
						itIdx->second.BaseAddress,
						itIdx->second.RegionSize);
				}
				_tprintf(_T("\t"));

				switch (itIdx->second.Type)
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

				switch (itIdx->second.AllocationProtect)
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
			void ShowProcessMemoryPages(const _TCHAR * ptProcessName)
		{
			HANDLE hProcess = NULL;

			hProcess = InitProcessHandle(ptProcessName);
			if (hProcess)
			{
				ShowProcessMemoryPages(hProcess);

				ExitProcessHandle(&hProcess);
			}
		}
	}
}