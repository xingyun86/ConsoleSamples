
#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus")
#include <map>

typedef std::map<unsigned long, unsigned long> WNDMAP;
typedef std::map<unsigned long, unsigned long>::iterator WNDMAPIT;

namespace PPSHUAI
{
namespace NearSideAutoHide{

#define NEAR_SIZE 1 //定义自动停靠有效距离
#define NEAR_SIDE 1 //窗体隐藏后在屏幕上保留的像素，以使鼠标可以触及

#define IDC_TIMER_NEARSIDEHIDE	0xFFFF
#define T_TIMEOUT_NEARSIDEHIDE	0xFF
	enum {
		ALIGN_NONE,          //不停靠
		ALIGN_TOP,          //停靠上边
		ALIGN_LEFT,          //停靠左边
		ALIGN_RIGHT          //停靠右边
	};
	static int g_nScreenX = 0;
	static int g_nScreenY = 0;
	static int g_nAlignType = ALIGN_NONE;   //全局变量，用于记录窗体停靠状态

	__inline static void InitScreenSize()
	{
		g_nScreenX = ::GetSystemMetrics(SM_CXSCREEN);
		g_nScreenY = ::GetSystemMetrics(SM_CYSCREEN);
	}
	//在窗体初始化是设定窗体状态，如果可以停靠，便停靠在边缘
	//我本想寻求其他方法来解决初始化，而不是为它专一寻求一个函数，
	//可是，窗体初始化时不发送WM_MOVING消息,我不得不重复类似任务.
	__inline static void NearSide(HWND hWnd, LPRECT lpRect)
	{
		LONG Width = lpRect->right - lpRect->left;
		LONG Height = lpRect->bottom - lpRect->top;
		BOOL bChange = 0;
		g_nAlignType = ALIGN_NONE;
		if (lpRect->left < NEAR_SIZE)
		{
			g_nAlignType = ALIGN_LEFT;
			if ((lpRect->left != 0) && lpRect->right != NEAR_SIDE)
			{
				lpRect->left = 0;
				lpRect->right = Width;
				bChange = FALSE;
			}
		}
		else if (lpRect->right > g_nScreenX - NEAR_SIZE)
		{
			g_nAlignType = ALIGN_RIGHT;
			if (lpRect->right != g_nScreenX && lpRect->left != g_nScreenX - NEAR_SIDE)
			{
				lpRect->right = g_nScreenX;
				lpRect->left = g_nScreenX - Width;
				bChange = FALSE;
			}
		}
		//调整上
		else if (lpRect->top < NEAR_SIZE)
		{
			g_nAlignType = ALIGN_TOP;
			if (lpRect->top != 0 && lpRect->bottom != NEAR_SIDE)
			{
				lpRect->top = 0;
				lpRect->bottom = Height;
				bChange = FALSE;
			}
		}
		if (bChange)
		{
			::MoveWindow(hWnd, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bChange);
		}
	}

	//窗体的显示隐藏由该函数完成,参数bHide决定显示还是隐藏.
	__inline static void AutoHideProc(HWND hWnd, LPRECT lpRect, BOOL bHide)
	{
		int nStep = 20;  //动画滚动窗体的步数,如果你觉得不够平滑,可以增大该值.
		int xStep = 0, xEnd = 0;
		int yStep = 0, yEnd = 0;
		LONG Width = lpRect->right - lpRect->left;
		LONG Height = lpRect->bottom - lpRect->top;

		//下边判断窗体该如何移动,由停靠方式决定
		switch (g_nAlignType)
		{
		case ALIGN_TOP:
		{
			//向上移藏
			xStep = 0;
			xEnd = lpRect->left;
			if (bHide)
			{
				yStep = -lpRect->bottom / nStep;
				yEnd = -Height + NEAR_SIDE;
			}
			else
			{
				yStep = -lpRect->top / nStep;
				yEnd = 0;
			}
			break;
		}
		case ALIGN_LEFT:
		{
			//向左移藏
			yStep = 0;
			yEnd = lpRect->top;
			if (bHide)
			{
				xStep = -lpRect->right / nStep;
				xEnd = -Width + NEAR_SIDE;
			}
			else
			{
				xStep = -lpRect->left / nStep;
				xEnd = 0;
			}
			break;
		}
		case ALIGN_RIGHT:
		{
			//向右移藏
			yStep = 0;
			yEnd = lpRect->top;
			if (bHide)
			{
				xStep = (g_nScreenX - lpRect->left) / nStep;
				xEnd = g_nScreenX - NEAR_SIDE;
			}
			else
			{
				xStep = (g_nScreenX - lpRect->right) / nStep;
				xEnd = g_nScreenX - Width;
			}
			break;
		}
		default:
			return;
		}
		//动画滚动窗体.
		for (int i = 0; i < nStep; i++)
		{
			lpRect->left += xStep;
			lpRect->top += yStep;
			::SetWindowPos(hWnd, NULL, lpRect->left, lpRect->top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
			::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			Sleep(3);
		}
		::SetWindowPos(hWnd, NULL, xEnd, yEnd, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
		if (!bHide) //如果窗体已被显示,设置定时器.监视鼠标.
		{
			::SetTimer(hWnd, WM_TIMER, WAIT_TIMEOUT, NULL);
		}
	}

	// WM_TIMER
	__inline static LRESULT OnTimer(HWND hWnd, UINT uTimerID)
	{
		LRESULT lResult = FALSE;
		switch (uTimerID)
		{
		case IDC_TIMER_NEARSIDEHIDE:
		{
			RECT rc = { 0 };
			POINT pt = { 0 };
			::GetCursorPos(&pt);
			::GetWindowRect(hWnd, &rc);

			if (!PtInRect(&rc, pt)) //若鼠标不在窗体内,隐藏窗体.
			{
				::KillTimer(hWnd, uTimerID);
				AutoHideProc(hWnd, &rc, TRUE);
			}
			lResult = TRUE;
		}
		break;
		default:
		{
			//no-op
		}
		break;
		}
		return lResult;
	}
	// WM_NCMOUSEMOVE
	__inline static LRESULT OnNcMouseMove(HWND hWnd)
	{
		RECT rc = { 0 };
		::GetWindowRect(hWnd, &rc);
		if (rc.left < 0 || rc.top < 0 || rc.right > g_nScreenX) //未显示
		{
			AutoHideProc(hWnd, &rc, FALSE);
		}
		else
		{
			::SetTimer(hWnd, IDC_TIMER_NEARSIDEHIDE, T_TIMEOUT_NEARSIDEHIDE, NULL);
		}
		return 0;
	}
	// WM_MOUSEMOVE
	__inline static LRESULT OnMouseMove(HWND hWnd)
	{
		RECT rc = { 0 };
		::GetWindowRect(hWnd, &rc);
		if (rc.left < 0 || rc.top < 0 || rc.right > g_nScreenX) //未显示
		{
			AutoHideProc(hWnd, &rc, FALSE);
		}
		else
		{
			::SetTimer(hWnd, IDC_TIMER_NEARSIDEHIDE, T_TIMEOUT_NEARSIDEHIDE, NULL);
		}
		return 0;
	}
	// WM_ENTERSIZEMOVE
	__inline static LRESULT OnEnterSizeMove(HWND hWnd)
	{
		::KillTimer(hWnd, IDC_TIMER_NEARSIDEHIDE);
		return 0;
	}
	// WM_EXITSIZEMOVE
	__inline static LRESULT OnExitSizeMove(HWND hWnd)
	{
		::SetTimer(hWnd, IDC_TIMER_NEARSIDEHIDE, T_TIMEOUT_NEARSIDEHIDE, NULL);
		return 0;
	}
	/////////////////////////////////////////////////////////////
	// WM_MOVING
	//下面代码处理窗体消息WM_MOVING，lParam是参数RECT指针
	__inline static LRESULT OnMoving(HWND hWnd, LPARAM lParam)
	{
		POINT pt = { 0 };
		LPRECT lpRect = (LPRECT)lParam;
		LONG Width = lpRect->right - lpRect->left;
		LONG Height = lpRect->bottom - lpRect->top;

		//未靠边界由pRect测试
		if (g_nAlignType == ALIGN_NONE)
		{
			if (lpRect->left < NEAR_SIZE) //在左边有效距离内
			{
				g_nAlignType = ALIGN_LEFT;
				lpRect->left = 0;
				lpRect->right = Width;
			}
			if (lpRect->right + NEAR_SIZE > g_nScreenX) //在右边有效距离内，g_nScreenX为屏幕宽度，可由GetSystemMetrics(SM_CYSCREEN)得到。
			{
				g_nAlignType = ALIGN_RIGHT;
				lpRect->right = g_nScreenX;
				lpRect->left = g_nScreenX - Width;
			}
			if (lpRect->top < NEAR_SIZE) //在上边有效距离内，自动靠拢。
			{
				g_nAlignType = ALIGN_TOP;
				lpRect->top = 0;
				lpRect->bottom = Height;
			}
		}
		else
		{
			//靠边界由鼠标控制
			::GetCursorPos(&pt);
			if (g_nAlignType == ALIGN_TOP)
			{
				lpRect->top = 0;
				lpRect->bottom = Height;
				if (pt.y > NEAR_SIZE) //鼠标在离开上边界解除上部停靠。
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->left < NEAR_SIZE) //在上部停靠时，我们也考虑左右边角。
					{
						lpRect->left = 0;
						lpRect->right = Width;
					}
					else if (lpRect->right + NEAR_SIZE > g_nScreenX)
					{
						lpRect->right = g_nScreenX;
						lpRect->left = g_nScreenX - Width;
					}
				}
			}
			if (g_nAlignType == ALIGN_LEFT)
			{
				lpRect->left = 0;
				lpRect->right = Width;
				if (pt.x > NEAR_SIZE) //鼠标在鼠标离开左边界时解除停靠。
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->top < NEAR_SIZE) //考虑左上角。
					{
						lpRect->top = 0;
						lpRect->bottom = Height;
					}
				}
			}
			else if (g_nAlignType == ALIGN_RIGHT)
			{
				lpRect->left = g_nScreenX - Width;
				lpRect->right = g_nScreenX;
				if (pt.x < g_nScreenX - NEAR_SIZE) //当鼠标离开右边界时，解除停靠。
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->top < NEAR_SIZE) //考虑右上角。
					{
						lpRect->top = 0;
						lpRect->bottom = Height;
					}
				}
			}
		}
		return 0;
	}
}

