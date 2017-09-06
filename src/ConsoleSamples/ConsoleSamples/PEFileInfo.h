
#include <windows.h>
#include <tchar.h>

#include "MACROS.h"

namespace PPSHUAI{
	namespace PE{
#pragma pack( push )
#pragma pack( 2 )
		typedef struct tagIconDirEntry
		{
			BYTE        bWidth;          // Width, in pixels, of the image
			BYTE        bHeight;         // Height, in pixels, of the image
			BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
			BYTE        bReserved;       // Reserved ( must be 0)
			WORD        wPlanes;         // Color Planes
			WORD        wBitCount;       // Bits per pixel
			DWORD       dwBytesInRes;    // How many bytes in this resource?
			DWORD       dwImageOffset;   // Where in the file is this image?
		} ICONDIRENTRY, *LPICONDIRENTRY;

		typedef struct tagIconDirHeader
		{
			WORD        wReserved;   // Reserved (must be 0)
			WORD        wType;       // Resource Type (1 for icons)
			WORD        wCount;      // How many images?
		} ICONDIRHEADER, *LPICONDIRHEADER;

		typedef struct tagIconDir
		{
			WORD           wReserved;   // Reserved (must be 0)
			WORD           wType;       // Resource Type (1 for icons)
			WORD           wCount;      // How many images?
			ICONDIRENTRY   Entries[1]; // An entry for each image (idCount of 'em)
		} ICONDIR, *LPICONDIR;

		typedef struct tagIconImage
		{
			BITMAPINFOHEADER	BitmapInfoHeader; // DIB header
			RGBQUAD				Colors[1];   // Color table
			BYTE				bXOR[1];      // DIB bits for XOR mask
			BYTE				AND[1];      // DIB bits for AND mask
		} ICONIMAGE, *LPICONIMAGE;
#pragma pack( pop )

#pragma pack( push )
#pragma pack( 2 )
		typedef struct tagGroupIconDirEntry
		{
			BYTE   bWidth;               // Width, in pixels, of the image
			BYTE   bHeight;              // Height, in pixels, of the image
			BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
			BYTE   bReserved;            // Reserved
			WORD   wPlanes;              // Color Planes
			WORD   wBitCount;            // Bits per pixel
			DWORD  dwBytesInRes;         // how many bytes in this resource?
			WORD   wID;                  // the ID
		} GROUPICONDIRENTRY, *LPGROUPICONDIRENTRY;
#pragma pack( pop )

		// #pragmas are used here to insure that the structure's
		// packing in memory matches the packing of the EXE or DLL.
#pragma pack( push )
#pragma pack( 2 )
		typedef struct tagGroupIconDirHeader
		{
			WORD				wReserved;   // Reserved (must be 0)
			WORD				wType;       // Resource type (1 for icons)
			WORD				wCount;      // How many images?
		} GROUPICONDIRHEADER, *LPGROUPICONDIRHEADER;
#pragma pack( pop )

		// #pragmas are used here to insure that the structure's
		// packing in memory matches the packing of the EXE or DLL.
#pragma pack( push )
#pragma pack( 2 )
		typedef struct tagGroupIconDir
		{
			WORD				wReserved;   // Reserved (must be 0)
			WORD				wType;       // Resource type (1 for icons)
			WORD				wCount;      // How many images?
			GROUPICONDIRENTRY	Entries[1]; // The entries for each image
		} GROUPICONDIR, *LPGROUPICONDIR;
#pragma pack( pop )

