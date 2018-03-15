
// Decrypting_a_File.cpp : Defines the entry point for the console 
// application.
//

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#include <conio.h>

// Link with the Advapi32.lib file.
#pragma comment (lib, "advapi32")

#define KEYLENGTH  0x00800000
#define ENCRYPT_ALGORITHM CALG_RC4 
#define ENCRYPT_BLOCK_SIZE 8 
namespace PPSHUAI
{
	namespace CRYPTO
	{
		class CCryptClass
		{
		public:
			CCryptClass():m_pbPrivateKeyData(NULL),
				m_dwPrivateKeySize(0),
				m_pbPublicKeyData(NULL),
				m_dwPublicKeySize(0),
				m_hCryptKey(NULL),
				m_hCryptProv(NULL),
				m_hCryptSessionKey(NULL),
				m_pbBlobData(NULL),
				m_dwDataSize(NULL)
			{
			}
			~CCryptClass()
			{
			}

			__inline DWORD crypt_init_private_key(BYTE * pbData, DWORD dwSize)
			{
				DWORD dwResult = (-1L);
				
				if (!m_pbPrivateKeyData)
				{
					set_private_key(pbData, dwSize);
				}
				
				dwResult = (0L);
				
				return dwResult;
			}
			__inline DWORD crypt_init_private_key()
			{
				DWORD dwResult = (-1L);
				const BYTE PrivateKeyWithExponentOfOne[] =
				{
					0x07, 0x02, 0x00, 0x00, 0x00, 0xA4, 0x00, 0x00,
					0x52, 0x53, 0x41, 0x32, 0x00, 0x02, 0x00, 0x00,
					0x01, 0x00, 0x00, 0x00, 0xAB, 0xEF, 0xFA, 0xC6,
					0x7D, 0xE8, 0xDE, 0xFB, 0x68, 0x38, 0x09, 0x92,
					0xD9, 0x42, 0x7E, 0x6B, 0x89, 0x9E, 0x21, 0xD7,
					0x52, 0x1C, 0x99, 0x3C, 0x17, 0x48, 0x4E, 0x3A,
					0x44, 0x02, 0xF2, 0xFA, 0x74, 0x57, 0xDA, 0xE4,
					0xD3, 0xC0, 0x35, 0x67, 0xFA, 0x6E, 0xDF, 0x78,
					0x4C, 0x75, 0x35, 0x1C, 0xA0, 0x74, 0x49, 0xE3,
					0x20, 0x13, 0x71, 0x35, 0x65, 0xDF, 0x12, 0x20,
					0xF5, 0xF5, 0xF5, 0xC1, 0xED, 0x5C, 0x91, 0x36,
					0x75, 0xB0, 0xA9, 0x9C, 0x04, 0xDB, 0x0C, 0x8C,
					0xBF, 0x99, 0x75, 0x13, 0x7E, 0x87, 0x80, 0x4B,
					0x71, 0x94, 0xB8, 0x00, 0xA0, 0x7D, 0xB7, 0x53,
					0xDD, 0x20, 0x63, 0xEE, 0xF7, 0x83, 0x41, 0xFE,
					0x16, 0xA7, 0x6E, 0xDF, 0x21, 0x7D, 0x76, 0xC0,
					0x85, 0xD5, 0x65, 0x7F, 0x00, 0x23, 0x57, 0x45,
					0x52, 0x02, 0x9D, 0xEA, 0x69, 0xAC, 0x1F, 0xFD,
					0x3F, 0x8C, 0x4A, 0xD0,

					0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

					0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

					0x64, 0xD5, 0xAA, 0xB1,
					0xA6, 0x03, 0x18, 0x92, 0x03, 0xAA, 0x31, 0x2E,
					0x48, 0x4B, 0x65, 0x20, 0x99, 0xCD, 0xC6, 0x0C,
					0x15, 0x0C, 0xBF, 0x3E, 0xFF, 0x78, 0x95, 0x67,
					0xB1, 0x74, 0x5B, 0x60,

					0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				};

				return crypt_init_private_key((BYTE *)PrivateKeyWithExponentOfOne, (DWORD)sizeof(PrivateKeyWithExponentOfOne));
			}