namespace AnimationDisplayer{
	static ULONG_PTR gdiplusToken = 0;
	void GdiplusInitialize()
	{		
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartupOutput gdiplusStartupOutput;

		// START GDI+ SUB SYSTEM
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);

	}
	void GdiplusExitialize()
	{
		// Shutdown GDI+ subystem
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
	class CAnimationDisplayer : public Gdiplus::Image
	{
	public:

		CAnimationDisplayer(const WCHAR* filename, BOOL useEmbeddedColorManagement = FALSE)
			: Image(filename, useEmbeddedColorManagement)
		{
			Initialize();

			m_bIsInitialized = true;

			TestForAnimatedGIF();
		}

		~CAnimationDisplayer()
		{
			Destroy();
		}

	public:

		void Draw(HDC hDC);

		void GetSize(SIZE & size)
		{
			size.cx = GetWidth();
			size.cy = GetHeight();
			//CSize(GetWidth(), GetHeight());
		}

		bool	IsAnimatedGIF() 
		{ 
			return m_nFrameCount > 1; 
		}
		void	SetPause(bool bPaused)
		{
			if (!IsAnimatedGIF())
				return;

			if (bPaused && !m_bPaused)
			{
				::ResetEvent(m_hPaused);
			}
			else
			{
				if (m_bPaused && !bPaused)
				{
					::SetEvent(m_hPaused);
				}
			}

			m_bPaused = bPaused;
		}
		bool	IsPaused() 
		{ 
			return m_bPaused; 
		}
		bool	InitAnimation(HWND hWnd, POINT pt)
		{
			m_hWnd = hWnd;
			m_pt = pt;

			if (!m_bIsInitialized)
			{
				//TRACE(_T("GIF not initialized\n"));
				return false;
			};

			if (IsAnimatedGIF())
			{
				if (m_hThread == NULL)
				{

					DWORD dwThreadID = 0;
					
					m_hThread = (HANDLE)::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_ThreadAnimationProc, this, CREATE_SUSPENDED, &dwThreadID);

					if (!m_hThread)
					{
						//TRACE(_T("Couldn't start a GIF animation thread\n"));
						return true;
					}
					else
					{
						::ResumeThread(m_hThread);
					}
				}
			}

			return false;

		}
		void	Destroy()
		{
			if (m_hThread)
			{
				// If pause un pause
				SetPause(false);

				SetEvent(m_hExitEvent);
				WaitForSingleObject(m_hThread, INFINITE);
			}

			CloseHandle(m_hThread);
			CloseHandle(m_hExitEvent);
			CloseHandle(m_hPaused);

			free(m_pPropertyItem);

			m_pPropertyItem = NULL;
			m_hThread = NULL;
			m_hExitEvent = NULL;
			m_hPaused = NULL;

			if (m_pStream)
			{
				m_pStream->Release();
			}

			m_bIsInitialized = false;
		}

	protected:

		bool TestForAnimatedGIF()
		{
			UINT count = 0;
			count = GetFrameDimensionsCount();
			GUID* pDimensionIDs = new GUID[count];

			// Get the list of frame dimensions from the Image object.
			GetFrameDimensionsList(pDimensionIDs, count);

			// Get the number of frames in the first dimension.
			m_nFrameCount = GetFrameCount(&pDimensionIDs[0]);

			// Assume that the image has a property item of type PropertyItemEquipMake.
			// Get the size of that property item.
			int nSize = GetPropertyItemSize(PropertyTagFrameDelay);

			// Allocate a buffer to receive the property item.
			m_pPropertyItem = (Gdiplus::PropertyItem*) malloc(nSize);

			GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem);
			
			delete  pDimensionIDs; pDimensionIDs = NULL;

			return m_nFrameCount > 1;
		}
		void Initialize()
		{
			m_pStream = NULL;
			m_nFramePosition = 0;
			m_nFrameCount = 0;
			m_hThread = NULL;
			m_bIsInitialized = false;
			m_pPropertyItem = NULL;
			//lastResult = InvalidParameter;

#ifdef INDIGO_CTRL_PROJECT
			m_hInst = _Module.GetResourceInstance();
#else
			m_hInst = GetModuleHandle(NULL);//AfxGetResourceHandle();
#endif

			m_bPaused = false;

			m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			m_hPaused = CreateEvent(NULL, TRUE, TRUE, NULL);

		}
		bool				DrawFrameGIF()
		{

			::WaitForSingleObject(m_hPaused, INFINITE);

			GUID   pageGuid = Gdiplus::FrameDimensionTime;

			LONG hmWidth = GetWidth();
			LONG hmHeight = GetHeight();

			HDC hDC = GetDC(m_hWnd);
			if (hDC)
			{
				Gdiplus::Graphics graphics(hDC);
				graphics.DrawImage((Gdiplus::Image *)this, (INT)m_pt.x, (INT)m_pt.y, (INT)hmWidth, (INT)hmHeight);
				ReleaseDC(m_hWnd, hDC);
			}

			SelectActiveFrame(&pageGuid, m_nFramePosition++);

			if (m_nFramePosition == m_nFrameCount)
				m_nFramePosition = 0;


			long lPause = ((long*)m_pPropertyItem->value)[m_nFramePosition] * 10;

			DWORD dwErr = WaitForSingleObject(m_hExitEvent, lPause);

			return dwErr == WAIT_OBJECT_0;
		}

		IStream*			m_pStream;

		bool LoadFromBuffer(BYTE* pBuff, int nSize)
		{
			bool bResult = false;

			HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, nSize);
			if (hGlobal)
			{
				void* pData = GlobalLock(hGlobal);
				if (pData)
					memcpy(pData, pBuff, nSize);

				GlobalUnlock(hGlobal);

				if (CreateStreamOnHGlobal(hGlobal, TRUE, &m_pStream) == S_OK)
					bResult = true;

			}


			return bResult;
		}
		bool GetResource(LPCTSTR lpName, LPCTSTR lpType, void* pResource, int& nBufSize)
		{
			HRSRC		hResInfo;
			HANDLE		hRes;
			LPSTR		lpRes = NULL;
			int			nLen = 0;
			bool		bResult = FALSE;

			// Find the resource

			hResInfo = FindResource(m_hInst, lpName, lpType);
			if (hResInfo == NULL)
			{
				DWORD dwErr = GetLastError();
				return false;
			}

			// Load the resource
			hRes = LoadResource(m_hInst, hResInfo);

			if (hRes == NULL)
				return false;

			// Lock the resource
			lpRes = (char*)LockResource(hRes);

			if (lpRes != NULL)
			{
				if (pResource == NULL)
				{
					nBufSize = SizeofResource(m_hInst, hResInfo);
					bResult = true;
				}
				else
				{
					if (nBufSize >= (int)SizeofResource(m_hInst, hResInfo))
					{
						memcpy(pResource, lpRes, nBufSize);
						bResult = true;
					}
				}

				UnlockResource(hRes);
			}

			// Free the resource
			FreeResource(hRes);

			return bResult;
		}

		bool Load(LPCTSTR sResourceType, LPCTSTR sResource)
		{
			bool bResult = false;


			BYTE*	pBuff = NULL;
			int		nSize = 0;
			if (GetResource(sResource, sResourceType, pBuff, nSize))
			{
				if (nSize > 0)
				{
					pBuff = new BYTE[nSize];

					if (GetResource(sResource, sResourceType, pBuff, nSize))
					{
						if (LoadFromBuffer(pBuff, nSize))
						{

							bResult = true;
						}
					}

					delete[] pBuff;
				}
			}


			m_bIsInitialized = bResult;

			return bResult;
		}

		void ThreadAnimation()
		{
			m_nFramePosition = 0;

			bool bExit = false;
			while (!(bExit = DrawFrameGIF()));
		}

		static UINT WINAPI _ThreadAnimationProc(LPVOID pParam)
		{
			//ASSERT(pParam);
			CAnimationDisplayer *pImage = reinterpret_cast<CAnimationDisplayer *> (pParam);
			pImage->ThreadAnimation();

			return 0;
		}

		HANDLE			m_hThread;
		HANDLE			m_hPaused;
		HANDLE			m_hExitEvent;
		HINSTANCE		m_hInst;
		HWND			m_hWnd;
		UINT			m_nFrameCount;
		UINT			m_nFramePosition;
		bool			m_bIsInitialized;
		bool			m_bPaused;
		Gdiplus::PropertyItem*	m_pPropertyItem;
		POINT			m_pt;
	};
}

namespace ShadowWindow{

#ifndef WS_EX_LAYERED
#define MY_WS_EX_LAYERED 0x00080000
#else
#define MY_WS_EX_LAYERED WS_EX_LAYERED
#endif

#ifndef AC_SRC_ALPHA
#define MY_AC_SRC_ALPHA 0x01
#else
#define MY_AC_SRC_ALPHA AC_SRC_ALPHA
#endif

#ifndef ULW_ALPHA
#define MY_ULW_ALPHA 0x00000002
#else
#define MY_ULW_ALPHA ULW_ALPHA
#endif

	const _TCHAR * G_ptszWndClassName = _T("PPSHUAISHADOWWINDOW");
	
	class CShadowBorder
	{
	public:
		CShadowBorder(void)
			: m_hWnd((HWND)INVALID_HANDLE_VALUE)
			, m_OriginParentProc(NULL)
			, m_nDarkness(150)
			, m_nSharpness(5)
			, m_nSize(0)
			, m_nxOffset(5)
			, m_nyOffset(5)
			, m_Color(RGB(0, 0, 0))
			, m_WndSize(0)
			, m_bUpdateShadow(false)
		{

		}
	public:
		virtual ~CShadowBorder(void)
		{
		}

	protected:

		// Instance handle, used to register window class and create window
		static HINSTANCE m_hInstance;

		// Parent HWND and CShadowBorder object pares, in order to find CShadowBorder in ParentProc()
		static WNDMAP m_ShadowWindowMap;

		//
		typedef BOOL(WINAPI *pfnUpdateLayeredWindow)(HWND hWnd, HDC hdcDst, POINT *pptDst,
			SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey,
			BLENDFUNCTION *pblend, DWORD dwFlags);
		static pfnUpdateLayeredWindow m_pUpdateLayeredWindow;

		HWND m_hWnd;

		LONG m_OriginParentProc;        // Original WndProc of parent window

		enum ShadowStatus
		{
			SS_ENABLED = 1,        // Shadow is enabled, if not, the following one is always false
			SS_VISABLE = 1 << 1,        // Shadow window is visible
			SS_PARENTVISIBLE = 1 << 2        // Parent window is visible, if not, the above one is always false
		};
		BYTE m_Status;

		unsigned char m_nDarkness;        // Darkness, transparency of blurred area
		unsigned char m_nSharpness;        // Sharpness, width of blurred border of shadow window
		signed char m_nSize;        // Shadow window size, relative to parent window size

		// The X and Y offsets of shadow window,
		// relative to the parent window, at center of both windows (not top-left corner), signed
		signed char m_nxOffset;
		signed char m_nyOffset;

		// Restore last parent window size, used to determine the update strategy when parent window is resized
		LPARAM m_WndSize;

		// Set this to true if the shadow should not be update until next WM_PAINT is received
		bool m_bUpdateShadow;

		COLORREF m_Color;        // Color of shadow

	public:
		static bool Initialize(HINSTANCE hInstance)
		{
			// Should not initiate more than once
			if (NULL != m_pUpdateLayeredWindow)
			{
				return false;
			}

			HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
			m_pUpdateLayeredWindow =
				(pfnUpdateLayeredWindow)GetProcAddress(hUser32,
				"UpdateLayeredWindow");

			// If the import did not succeed, make sure your app can handle it!
			if (NULL == m_pUpdateLayeredWindow)
			{
				return false;
			}
			// Store the instance handle
			m_hInstance = hInstance;

			// Register window class for shadow window
			WNDCLASSEX wcex;

			memset(&wcex, 0, sizeof(wcex));

			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = DefWindowProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = hInstance;
			wcex.hIcon = NULL;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = NULL;
			wcex.lpszClassName = G_ptszWndClassName;
			wcex.hIconSm = NULL;

			RegisterClassEx(&wcex);

			return true;
		}
		void Create(HWND hParentWnd)
		{
			// Do nothing if the system does not support layered windows
			// Already initialized
			if ((NULL != m_pUpdateLayeredWindow) && (m_hInstance != INVALID_HANDLE_VALUE))
			{
				// Add parent window - shadow pair to the map
				//_ASSERT(m_ShadowWindowMap.find((unsigned long)hParentWnd) == m_ShadowWindowMap.end());    // Only one shadow for each window
				//m_ShadowWindowMap[hParentWnd] = (unsigned long)(this);
				WNDMAPIT it = m_ShadowWindowMap.find((unsigned long)hParentWnd);
				if (it != m_ShadowWindowMap.end())
				{
					it->second = (unsigned long)(this);
				}
				else
				{
					m_ShadowWindowMap.insert(std::make_pair((unsigned long)hParentWnd, (unsigned long)(this)));
				}

				// Create the shadow window
				m_hWnd = CreateWindowEx(MY_WS_EX_LAYERED | WS_EX_TRANSPARENT, G_ptszWndClassName, NULL,
					WS_VISIBLE/* | WS_CAPTION | WS_POPUPWINDOW*/,
					CW_USEDEFAULT, 0, 0, 0, hParentWnd, NULL, m_hInstance, NULL);

				// Determine the initial show state of shadow according to parent window's state
				LONG lParentStyle = GetWindowLong(hParentWnd, GWL_STYLE);
				if (!(WS_VISIBLE & lParentStyle))    // Parent invisible
				{
					m_Status = SS_ENABLED;
				}
				else if ((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle)    // Parent visible but does not need shadow
				{
					m_Status = SS_ENABLED | SS_PARENTVISIBLE;
				}
				else    // Show the shadow
				{
					m_Status = SS_ENABLED | SS_VISABLE | SS_PARENTVISIBLE;
					::ShowWindow(m_hWnd, SW_SHOWNA);
					UpdateShadow(hParentWnd);
				}

				// Replace the original WndProc of parent window to steal messages
				m_OriginParentProc = ::GetWindowLongPtr(hParentWnd, GWLP_WNDPROC);

#pragma warning(disable: 4311)    // temporrarily disable the type_cast warning in Win32
				::SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, (LONG_PTR)ParentProc);
#pragma warning(default: 4311)
			}
		}

		bool SetSize(int NewSize = 0)
		{
			if (NewSize > 20 || NewSize < -20)
				return false;

			m_nSize = (signed char)NewSize;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}
		bool SetSharpness(unsigned int NewSharpness = 5)
		{
			if (NewSharpness > 20)
			{
				return false;
			}
			m_nSharpness = (unsigned char)NewSharpness;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}
		bool SetDarkness(unsigned int NewDarkness = 200)
		{
			if (NewDarkness > 255)
			{
				return false;
			}
			m_nDarkness = (unsigned char)NewDarkness;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}
		bool SetPosition(int NewXOffset = 5, int NewYOffset = 5)
		{
			if (NewXOffset > 20 || NewXOffset < -20 ||
				NewYOffset > 20 || NewYOffset < -20)
			{
				return false;
			}
			m_nxOffset = (signed char)NewXOffset;
			m_nyOffset = (signed char)NewYOffset;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}
		bool SetColor(COLORREF NewColor = 0)
		{
			m_Color = NewColor;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}

	protected:
		//static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			WNDMAPIT it = m_ShadowWindowMap.find((unsigned long)(hwnd));
			// Shadow must have been attached
			if (it != m_ShadowWindowMap.end())
			{
				CShadowBorder *pThis = (CShadowBorder *)it->second;

				switch (uMsg)
				{
				case WM_MOVE:
					if (pThis->m_Status & SS_VISABLE)
					{
						RECT WndRect;
						GetWindowRect(hwnd, &WndRect);
						SetWindowPos(pThis->m_hWnd, 0,
							WndRect.left + pThis->m_nxOffset - pThis->m_nSize, WndRect.top + pThis->m_nyOffset - pThis->m_nSize,
							0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
					}
					break;

				case WM_SIZE:
					if (pThis->m_Status & SS_ENABLED)
					{
						if (SIZE_MAXIMIZED == wParam || SIZE_MINIMIZED == wParam)
						{
							::ShowWindow(pThis->m_hWnd, SW_HIDE);
							pThis->m_Status &= ~SS_VISABLE;
						}
						else if (pThis->m_Status & SS_PARENTVISIBLE)    // Parent maybe resized even if invisible
						{
							// Awful! It seems that if the window size was not decreased
							// the window region would never be updated until WM_PAINT was sent.
							// So do not Update() until next WM_PAINT is received in this case
							if (LOWORD(lParam) > LOWORD(pThis->m_WndSize) || HIWORD(lParam) > HIWORD(pThis->m_WndSize))
							{
								pThis->m_bUpdateShadow = true;
							}
							else
							{
								pThis->UpdateShadow(hwnd);
							}
							if (!(pThis->m_Status & SS_VISABLE))
							{
								::ShowWindow(pThis->m_hWnd, SW_SHOWNA);
								pThis->m_Status |= SS_VISABLE;
							}
						}
						pThis->m_WndSize = lParam;
					}
					break;

				case WM_PAINT:
				{
					if (pThis->m_bUpdateShadow)
					{
						pThis->UpdateShadow(hwnd);
						pThis->m_bUpdateShadow = false;
					}
				}
				break;
				// In some cases of sizing, the up-right corner of the parent window region would not be properly updated
				// UpdateShadow() again when sizing is finished
				case WM_EXITSIZEMOVE:
				{
					if (pThis->m_Status & SS_VISABLE)
					{
						pThis->UpdateShadow(hwnd);
					}
				}
				break;

				case WM_SHOWWINDOW:
				{
					if (pThis->m_Status & SS_ENABLED)
					{
						if (!wParam)    // the window is being hidden
						{
							::ShowWindow(pThis->m_hWnd, SW_HIDE);
							pThis->m_Status &= ~(SS_VISABLE | SS_PARENTVISIBLE);
						}
						else if (!(pThis->m_Status & SS_PARENTVISIBLE))
						{
							//pThis->Update(hwnd);
							pThis->m_bUpdateShadow = true;
							::ShowWindow(pThis->m_hWnd, SW_SHOWNA);
							pThis->m_Status |= SS_VISABLE | SS_PARENTVISIBLE;
						}
					}
				}
				break;

				case WM_DESTROY:
				{
					DestroyWindow(pThis->m_hWnd);    // Destroy the shadow
				}
				break;

				case WM_NCDESTROY:
				{
					m_ShadowWindowMap.erase(it);    // Remove this window and shadow from the map
				}
				break;

				}

#pragma warning(disable: 4312)    // temporrarily disable the type_cast warning in Win32
				// Call the default(original) window procedure for other messages or messages processed but not returned
				return ((WNDPROC)pThis->m_OriginParentProc)(hwnd, uMsg, wParam, lParam);
#pragma warning(default: 4312)
			}
			else
			{
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
				return FALSE;
			}
		}

