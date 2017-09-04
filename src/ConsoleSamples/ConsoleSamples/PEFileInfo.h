#include <stdio.h>
#include <windows.h>
#include <tchar.h>
namespace PPSHUAI{
	namespace PE{
		typedef struct tagICONDIRENTRY
		{
			BYTE bWidth;
			BYTE bHeight;
			BYTE bColorCount;
			BYTE bReserved;
			WORD wPlanes;
			WORD wBitCount;
			DWORD dwBytesInRes;
			DWORD dwImageOffset;
		}ICONDIRENTRY, *PICONDIRENTRY;

		typedef struct tagICONDIR
		{
			WORD idReserved;
			WORD idType;
			WORD idCount;
			//ICONDIRENTRY idEntries;
		}ICONDIR, *PICONDIR;

		typedef struct tagGRPICONDIRENTRY
		{
			BYTE bWidth;
			BYTE bHeight;
			BYTE bColorCount;
			BYTE bReserved;
			WORD wPlanes;
			WORD wBitCount;
			DWORD dwBytesInRes;
			WORD nID;
		}GRPICONDIRENTRY, *PGRPICONDIRENTRY;
		typedef struct tagGRPICONDIR
		{
			WORD idReserved;
			WORD idType;
			WORD idCount;
			GRPICONDIRENTRY idEntries;
		}GRPICONDIR, *PGRPICONDIR;

		//////////////////////////////////////////////
		//函数说明：修改EXE图标
		//
		//参    数：IconFile 图标文件 
		//              ExeFile 被修改的EXE文件
		//
		//返回值： 成功为True，否则False
		/////////////////////////////////////////////
		__inline static BOOL ChangeExeIcon(LPWSTR IconFile, LPWSTR ExeFile)
		{
			ICONDIR stID = { 0 };
			ICONDIRENTRY stIDE = { 0 };
			GRPICONDIR stGID = { 0 };
			HANDLE hFile = NULL;
			DWORD dwSize = 0;
			DWORD dwGSize = 0;
			DWORD dwReserved = 0;
			HANDLE hUpdate = NULL;
			PBYTE pIcon = NULL;
			PBYTE pGrpIcon = NULL;
			BOOL bResult = FALSE;
			hFile = CreateFile(IconFile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return bResult;
			}
			ZeroMemory(&stID, sizeof(ICONDIR));
			bResult = ReadFile(hFile, &stID, sizeof(ICONDIR), &dwReserved, NULL);
			ZeroMemory(&stIDE, sizeof(ICONDIRENTRY));
			bResult = ReadFile(hFile, &stIDE, sizeof(ICONDIRENTRY), &dwReserved, NULL);
			dwSize = stIDE.dwBytesInRes;
			pIcon = (PBYTE)malloc(dwSize);
			if (pIcon)
			{
				SetFilePointer(hFile, stIDE.dwImageOffset, NULL, FILE_BEGIN);
				bResult = ReadFile(hFile, (LPVOID)pIcon, dwSize, &dwReserved, NULL);
				if (bResult)
				{
					ZeroMemory(&stGID, sizeof(GRPICONDIR));
					stGID.idCount = stID.idCount;
					stGID.idReserved = 0;
					stGID.idType = 1;
					CopyMemory(&stGID.idEntries, &stIDE, 12);
					stGID.idEntries.nID = 0;
					dwGSize = sizeof(GRPICONDIR);
					pGrpIcon = (PBYTE)malloc(dwGSize);
					CopyMemory(pGrpIcon, &stGID, dwGSize);

					BeginUpdateResource(ExeFile, FALSE);
					bResult = UpdateResource(hUpdate, RT_GROUP_ICON, MAKEINTRESOURCE(1), 0, (LPVOID)pGrpIcon, dwGSize);
					bResult = UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(1), 0, (LPVOID)pIcon, dwSize);
					EndUpdateResource(hUpdate, FALSE);
				}

				free(pIcon);
				pIcon = NULL;	
			}
			
			::CloseHandle(hFile);
			hFile = NULL;

			return bResult;
		}
	}
}