		__inline static BOOL ReadIconFile(LPCTSTR lpIconFile)
		{
			HANDLE hFile = NULL;
			WORD wIndex = 0;
			ICONDIRHEADER idh = { 0 };
			ICONDIR * pIconDir = NULL;
			DWORD dwNumberOfBytesRead = 0;

			hFile = CreateFile(lpIconFile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				// We need an ICONDIR to hold the data
				//pIconDir = (ICONDIR *)malloc(sizeof(ICONDIR));
				// Read the Reserved word
				//ReadFile(hFile, &(pIconDir->idReserved), sizeof(WORD), &dwNumberOfBytesRead, NULL);
				// Read the Type word - make sure it is 1 for icons
				//ReadFile(hFile, &(pIconDir->idType), sizeof(WORD), &dwNumberOfBytesRead, NULL);
				// Read the count - how many images in this file?
				//ReadFile(hFile, &(pIconDir->idCount), sizeof(WORD), &dwNumberOfBytesRead, NULL);
				
				ReadFile(hFile, &idh, sizeof(idh), &dwNumberOfBytesRead, NULL);
								
				// Reallocate IconDir so that idEntries has enough room for idCount elements
				pIconDir = (ICONDIR *)malloc(sizeof(idh) + (pIconDir->wCount * sizeof(ICONDIRENTRY)));
				// Read the ICONDIRENTRY elements
				ReadFile(hFile, pIconDir->Entries, pIconDir->wCount * sizeof(ICONDIRENTRY), &dwNumberOfBytesRead, NULL);
				
				// Loop through and read in each image
				for (wIndex = 0; wIndex < pIconDir->wCount; wIndex++)
				{
					// Allocate memory to hold the image
					ICONDIRENTRY * pIconImage = (ICONDIRENTRY *)malloc(pIconDir->Entries[wIndex].dwBytesInRes);
					// Seek to the location in the file that has the image
					SetFilePointer(hFile, pIconDir->Entries[wIndex].dwImageOffset, NULL, FILE_BEGIN);
					// Read the image data
					ReadFile(hFile, pIconImage, pIconDir->Entries[wIndex].dwBytesInRes, &dwNumberOfBytesRead, NULL);
					// Here, pIconImage is an ICONIMAGE structure. Party on it :)
					// Then, free the associated memory
					free(pIconImage);
					pIconImage = NULL;
				}
				// Clean up the ICONDIR memory
				free(pIconDir);
				pIconDir = NULL;

				CloseHandle(hFile);
				hFile = NULL;
			}
		}

		//////////////////////////////////////////////
		//函数说明：修改EXE图标
		//
		//参    数：IconFile 图标文件 
		//              ExeFile 被修改的EXE文件
		//
		//返回值： 成功为True，否则False
		/////////////////////////////////////////////
		__inline static BOOL ChangeExeIcon(LPCTSTR lpIconFile, LPCTSTR lpExeFile)
		{
			ICONDIRHEADER idh = { 0 };
			ICONDIR * pid = NULL;
			ICONDIRENTRY ide = { 0 };
			GROUPICONDIR gid = { 0 };
			DWORD dwNumberOfBytesRead = 0;
			HANDLE hFile = NULL;
			HANDLE hUpdate = NULL;
			PBYTE pIcon = NULL;
			PBYTE pGroupIcon = NULL;
			BOOL bResult = FALSE;
			
			hFile = CreateFile(lpIconFile, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				ZeroMemory(&idh, sizeof(idh));
				bResult = ReadFile(hFile, &idh, sizeof(idh), &dwNumberOfBytesRead, NULL);
				
				//读取第一个图标资源
				ZeroMemory(&ide, sizeof(ide));
				bResult = ReadFile(hFile, &ide, sizeof(ide), &dwNumberOfBytesRead, NULL);
				pIcon = (PBYTE)malloc(ide.dwBytesInRes);
				if (pIcon)
				{
					SetFilePointer(hFile, ide.dwImageOffset, NULL, FILE_BEGIN);
					bResult = ReadFile(hFile, (LPVOID)pIcon, ide.dwBytesInRes, &dwNumberOfBytesRead, NULL);
					if (bResult)
					{
						ZeroMemory(&gid, sizeof(gid));
						gid.wCount = idh.wCount;
						gid.wReserved = 0;
						gid.wType = 1;
						CopyMemory(&gid.Entries, &ide, sizeof(*gid.Entries));
						(*gid.Entries).wID = 0;
						pGroupIcon = (PBYTE)malloc(sizeof(gid));
						if (pGroupIcon)
						{
							CopyMemory(pGroupIcon, &gid, sizeof(gid));

							hUpdate = BeginUpdateResource(lpExeFile, FALSE);
							if (hUpdate)
							{
								bResult = UpdateResource(hUpdate, RT_GROUP_ICON, MAKEINTRESOURCE(RES_ICON), 0, (LPVOID)pGroupIcon, sizeof(gid));
								bResult = UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(RES_ICON), 0, (LPVOID)pIcon, ide.dwBytesInRes);
								//bResult = UpdateResource(hUpdate, RT_GROUP_ICON, MAKEINTRESOURCE(RES_ICON), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPVOID)pGroupIcon, sizeof(gid));
								//bResult = UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(RES_ICON), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPVOID)pIcon, ide.dwBytesInRes);
								EndUpdateResource(hUpdate, FALSE);
							}

							free(pGroupIcon);
							pGroupIcon = NULL;
						}
					}

					free(pIcon);
					pIcon = NULL;
				}
				else
				{
					bResult = FALSE;
				}
			}