		// Redraw, resize and move the shadow
		// called when window resized or shadow properties changed, but not only moved without resizing
		void UpdateShadow(HWND hParent)
		{
			//int ShadowSize = 5;
			//int Multi = 100 / ShadSize;

			RECT rcParentWindow = { 0 };
			GetWindowRect(hParent, &rcParentWindow);
			int nShadowWindowWidth = rcParentWindow.right - rcParentWindow.left + m_nSize * 2;
			int nShadowWindowHeight = rcParentWindow.bottom - rcParentWindow.top + m_nSize * 2;

			// Create the alpha blending bitmap
			BITMAPINFO bmi = { 0 };    // bitmap header
			HDC hDC = GetDC(hParent);
			WORD wBitsCount = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

			if (wBitsCount <= 1)
			{
				wBitsCount = 1;
			}
			else if (wBitsCount <= 4)
			{
				wBitsCount = 4;
			}
			else if (wBitsCount <= 8)
			{
				wBitsCount = 8;
			}
			else if (wBitsCount <= 24)
			{
				wBitsCount = 24;
			}
			else
			{
				wBitsCount = 32;
			}

			ZeroMemory(&bmi, sizeof(BITMAPINFO));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = nShadowWindowWidth;
			bmi.bmiHeader.biHeight = nShadowWindowHeight;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = wBitsCount;   // four 8-bit components
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = nShadowWindowWidth * nShadowWindowHeight * 4;

			BYTE *pvBits;    // pointer to DIB section
			HBITMAP hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);

			ZeroMemory(pvBits, bmi.bmiHeader.biSizeImage);
			MakeShadow((UINT32 *)pvBits, hParent, &rcParentWindow);
			//SaveHBitmapToFile(hbitmap, TEXT("test.bmp"));
			//PPSHUAI::GUIWND::HBitmapToFile(hbitmap, TEXT("temp.bmp"));

			HDC hMemDC = CreateCompatibleDC(NULL);
			HBITMAP hOriBmp = (HBITMAP)SelectObject(hMemDC, hbitmap);

			POINT ptDst = { rcParentWindow.left + m_nxOffset - m_nSize, rcParentWindow.top + m_nyOffset - m_nSize };
			POINT ptSrc = { 0, 0 };
			SIZE WindowSize = { nShadowWindowWidth, nShadowWindowHeight };
			BLENDFUNCTION blendPixelFunction = { AC_SRC_OVER, 0, 255, MY_AC_SRC_ALPHA };

			MoveWindow(m_hWnd, ptDst.x, ptDst.y, nShadowWindowWidth, nShadowWindowHeight, FALSE);

			BOOL bRet = m_pUpdateLayeredWindow(m_hWnd, NULL, &ptDst, &WindowSize, hMemDC,
				&ptSrc, 0, &blendPixelFunction, MY_ULW_ALPHA);

			//_ASSERT(bRet); // something was wrong....

			// Delete used resources
			SelectObject(hMemDC, hOriBmp);
			DeleteObject(hbitmap);
			DeleteDC(hMemDC);

		}

		// Fill in the shadow window alpha blend bitmap with shadow image pixels
		void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent)
		{
			// The shadow algorithm:
			// Get the region of parent window,
			// Apply morphologic erosion to shrink it into the size (ShadowWndSize - Sharpness)
			// Apply modified (with blur effect) morphologic dilation to make the blurred border
			// The algorithm is optimized by assuming parent window is just "one piece" and without "wholes" on it

			// Get the region of parent window,
			HRGN hParentRgn = CreateRectRgn(0, 0, rcParent->right - rcParent->left, rcParent->bottom - rcParent->top);
			GetWindowRgn(hParent, hParentRgn);

			// Determine the Start and end point of each horizontal scan line
			SIZE szParent = { rcParent->right - rcParent->left, rcParent->bottom - rcParent->top };
			SIZE szShadow = { szParent.cx + 2 * m_nSize, szParent.cy + 2 * m_nSize };
			// Extra 2 lines (set to be empty) in ptAnchors are used in dilation
			int nAnchors = max(szParent.cy, szShadow.cy);    // # of anchor points pares
			int(*ptAnchors)[2] = new int[nAnchors + 2][2];
			int(*ptAnchorsOri)[2] = new int[szParent.cy][2];    // anchor points, will not modify during erosion
			ptAnchors[0][0] = szParent.cx;
			ptAnchors[0][1] = 0;
			ptAnchors[nAnchors + 1][0] = szParent.cx;
			ptAnchors[nAnchors + 1][1] = 0;
			if (m_nSize > 0)
			{
				// Put the parent window anchors at the center
				for (int i = 0; i < m_nSize; i++)
				{
					ptAnchors[i + 1][0] = szParent.cx;
					ptAnchors[i + 1][1] = 0;
					ptAnchors[szShadow.cy - i][0] = szParent.cx;
					ptAnchors[szShadow.cy - i][1] = 0;
				}
				ptAnchors += m_nSize;
			}
			for (int i = 0; i < szParent.cy; i++)
			{
				// find start point
				int j = 0;
				for (j = 0; j < szParent.cx; j++)
				{
					if (PtInRegion(hParentRgn, j, i))
					{
						ptAnchors[i + 1][0] = j + m_nSize;
						ptAnchorsOri[i][0] = j;
						break;
					}
				}

				if (j >= szParent.cx)    // Start point not found
				{
					ptAnchors[i + 1][0] = szParent.cx;
					ptAnchorsOri[i][1] = 0;
					ptAnchors[i + 1][0] = szParent.cx;
					ptAnchorsOri[i][1] = 0;
				}
				else
				{
					// find end point
					for (j = szParent.cx - 1; j >= ptAnchors[i + 1][0]; j--)
					{
						if (PtInRegion(hParentRgn, j, i))
						{
							ptAnchors[i + 1][1] = j + 1 + m_nSize;
							ptAnchorsOri[i][1] = j + 1;
							break;
						}
					}
				}
			}

			if (m_nSize > 0)
			{
				ptAnchors -= m_nSize;    // Restore pos of ptAnchors for erosion
			}
			int(*ptAnchorsTmp)[2] = new int[nAnchors + 2][2];    // Store the result of erosion
			// First and last line should be empty
			ptAnchorsTmp[0][0] = szParent.cx;
			ptAnchorsTmp[0][1] = 0;
			ptAnchorsTmp[nAnchors + 1][0] = szParent.cx;
			ptAnchorsTmp[nAnchors + 1][1] = 0;
			int nEroTimes = 0;
			// morphologic erosion
			for (int i = 0; i < m_nSharpness - m_nSize; i++)
			{
				nEroTimes++;
				//ptAnchorsTmp[1][0] = szParent.cx;
				//ptAnchorsTmp[1][1] = 0;
				//ptAnchorsTmp[szParent.cy + 1][0] = szParent.cx;
				//ptAnchorsTmp[szParent.cy + 1][1] = 0;
				for (int j = 1; j < nAnchors + 1; j++)
				{
					ptAnchorsTmp[j][0] = max(ptAnchors[j - 1][0], max(ptAnchors[j][0], ptAnchors[j + 1][0])) + 1;
					ptAnchorsTmp[j][1] = min(ptAnchors[j - 1][1], min(ptAnchors[j][1], ptAnchors[j + 1][1])) - 1;
				}
				// Exchange ptAnchors and ptAnchorsTmp;
				int(*ptAnchorsXange)[2] = ptAnchorsTmp;
				ptAnchorsTmp = ptAnchors;
				ptAnchors = ptAnchorsXange;
			}

			// morphologic dilation
			ptAnchors += (m_nSize < 0 ? -m_nSize : 0) + 1;    // now coordinates in ptAnchors are same as in shadow window
			// Generate the kernel
			int nKernelSize = m_nSize > m_nSharpness ? m_nSize : m_nSharpness;
			int nCenterSize = m_nSize > m_nSharpness ? (m_nSize - m_nSharpness) : 0;
			UINT32 *pKernel = new UINT32[(2 * nKernelSize + 1) * (2 * nKernelSize + 1)];
			UINT32 *pKernelIter = pKernel;
			for (int i = 0; i <= 2 * nKernelSize; i++)
			{
				for (int j = 0; j <= 2 * nKernelSize; j++)
				{
					double dLength = sqrt((i - nKernelSize) * (i - nKernelSize) + (j - nKernelSize) * (double)(j - nKernelSize));
					if (dLength < nCenterSize)
					{
						*pKernelIter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
					}
					else if (dLength <= nKernelSize)
					{
						UINT32 nFactor = ((UINT32)((1 - (dLength - nCenterSize) / (m_nSharpness + 1)) * m_nDarkness));
						*pKernelIter = nFactor << 24 | PreMultiply(m_Color, nFactor);
					}
					else
					{
						*pKernelIter = 0;
					}
					pKernelIter++;
				}
			}
			// Generate blurred border
			for (int i = nKernelSize; i < szShadow.cy - nKernelSize; i++)
			{
				int j = 0;
				if (ptAnchors[i][0] < ptAnchors[i][1])
				{
					// Start of line
					for (j = ptAnchors[i][0];
						j < min(max(ptAnchors[i - 1][0], ptAnchors[i + 1][0]) + 1, ptAnchors[i][1]);
						j++)
					{
						for (int k = 0; k <= 2 * nKernelSize; k++)
						{
							UINT32 *pPixel = pShadBits +
								(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
							UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
							for (int l = 0; l <= 2 * nKernelSize; l++)
							{
								if (*pPixel < *pKernelPixel)
								{
									*pPixel = *pKernelPixel;
								}
								pPixel++;
								pKernelPixel++;
							}
						}
					}    // for() start of line

					// End of line
					for (j = max(j, min(ptAnchors[i - 1][1], ptAnchors[i + 1][1]) - 1);
						j < ptAnchors[i][1];
						j++)
					{
						for (int k = 0; k <= 2 * nKernelSize; k++)
						{
							UINT32 *pPixel = pShadBits +
								(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
							UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
							for (int l = 0; l <= 2 * nKernelSize; l++)
							{
								if (*pPixel < *pKernelPixel)
								{
									*pPixel = *pKernelPixel;
								}
								pPixel++;
								pKernelPixel++;
							}
						}
					}    // for() end of line
				}
			}    // for() Generate blurred border

			// Erase unwanted parts and complement missing
			UINT32 clCenter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
			for (int i = min(nKernelSize, max(m_nSize - m_nyOffset, 0));
				i < max(szShadow.cy - nKernelSize, min(szParent.cy + m_nSize - m_nyOffset, szParent.cy + 2 * m_nSize));
				i++)
			{
				UINT32 *pLine = pShadBits + (szShadow.cy - i - 1) * szShadow.cx;
				if (i - m_nSize + m_nyOffset < 0 || i - m_nSize + m_nyOffset >= szParent.cy)    // Line is not covered by parent window
				{
					for (int j = ptAnchors[i][0]; j < ptAnchors[i][1]; j++)
					{
						*(pLine + j) = clCenter;
					}
				}
				else
				{
					for (int j = ptAnchors[i][0];
						j < min(ptAnchorsOri[i - m_nSize + m_nyOffset][0] + m_nSize - m_nxOffset, ptAnchors[i][1]);
						j++)
						*(pLine + j) = clCenter;
					for (int j = max(ptAnchorsOri[i - m_nSize + m_nyOffset][0] + m_nSize - m_nxOffset, 0);
						j < min((long)ptAnchorsOri[i - m_nSize + m_nyOffset][1] + m_nSize - m_nxOffset, szShadow.cx);
						j++)
						*(pLine + j) = 0;
					for (int j = max(ptAnchorsOri[i - m_nSize + m_nyOffset][1] + m_nSize - m_nxOffset, ptAnchors[i][0]);
						j < ptAnchors[i][1];
						j++)
						*(pLine + j) = clCenter;
				}
			}

			// Delete used resources
			delete[](ptAnchors - (m_nSize < 0 ? -m_nSize : 0) - 1);
			delete[] ptAnchorsTmp;
			delete[] ptAnchorsOri;
			delete[] pKernel;
			DeleteObject(hParentRgn);
		}

		// Helper to calculate the alpha-premultiled value for a pixel
		inline DWORD PreMultiply(COLORREF cl, unsigned char nAlpha)
		{
			// It's strange that the byte order of RGB in 32b BMP is reverse to in COLORREF
			return (GetRValue(cl) * (DWORD)nAlpha / 255) << 16 |
				(GetGValue(cl) * (DWORD)nAlpha / 255) << 8 |
				(GetBValue(cl) * (DWORD)nAlpha / 255);
		}
	};

	CShadowBorder::pfnUpdateLayeredWindow CShadowBorder::m_pUpdateLayeredWindow = NULL;
	
	HINSTANCE CShadowBorder::m_hInstance = (HINSTANCE)INVALID_HANDLE_VALUE;

	WNDMAP CShadowBorder::m_ShadowWindowMap;
}

namespace GUI{

	//显示在屏幕中央
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