			__inline DWORD crypt_init_key(BYTE * pbBlobData, DWORD dwDataSize)
			{
				BOOL bResult = FALSE;
				DWORD dwResult = (-1L);
				std::string strData = ("");
				
				bResult = CryptAcquireContext(&m_hCryptProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, 0);
				dwResult = GetLastError();
				if (!bResult)
				{
					if (dwResult == NTE_BAD_KEYSET)
					{
						bResult = CryptAcquireContext(&m_hCryptProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_NEWKEYSET);
						dwResult = GetLastError();
						if (!bResult)
						{
							//MessageBox("Error [0x%x]: CryptAcquireContext() failed.", "Information", MB_OK);
							goto __LEAVE_CLEAN__;
						}
					}
					else
					{
						goto __LEAVE_CLEAN__;
					}
				}

				if (pbBlobData)
				{
					bResult = CryptImportKey(m_hCryptProv, pbBlobData, dwDataSize, 0, 0, &m_hCryptSessionKey);
					dwResult = GetLastError();
					if (!bResult)
					{
						//MessageBox("Error [0x%x]: CryptImportKey() failed.", "Information", MB_OK);
						goto __LEAVE_CLEAN__;
					}
				}
				else
				{
					if (!m_pbPrivateKeyData)
					{
						dwResult = crypt_init_private_key();
					}
					
					bResult = CryptImportKey(m_hCryptProv, m_pbPrivateKeyData, m_dwPrivateKeySize, 0, 0, &m_hCryptKey);
					dwResult = GetLastError();
					if (!bResult)
					{
						//MessageBox("Error CryptImportKey() failed.", "Information", MB_OK);
						goto __LEAVE_CLEAN__;
					}
					bResult = CryptGenKey(m_hCryptProv, CALG_RC4, CRYPT_EXPORTABLE, &m_hCryptSessionKey);
					dwResult = GetLastError();
					if (!bResult)
					{
						//MessageBox("Error CryptGenKey() failed.", "Information", MB_OK);
						goto __LEAVE_CLEAN__;
					}
					bResult = CryptExportKey(m_hCryptSessionKey, NULL, PUBLICKEYBLOB, 0, NULL, &m_dwPublicKeySize);
					dwResult = GetLastError();
					if (!bResult)
					{
						m_pbPublicKeyData = (BYTE *)realloc(m_pbPublicKeyData, m_dwPublicKeySize);
						if (m_pbPublicKeyData)
						{
							bResult = CryptExportKey(m_hCryptSessionKey, NULL, PUBLICKEYBLOB, 0, m_pbPublicKeyData, &m_dwPublicKeySize);
							dwResult = GetLastError();
						}
					}
				}

			__LEAVE_CLEAN__:

				return dwResult;
			}

			__inline DWORD crypt_encode_data(BYTE * pbBlobData, DWORD dwDataSize)
			{
				DWORD dwResult = (-1L);
				if (pbBlobData && dwDataSize)
				{
					m_dwDataSize = dwDataSize;
					m_pbBlobData = (BYTE *)realloc(m_pbBlobData, m_dwDataSize);
					if (m_pbBlobData)
					{
						memset(m_pbBlobData, 0, m_dwDataSize);
						memcpy(m_pbBlobData, pbBlobData, dwDataSize);
						CryptEncrypt(m_hCryptSessionKey, 0, TRUE, 0, m_pbBlobData, &m_dwDataSize, m_dwDataSize);
						dwResult = GetLastError();
					}
				}

				return dwResult;
			}

			__inline DWORD crypt_decode_data(BYTE * pbBlobData, DWORD dwDataSize)
			{
				DWORD dwResult = (-1L);
				if (pbBlobData && dwDataSize)
				{
					m_dwDataSize = dwDataSize;
					m_pbBlobData = (BYTE *)realloc(m_pbBlobData, m_dwDataSize);
					if (m_pbBlobData)
					{
						memset(m_pbBlobData, 0, m_dwDataSize);
						memcpy(m_pbBlobData, pbBlobData, dwDataSize);
						CryptDecrypt(m_hCryptSessionKey, 0, TRUE, 0, m_pbBlobData, &m_dwDataSize);
						dwResult = GetLastError();
					}
				}

				return dwResult;
			}

			__inline DWORD crypt_exit_key()
			{
				DWORD dwResult = (-1L);

				if (m_pbBlobData)
				{
					free(m_pbBlobData);
					m_pbBlobData = NULL;
					m_dwDataSize = 0;
				}
				if (m_pbPrivateKeyData)
				{
					free(m_pbPrivateKeyData);
					m_pbPrivateKeyData = NULL;
					m_dwPrivateKeySize = 0;
				}
				if (m_pbPublicKeyData)
				{
					free(m_pbPublicKeyData);
					m_pbPublicKeyData = NULL;
					m_dwPublicKeySize = 0;
				}
				if (m_hCryptKey)
				{
					CryptDestroyKey(m_hCryptKey);
					dwResult = GetLastError();
					m_hCryptKey = NULL;
				}
				if (m_hCryptSessionKey)
				{
					CryptDestroyKey(m_hCryptSessionKey);
					dwResult = GetLastError();
					m_hCryptSessionKey = NULL;
				}
				if (m_hCryptProv)
				{
					CryptReleaseContext(m_hCryptProv, 0);
					dwResult = GetLastError();
					m_hCryptSessionKey = NULL;
				}

				return dwResult;
			}