			::CloseHandle(hFile);
			hFile = NULL;

			return bResult;
		}
		
		// Declare callback functions.
		//    FUNCTION: EnumResLangProc(HANDLE, LPSTR, LPSTR, WORD, LONG)
		//
		//    PURPOSE:  Resource language callback
		__inline static BOOL EnumResLangProc(
			HMODULE hPEModule, // module handle
			LPCTSTR lpType,  // address of resource type
			LPCTSTR lpName,  // address of resource name
			WORD wLang,      // resource language
			LONG lParam)     // extra parameter, could be
			// used for error checking
		{
			BOOL bResult = FALSE;
			HRSRC hResInfo = NULL;
			tstring tsFilePath = PPSHUAI::FilePath::GetProgramPath() + _T("PE");
			std::string strData = ("");
			// Printf the resource language to the resource information file.
			_tprintf_p(_T("Language: %u\r\n"), (USHORT)wLang);

			if (!IS_INTRESOURCE(lpType))
			{
				tsFilePath.append(_T("\\")).append(PPSHUAI::STRING_FORMAT(_T("%d"), wLang)).append(_T("\\")).append(lpType);
			}
			else
			{
				std::map<SIZE_T, tstring>::iterator it;
				std::map<SIZE_T, tstring> stmap = {
					/*
					* Predefined Resource Types
					*/
					{ (SIZE_T)(USHORT)RT_CURSOR, _T("CURSOR") },
					{ (SIZE_T)(USHORT)RT_BITMAP, _T("BITMAP") },
					{ (SIZE_T)(USHORT)RT_ICON, _T("ICON") },
					{ (SIZE_T)(USHORT)RT_MENU, _T("MENU") },
					{ (SIZE_T)(USHORT)RT_DIALOG, _T("DIALOG") },
					{ (SIZE_T)(USHORT)RT_STRING, _T("STRING") },
					{ (SIZE_T)(USHORT)RT_FONTDIR, _T("FONTDIR") },
					{ (SIZE_T)(USHORT)RT_FONT, _T("FONT") },
					{ (SIZE_T)(USHORT)RT_ACCELERATOR, _T("ACCELERATOR") },
					{ (SIZE_T)(USHORT)RT_RCDATA, _T("RCDATA") },
					{ (SIZE_T)(USHORT)RT_MESSAGETABLE, _T("MESSAGETABLE") },

					{ (SIZE_T)(USHORT)RT_GROUP_CURSOR, _T("GROUP_CURSOR") },
					{ (SIZE_T)(USHORT)RT_GROUP_ICON, _T("GROUP_ICON") },
					{ (SIZE_T)(USHORT)RT_VERSION, _T("VERSION") },
					{ (SIZE_T)(USHORT)RT_DLGINCLUDE, _T("DLGINCLUDE") },
					{ (SIZE_T)(USHORT)RT_PLUGPLAY, _T("PLUGPLAY") },
					{ (SIZE_T)(USHORT)RT_VXD, _T("VXD") },
					{ (SIZE_T)(USHORT)RT_ANICURSOR, _T("ANICURSOR") },
					{ (SIZE_T)(USHORT)RT_ANIICON, _T("ANIICON") },
					{ (SIZE_T)(USHORT)RT_HTML, _T("HTML") },
					{ (SIZE_T)(USHORT)RT_MANIFEST, _T("MANIFEST") },
				};
				tsFilePath.append(_T("\\")).append(PPSHUAI::STRING_FORMAT(_T("%d"), wLang)).append(_T("\\"));
				it = stmap.find((SIZE_T)(USHORT)lpType);
				if (it != stmap.end())
				{
					tsFilePath.append(it->second);
				}
				else
				{
					tsFilePath.append(PPSHUAI::STRING_FORMAT(_T("%u"), lpType));
				}
			}

			if (!PPSHUAI::FilePath::IsFileExistEx(tsFilePath.c_str()))
			{
				PPSHUAI::FilePath::CreateCascadeDirectory(tsFilePath.c_str());
			}

			if (!IS_INTRESOURCE(lpName))
			{
				tsFilePath.append(_T("\\")).append(lpName);
			}
			else
			{
				tsFilePath.append(_T("\\")).append(PPSHUAI::STRING_FORMAT(_T("%d"), (USHORT)lpName));
			}

			hResInfo = FindResourceEx(hPEModule, lpType, lpName, wLang);
			if (hResInfo)
			{
				// Read the resource handle and size.
				_tprintf_p(_T("\thResInfo == %lx,  Size == %lu\r\n"),
					hResInfo,
					SizeofResource(hPEModule, hResInfo));

				HGLOBAL hGlobal = ::LoadResource(hPEModule, hResInfo);
				LPVOID  lpData = ::LockResource(hGlobal);
				if (lpData)
				{
					strData.assign((LPSTR)lpData, SizeofResource(hPEModule, hResInfo));

					PPSHUAI::String::file_writer(strData, PPSHUAI::Convert::TToA(tsFilePath));

					::FreeResource(lpData);
					lpData = NULL;
				}
			}

			return TRUE;
		}