		//居中显示
		ptAppWnd.x = (rcScreen.right - rcScreen.left - szAppWnd.cx) / 2;
		ptAppWnd.y = (rcScreen.bottom - rcScreen.top - szAppWnd.cy) / 2;
		MoveWindow(hWnd, ptAppWnd.x, ptAppWnd.y, szAppWnd.cx, szAppWnd.cy, TRUE);
	}

	//显示在父窗口中央
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

		//居中显示
		ptAppWnd.x = (rcParent.right - rcParent.left - szAppWnd.cx) / 2;
		ptAppWnd.y = (rcParent.bottom - rcParent.top - szAppWnd.cy) / 2;
		MoveWindow(hWnd, ptAppWnd.x, ptAppWnd.y, szAppWnd.cx, szAppWnd.cy, TRUE);
	}

	__inline static
		void NotifyUpdate(HWND hWnd, RECT *pRect, BOOL bErase = TRUE)
	{
		RECT rcWnd = { 0 };

		::GetClientRect(hWnd, &rcWnd);
		if (memcmp(pRect, &rcWnd, sizeof(RECT)))
		{
			::InvalidateRect(hWnd, &rcWnd, bErase);
			memcpy(pRect, &rcWnd, sizeof(RECT));
		}
	}

	__inline static
		bool SaveBitmapToFile(HDC hDC, HBITMAP hBitmap, LPCTSTR ptFileName)
	{
		//	HDC hDC;
		//设备描述表
		int iBits;
		//当前显示分辨率下每个像素所占字节数
		WORD wBitCount;
		//位图中每个像素所占字节数
		//定义调色板大小， 位图中像素字节大小 ，  位图文件大小 ， 写入文件字节数
		DWORD  dwPaletteSize = 0, dwBmBitsSize, dwDIBSize, dwWritten;
		BITMAP  Bitmap;
		//位图属性结构
		BITMAPFILEHEADER   bmfHdr;
		//位图文件头结构
		BITMAPINFOHEADER   bi;
		//位图信息头结构
		LPBITMAPINFOHEADER lpbi;
		//指向位图信息头结构
		HANDLE  fh, hDib, hPal;
		HPALETTE  hOldPal = NULL;

		//定义文件，分配内存句柄，调色板句柄
		//计算位图文件每个像素所占字节数
		//hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
		//DeleteDC(hDC);
		if (iBits <= 1)
		{
			wBitCount = 1;
		}
		else if (iBits <= 4)
		{
			wBitCount = 4;
		}
		else if (iBits <= 8)
		{
			wBitCount = 8;
		}
		else if (iBits <= 24)
		{
			wBitCount = 24;
		}
		else
		{
			wBitCount = 32;
		}

		//计算调色板大小
		if (wBitCount <= 8)
		{
			dwPaletteSize = (1 << wBitCount)*sizeof(RGBQUAD);
		}

		//设置位图信息头结构
		GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = Bitmap.bmWidth;
		bi.biHeight = Bitmap.bmHeight;
		bi.biPlanes = 1;
		bi.biBitCount = wBitCount;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;
		dwBmBitsSize = ((Bitmap.bmWidth*wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

		//为位图内容分配内存
		hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
		*lpbi = bi;
		// 处理调色板
		hPal = GetStockObject(DEFAULT_PALETTE);
		if (hPal)
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}
		// 获取该调色板下新的像素值
		GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);
		//恢复调色板   
		if (hOldPal)
		{
			SelectPalette(hDC, hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//创建位图文件    
		fh = CreateFile(ptFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (fh == INVALID_HANDLE_VALUE)
		{
			GlobalUnlock(hDib);
			GlobalFree(hDib);
			return FALSE;
		}
		//设置位图文件头
		bmfHdr.bfType = 0x4D42;  // "BM"
		dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
		bmfHdr.bfSize = dwDIBSize;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

		WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
		WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize, &dwWritten, NULL);

		GlobalUnlock(hDib);
		GlobalFree(hDib);

		CloseHandle(fh);
		return false;
	}
	__inline static
		bool SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR ptFileName)
	{
		HDC hDC;
		//设备描述表
		int iBits;
		//当前显示分辨率下每个像素所占字节数
		WORD wBitCount;
		//位图中每个像素所占字节数
		//定义调色板大小， 位图中像素字节大小 ，  位图文件大小 ， 写入文件字节数
		DWORD  dwPaletteSize = 0, dwBmBitsSize, dwDIBSize, dwWritten;
		BITMAP  Bitmap;
		//位图属性结构
		BITMAPFILEHEADER   bmfHdr;
		//位图文件头结构
		BITMAPINFOHEADER   bi;
		//位图信息头结构
		LPBITMAPINFOHEADER lpbi;
		//指向位图信息头结构
		HANDLE  fh, hDib, hPal;
		HPALETTE  hOldPal = NULL;

		//定义文件，分配内存句柄，调色板句柄
		//计算位图文件每个像素所占字节数
		hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
		DeleteDC(hDC);
		if (iBits <= 1)
		{
			wBitCount = 1;
		}
		else if (iBits <= 4)
		{
			wBitCount = 4;
		}
		else if (iBits <= 8)
		{
			wBitCount = 8;
		}
		else if (iBits <= 24)
		{
			wBitCount = 24;
		}
		else
		{
			wBitCount = 32;
		}

		//计算调色板大小
		if (wBitCount <= 8)
		{
			dwPaletteSize = (1 << wBitCount)*sizeof(RGBQUAD);
		}

		//设置位图信息头结构
		GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = Bitmap.bmWidth;
		bi.biHeight = Bitmap.bmHeight;
		bi.biPlanes = 1;
		bi.biBitCount = wBitCount;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;
		dwBmBitsSize = ((Bitmap.bmWidth*wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

		//为位图内容分配内存
		hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
		*lpbi = bi;
		// 处理调色板
		hPal = GetStockObject(DEFAULT_PALETTE);
		if (hPal)
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}
		// 获取该调色板下新的像素值
		GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);
		//恢复调色板   
		if (hOldPal)
		{
			SelectPalette(hDC, hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//创建位图文件    
		fh = CreateFile(ptFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (fh == INVALID_HANDLE_VALUE)
		{
			GlobalUnlock(hDib);
			GlobalFree(hDib);
			return FALSE;
		}
		//设置位图文件头
		bmfHdr.bfType = 0x4D42;  // "BM"
		dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
		bmfHdr.bfSize = dwDIBSize;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

		WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
		WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize, &dwWritten, NULL);

		GlobalUnlock(hDib);
		GlobalFree(hDib);

		CloseHandle(fh);
		return false;
	}
	__inline static
		HBITMAP GetBitmapFromDC(HDC hDC, RECT rc, RECT rcMemory = { 0 })
	{
		HDC hMemoryDC = NULL;
		HBITMAP hBitmap = NULL;
		HBITMAP hBitmapTemp = NULL;

		//创建设备上下文(HDC)
		hMemoryDC = CreateCompatibleDC(hDC);

		//创建HBITMAP
		hBitmap = CreateCompatibleBitmap(hDC, rc.right - rc.left, rc.bottom - rc.top);
		hBitmapTemp = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

		if (rcMemory.right <= rcMemory.left)
		{
			rcMemory.right = rcMemory.left + rc.right - rc.left;
		}
		if (rcMemory.bottom <= rcMemory.top)
		{
			rcMemory.bottom = rcMemory.top + rc.bottom - rc.top;
		}

		//得到位图缓冲区
		StretchBlt(hMemoryDC, rcMemory.left, rcMemory.top, rcMemory.right - rcMemory.left, rcMemory.bottom - rcMemory.top,
			hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SRCCOPY);

		//得到最终的位图信息
		hBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmapTemp);

		//释放内存
		DeleteObject(hBitmapTemp);
		hBitmapTemp = NULL;
		::DeleteDC(hMemoryDC);
		hMemoryDC = NULL;

		return hBitmap;
	}

	__inline static
		void DrawMemoryBitmap(HDC &dc, HWND hWnd, LONG lWidth, LONG lHeight, HBITMAP hBitmap)
	{
		RECT rect;
		HBITMAP hOldBitmap;
		int disHeight, disWidth;

		GetClientRect(hWnd, &rect);//获取客户区大小
		disHeight = rect.bottom - rect.top;
		disWidth = rect.right - rect.left;

		HDC mDc = ::CreateCompatibleDC(dc);//创建当前上下文的兼容dc(内存DC)
		hOldBitmap = (HBITMAP)::SelectObject(mDc, hBitmap);//将位图加载到内存DC

		//拷贝内存DC数据块到当前DC，自动拉伸
		::StretchBlt(dc, 0, 0, disWidth, disHeight, mDc, 0, 0, lWidth, lHeight, SRCCOPY);

		//恢复内存原始数据
		::SelectObject(mDc, hOldBitmap);

		//删除资源，防止泄漏
		::DeleteObject(hBitmap);
		::DeleteDC(mDc);
	}

	__inline static
		void DrawImage(HDC &dc, HWND hWnd, LONG lWidth, LONG lHeight, LPCTSTR ptFileName)
	{
		RECT rect;
		HBITMAP hOrgBitmap;
		HBITMAP hOldBitmap;
		int disHeight, disWidth;

		GetClientRect(hWnd, &rect);//获取客户区大小
		disHeight = rect.bottom - rect.top;
		disWidth = rect.right - rect.left;

		//加载图片
		hOrgBitmap = (HBITMAP)::LoadImage(GetModuleHandle(NULL), ptFileName, IMAGE_BITMAP, lWidth, lHeight, LR_LOADFROMFILE);

		HDC mDc = ::CreateCompatibleDC(dc);//创建当前上下文的兼容dc(内存DC)
		hOldBitmap = (HBITMAP)::SelectObject(mDc, hOrgBitmap);//将位图加载到内存DC

		//拷贝内存DC数据块到当前DC，自动拉伸
		::StretchBlt(dc, 0, 0, disWidth, disHeight, mDc, 0, 0, lWidth, lHeight, SRCCOPY);

		//恢复内存原始数据
		::SelectObject(mDc, hOldBitmap);

		//删除资源，防止泄漏
		::DeleteObject(hOrgBitmap);
		::DeleteDC(mDc);
		mDc = NULL;
	}

	__inline static
		HBITMAP DrawAlphaBlend(HWND hWnd, HDC hDCWnd)
	{
		typedef struct _ALPHABLENDRECT {
			HDC HDCDST;
			RECT RCDST;
			HDC HDCSRC;
			RECT RCSRC;
			BLENDFUNCTION BF;// structure for alpha blending 
		}ALPHABLENDRECT, *PALPHABLENDRECT;
		UINT32 x = 0;// stepping variables 
		UINT32 y = 0;// stepping variables 
		HDC hDC = NULL;// handle of the DC we will create 
		UCHAR uA = 0x00;// used for doing transparent gradient 
		UCHAR uR = 0x00;
		UCHAR uG = 0x00;
		UCHAR uB = 0x00;
		float fAF = 0.0f;// used to do premultiply
		VOID * pvBits = 0;// pointer to DIB section
		RECT rcWnd = { 0 };// used for getting window dimensions
		HBITMAP hBitmap = NULL;// bitmap handle
		BITMAPINFO bmi = { 0 };// bitmap header
		ULONG ulWindowWidth = 0;// window width/height
		ULONG ulBitmapWidth = 0;// bitmap width/height
		ULONG ulWindowHeight = 0;// window width/height
		ULONG ulBitmapHeight = 0;// bitmap width/height
		ALPHABLENDRECT abrc = { 0 };

		// get dest dc
		abrc.HDCDST = hDCWnd;

		// get window dimensions 
		::GetClientRect(hWnd, &rcWnd);

		// calculate window width/height 
		ulWindowWidth = rcWnd.right - rcWnd.left;
		ulWindowHeight = rcWnd.bottom - rcWnd.top;

		// make sure we have at least some window size 
		if ((!ulWindowWidth) || (!ulWindowHeight))
		{
			return NULL;
		}

		// divide the window into 3 horizontal areas 
		ulWindowHeight = ulWindowHeight / 3;

		// create a DC for our bitmap -- the source DC for AlphaBlend  
		abrc.HDCSRC = ::CreateCompatibleDC(abrc.HDCDST);

		// zero the memory for the bitmap info 
		::ZeroMemory(&bmi, sizeof(BITMAPINFO));

		// setup bitmap info  
		// set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = ulBitmapWidth = ulWindowWidth;// -(ulWindowWidth / 5) * 2;
		bmi.bmiHeader.biHeight = ulBitmapHeight = ulWindowHeight - (ulWindowHeight / 5) * 2;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;// four 8-bit components 
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

		// create our DIB section and select the bitmap into the dc 
		hBitmap = ::CreateDIBSection(abrc.HDCSRC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x00);
		::SelectObject(abrc.HDCSRC, hBitmap);

		// in top window area, constant alpha = 50%, but no source alpha 
		// the color format for each pixel is 0xaarrggbb  
		// set all pixels to blue and set source alpha to zero 
		for (y = 0; y < ulBitmapHeight; y++)
		{
			for (x = 0; x < ulBitmapWidth; x++)
			{
				((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x0000FF00;// 0x000000FF;
			}
		}

		abrc.RCDST.left = 0;// ulWindowWidth / 5;
		abrc.RCDST.top = ulWindowHeight / 5;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = 0;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0x7F;// half of 0x7F = 50% transparency 

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}
		// in middle window area, constant alpha = 100% (disabled), source  
		// alpha is 0 in middle of bitmap and opaque in rest of bitmap  
		for (y = 0; y < ulBitmapHeight; y++)
		{
			for (x = 0; x < ulBitmapWidth; x++)
			{
				if ((x > (int)(ulBitmapWidth / 5)) && (x < (ulBitmapWidth - ulBitmapWidth / 5)) &&
					(y >(int)(ulBitmapHeight / 5)) && (y < (ulBitmapHeight - ulBitmapHeight / 5)))
				{
					//in middle of bitmap: source alpha = 0 (transparent). 
					// This means multiply each color component by 0x00. 
					// Thus, after AlphaBlend, we have A, 0x00 * R,  
					// 0x00 * G,and 0x00 * B (which is 0x00000000) 
					// for now, set all pixels to red 
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x00FF0000;
				}
				else
				{
					// in the rest of bitmap, source alpha = 0xFF (opaque)  
					// and set all pixels to blue  
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0xFF00FF00;// 0xFF0000FF;
				}
			}
		}

		abrc.RCDST.left = 0;// ulWindowWidth / 5;
		abrc.RCDST.top = ulWindowHeight / 5 + ulWindowHeight;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = MY_AC_SRC_ALPHA;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0xFF;// half of 0xFF = 50% transparency 

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}

		// bottom window area, use constant alpha = 75% and a changing 
		// source alpha. Create a gradient effect using source alpha, and  
		// then fade it even more with constant alpha 
		uR = 0x00;
		uG = 0xFF;// 0x00;
		uB = 0x00;// 0xFF;

		for (y = 0; y < ulBitmapHeight; y++)
		{
			for (x = 0; x < ulBitmapWidth; x++)
			{
				// for a simple gradient, base the alpha value on the x  
				// value of the pixel  
				uA = (UCHAR)((float)x / (float)ulBitmapWidth * 0xFF);
				//calculate the factor by which we multiply each component 
				fAF = (float)uA / (float)0xFF;
				// multiply each pixel by fAlphaFactor, so each component  
				// is less than or equal to the alpha value. 
				((UINT32 *)pvBits)[x + y * ulBitmapWidth] =
					((UCHAR)(uA * 0x1) << 0x18) | //0xAA000000 
					((UCHAR)(uR * fAF) << 0x10) | //0x00RR0000 
					((UCHAR)(uG * fAF) << 0x08) | //0x0000GG00 
					((UCHAR)(uB * fAF) << 0x00); //0x000000BB 
			}
		}

		abrc.RCDST.left = 0;// ulWindowWidth / 5;
		abrc.RCDST.top = ulWindowHeight / 5 + 2 * ulWindowHeight;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = MY_AC_SRC_ALPHA;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0xBF;// use constant alpha, with 75% opaqueness  

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}

		// do cleanup 
		DeleteObject(hBitmap);
		DeleteDC(hDC);
		hDC = NULL;

		return hBitmap;
	}

	__inline static
		HBITMAP DrawAlphaBlendRect(HWND hWnd, HDC hDCWnd, RECT * pRect)
	{
		typedef struct _ALPHABLENDRECT {
			HDC HDCDST;
			RECT RCDST;
			HDC HDCSRC;
			RECT RCSRC;
			BLENDFUNCTION BF;// structure for alpha blending 
		}ALPHABLENDRECT, *PALPHABLENDRECT;
		UINT32 x = 0;// stepping variables
		UINT32 y = 0;// stepping variables 
		UCHAR uA = 0x00;// used for doing transparent gradient 
		UCHAR uR = 0x00;
		UCHAR uG = 0x00;
		UCHAR uB = 0x00;
		float fAF = 0.0f;// used to do premultiply
		VOID * pvBits = 0;// pointer to DIB section
		RECT rcWnd = { 0 };// used for getting window dimensions
		HBITMAP hBitmap = NULL;// bitmap handle
		BITMAPINFO bmi = { 0 };// bitmap header
		ULONG ulWindowWidth = 0;// window width/height
		ULONG ulBitmapWidth = 0;// bitmap width/height
		ULONG ulWindowHeight = 0;// window width/height
		ULONG ulBitmapHeight = 0;// bitmap width/height
		ALPHABLENDRECT abrc = { 0 };

		// get dest dc
		abrc.HDCDST = hDCWnd;

		// get window dimensions 
		::GetClientRect(hWnd, &rcWnd);

		// calculate window width/height 
		ulWindowWidth = rcWnd.right - rcWnd.left;
		ulWindowHeight = rcWnd.bottom - rcWnd.top;

		// make sure we have at least some window size 
		if ((!ulWindowWidth) || (!ulWindowHeight))
		{
			return NULL;
		}

		// create a DC for our bitmap -- the source DC for AlphaBlend  
		abrc.HDCSRC = ::CreateCompatibleDC(abrc.HDCDST);

		// zero the memory for the bitmap info 
		::ZeroMemory(&bmi, sizeof(BITMAPINFO));

		// setup bitmap info  
		// set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = ulBitmapWidth = ulWindowWidth;
		bmi.bmiHeader.biHeight = ulBitmapHeight = ulWindowHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;// four 8-bit components 
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

		// create our DIB section and select the bitmap into the dc 
		hBitmap = ::CreateDIBSection(abrc.HDCSRC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x00);
		::SelectObject(abrc.HDCSRC, hBitmap);

		// in middle window area, constant alpha = 100% (disabled), source  
		// alpha is 0 in middle of bitmap and opaque in rest of bitmap  
		for (y = 0; y < ulBitmapHeight; y++)
		{
			for (x = 0; x < ulBitmapWidth; x++)
			{
				if ((x > (int)(pRect->left)) && (x < (ulBitmapWidth - pRect->right - 1)) &&
					(y >(int)(pRect->top)) && (y < (ulBitmapHeight - pRect->bottom - 1)))
				{
					//in middle of bitmap: source alpha = 0 (transparent). 
					// This means multiply each color component by 0x00. 
					// Thus, after AlphaBlend, we have A, 0x00 * R,  
					// 0x00 * G,and 0x00 * B (which is 0x00000000) 
					// for now, set all pixels to red 
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x7F0000FF;// 0x00FF0000;
				}
				else
				{
					// in the rest of bitmap, source alpha = 0xFF (opaque)  
					// and set all pixels to blue  
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x7F00FF00;// 0xFF0000FF;
				}
			}
		}

		abrc.RCDST.left = 0;
		abrc.RCDST.top = 0;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = MY_AC_SRC_ALPHA;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0xFF;// use constant alpha, with 75% opaqueness  

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}

		SaveBitmapToFile(abrc.HDCDST, hBitmap, _T("d:\\test.bmp"));

		// do cleanup 
		DeleteObject(hBitmap);
		DeleteDC(abrc.HDCSRC);
		abrc.HDCSRC = NULL;

		return hBitmap;
	}
	