			__inline BYTE * data()
			{
				return m_pbBlobData;
			}
			__inline DWORD size()
			{
				return m_dwDataSize;
			}

			__inline BYTE * public_key()
			{
				return m_pbPublicKeyData;
			}
			__inline DWORD public_key_size()
			{
				return m_dwPublicKeySize;
			}
			__inline BYTE * private_key()
			{
				return m_pbPrivateKeyData;
			}
			__inline DWORD private_key_size()
			{
				return m_dwPrivateKeySize;
			}

			__inline void set_private_key(BYTE * pbPrivateKeyData, DWORD dwPrivateKeyDataSize)
			{
				m_dwPrivateKeySize = dwPrivateKeyDataSize;
				m_pbPrivateKeyData = (BYTE *)realloc(m_pbPrivateKeyData, m_dwPrivateKeySize);
				if (m_pbPrivateKeyData)
				{
					memcpy(m_pbPrivateKeyData, pbPrivateKeyData, m_dwPrivateKeySize);
				}
			}

			private:
				BYTE * m_pbBlobData;
				DWORD m_dwDataSize;

				BYTE * m_pbPrivateKeyData;
				DWORD m_dwPrivateKeySize;

				BYTE * m_pbPublicKeyData;
				DWORD m_dwPublicKeySize;

				HCRYPTKEY m_hCryptKey;
				HCRYPTPROV m_hCryptProv;
				HCRYPTKEY m_hCryptSessionKey;

			public:
#define PUBLICK_KEY_NAME	"public.key"
				__inline static DWORD encrypt(BYTE ** pbDstData, DWORD * pnDstSize, BYTE * pbSrcData, DWORD dwSrcSize)
				{
					CCryptClass cc;
					DWORD dwResult = (-1L);

					if (!(dwResult = cc.crypt_init_key(0, 0)))
					{
						if (!(dwResult = cc.crypt_encode_data(pbSrcData, dwSrcSize)))
						{
							(*pnDstSize) = cc.size();
							(*pbDstData) = (BYTE *)malloc((*pnDstSize));
							if ((*pbDstData))
							{
								memcpy((*pbDstData), cc.data(), (*pnDstSize));
								String::file_writer(std::string((char *)cc.public_key(), cc.public_key_size()), PUBLICK_KEY_NAME);
							}
							dwResult = cc.crypt_exit_key();
						}
					}

					return dwResult;
				}
				__inline static DWORD decrypt(BYTE ** pbDstData, DWORD * pnDstSize, BYTE * pbSrcData, DWORD dwSrcSize)
				{
					CCryptClass cc;
					DWORD dwResult = (-1L);
					std::string strPublicKey = "";
					String::file_reader(strPublicKey, PUBLICK_KEY_NAME);

					if (!(dwResult = cc.crypt_init_key((BYTE *)strPublicKey.c_str(), strPublicKey.size())))
					{
						if (!(dwResult = cc.crypt_decode_data(pbSrcData, dwSrcSize)))
						{
							(*pnDstSize) = cc.size();
							(*pbDstData) = (BYTE *)malloc((*pnDstSize));
							if ((*pbDstData))
							{
								memcpy((*pbDstData), cc.data(), (*pnDstSize));
							}
							dwResult = cc.crypt_exit_key();
						}
					}

					return dwResult;
				}
		};
	}
	
}
//-------------------------------------------------------------------
//  This example uses the function MyHandleError, a simple error
//  handling function, to print an error message to the  
//  standard error (stderr) file and exit the program. 
//  For most applications, replace this function with one 
//  that does more extensive error reporting.

__inline static void MyHandleError(LPTSTR psz, int nErrorNumber)
{
	_ftprintf(stderr, TEXT("An error occurred in the program. \n"));
	_ftprintf(stderr, TEXT("%s\n"), psz);
	_ftprintf(stderr, TEXT("Error number %x.\n"), nErrorNumber);
}

