
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

			//申请代码的内存区
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

				//申请代码的内存区
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

		// 内存数据映射初始化
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

		// 内存搜索数据
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

		//相对内存映射地址=>>绝对进程内存地址
		__inline static
			SIZE_T MapMemoryRelativeTransferAbsolute(std::map<SIZE_T, SIZE_T> * psssmap, std::map<SIZE_T, SIZE_T> * prssmap, std::map<SIZE_T, SIZE_T> * prvassmap, std::map<SIZE_T, MEMORY_BASIC_INFORMATION> * psmbimap)
		{
			std::map<SIZE_T, SIZE_T>::iterator itEnd;
			std::map<SIZE_T, SIZE_T>::iterator itIdx;
			std::map<SIZE_T, SIZE_T>::iterator itRSEnd;
			std::map<SIZE_T, SIZE_T>::iterator itRSIdx;
						
			// 相对内存映射地址列表
			itEnd = prssmap->end();
			itIdx = prssmap->begin();
			for (; itIdx != itEnd; itIdx++)
			{
				// 辅助内存索引映射地址列表
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

		// 内存映射资源释放
		__inline static 
			void MapMemoryExitialize(HANDLE * phFileMapping, LPVOID * pbData)
		{
			PPSHUAI::FilePath::MapRelease(phFileMapping, pbData);
		}
	}
}