#define ARGB(uA,uR,uG,uB) ((UCHAR)(uA) << 0x18) | ((UCHAR)(uR) << 0x10) | ((UCHAR)(uG) << 0x08) | ((UCHAR)(uB) << 0x00)

	__inline static
		HBITMAP DrawAlphaBlendRect(HWND hWnd, ULONG(&uARGB)[2], HDC hDCWnd, RECT * pRect)
	{
		typedef struct _ALPHABLENDRECT {
			HDC HDCDST;
			RECT RCDST;
			HDC HDCSRC;
			RECT RCSRC;
			BLENDFUNCTION BF;// structure for alpha blending 
		}ALPHABLENDRECT, *PALPHABLENDRECT;
		UINT32 x = 0;// stepping variables
		UINT32 y = 0;// stepping variables 
		UCHAR uA = 0x00;// used for doing transparent gradient 
		UCHAR uR = 0x00;
		UCHAR uG = 0x00;
		UCHAR uB = 0x00;
		float fAF = 0.0f;// used to do premultiply
		VOID * pvBits = 0;// pointer to DIB section
		RECT rcWnd = { 0 };// used for getting window dimensions
		HBITMAP hBitmap = NULL;// bitmap handle
		BITMAPINFO bmi = { 0 };// bitmap header
		ULONG ulWindowWidth = 0;// window width/height
		ULONG ulBitmapWidth = 0;// bitmap width/height
		ULONG ulWindowHeight = 0;// window width/height
		ULONG ulBitmapHeight = 0;// bitmap width/height
		ALPHABLENDRECT abrc = { 0 };

		// get dest dc
		abrc.HDCDST = hDCWnd;

		// get window dimensions 
		::GetClientRect(hWnd, &rcWnd);

		// calculate window width/height 
		ulWindowWidth = rcWnd.right - rcWnd.left;
		ulWindowHeight = rcWnd.bottom - rcWnd.top;

		// make sure we have at least some window size 
		if ((!ulWindowWidth) || (!ulWindowHeight))
		{
			return NULL;
		}

		// create a DC for our bitmap -- the source DC for AlphaBlend  
		abrc.HDCSRC = ::CreateCompatibleDC(abrc.HDCDST);

		// zero the memory for the bitmap info 
		::ZeroMemory(&bmi, sizeof(BITMAPINFO));

		// setup bitmap info  
		// set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = ulBitmapWidth = ulWindowWidth;
		bmi.bmiHeader.biHeight = ulBitmapHeight = ulWindowHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;// four 8-bit components 
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

		// create our DIB section and select the bitmap into the dc 
		hBitmap = ::CreateDIBSection(abrc.HDCSRC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x00);
		::SelectObject(abrc.HDCSRC, hBitmap);

		// in middle window area, constant alpha = 100% (disabled), source  
		// alpha is 0 in middle of bitmap and opaque in rest of bitmap  
		for (x = 0; x < ulBitmapWidth; x++)
		{
			for (y = 0; y < ulBitmapHeight; y++)
			{
				if ((x >(int)(pRect->right)) && (x < (ulBitmapWidth - pRect->left - 1)) &&
					(y >(int)(pRect->bottom)) && (y < (ulBitmapHeight - pRect->top - 1)))
				{
					//in middle of bitmap: source alpha = 0 (transparent). 
					// This means multiply each color component by 0x00. 
					// Thus, after AlphaBlend, we have A, 0x00 * R,  
					// 0x00 * G,and 0x00 * B (which is 0x00000000) 
					// for now, set all pixels to red 
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = uARGB[0];//0x7F0000FF;// 0x00FF0000;
				}
				else
				{
					// in the rest of bitmap, source alpha = 0xFF (opaque)  
					// and set all pixels to blue  
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = uARGB[1];//0x7F00FF00;// 0xFF0000FF;
				}
			}
		}

		abrc.RCDST.left = 0;
		abrc.RCDST.top = 0;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = AC_SRC_ALPHA;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0xFF;// use constant alpha, with 75% opaqueness  

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}

		//SaveBitmapToFile(abrc.HDCDST, hBitmap, _T("d:\\test.bmp"));

		// do cleanup 
		DeleteObject(hBitmap);
		DeleteDC(abrc.HDCSRC);
		abrc.HDCSRC = NULL;

		return hBitmap;
	}

	__inline static
		HFONT CreatePaintFont(LPCTSTR ptszFaceName = _T("宋体"),
		LONG lfHeight = 12,
		LONG lfWidth = 0,
		LONG lfEscapement = 0,
		LONG lfOrientation = 0,
		LONG lfWeight = FW_NORMAL,
		BYTE lfItalic = FALSE,
		BYTE lfUnderline = FALSE,
		BYTE lfStrikeOut = FALSE,
		BYTE lfCharSet = ANSI_CHARSET,
		BYTE lfOutPrecision = OUT_DEFAULT_PRECIS,
		BYTE lfClipPrecision = CLIP_DEFAULT_PRECIS,
		BYTE lfQuality = DEFAULT_QUALITY,
		BYTE lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS)
	{
		return CreateFont(lfHeight, lfWidth,
			lfEscapement,
			lfOrientation,
			lfWeight,
			lfItalic,
			lfUnderline,
			lfStrikeOut,
			lfCharSet,
			lfOutPrecision,
			lfClipPrecision,
			lfQuality,
			lfPitchAndFamily,
			ptszFaceName);
	}
	__inline static
		void RectDrawText(HDC hDC, RECT * pRect, LPCTSTR ptszText, COLORREF clrTextColor = COLOR_WINDOWTEXT, HFONT hFont = NULL, UINT uFormat = DT_LEFT | DT_WORDBREAK)
	{
		HFONT hFontOld = NULL;

		if (hFont)
		{
			hFontOld = (HFONT)SelectObject(hDC, hFont);

			SetBkMode(hDC, TRANSPARENT);
			SetTextColor(hDC, clrTextColor);

			DrawText(hDC, ptszText, lstrlen(ptszText), pRect, uFormat);

			(HFONT)SelectObject(hDC, hFontOld);
		}
	}

	__inline static
		HGLOBAL OpenResource(LPVOID & lpData, DWORD & dwSize, HMODULE hModule, LPCTSTR lpName, LPCTSTR lpType)
	{
		BOOL bResult = FALSE;
		HRSRC hRsrcRes = NULL;// handle/ptr. to res. info. in hSource 
		HGLOBAL hGLOBAL = NULL;// handle to loaded resource

		// Locate the resource in the source image file. 
		hRsrcRes = ::FindResource(hModule, lpName, lpType);
		if (hRsrcRes == NULL)
		{
			goto __LEAVE_CLEAN__;
		}

		// Load the resource into global memory. 
		hGLOBAL = ::LoadResource(hModule, hRsrcRes);
		if (hGLOBAL == NULL)
		{
			goto __LEAVE_CLEAN__;
		}

		// Lock the resource into global memory. 
		lpData = ::LockResource(hGLOBAL);
		if (lpData == NULL)
		{
			FreeResource(hGLOBAL);
			hGLOBAL = NULL;
			goto __LEAVE_CLEAN__;
		}

		dwSize = ::SizeofResource(hModule, hRsrcRes);

	__LEAVE_CLEAN__:

		return hGLOBAL;
	}

	__inline static
		void CloseResource(HGLOBAL & hGlobal)
	{
		if (hGlobal)
		{
			FreeResource(hGlobal);
			hGlobal = NULL;
		}
	}

	__inline static
		BOOL ParseResrc(LPCTSTR ptszFileName, UINT uResourceID, LPCTSTR ptszTypeName)
	{
		DWORD dwSize = 0;
		HANDLE hFile = NULL;
		BOOL bResult = FALSE;
		LPVOID lpData = NULL;
		HGLOBAL hGlobal = NULL;
		DWORD dwByteWritten = 0;

		hGlobal = OpenResource(lpData, dwSize, GetModuleHandle(NULL), MAKEINTRESOURCE(uResourceID), ptszTypeName);

		//我们用刚才得到的pBuffer和dwSize来做一些需要的事情。可以直接在内存中使
		//用，也可以写入到硬盘文件。这里我们简单的写入到硬盘文件，如果我们的自定
		//义资源是作为嵌入DLL来应用，情况可能要复杂一些。
		hFile = CreateFile(ptszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile && hGlobal && dwSize > 0)
		{
			WriteFile(hFile, hGlobal, dwSize, &dwByteWritten, NULL);
			CloseHandle(hFile);

			bResult = TRUE;
		}

		CloseResource(hGlobal);

		return bResult;
	}

	__inline static
		BOOL DragMove(HWND hWnd, RECT * pRect)
	{
		POINT pt = { 0 };
		BOOL bResult = FALSE;
		RECT rcWndOuter = { 0 };
		RECT rcWndInner = { 0 };
		GetCursorPos(&pt);
		::GetWindowRect(hWnd, &rcWndOuter);

		rcWndInner.left = rcWndOuter.left + pRect->left;
		rcWndInner.top = rcWndOuter.top + pRect->top;
		rcWndInner.right = rcWndOuter.right - pRect->right;
		rcWndInner.bottom = rcWndOuter.bottom - pRect->bottom;

		if ((PtInRect(&rcWndOuter, pt) && !PtInRect(&rcWndInner, pt)))
		{
#ifndef _SYSCOMMAND_SC_DRAGMOVE
#define _SYSCOMMAND_SC_DRAGMOVE  0xF012
#endif // !_SYSCOMMAND_SC_DRAGMOVE
			::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, NULL, 0);
			::SendMessage(hWnd, WM_SYSCOMMAND, _SYSCOMMAND_SC_DRAGMOVE, 0);
			bResult = TRUE;
#ifdef _SYSCOMMAND_SC_DRAGMOVE
#undef _SYSCOMMAND_SC_DRAGMOVE  
#endif // !_SYSCOMMAND_SC_DRAGMOVE
		}

		return bResult;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能：设置圆角窗口
	//函数参数：
	//	hWnd		要设置的窗口句柄
	//	pstEllipse	要设置圆角的横向半径和纵向半径
	//	prcExcepted	要排除圆角的左上右下侧大小
	//返回值：无返回
	__inline static void SetWindowEllispeFrame(HWND hWnd, SIZE * pszEllipse = 0, RECT * prcExcepted = 0)
	{
		HRGN hRgnWindow = 0;
		POINT ptPosition = { 0, 0 };
		RECT rcWindow = { 0, 0, 0, 0 };

		::GetWindowRect(hWnd, &rcWindow);
		if (prcExcepted)
		{
			ptPosition.x = prcExcepted->left;
			ptPosition.y = prcExcepted->top;
			rcWindow.left += prcExcepted->left;
			rcWindow.top += prcExcepted->top;
			rcWindow.right -= prcExcepted->right;
			rcWindow.bottom -= prcExcepted->bottom;
		}

		hRgnWindow = ::CreateRoundRectRgn(ptPosition.x, ptPosition.y, \
			rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, pszEllipse->cx, pszEllipse->cy);
		if (hRgnWindow)
		{
			::SetWindowRgn(hWnd, hRgnWindow, TRUE);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：不失真模式获取HWND的HBITMAP
	// 参    数：窗口句柄
	// 返 回 值：返回HBITMAP
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBITMAPFromHWND(HWND hWnd)
	{
		INT nX = 0;
		INT nY = 0;
		INT nWidth = 0;
		INT nHeight = 0;
		RECT rect = { 0 };
		INT nMode = 0;
		HDC hWndDC = NULL;
		HDC hMemDC = NULL;
		HBITMAP hOldBitmap = NULL;
		HBITMAP hNewBitmap = NULL;

		GetClientRect(hWnd, &rect);

		nX = rect.left;
		nY = rect.top;
		nWidth = rect.right - rect.left;
		nHeight = rect.bottom - rect.top;

		hWndDC = GetDC(hWnd);

		// 为屏幕设备描述表创建兼容的内存设备描述表
		hMemDC = CreateCompatibleDC(hWndDC);

		// 创建一个与屏幕设备描述表兼容的位图
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// 把新位图选到内存设备描述表中
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// 设置不失真模式
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// 把屏幕设备描述表拷贝到内存设备描述表中
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// 设置不失真模式
		SetStretchBltMode(hMemDC, nMode);

		// 得到屏幕位图的句柄
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// 清除DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// 释放DC
		ReleaseDC(hWnd, hWndDC);

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：不失真模式获取HWND的HBITMAP
	// 参    数：窗口句柄，指定坐标位置及大小
	// 返 回 值：返回HBITMAP
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBITMAPFromHWND(HWND hWnd, int nX, int nY,
		int nWidth, int nHeight)
	{
		INT nMode = 0;
		HDC hWndDC = NULL;
		HDC hMemDC = NULL;
		HBITMAP hOldBitmap = NULL;
		HBITMAP hNewBitmap = NULL;

		hWndDC = GetDC(hWnd);

		// 为屏幕设备描述表创建兼容的内存设备描述表
		hMemDC = CreateCompatibleDC(hWndDC);

		// 创建一个与屏幕设备描述表兼容的位图
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// 把新位图选到内存设备描述表中
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// 设置不失真模式
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// 把屏幕设备描述表拷贝到内存设备描述表中
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// 设置不失真模式
		SetStretchBltMode(hMemDC, nMode);

		// 得到屏幕位图的句柄
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// 清除DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// 释放DC
		ReleaseDC(hWnd, hWndDC);

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：不失真模式获取HWND的HBITMAP
	// 参    数：窗口句柄，指定坐标位置及大小
	// 返 回 值：返回HBITMAP
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBITMAPFromHWND(HWND hWnd, int nX, int nY,
		int nWidth, int nHeight,
		HGDIOBJ * phGdiObj)
	{
		INT nMode = 0;
		HDC hWndDC = NULL;
		HDC hMemDC = NULL;
		HBITMAP hOldBitmap = NULL;
		HBITMAP hNewBitmap = NULL;

		hWndDC = GetDC(hWnd);

		// 为屏幕设备描述表创建兼容的内存设备描述表
		hMemDC = CreateCompatibleDC(hWndDC);

		// 创建一个与屏幕设备描述表兼容的位图
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// 把新位图选到内存设备描述表中
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// 设置不失真模式
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// 把屏幕设备描述表拷贝到内存设备描述表中
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// 设置不失真模式
		SetStretchBltMode(hMemDC, nMode);

		// 得到屏幕位图的句柄
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// 清除DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// 释放DC
		ReleaseDC(hWnd, hWndDC);

		if (phGdiObj)
		{
			if ((*phGdiObj))
			{
				DeleteObject((*phGdiObj));
				(*phGdiObj) = NULL;
			}
			(*phGdiObj) = hNewBitmap;
		}

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：不失真模式获取HDC的HBITMAP
	// 参    数：设备DC，大小
	// 返 回 值：返回HBITMAP
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBitmapFromHdc(HDC hDC, DWORD BitWidth, DWORD BitHeight)
	{
		HDC hMemDC;
		INT nMode = 0;
		HBITMAP hBitmap, hBitTemp;
		//创建设备上下文(HDC)
		hMemDC = CreateCompatibleDC(hDC);
		//创建HBITMAP
		hBitmap = CreateCompatibleBitmap(hDC, BitWidth, BitHeight);
		hBitTemp = (HBITMAP)SelectObject(hMemDC, hBitmap);

		nMode = SetStretchBltMode(hDC, COLORONCOLOR);//设置不失真模式
		//得到位图缓冲区
		StretchBlt(hMemDC, 0, 0, BitWidth, BitHeight, hDC, 0, 0, BitWidth, BitHeight, SRCCOPY);
		SetStretchBltMode(hDC, nMode);
		//得到最终的位图信息
		hBitmap = (HBITMAP)SelectObject(hMemDC, hBitTemp);
		//释放内存
		DeleteObject(hBitTemp);
		DeleteDC(hMemDC);

		return hBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：获取HWND到IStream数据流,返回内存句柄(使用完毕需手动释放ReleaseGlobalHandle)
	// 参    数：窗口句柄,输出IStream流
	// 返 回 值：返回HANDLE.(使用完毕需手动释放ReleaseGlobalHandle)
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HANDLE HWNDToIStream(HWND hWnd, IStream ** ppStream)
	{
		BITMAP bmp = { 0 };
		HANDLE hFile = NULL;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		HANDLE hMemoryDIB = 0;
		BYTE *lpBitmap = NULL;
		DWORD dwBytesWritten = 0;
		IStream * pStream = NULL;
		HDC hWndDC = GetDC(hWnd);
		HBITMAP hWndBitmap = HBITMAPFromHWND(hWnd);

		// Get the BITMAP from the HBITMAP
		GetObject(hWndBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hMemoryDIB = GlobalAlloc(GHND, dwFileSize);
		if (hMemoryDIB)
		{
			lpBitmap = (BYTE *)GlobalLock(hMemoryDIB);

			memcpy(lpBitmap, &bfh, sizeof(BITMAPFILEHEADER));
			memcpy(lpBitmap + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

			// Gets the "bits" from the bitmap and copies them into a buffer
			// which is pointed to by lpBitmap.
			GetDIBits(hWndDC,
				hWndBitmap,
				0,
				(UINT)bmp.bmHeight,
				lpBitmap + bfh.bfOffBits,
				(BITMAPINFO *)&bih, DIB_RGB_COLORS);

			CreateStreamOnHGlobal(lpBitmap, FALSE, ppStream);
		}

		return hMemoryDIB;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：获取HWND到IStream数据流,返回内存句柄(使用完毕需手动释放ReleaseGlobalHandle)
	// 参    数：窗口句柄,指定坐标及大小，输出IStream流
	// 返 回 值：返回HANDLE.(使用完毕需手动释放ReleaseGlobalHandle)
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HANDLE HWNDToIStream(HWND hWnd, int nX, int nY,
		int nWidth, int nHeight, IStream ** ppStream)
	{
		BITMAP bmp = { 0 };
		HANDLE hFile = NULL;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		HANDLE hMemoryDIB = 0;
		BYTE *lpBitmap = NULL;
		DWORD dwBytesWritten = 0;
		IStream * pStream = NULL;
		HDC hWndDC = GetDC(hWnd);
		HBITMAP hWndBitmap = HBITMAPFromHWND(hWnd, nX, nY, nWidth, nHeight);

		// Get the BITMAP from the HBITMAP
		GetObject(hWndBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hMemoryDIB = GlobalAlloc(GHND, dwFileSize);
		if (hMemoryDIB)
		{
			lpBitmap = (BYTE *)GlobalLock(hMemoryDIB);

			memcpy(lpBitmap, &bfh, sizeof(BITMAPFILEHEADER));
			memcpy(lpBitmap + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

			// Gets the "bits" from the bitmap and copies them into a buffer
			// which is pointed to by lpBitmap.
			GetDIBits(hWndDC,
				hWndBitmap,
				0,
				(UINT)bmp.bmHeight,
				lpBitmap + bfh.bfOffBits,
				(BITMAPINFO *)&bih, DIB_RGB_COLORS);

			CreateStreamOnHGlobal(lpBitmap, FALSE, ppStream);
		}

		return hMemoryDIB;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：释放内存句柄
	// 参    数：窗口句柄,输出IStream流
	// 返 回 值：无返回值
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static void ReleaseGlobalHandle(HANDLE * hMemoryHandle)
	{
		if (hMemoryHandle && (*hMemoryHandle))
		{
			GlobalUnlock((*hMemoryHandle));
			(*hMemoryHandle) = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：保存HBITMAP到BMP图片文件
	// 参    数：窗口句柄,要保存文件名称
	// 返 回 值：bool类型。成功返回true;失败返回false;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static bool HBitmapToFile(HBITMAP hBitmap, const TCHAR *pFileName)
	{
		BITMAP bmp = { 0 };
		WORD wBitCount = 0;
		HANDLE hFile = NULL;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BYTE *lpBitmap = NULL;
		HANDLE hDIBData = NULL;
		DWORD dwBytesWritten = 0;
		BITMAPFILEHEADER bfh = { 0 };
		BITMAPINFOHEADER bih = { 0 };

		HDC hDC = GetWindowDC(NULL);

		//计算位图文件每个像素所占字节数
		wBitCount = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

		if (wBitCount <= 1)
		{
			wBitCount = 1;
		}
		else if (wBitCount <= 4)
		{
			wBitCount = 4;
		}
		else if (wBitCount <= 8)
		{
			wBitCount = 8;
		}
		else
		{
			wBitCount = 24;
		}

		// Get the BITMAP from the HBITMAP
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = wBitCount;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hDIBData = GlobalAlloc(GHND, dwFileSize);
		if (!hDIBData)
		{
			return false;
		}
		lpBitmap = (BYTE *)GlobalLock(hDIBData);

		memcpy(lpBitmap, &bfh, sizeof(BITMAPFILEHEADER));
		memcpy(lpBitmap + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

		// Gets the "bits" from the bitmap and copies them into a buffer
		// which is pointed to by lpBitmap.
		GetDIBits(hDC,
			hBitmap,
			0,
			(UINT)bmp.bmHeight,
			lpBitmap + bfh.bfOffBits,
			(BITMAPINFO *)&bih, DIB_RGB_COLORS);

		// A file is created, this is where we will save the screen capture.
		hFile = CreateFile(pFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		WriteFile(hFile, (LPSTR)lpBitmap, dwFileSize, &dwBytesWritten, NULL);

		//Unlock and Free the DIB from the heap
		GlobalUnlock(hDIBData);
		GlobalFree(hDIBData);
		hDIBData = NULL;

		//Close the handle for the file that was created
		CloseHandle(hFile);
		hFile = NULL;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：保存HBITMAP到BMP图片文件
	// 参    数：位图句柄,要保存文件名称
	// 返 回 值：bool类型。成功返回true;失败返回false;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static bool HBitmapToFileEx(HBITMAP hBitmap, const TCHAR * ptFileName)
	{
		HDC hDC = NULL;
		BITMAP bmp = { 0 };
		WORD wBitsCount = 0;
		bool result = false;
		HANDLE hFile = NULL;
		HANDLE hDIBData = 0;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		BYTE *lpByteData = NULL;
		DWORD dwBytesWritten = 0;
		HPALETTE hPalette = NULL;
		HPALETTE hPaletteBackup = NULL;

		//计算位图文件每个像素所占字节数
		hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
		wBitsCount = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

		if (wBitsCount <= 1)
		{
			wBitsCount = 1;
		}
		else if (wBitsCount <= 4)
		{
			wBitsCount = 4;
		}
		else if (wBitsCount <= 8)
		{
			wBitsCount = 8;
		}
		else
		{
			wBitsCount = 24;
		}
		// Get the BITMAP from the HBITMAP
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = wBitsCount;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hDIBData = GlobalAlloc(GHND, dwFileSize);
		if (hDIBData)
		{
			lpByteData = (BYTE *)GlobalLock(hDIBData);

			memcpy(lpByteData, &bfh, sizeof(BITMAPFILEHEADER));
			memcpy(lpByteData + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

			// Gets default palette
			hPalette = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
			if (hPalette)
			{
				hDC = ::GetDC(NULL);
				hPaletteBackup = ::SelectPalette(hDC, (HPALETTE)hPalette, FALSE);
				::RealizePalette(hDC);
			}

			// Gets the "bits" from the bitmap and copies them into a buffer
			// which is pointed to by lpByteData.
			GetDIBits(hDC,
				hBitmap,
				0,
				(UINT)bmp.bmHeight,
				lpByteData + bfh.bfOffBits,
				(BITMAPINFO *)&bih, DIB_RGB_COLORS);
			// Recover original palette
			if (hPaletteBackup)
			{
				::SelectPalette(hDC, (HPALETTE)hPaletteBackup, TRUE);
				::RealizePalette(hDC);
				::ReleaseDC(NULL, hDC);
			}

			// A file is created, this is where we will save the screen capture.
			hFile = CreateFile(ptFileName,
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				WriteFile(hFile, (void *)lpByteData, dwFileSize, &dwBytesWritten, NULL);

				//Close the handle for the file that was created
				CloseHandle(hFile);
				hFile = NULL;

				result = true;
			}

			//Unlock and Free the DIB from the heap
			GlobalUnlock(hDIBData);
			GlobalFree(hDIBData);
			hDIBData = NULL;
		}

		return result;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：保存HWND到BMP图片文件
	// 参    数：窗口句柄,要保存文件名称
	// 返 回 值：bool类型。成功返回true;失败返回false;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static bool HWNDToFile(HWND hWnd, TCHAR * ptFileName)
	{
		BITMAP bmp = { 0 };
		bool result = false;
		HANDLE hFile = NULL;
		HANDLE hDIBData = 0;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		BYTE *lpByteData = NULL;
		DWORD dwBytesWritten = 0;
		HDC hWndDC = GetDC(hWnd);
		HBITMAP hBitmap = HBITMAPFromHWND(hWnd);

		// Get the BITMAP from the HBITMAP
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hDIBData = GlobalAlloc(GHND, dwFileSize);
		if (hDIBData)
		{
			lpByteData = (BYTE *)GlobalLock(hDIBData);

			memcpy(lpByteData, &bfh, sizeof(BITMAPFILEHEADER));
			memcpy(lpByteData + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

			// Gets the "bits" from the bitmap and copies them into a buffer
			// which is pointed to by lpByteData.
			GetDIBits(hWndDC,
				hBitmap,
				0,
				(UINT)bmp.bmHeight,
				lpByteData + bfh.bfOffBits,
				(BITMAPINFO *)&bih, DIB_RGB_COLORS);

			// A file is created, this is where we will save the screen capture.
			hFile = CreateFile(ptFileName,
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				WriteFile(hFile, (void *)lpByteData, dwFileSize, &dwBytesWritten, NULL);

				//Close the handle for the file that was created
				CloseHandle(hFile);
				hFile = NULL;

				result = true;
			}

			//Unlock and Free the DIB from the heap
			GlobalUnlock(hDIBData);
			GlobalFree(hDIBData);
			hDIBData = NULL;
		}

		return result;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：显示HBITMAP到HWND上
	// 参    数：窗口句柄,HBITMAP位图句柄
	// 返 回 值：bool类型。成功返回true;失败返回false;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP showBitmap(HWND hStaticWnd, HBITMAP hStaticBitmap)
	{
		INT nMode = 0;
		RECT rect = { 0 };
		BITMAP bmp = { 0 };
		HDC hParentDC = NULL;
		HDC hStaticDC = NULL;
		HDC hMemoryDC = NULL;
		HWND hParentWnd = NULL;
		HBITMAP hOldBitmap = NULL;
		HBITMAP hNewBitmap = NULL;

		hParentWnd = GetParent(hStaticWnd);

		//获取图片显示框的大小
		GetClientRect(hStaticWnd, &rect);

		//获取位图的大小信息
		GetObject(hStaticBitmap, sizeof(bmp), &bmp);

		if ((rect.right - rect.left) >= bmp.bmWidth &&
			(rect.bottom - rect.top) >= bmp.bmHeight)
		{
			return  hNewBitmap;
		}

		hParentDC = GetDC(hParentWnd);
		hStaticDC = GetDC(hStaticWnd);

		hMemoryDC = CreateCompatibleDC(hParentDC);

		hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hStaticBitmap);

		//设置不失真模式
		nMode = SetStretchBltMode(hStaticDC, COLORONCOLOR);

		StretchBlt(hStaticDC, rect.left, rect.top, rect.right - rect.left,
			rect.bottom - rect.top, hMemoryDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		SetStretchBltMode(hStaticDC, nMode);

		DeleteDC(hMemoryDC);
		ReleaseDC(hStaticWnd, hStaticDC);
		ReleaseDC(hParentWnd, hParentDC);

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：显示HBITMAP到HWND上
	// 参    数：主窗口句柄,源子控件ID，目标子控件ID
	// 返 回 值：无返回值;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static void showPicture(HWND hWnd, INT nSrcID, INT nDstID)
	{
		RECT rect = { 0 };
		HGDIOBJ hOldBitmap = NULL;
		HGDIOBJ hNewBitmap = NULL;
		hNewBitmap = HBITMAPFromHWND(GetDlgItem(hWnd, nSrcID));
		if (hNewBitmap)
		{
			hOldBitmap = (HGDIOBJ)SendDlgItemMessage(hWnd, nDstID, STM_SETIMAGE,
				(WPARAM)IMAGE_BITMAP, (LPARAM)hNewBitmap);
			if (hOldBitmap)
			{
				DeleteObject(hOldBitmap);
				hOldBitmap = NULL;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：显示HBITMAP到HWND上
	// 参    数：主窗口句柄,HBITMAP句柄，目标子控件ID
	// 返 回 值：无返回值;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static void showPicture(HWND hWnd, HGDIOBJ hBitmap, INT nDstID)
	{
		RECT rect = { 0 };
		HGDIOBJ hOldBitmap = NULL;

		if (hBitmap)
		{
			hOldBitmap = (HGDIOBJ)SendDlgItemMessage(hWnd, nDstID, STM_SETIMAGE,
				(WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
			if (hOldBitmap)
			{
				DeleteObject(hOldBitmap);
				hOldBitmap = NULL;
			}
		}
	}

	// Helper to calculate the alpha-premultiled value for a pixel
	__inline static DWORD PreMultiply(COLORREF cl, unsigned char nAlpha)
	{
		// It's strange that the byte order of RGB in 32b BMP is reverse to in COLORREF
		return (GetRValue(cl) * (DWORD)nAlpha / 255) << 16 |
			(GetGValue(cl) * (DWORD)nAlpha / 255) << 8 |
			(GetBValue(cl) * (DWORD)nAlpha / 255);
	}

	__inline static void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent,
		INT nDarkness = 150,
		INT nSharpness = 5,
		INT nSize = 12,
		INT nxOffset = 5,
		INT nyOffset = 5,
		COLORREF Color = RGB(255, 0, 0))
	{
		// The shadow algorithm:
		// Get the region of parent window,
		// Apply morphologic erosion to shrink it into the size (ShadowWndSize - Sharpness)
		// Apply modified (with blur effect) morphologic dilation to make the blurred border
		// The algorithm is optimized by assuming parent window is just "one piece" and without "wholes" on it

		RECT rcRegion = { 0, 0, rcParent->right - rcParent->left, rcParent->bottom - rcParent->top };

		// Get the region of parent window,
		HRGN hParentRgn = CreateRectRgn(rcRegion.left, rcRegion.top, rcRegion.right, rcRegion.bottom);
		GetWindowRgn(hParent, hParentRgn);

		// Determine the Start and end point of each horizontal scan line
		SIZE szParent = { rcParent->right - rcParent->left, rcParent->bottom - rcParent->top };
		SIZE szShadow = { szParent.cx + 2 * nSize, szParent.cy + 2 * nSize };
		// Extra 2 lines (set to be empty) in ptAnchors are used in dilation
		int nAnchors = max(szParent.cy, szShadow.cy);    // # of anchor points pares
		int(*ptAnchors)[2] = new int[nAnchors + 2][2];
		int(*ptAnchorsOri)[2] = new int[szParent.cy][2];    // anchor points, will not modify during erosion
		ptAnchors[0][0] = szParent.cx;
		ptAnchors[0][1] = 0;
		ptAnchors[nAnchors + 1][0] = szParent.cx;
		ptAnchors[nAnchors + 1][1] = 0;
		if (nSize > 0)
		{
			// Put the parent window anchors at the center
			for (int i = 0; i < nSize; i++)
			{
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchors[i + 1][1] = 0;
				ptAnchors[szShadow.cy - i][0] = szParent.cx;
				ptAnchors[szShadow.cy - i][1] = 0;
			}
			ptAnchors += nSize;
		}
		for (int i = 0; i < szParent.cy; i++)
		{
			// find start point
			int j;
			for (j = 0; j < szParent.cx; j++)
			{
				if (PtInRegion(hParentRgn, j, i))
				{
					ptAnchors[i + 1][0] = j + nSize;
					ptAnchorsOri[i][0] = j;
					break;
				}
			}

			if (j >= szParent.cx)    // Start point not found
			{
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchorsOri[i][1] = 0;
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchorsOri[i][1] = 0;
			}
			else
			{
				// find end point
				for (j = szParent.cx - 1; j >= ptAnchors[i + 1][0]; j--)
				{
					if (PtInRegion(hParentRgn, j, i))
					{
						ptAnchors[i + 1][1] = j + nSize;
						ptAnchorsOri[i][1] = j + 1;
						break;
					}
				}
			}
		}

		if (nSize > 0)
		{
			ptAnchors -= nSize;    // Restore pos of ptAnchors for erosion
		}
		int(*ptAnchorsTmp)[2] = new int[nAnchors + 2][2];    // Store the result of erosion
		// First and last line should be empty
		ptAnchorsTmp[0][0] = szParent.cx;
		ptAnchorsTmp[0][1] = 0;
		ptAnchorsTmp[nAnchors + 1][0] = szParent.cx;
		ptAnchorsTmp[nAnchors + 1][1] = 0;
		int nEroTimes = 0;
		// morphologic erosion
		for (int i = 0; i < nSharpness - nSize; i++)
		{
			nEroTimes++;
			//ptAnchorsTmp[1][0] = szParent.cx;
			//ptAnchorsTmp[1][1] = 0;
			//ptAnchorsTmp[szParent.cy + 1][0] = szParent.cx;
			//ptAnchorsTmp[szParent.cy + 1][1] = 0;
			for (int j = 1; j < nAnchors + 1; j++)
			{
				ptAnchorsTmp[j][0] = max(ptAnchors[j - 1][0], max(ptAnchors[j][0], ptAnchors[j + 1][0])) + 1;
				ptAnchorsTmp[j][1] = min(ptAnchors[j - 1][1], min(ptAnchors[j][1], ptAnchors[j + 1][1])) - 1;
			}
			// Exchange ptAnchors and ptAnchorsTmp;
			int(*ptAnchorsXange)[2] = ptAnchorsTmp;
			ptAnchorsTmp = ptAnchors;
			ptAnchors = ptAnchorsXange;
		}

		// morphologic dilation
		ptAnchors += (nSize < 0 ? -nSize : 0) + 1;    // now coordinates in ptAnchors are same as in shadow window
		// Generate the kernel
		int nKernelSize = nSize > nSharpness ? nSize : nSharpness;
		int nCenterSize = nSize > nSharpness ? (nSize - nSharpness) : 0;
		UINT32 *pKernel = new UINT32[(2 * nKernelSize + 1) * (2 * nKernelSize + 1)];
		UINT32 *pKernelIter = pKernel;
		for (int i = 0; i <= 2 * nKernelSize; i++)
		{
			for (int j = 0; j <= 2 * nKernelSize; j++)
			{
				double dLength = sqrt((i - nKernelSize) * (i - nKernelSize) + (j - nKernelSize) * (double)(j - nKernelSize));
				if (dLength < nCenterSize)
				{
					*pKernelIter = nDarkness << 24 | PreMultiply(Color, nDarkness);
				}
				else if (dLength <= nKernelSize)
				{
					UINT32 nFactor = ((UINT32)((1 - (dLength - nCenterSize) / (nSharpness + 1)) * nDarkness));
					*pKernelIter = nFactor << 24 | PreMultiply(Color, nFactor);
				}
				else
				{
					*pKernelIter = 0;
				}
				pKernelIter++;
			}
		}
		// Generate blurred border
		for (int i = nKernelSize; i < szShadow.cy - nKernelSize; i++)
		{
			int j = 0;
			if (ptAnchors[i][0] < ptAnchors[i][1])
			{
				// Start of line
				for (j = ptAnchors[i][0];
					j < min(max(ptAnchors[i - 1][0], ptAnchors[i + 1][0]) + 1, ptAnchors[i][1]);
					j++)
				{
					for (int k = 0; k <= 2 * nKernelSize; k++)
					{
						UINT32 *pPixel = pShadBits +
							(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
						UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
						for (int l = 0; l <= 2 * nKernelSize; l++)
						{
							if (*pPixel < *pKernelPixel)
							{
								*pPixel = *pKernelPixel;
							}
							pPixel++;
							pKernelPixel++;
						}
					}
				}    // for() start of line

				// End of line
				for (j = max(j, min(ptAnchors[i - 1][1], ptAnchors[i + 1][1]) - 1);
					j < ptAnchors[i][1];
					j++)
				{
					for (int k = 0; k <= 2 * nKernelSize; k++)
					{
						UINT32 *pPixel = pShadBits +
							(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
						UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
						for (int l = 0; l <= 2 * nKernelSize; l++)
						{
							if (*pPixel < *pKernelPixel)
							{
								*pPixel = *pKernelPixel;
							}
							pPixel++;
							pKernelPixel++;
						}
					}
				}    // for() end of line
			}
		}    // for() Generate blurred border

		// Erase unwanted parts and complement missing
		UINT32 clCenter = nDarkness << 24 | PreMultiply(Color, nDarkness);
		for (int i = min(nKernelSize, max(nSize - nyOffset, 0));
			i < max(szShadow.cy - nKernelSize, min(szParent.cy + nSize - nyOffset, szParent.cy + 2 * nSize));
			i++)
		{
			UINT32 *pLine = pShadBits + (szShadow.cy - i - 1) * szShadow.cx;
			if (i - nSize + nyOffset < 0 || i - nSize + nyOffset >= szParent.cy)    // Line is not covered by parent window
			{
				for (int j = ptAnchors[i][0]; j < ptAnchors[i][1]; j++)
				{
					*(pLine + j) = clCenter;
				}
			}
			else
			{
				for (int j = ptAnchors[i][0];
					j < min(ptAnchorsOri[i - nSize + nyOffset][0] + nSize - nxOffset, ptAnchors[i][1]);
					j++)
					*(pLine + j) = clCenter;
				for (int j = max(ptAnchorsOri[i - nSize + nyOffset][0] + nSize - nxOffset, 0);
					j < min((long)ptAnchorsOri[i - nSize + nyOffset][1] + nSize - nxOffset, szShadow.cx);
					j++)
					*(pLine + j) = 0;
				for (int j = max(ptAnchorsOri[i - nSize + nyOffset][1] + nSize - nxOffset, ptAnchors[i][0]);
					j < ptAnchors[i][1];
					j++)
					*(pLine + j) = clCenter;
			}
		}

		// Delete used resources
		delete[](ptAnchors - (nSize < 0 ? -nSize : 0) - 1);
		delete[] ptAnchorsTmp;
		delete[] ptAnchorsOri;
		delete[] pKernel;
		DeleteObject(hParentRgn);
	}

	__inline static HBITMAP CreateShadowBitmap(HWND hParent,
		INT nDarkness = 150,
		INT nSharpness = 5,
		INT nSize = 12,
		INT nxOffset = 5,
		INT nyOffset = 5,
		COLORREF Color = RGB(255, 0, 0))
	{
		RECT rc = { 0 };
		BYTE *pvBits = NULL;    // pointer to DIB section
		HBITMAP hBitmap = NULL;

		int nShadowWindowWidth = 0;
		int nShadowWindowHeight = 0;

		// Create the alpha blending bitmap
		BITMAPINFO bmi = { 0 };    // bitmap header
		HDC hDC = GetDC(hParent);
		WORD wBitsCount = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

		if (wBitsCount <= 1)
		{
			wBitsCount = 1;
		}
		else if (wBitsCount <= 4)
		{
			wBitsCount = 4;
		}
		else if (wBitsCount <= 8)
		{
			wBitsCount = 8;
		}
		else if (wBitsCount <= 24)
		{
			wBitsCount = 24;
		}
		else
		{
			wBitsCount = 32;
		}

		GetWindowRect(hParent, &rc);

		nShadowWindowWidth = rc.right - rc.left + nSize * 2;
		nShadowWindowHeight = rc.bottom - rc.top + nSize * 2;

		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = nShadowWindowWidth;
		bmi.bmiHeader.biHeight = nShadowWindowHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = wBitsCount;   // four 8-bit components
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = nShadowWindowWidth * nShadowWindowHeight * 4;

		hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);

		ZeroMemory(pvBits, bmi.bmiHeader.biSizeImage);
		MakeShadow((UINT32 *)pvBits, hParent, &rc,
			nDarkness, nSharpness, nSize, nxOffset,
			nyOffset, Color);
		return hBitmap;
	}
	__inline static BOOL SaveHBitmapToFile(HBITMAP hBitmap, const TCHAR *pFileName)
	{
		HDC hDC = NULL;
		//当前分辨率下每象素所占字节数
		WORD wDPIBits = 0;
		//位图中每象素所占字节数
		WORD wBitCount = 0;
		//定义调色板大小
		DWORD dwPaletteSize = 0;
		//位图中像素字节大小
		DWORD dwBmBitsSize = 0;
		//位图文件大小
		DWORD dwDibBitSize = 0;
		//写入文件字节数
		DWORD dwWritten = 0;
		//位图属性结构
		BITMAP bmp = { 0 };
		//位图文件头结构
		BITMAPFILEHEADER bmpfh = { 0 };
		//位图信息头结构
		BITMAPINFOHEADER bmpih = { 0 };
		//指向位图信息头结构
		BITMAPINFOHEADER * pbmpih = NULL;
		//定义文件
		HANDLE hFile = NULL;
		//分配内存句柄
		HGLOBAL hDibBit = NULL;
		//当前调色板句柄
		HPALETTE hPaltte = NULL;
		//备份调色板句柄
		HPALETTE hPaltteBackup = NULL;

		//计算位图文件每个像素所占字节数
		hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		wDPIBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

		if (wDPIBits <= 1)
		{
			wBitCount = 1;
		}
		else if (wDPIBits <= 4)
		{
			wBitCount = 4;
		}
		else if (wDPIBits <= 8)
		{
			wBitCount = 8;
		}
		else if (wDPIBits <= 24)
		{
			wBitCount = 24;
		}
		else
		{
			wBitCount = 32;
		}

		GetObject(hBitmap, sizeof(bmp), (LPSTR)&bmp);
		bmpih.biSize = sizeof(BITMAPINFOHEADER);
		bmpih.biWidth = bmp.bmWidth;
		bmpih.biHeight = bmp.bmHeight;
		bmpih.biPlanes = 1;
		bmpih.biBitCount = wBitCount;
		bmpih.biCompression = BI_RGB;
		bmpih.biSizeImage = 0;
		bmpih.biXPelsPerMeter = 0;
		bmpih.biYPelsPerMeter = 0;
		bmpih.biClrImportant = 0;
		bmpih.biClrUsed = 0;

		dwBmBitsSize = ((bmp.bmWidth * bmpih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//为位图内容分配内存
		hDibBit = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		pbmpih = (LPBITMAPINFOHEADER)GlobalLock(hDibBit);
		*pbmpih = bmpih;

		//处理调色板
		hPaltte = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
		if (hPaltte)
		{
			hDC = ::GetDC(NULL);
			hPaltteBackup = ::SelectPalette(hDC, (HPALETTE)hPaltte, FALSE);
			::RealizePalette(hDC);
		}

		//获取该调色板下新的像素值
		GetDIBits(hDC,
			hBitmap,
			0,
			(UINT)bmp.bmHeight,
			((BYTE *)pbmpih + sizeof(BITMAPINFOHEADER) + dwPaletteSize),
			(BITMAPINFO *)pbmpih,
			DIB_RGB_COLORS);

		//恢复调色板
		if (hPaltteBackup)
		{
			::SelectPalette(hDC, (HPALETTE)hPaltteBackup, TRUE);
			::RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//创建位图文件
		hFile = CreateFile((LPCTSTR)pFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			//设置位图文件头
			bmpfh.bfType = 0x4D42;   //"BM"
			bmpfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
			bmpfh.bfReserved1 = 0;
			bmpfh.bfReserved2 = 0;
			bmpfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
			dwDibBitSize = bmpfh.bfSize;

			//写入位图文件头
			WriteFile(hFile, (void *)&bmpfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

			//写入位图文件其余内容
			WriteFile(hFile, (void *)pbmpih, dwDibBitSize, &dwWritten, NULL);

			//清除
			GlobalUnlock(hDibBit);
			GlobalFree(hDibBit);
			hDibBit = NULL;

			//关闭文件
			CloseHandle(hFile);
			hFile = NULL;
		}

		DeleteDC(hDC);
		hDC = NULL;

		return TRUE;
	}
	
	__inline static BOOL WindowClassesRegister(HINSTANCE hInstance,
		LPCTSTR lpClassName,
		WNDPROC lpfnWndProc,
		LPCTSTR lpszMenuName = NULL,
		HBRUSH hbrBackground = (HBRUSH)COLOR_BACKGROUND,
		HICON hIcon = LoadIcon(NULL, IDI_APPLICATION),
		HICON hIconSm = LoadIcon(NULL, IDI_APPLICATION),
		HICON hCursor = LoadCursor(NULL, IDC_ARROW),
		UINT uStyle = CS_DBLCLKS,
		INT cbClsExtra = 0,
		INT cbWndExtra = 0
		)
	{
		//Data structure for the windowclass
		WNDCLASSEX wcex = { 0 };

		wcex.cbSize = sizeof(WNDCLASSEX);

		// The Window structure
		wcex.hInstance = hInstance;
		wcex.lpszClassName = lpClassName;
		//This function is called by windows
		wcex.lpfnWndProc = lpfnWndProc;
		//Catch double-clicks
		wcex.style = uStyle;


		// Use default icon and mouse-pointer
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIconSm;
		wcex.hCursor = hCursor;
		//No menu
		wcex.lpszMenuName = lpszMenuName;
		//No extra bytes after the window class
		wcex.cbClsExtra = cbClsExtra;
		//structure or the window instance
		wcex.cbWndExtra = cbWndExtra;
		// Use Windows's default colour as the background of the window
		wcex.hbrBackground = hbrBackground;

		// Register the window class, and if it fails quit the program
		return RegisterClassEx(&wcex);
	}

	__inline static HWND CreateCurtomWindow(HINSTANCE hInstance,
		LPCTSTR lpszClassName,
		LPCTSTR lpszTitleName = _T("TemplateWindowsApplication"),
		DWORD dwStyle = WS_OVERLAPPEDWINDOW,
		DWORD dwExtendStyle = 0,
		RECT rcRect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT + 300, CW_USEDEFAULT + 200 },
		HWND hWndParent = HWND_DESKTOP,
		HMENU hMenu = NULL,
		LPVOID lpCreationData = NULL)
	{
		// The class is registered, let's create the program
		return CreateWindowEx(
			dwExtendStyle,// Extended possibilites for variation
			lpszClassName,// Classname
			lpszTitleName,//Title Text
			dwStyle,//default window
			rcRect.left,//Windows decides the position
			rcRect.top,//where the window ends up on the screen
			rcRect.right - rcRect.left,//The programs width
			rcRect.bottom - rcRect.top,//and height in pixels
			hWndParent,//The window is a child-window to desktop
			hMenu,//No menu */
			hInstance,//Program Instance handler
			lpCreationData//No Window Creation data
			);
	}

	__inline static UINT_PTR StartupWindows(HWND hWnd, INT nCmdShow)
	{
		MSG msg = { 0 };

		// Make the window visible on the screen
		::ShowWindow(hWnd, nCmdShow);

		// Update the window visible on the screen
		::UpdateWindow(hWnd);

		// Run the message loop. It will run until GetMessage() returns 0
		while (::GetMessage(&msg, NULL, 0, 0))
		{
			// Translate virtual-key messages into character messages
			::TranslateMessage(&msg);
			// Send message to WindowProcedure
			::DispatchMessage(&msg);
		}

		// The program return-value is 0 - The value that PostQuitMessage() gave
		return msg.wParam;
	}

	//  This function is called by the Windows function DispatchMessage()
	//LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)

	class CWindowsManager{
	public:
		
		CWindowsManager()
		{
			CWindowsManager::m_pAnimation = NULL;
			memset((void *)&CWindowsManager::m_rcWindow, 0, sizeof(RECT));
		}
		virtual ~CWindowsManager()
		{
			
		}

	public:

		//  This function is called by the Windows function DispatchMessage()
		__inline static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lResult = 0;
			// handle the messages
			switch (message)
			{
			case WM_CREATE:
			{
				HMENU hMenu = GetMenu(NULL);

				// GDI+
				m_pAnimation = new AnimationDisplayer::CAnimationDisplayer(Convert::TToW(_T("d:\\test.gif")).c_str());

				RECT rc = { 0 };
				::GetClientRect(hWnd, &rc);

				int cx = (rc.right - CWindowsManager::m_pAnimation->GetWidth()) / 2;

				POINT pt = { cx, 10 };
				CWindowsManager::m_pAnimation->InitAnimation(hWnd, pt);
				
				GUI::NotifyUpdate(hWnd, &CWindowsManager::m_rcWindow);
			}
			break;
			
			case WM_SIZE:
			case WM_SIZING:
			case WM_ENTERSIZEMOVE:
			case WM_EXITSIZEMOVE:
			{
				GUI::NotifyUpdate(hWnd, &CWindowsManager::m_rcWindow);
			}
			break;
			case WM_PAINT:
			{
				ULONG uARGB[2] = { ARGB(0x7F, 0x00, 0x7F, 0x00), ARGB(0x7F, 0xFF, 0x00, 0x00) };
				RECT rcWnd = { 0 };
				PAINTSTRUCT ps = { 0 };
				RECT rcMemory = { 1, 1, 1, 1 };
				HDC hDC = ::BeginPaint(hWnd, &ps);
				GetClientRect(hWnd, &rcWnd);
				GUI::DrawMemoryBitmap(hDC, hWnd, rcWnd.right, rcWnd.bottom,
					GUI::DrawAlphaBlendRect(hWnd, uARGB, hDC, &rcMemory));
				
				::EndPaint(hWnd, &ps);
			}
			break;
			case WM_DESTROY:
			{
				if (CWindowsManager::m_pAnimation)
				{
					delete CWindowsManager::m_pAnimation;
					CWindowsManager::m_pAnimation = NULL;
				}
				// send a WM_QUIT to the message queue
				PostQuitMessage(0);
			}
			break;
			default:
			{
				// for messages that we don't deal with
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
			}

			return lResult;
		}

		UINT_PTR RunApp()
		{
			HWND hWnd = NULL;
			UINT_PTR uResult = 0;
			_TCHAR tzClassName[] = _T("PPSHUAIWINDOW");
			HINSTANCE hInstance = GetModuleHandle(NULL);

			AnimationDisplayer::GdiplusInitialize();

			if (GUI::WindowClassesRegister(hInstance, tzClassName, &CWindowsManager::WindowProcedure))
			{
				//return 0;
			}
			hWnd = GUI::CreateCurtomWindow(hInstance, tzClassName);
		
			uResult = GUI::StartupWindows(hWnd, SW_SHOW);

			AnimationDisplayer::GdiplusExitialize();

			return uResult;
		}

	public:
		
		static AnimationDisplayer::CAnimationDisplayer * CWindowsManager::m_pAnimation;
		static RECT CWindowsManager::m_rcWindow;
		static _TCHAR m_tszGifFileName[MAX_PATH + 1];
	
	private:

	};
	
	RECT CWindowsManager::m_rcWindow = { 0 };
	_TCHAR CWindowsManager::m_tszGifFileName[MAX_PATH + 1] = { 0 };
	AnimationDisplayer::CAnimationDisplayer * CWindowsManager::m_pAnimation = NULL;

}
}