//-------------------------------------------------------------------
// Code for the function MyDecryptFile called by main.
//-------------------------------------------------------------------
// Parameters passed are:
//  pszSource, the name of the input file, an encrypted file.
//  pszDestination, the name of the output, a plaintext file to be 
//   created.
//  pszPassword, either NULL if a password is not to be used or the 
//   string that is the password.
__inline static bool MyDecryptFile(
	LPTSTR pszSourceFile,
	LPTSTR pszDestinationFile,
	LPTSTR pszPassword)
{
	//---------------------------------------------------------------
	// Declare and initialize local variables.
	bool fReturn = false;
	HANDLE hSourceFile = INVALID_HANDLE_VALUE;
	HANDLE hDestinationFile = INVALID_HANDLE_VALUE;
	HCRYPTKEY hKey = NULL;
	HCRYPTHASH hHash = NULL;

	HCRYPTPROV hCryptProv = NULL;

	DWORD dwCount;
	PBYTE pbBuffer = NULL;
	DWORD dwBlockLen;
	DWORD dwBufferLen;

	//---------------------------------------------------------------
	// Open the source file. 
	hSourceFile = CreateFile(
		pszSourceFile,
		FILE_READ_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE != hSourceFile)
	{
		_tprintf(
			TEXT("The source encrypted file, %s, is open. \n"),
			pszSourceFile);
	}
	else
	{
		MyHandleError(
			TEXT("Error opening source plaintext file!\n"),
			GetLastError());
		goto Exit_MyDecryptFile;
	}

	//---------------------------------------------------------------
	// Open the destination file. 
	hDestinationFile = CreateFile(
		pszDestinationFile,
		FILE_WRITE_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE != hDestinationFile)
	{
		_tprintf(
			TEXT("The destination file, %s, is open. \n"),
			pszDestinationFile);
	}
	else
	{
		MyHandleError(
			TEXT("Error opening destination file!\n"),
			GetLastError());
		goto Exit_MyDecryptFile;
	}

	//---------------------------------------------------------------
	// Get the handle to the default provider. 
	if (CryptAcquireContext(
		&hCryptProv,
		NULL,
		MS_ENHANCED_PROV,
		PROV_RSA_FULL,
		0))
	{
		_tprintf(
			TEXT("A cryptographic provider has been acquired. \n"));
	}
	else
	{
		MyHandleError(
			TEXT("Error during CryptAcquireContext!\n"),
			GetLastError());
		goto Exit_MyDecryptFile;
	}

	//---------------------------------------------------------------
	// Create the session key.
	if (!pszPassword || !pszPassword[0])
	{
		//-----------------------------------------------------------
		// Decrypt the file with the saved session key. 

		DWORD dwKeyBlobLen;
		PBYTE pbKeyBlob = NULL;

		// Read the key BLOB length from the source file. 
		if (!ReadFile(
			hSourceFile,
			&dwKeyBlobLen,
			sizeof(DWORD),
			&dwCount,
			NULL))
		{
			MyHandleError(
				TEXT("Error reading key BLOB length!\n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}

		// Allocate a buffer for the key BLOB.
		if (!(pbKeyBlob = (PBYTE)malloc(dwKeyBlobLen)))
		{
			MyHandleError(
				TEXT("Memory allocation error.\n"),
				E_OUTOFMEMORY);
		}

		//-----------------------------------------------------------
		// Read the key BLOB from the source file. 
		if (!ReadFile(
			hSourceFile,
			pbKeyBlob,
			dwKeyBlobLen,
			&dwCount,
			NULL))
		{
			MyHandleError(
				TEXT("Error reading key BLOB length!\n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// Import the key BLOB into the CSP. 
		if (!CryptImportKey(
			hCryptProv,
			pbKeyBlob,
			dwKeyBlobLen,
			0,
			0,
			&hKey))
		{
			MyHandleError(
				TEXT("Error during CryptImportKey!/n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}

		if (pbKeyBlob)
		{
			free(pbKeyBlob);
		}
	}
	else
	{
		//-----------------------------------------------------------
		// Decrypt the file with a session key derived from a 
		// password. 

		//-----------------------------------------------------------
		// Create a hash object. 
		if (!CryptCreateHash(
			hCryptProv,
			CALG_MD5,
			0,
			0,
			&hHash))
		{
			MyHandleError(
				TEXT("Error during CryptCreateHash!\n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// Hash in the password data. 
		if (!CryptHashData(
			hHash,
			(BYTE *)pszPassword,
			lstrlen(pszPassword) + 1,
			0))
		{
			MyHandleError(
				TEXT("Error during CryptHashData!\n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// Derive a session key from the hash object. 
		if (!CryptDeriveKey(
			hCryptProv,
			ENCRYPT_ALGORITHM,
			hHash,
			KEYLENGTH,
			&hKey))
		{
			MyHandleError(
				TEXT("Error during CryptDeriveKey!\n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}
	}

	//---------------------------------------------------------------
	// The decryption key is now available, either having been 
	// imported from a BLOB read in from the source file or having 
	// been created by using the password. This point in the program 
	// is not reached if the decryption key is not available.

	//---------------------------------------------------------------
	// Determine the number of bytes to decrypt at a time. 
	// This must be a multiple of ENCRYPT_BLOCK_SIZE. 

	dwBlockLen = 2 * 1024 * 1024 - 2 * 1024 * 1024 % ENCRYPT_BLOCK_SIZE;
	dwBufferLen = dwBlockLen;

	//---------------------------------------------------------------
	// Allocate memory for the file read buffer. 
	if (!(pbBuffer = (PBYTE)malloc(dwBufferLen)))
	{
		MyHandleError(TEXT("Out of memory!\n"), E_OUTOFMEMORY);
		goto Exit_MyDecryptFile;
	}

	//---------------------------------------------------------------
	// Decrypt the source file, and write to the destination file. 
	bool fEOF = false;
	do
	{
		//-----------------------------------------------------------
		// Read up to dwBlockLen bytes from the source file. 
		if (!ReadFile(
			hSourceFile,
			pbBuffer,
			dwBlockLen,
			&dwCount,
			NULL))
		{
			MyHandleError(
				TEXT("Error reading from source file!\n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}

		if (dwCount <= dwBlockLen)
		{
			fEOF = TRUE;
		}

		//-----------------------------------------------------------
		// Decrypt the block of data. 
		if (!CryptDecrypt(
			hKey,
			0,
			fEOF,
			0,
			pbBuffer,
			&dwCount))
		{
			MyHandleError(
				TEXT("Error during CryptDecrypt!\n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// Write the decrypted data to the destination file. 
		if (!WriteFile(
			hDestinationFile,
			pbBuffer,
			dwCount,
			&dwCount,
			NULL))
		{
			MyHandleError(
				TEXT("Error writing ciphertext.\n"),
				GetLastError());
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// End the do loop when the last block of the source file 
		// has been read, encrypted, and written to the destination 
		// file.
	} while (!fEOF);

	fReturn = true;

Exit_MyDecryptFile:

	//---------------------------------------------------------------
	// Free the file read buffer.
	if (pbBuffer)
	{
		free(pbBuffer);
	}

	//---------------------------------------------------------------
	// Close files.
	if (hSourceFile)
	{
		CloseHandle(hSourceFile);
	}

	if (hDestinationFile)
	{
		CloseHandle(hDestinationFile);
	}

	//-----------------------------------------------------------
	// Release the hash object. 
	if (hHash)
	{
		if (!(CryptDestroyHash(hHash)))
		{
			MyHandleError(
				TEXT("Error during CryptDestroyHash.\n"),
				GetLastError());
		}

		hHash = NULL;
	}

	//---------------------------------------------------------------
	// Release the session key. 
	if (hKey)
	{
		if (!(CryptDestroyKey(hKey)))
		{
			MyHandleError(
				TEXT("Error during CryptDestroyKey!\n"),
				GetLastError());
		}
	}

	//---------------------------------------------------------------
	// Release the provider handle. 
	if (hCryptProv)
	{
		if (!(CryptReleaseContext(hCryptProv, 0)))
		{
			MyHandleError(
				TEXT("Error during CryptReleaseContext!\n"),
				GetLastError());
		}
	}

	return fReturn;
}

__inline static 
int crypt_tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		_tprintf(TEXT("Usage: <example.exe> <source file> ")
			TEXT("<destination file> | <password>\n"));
		_tprintf(TEXT("<password> is optional.\n"));
		_tprintf(TEXT("Press any key to exit."));
		_gettch();
		return 1;
	}

	LPTSTR pszSource = argv[1];
	LPTSTR pszDestination = argv[2];
	LPTSTR pszPassword = NULL;

	if (argc >= 4)
	{
		pszPassword = argv[3];
	}
	pszPassword = _T("\x83\x63\xa0\xec\x47\xc7\xf6\xb3\x3e\x0d\xf6\x17\xa7\x2c\x6c\x00");

	//---------------------------------------------------------------
	// Call EncryptFile to do the actual encryption.
	if (MyDecryptFile(pszSource, pszDestination, pszPassword))
	{
		_tprintf(
			TEXT("Encryption of the file %s was successful. \n"),
			pszSource);
		_tprintf(
			TEXT("The encrypted data is in file %s.\n"),
			pszDestination);
	}
	else
	{
		MyHandleError(
			TEXT("Error encrypting file!\n"),
			GetLastError());
	}

	return 0;
}