		//    FUNCTION: EnumResNameProc(HANDLE, LPSTR, LPSTR, LONG)
		//
		//    PURPOSE:  Resource name callback
		__inline static BOOL EnumResNameProc(
			HMODULE hPEModule,  // module handle
			LPCTSTR lpType,   // address of resource type
			LPTSTR lpName,    // address of resource name
			LONG lParam)      // extra parameter, could be
			// used for error checking
		{
			BOOL bResult = FALSE;

			// Write the resource name to a resource information file.
			// The name may be a string or an unsigned decimal
			// integer, so test before printing.
			if (!IS_INTRESOURCE(lpName))
			{
				//_tprintf_p(_T("Name: %s\r\n"), lpName);
			}
			else
			{
				//_tprintf_p(_T("Name: %u\r\n"), (USHORT)lpName);
			}

			// Find the languages of all resources of type
			// lpType and name lpName.
			EnumResourceLanguages(hPEModule,
				lpType,
				lpName,
				(ENUMRESLANGPROC)EnumResLangProc,
				0);

			return TRUE;
		}

		//    FUNCTION: EnumResTypeProc(HANDLE, LPSTR, LONG)
		//
		//    PURPOSE:  Resource type callback
		__inline static BOOL EnumResTypeProc(
			HMODULE hPEModule,  // module handle
			LPTSTR lpType,    // address of resource type
			LONG lParam)      // extra parameter, could be
			// used for error checking
		{
			BOOL bResult = FALSE;

			// Write the resource type to a resource information file.
			// The type may be a string or an unsigned decimal
			// integer, so test before printing.
			if (!IS_INTRESOURCE(lpType))
			{
				//_tprintf_p(_T("Type: %s\r\n"), lpType);
			}
			else
			{
				//_tprintf_p(_T("Type: %u\r\n"), (USHORT)lpType);
			}

			// Find the names of all resources of type lpType.
			EnumResourceNames(hPEModule,
				lpType,
				(ENUMRESNAMEPROC)EnumResNameProc,
				0);

			return TRUE;
		}

		__inline static BOOL EnumPEResources(LPCTSTR lpPEFileName)
		{
			HMODULE hPEModule = LoadLibraryEx(lpPEFileName, NULL, LOAD_LIBRARY_AS_DATAFILE);
			if (hPEModule)
			{
				EnumResourceTypes(hPEModule,          // module handle
					(ENUMRESTYPEPROC)EnumResTypeProc, // callback function
					0);                               // extra parameter

				FreeLibrary(hPEModule);
				hPEModule = NULL;
			}
			return TRUE;
		}

	}
}
