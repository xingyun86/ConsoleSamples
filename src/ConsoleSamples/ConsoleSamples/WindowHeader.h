
#include <windows.h>
#include <commctrl.h>
#include <olectl.h>
#include <ocidl.h>
#include <ole2.h>

#include <tchar.h>
#include <math.h>

#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comctl32.lib")

#include <gdiplus.h>
#pragma comment(lib, "gdiplus")

#include <map>
#include <vector>

#include "MACROS.h"

namespace PPSHUAI
{

namespace NearSideAutoHide{

#define NEAR_SIZE 1 //�����Զ�ͣ����Ч����
#define NEAR_SIDE 1 //�������غ�����Ļ�ϱ��������أ���ʹ�����Դ���

#define IDC_TIMER_NEARSIDEHIDE	0xFFFF
#define T_TIMEOUT_NEARSIDEHIDE	0xFF
	enum {
		ALIGN_NONE,          //��ͣ��
		ALIGN_TOP,          //ͣ���ϱ�
		ALIGN_LEFT,          //ͣ�����
		ALIGN_RIGHT          //ͣ���ұ�
	};
	static int g_nScreenX = 0;
	static int g_nScreenY = 0;
	static int g_nAlignType = ALIGN_NONE;   //ȫ�ֱ��������ڼ�¼����ͣ��״̬

	__inline static void InitScreenSize()
	{
		g_nScreenX = ::GetSystemMetrics(SM_CXSCREEN);
		g_nScreenY = ::GetSystemMetrics(SM_CYSCREEN);
	}
	//�ڴ����ʼ�����趨����״̬���������ͣ������ͣ���ڱ�Ե
	//�ұ���Ѱ�����������������ʼ����������Ϊ��רһѰ��һ��������
	//���ǣ������ʼ��ʱ������WM_MOVING��Ϣ,�Ҳ��ò��ظ���������.
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
		//������
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

	//�������ʾ�����ɸú������,����bHide������ʾ��������.
	__inline static void AutoHideProc(HWND hWnd, LPRECT lpRect, BOOL bHide)
	{
		int nStep = 20;  //������������Ĳ���,�������ò���ƽ��,���������ֵ.
		int xStep = 0, xEnd = 0;
		int yStep = 0, yEnd = 0;
		LONG Width = lpRect->right - lpRect->left;
		LONG Height = lpRect->bottom - lpRect->top;

		//�±��жϴ��������ƶ�,��ͣ����ʽ����
		switch (g_nAlignType)
		{
		case ALIGN_TOP:
		{
			//�����Ʋ�
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
			//�����Ʋ�
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
			//�����Ʋ�
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
		//������������.
		for (int i = 0; i < nStep; i++)
		{
			lpRect->left += xStep;
			lpRect->top += yStep;
			::SetWindowPos(hWnd, NULL, lpRect->left, lpRect->top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
			::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			Sleep(3);
		}
		::SetWindowPos(hWnd, NULL, xEnd, yEnd, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
		if (!bHide) //��������ѱ���ʾ,���ö�ʱ��.�������.
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

			if (!PtInRect(&rc, pt)) //����겻�ڴ�����,���ش���.
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
		if (rc.left < 0 || rc.top < 0 || rc.right > g_nScreenX) //δ��ʾ
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
		if (rc.left < 0 || rc.top < 0 || rc.right > g_nScreenX) //δ��ʾ
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
	//������봦������ϢWM_MOVING��lParam�ǲ���RECTָ��
	__inline static LRESULT OnMoving(HWND hWnd, LPARAM lParam)
	{
		POINT pt = { 0 };
		LPRECT lpRect = (LPRECT)lParam;
		LONG Width = lpRect->right - lpRect->left;
		LONG Height = lpRect->bottom - lpRect->top;

		//δ���߽���pRect����
		if (g_nAlignType == ALIGN_NONE)
		{
			if (lpRect->left < NEAR_SIZE) //�������Ч������
			{
				g_nAlignType = ALIGN_LEFT;
				lpRect->left = 0;
				lpRect->right = Width;
			}
			if (lpRect->right + NEAR_SIZE > g_nScreenX) //���ұ���Ч�����ڣ�g_nScreenXΪ��Ļ��ȣ�����GetSystemMetrics(SM_CYSCREEN)�õ���
			{
				g_nAlignType = ALIGN_RIGHT;
				lpRect->right = g_nScreenX;
				lpRect->left = g_nScreenX - Width;
			}
			if (lpRect->top < NEAR_SIZE) //���ϱ���Ч�����ڣ��Զ���£��
			{
				g_nAlignType = ALIGN_TOP;
				lpRect->top = 0;
				lpRect->bottom = Height;
			}
		}
		else
		{
			//���߽���������
			::GetCursorPos(&pt);
			if (g_nAlignType == ALIGN_TOP)
			{
				lpRect->top = 0;
				lpRect->bottom = Height;
				if (pt.y > NEAR_SIZE) //������뿪�ϱ߽����ϲ�ͣ����
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->left < NEAR_SIZE) //���ϲ�ͣ��ʱ������Ҳ�������ұ߽ǡ�
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
				if (pt.x > NEAR_SIZE) //���������뿪��߽�ʱ���ͣ����
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->top < NEAR_SIZE) //�������Ͻǡ�
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
				if (pt.x < g_nScreenX - NEAR_SIZE) //������뿪�ұ߽�ʱ�����ͣ����
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->top < NEAR_SIZE) //�������Ͻǡ�
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

namespace GdiplusDisplay{
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
		static std::map<HWND, PVOID> m_ShadowWindowMap;

		//
		typedef BOOL(WINAPI *pfnUpdateLayeredWindow)(HWND hWnd, HDC hdcDst, POINT *pptDst,
			SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey,
			BLENDFUNCTION *pblend, DWORD dwFlags);
		static pfnUpdateLayeredWindow m_pUpdateLayeredWindow;

		HWND m_hWnd;

		LONG_PTR m_OriginParentProc;        // Original WndProc of parent window

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
				("UpdateLayeredWindow"));

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
				std::map<HWND, PVOID>::iterator it = m_ShadowWindowMap.find(hParentWnd);
				if (it != m_ShadowWindowMap.end())
				{
					it->second = (PVOID)(this);
				}
				else
				{
					m_ShadowWindowMap.insert(std::map<HWND, PVOID>::value_type(hParentWnd, (this)));
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
			std::map<HWND, PVOID>::iterator it = m_ShadowWindowMap.find(hwnd);
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

	std::map<HWND, PVOID> CShadowBorder::m_ShadowWindowMap;
}

namespace GUI{
	__inline static LONG_PTR SetWindowUserData(HWND hWnd, LONG_PTR lPtr)
	{
		return ::SetWindowLongPtr(hWnd, GWLP_USERDATA, lPtr);
	}
	__inline static LONG_PTR GetWindowUserData(HWND hWnd)
	{
		return GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	typedef enum COLUMN_DATATYPE{
		CDT_NULL = 0,
		CDT_DEC = 1,
		CDT_HEX = 2,
		CDT_STRING = 3,
		CDT_OTHERS,
	};
	typedef struct tagSortDataInfo{
		HWND hWnd;//Master Window
		HWND hListCtrlWnd;//ListCtrl Window		
		INT nColumnItem;//Column item index
		bool bSortFlag;//Sort flag asc or desc
		COLUMN_DATATYPE cdType;//Column sort type
	}SORTDATAINFO, *PSORTDATAINFO;
	// Sort the item in reverse alphabetical order.
	__inline static int CALLBACK ListCtrlCompareProcess(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		// lParamSort contains a pointer to the list view control.
		int nResult = 0;
		LV_ITEM lvi = { 0 };
		unsigned long ulX = 0;
		unsigned long ulY = 0;

		_TCHAR tzItem1[MAXWORD] = { 0 };
		_TCHAR tzItem2[MAXWORD] = { 0 };

		SORTDATAINFO * pSDI = (SORTDATAINFO *)lParamSort;
		if (pSDI)
		{
			lvi.mask = LVIF_TEXT;
			lvi.iSubItem = pSDI->nColumnItem;

			lvi.cchTextMax = sizeof(tzItem1);
			lvi.pszText = tzItem1;
			lvi.iItem = lParam1;
			ListView_GetItem(pSDI->hListCtrlWnd, &lvi);

			lvi.cchTextMax = sizeof(tzItem2);
			lvi.pszText = tzItem2;
			lvi.iItem = lParam2;
			ListView_GetItem(pSDI->hListCtrlWnd, &lvi);
		}

		switch (pSDI->cdType)
		{
		case CDT_NULL:
			break;
		case CDT_DEC:
			ulX = _tcstoul(tzItem2, 0, 10);
			ulY = _tcstoul(tzItem1, 0, 10);
			nResult = ulX - ulY;
			break;
		case CDT_HEX:
			ulX = _tcstoul(tzItem2, 0, 16);
			ulY = _tcstoul(tzItem1, 0, 16);
			nResult = ulX - ulY;
			break;
		case CDT_STRING:
			nResult = lstrcmpi(tzItem2, tzItem1);
			break;
		case CDT_OTHERS:
			break;
		default:
			break;
		}

		return ((pSDI->bSortFlag ? (nResult) : (-nResult)));
	}
	
	//ͼ��������
#define IMAGEICONINDEX_NAME "IMAGEICONINDEX_NAME"
	__inline static void ImageListInit(HIMAGELIST hImageList, TSTRINGVECTORMAP * pTVMAP, LPCTSTR lpIconColumn)
	{
		LONG lIdx = 0;
		SIZE_T stRowIdx = 0;
		SHFILEINFO shfi = { 0 };
		std::map<TSTRING, LONG> tlmap;
		std::map<TSTRING, LONG>::iterator itFinder;
		if (hImageList)
		{
			ImageList_RemoveAll(hImageList);

			pTVMAP->insert(TSTRINGVECTORMAP::value_type(_T(IMAGEICONINDEX_NAME), TSTRINGVECTOR()));

			for (stRowIdx = 0; stRowIdx < pTVMAP->at(lpIconColumn).size(); stRowIdx++)
			{
				itFinder = tlmap.find(pTVMAP->at(lpIconColumn).at(stRowIdx));
				if (itFinder != tlmap.end())
				{
					lIdx = itFinder->second;
				}
				else
				{
					memset(&shfi, 0, sizeof(shfi));
					SHGetFileInfo(pTVMAP->at(lpIconColumn).at(stRowIdx).c_str(), 0, &shfi, sizeof(shfi), SHGFI_DISPLAYNAME | SHGFI_ICON);
					lIdx = ImageList_AddIcon(hImageList, shfi.hIcon);
					tlmap.insert(std::map<TSTRING, LONG>::value_type(pTVMAP->at(lpIconColumn).at(stRowIdx), lIdx));
				}

				pTVMAP->at(_T(IMAGEICONINDEX_NAME)).push_back(STRING_FORMAT(_T("%ld"), lIdx));
			}
		}
	}

	__inline static void ListCtrlDeleteAllColumns(HWND hListViewWnd)
	{
		while (ListView_DeleteColumn(hListViewWnd, ListView_GetHeader(hListViewWnd)));
	}
	__inline static void ListCtrlDeleteAllRows(HWND hListViewWnd)
	{
		ListView_DeleteAllItems(hListViewWnd);
	}
	__inline static void ListCtrlInsertData(TSTRINGVECTORMAP * pTVMAP, HWND hListViewWnd, HIMAGELIST hImageList = NULL, LPCTSTR lpListCtrlText = _T(""), LPCTSTR lpHeaderText = _T("|3|3|3|3|3|3|3|3|3|3"))
	{
		SIZE_T stIndex = 0;
		SIZE_T stCount = 0;
		SIZE_T stRowIdx = 0;
		SIZE_T stColIdx = 0;
		LV_ITEM lvi = { 0 };
		LV_COLUMN lvc = { 0 };
		TSTRINGVECTORMAP::iterator itEnd;
		TSTRINGVECTORMAP::iterator itIdx;
		
		if (lpListCtrlText)
		{
			SetWindowText(hListViewWnd, lpListCtrlText);
		}

		if (lpHeaderText)
		{
			SetWindowText(ListView_GetHeader(hListViewWnd), lpHeaderText);
		}

		stColIdx = 0;
		itEnd = pTVMAP->end();
		itIdx = pTVMAP->begin();
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		for (; itIdx != itEnd; itIdx++, stColIdx++)
		{
			if (!itIdx->first.compare(_T("ͼ���ļ�")) || !itIdx->first.compare(_T(IMAGEICONINDEX_NAME)))
			{
				stColIdx--;
				continue;
			}

			lvc.iSubItem = stColIdx;
			lvc.pszText = (LPTSTR)itIdx->first.c_str();
			lvc.cx = 120;
			ListView_InsertColumn(hListViewWnd, stColIdx, &lvc);

			stRowIdx = 0;
			lvi.iSubItem = stColIdx;
			lvi.mask = LVIF_TEXT;
			stCount = itIdx->second.size();
			for (stIndex = 0; stIndex < stCount; stIndex++, stRowIdx++)
			{					
				if (hImageList && pTVMAP->find(_T(IMAGEICONINDEX_NAME)) != pTVMAP->end())
				{
					lvi.mask |= LVIF_IMAGE;
					lvi.iImage = _ttol(pTVMAP->at(_T(IMAGEICONINDEX_NAME)).at(stIndex).c_str());
				}
				lvi.iItem = stRowIdx;
				lvi.pszText = (LPTSTR)itIdx->second.at(stIndex).c_str();
				if (!lvi.iSubItem)
				{
					ListView_InsertItem(hListViewWnd, &lvi);
				}
				else
				{
					ListView_SetItem(hListViewWnd, &lvi);
				}
			}
			lvi.mask = LVIF_PARAM;
			lvi.lParam = stRowIdx;
			ListView_SetItem(hListViewWnd, &lvi);
		}
	}
	
	__inline static LONG_PTR ListCtrlSetSortDataInfo(HWND hListCtrlWnd, SORTDATAINFO * pSDI)
	{
		return SetWindowUserData(ListView_GetHeader(hListCtrlWnd), (LONG_PTR)pSDI);
	}
	__inline static SORTDATAINFO * ListCtrlGetLSortDataInfo(HWND hListCtrlWnd)
	{
		return (SORTDATAINFO *)GetWindowUserData(ListView_GetHeader(hListCtrlWnd));
	}
	__inline static UINT ListCtrlGetSelectedRowCount(HWND hListViewWnd)
	{
		return ListView_GetSelectedCount(hListViewWnd);
	}
	__inline static void ListCtrlGetSelectedRow(std::map<UINT, UINT> * pssmap, HWND hListViewWnd)
	{
		UINT nSelectIndex = 0;
		while ((nSelectIndex = ListView_GetNextItem(hListViewWnd, (-1), LVNI_SELECTED)) != (-1))
		{
			pssmap->insert(std::map<UINT, UINT>::value_type(nSelectIndex, nSelectIndex));
		}		
	}
	
	__inline static LRESULT ListCtrlOnNotify(HWND hListCtrlWnd, LPNMHDR lpNMHDR)
	{
		int nItemPos = 0;
		switch (lpNMHDR->code)
		{
		case NM_RCLICK:
		{
			if ((nItemPos = ListView_GetNextItem(hListCtrlWnd, -1, LVNI_SELECTED)) != -1)
			{
				HMENU hMenu = NULL;
				POINT point = { 0 };
				
				GetCursorPos(&point);
				
				//��̬��������ʽ�˵�����
				hMenu = CreatePopupMenu();
				if (hMenu)
				{
					AppendMenu(hMenu, MF_STRING, (0), _T("ѡ��"));					
					TrackPopupMenuEx(hMenu, TPM_RIGHTBUTTON | TPM_VERPOSANIMATION | TPM_LEFTALIGN | TPM_VERTICAL, point.x, point.y, hListCtrlWnd, NULL);
					DestroyMenu(hMenu);
					hMenu = NULL;
				}
			}
		}
		break;
		case NM_DBLCLK:
		{
			if ((nItemPos = ListView_GetNextItem(hListCtrlWnd, -1, LVNI_SELECTED)) != -1)
			{
				
			}
		}
		break;
		//case LVN_COLUMNCLICK:
		case HDN_ITEMCLICK:
		{
			_TCHAR tzText[MAXWORD] = { 0 };
			_TCHAR tzValue[MAXWORD] = { 0 };
			tstring::size_type stIndexPos = 0;
			tstring::size_type stStartPos = 0;
			tstring::size_type stFinalPos = 0;

			LPNMHEADER lpNMHEADER = (LPNMHEADER)lpNMHDR;

			SORTDATAINFO * pSDI = (SORTDATAINFO *)GetWindowUserData(ListView_GetHeader(hListCtrlWnd));
			if (pSDI)
			{
				pSDI->hListCtrlWnd = hListCtrlWnd;
				if (pSDI->nColumnItem != lpNMHEADER->iItem)
				{
					pSDI->nColumnItem = lpNMHEADER->iItem;
					pSDI->bSortFlag = true;
				}
				else
				{
					pSDI->bSortFlag = (!pSDI->bSortFlag);
				}

				GetWindowText(ListView_GetHeader(pSDI->hListCtrlWnd), tzText, sizeof(tzText));
				for (stIndexPos = 0;
					stIndexPos < lpNMHEADER->iItem
					&& (stStartPos = tstring(tzText).find(_T("|"), stStartPos + 1));
				stIndexPos++){
					;
				}
				stFinalPos = tstring(tzText).find(_T("|"), stStartPos + 1);
				lstrcpyn(tzValue, (LPCTSTR)tzText + stStartPos + 1, stFinalPos - stStartPos);
				pSDI->cdType = (COLUMN_DATATYPE)_ttol(tzValue);

				ListView_SortItemsEx(pSDI->hListCtrlWnd, &ListCtrlCompareProcess, pSDI);
			}
		}
		break;
		case LVN_ITEMCHANGED:
		{
			if ((nItemPos = ListView_GetNextItem(hListCtrlWnd, -1, LVNI_SELECTED)) != -1)
			{
				//
			}
		}
		break;
		default:
			break;
		}

		return 0;
	}

	__inline static void RegisterDropFilesEvent(HWND hWnd)
	{
#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDAYA	0x0049
#endif

#ifndef MSGFLT_ADD
#define MSGFLT_ADD 1
#endif

#ifndef MSGFLT_REMOVE
#define MSGFLT_REMOVE 2
#endif
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_ACCEPTFILES);
		typedef BOOL(WINAPI *LPFN_ChangeWindowMessageFilter)(__in UINT message, __in DWORD dwFlag);
		LPFN_ChangeWindowMessageFilter pfnChangeWindowMessageFilter = (LPFN_ChangeWindowMessageFilter)GetProcAddress(GetModuleHandle(_T("USER32.DLL")), "ChangeWindowMessageFilter");
		if (pfnChangeWindowMessageFilter)
		{
			pfnChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
			pfnChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
			pfnChangeWindowMessageFilter(WM_COPYGLOBALDAYA, MSGFLT_ADD);// 0x0049 == WM_COPYGLOBALDATA
		}
	}
	__inline static size_t GetDropFiles(std::map<TSTRING, TSTRING> * pttmap, HDROP hDropInfo)
	{
		UINT nIndex = 0;
		UINT nNumOfFiles = 0;
		_TCHAR tszFilePathName[MAX_PATH + 1] = { 0 };

		//�õ��ļ�����
		nNumOfFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

		for (nIndex = 0; nIndex < nNumOfFiles; nIndex++)
		{
			//�õ��ļ���
			DragQueryFile(hDropInfo, nIndex, (LPTSTR)tszFilePathName, _MAX_PATH);
			pttmap->insert(std::map<TSTRING, tstring>::value_type(tszFilePathName, tszFilePathName));
		}

		DragFinish(hDropInfo);

		return nNumOfFiles;
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
	
	__inline static void StaticSetIconImage(HWND hWndStatic, HICON hIcon)
	{
		HICON hLastIcon = NULL;
		hLastIcon = (HICON)SendMessage(hWndStatic, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		if (hLastIcon)
		{
			DeleteObject(hLastIcon);
			hLastIcon = NULL;
		}
	}
	__inline static void StaticSetBitmapImage(HWND hWndStatic, HBITMAP hBitmap)
	{
		HBITMAP hLastBitmap = NULL;
		hLastBitmap = (HBITMAP)SendMessage(hWndStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		if (hLastBitmap)
		{
			DeleteObject(hLastBitmap);
			hLastBitmap = NULL;
		}
	}

	__inline static HICON HIconFromFile(LPCTSTR lpFileName, SIZE size = { 0, 0 })
	{
		return (HICON)LoadImage(GetModuleHandle(NULL), 
			lpFileName,
			IMAGE_ICON,
			size.cx,//GetSystemMetrics(SM_CXSMICON),
			size.cy,//GetSystemMetrics(SM_CYSMICON),
			LR_LOADFROMFILE);
	}
	__inline static HBITMAP HBitmapFromFile(LPCTSTR lpFileName, SIZE size = { 0, 0 })
	{
		return (HBITMAP)LoadImage(GetModuleHandle(NULL),
			lpFileName,
			IMAGE_BITMAP,
			size.cx,//GetSystemMetrics(SM_CXSMICON),
			size.cy,//GetSystemMetrics(SM_CYSMICON),
			LR_LOADFROMFILE);
	}
	__inline static HCURSOR HCursorFromFile(LPCTSTR lpFileName, SIZE size = { 0, 0 })
	{
		return (HCURSOR)LoadImage(GetModuleHandle(NULL),
			lpFileName,
			IMAGE_CURSOR,
			size.cx,//GetSystemMetrics(SM_CXSMICON),
			size.cy,//GetSystemMetrics(SM_CYSMICON),
			LR_LOADFROMFILE);
	}
	__inline static
		void NotifyUpdate(HWND hWnd, RECT *pRect = NULL, BOOL bErase = TRUE)
	{
		RECT rcWnd = { 0 };

		::GetClientRect(hWnd, &rcWnd);
		if (pRect)
		{			
			if (memcmp(pRect, &rcWnd, sizeof(RECT)))
			{
				::InvalidateRect(hWnd, &rcWnd, bErase);
				memcpy(pRect, &rcWnd, sizeof(RECT));
			}
		}
		else
		{
			::InvalidateRect(hWnd, &rcWnd, bErase);
		}
	}

	__inline static
		bool SaveBitmapToFile(HDC hDC, HBITMAP hBitmap, LPCTSTR ptFileName)
	{
		//	HDC hDC;
		//�豸������
		int iBits;
		//��ǰ��ʾ�ֱ�����ÿ��������ռ�ֽ���
		WORD wBitCount;
		//λͼ��ÿ��������ռ�ֽ���
		//�����ɫ���С�� λͼ�������ֽڴ�С ��  λͼ�ļ���С �� д���ļ��ֽ���
		DWORD  dwPaletteSize = 0, dwBmBitsSize, dwDIBSize, dwWritten;
		BITMAP  Bitmap;
		//λͼ���Խṹ
		BITMAPFILEHEADER   bmfHdr;
		//λͼ�ļ�ͷ�ṹ
		BITMAPINFOHEADER   bi;
		//λͼ��Ϣͷ�ṹ
		LPBITMAPINFOHEADER lpbi;
		//ָ��λͼ��Ϣͷ�ṹ
		HANDLE  fh, hDib, hPal;
		HPALETTE  hOldPal = NULL;

		//�����ļ��������ڴ�������ɫ����
		//����λͼ�ļ�ÿ��������ռ�ֽ���
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

		//�����ɫ���С
		if (wBitCount <= 8)
		{
			dwPaletteSize = (1 << wBitCount)*sizeof(RGBQUAD);
		}

		//����λͼ��Ϣͷ�ṹ
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

		//Ϊλͼ���ݷ����ڴ�
		hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
		*lpbi = bi;
		// �����ɫ��
		hPal = GetStockObject(DEFAULT_PALETTE);
		if (hPal)
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}
		// ��ȡ�õ�ɫ�����µ�����ֵ
		GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);
		//�ָ���ɫ��   
		if (hOldPal)
		{
			SelectPalette(hDC, hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//����λͼ�ļ�    
		fh = CreateFile(ptFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (fh == INVALID_HANDLE_VALUE)
		{
			GlobalUnlock(hDib);
			GlobalFree(hDib);
			return FALSE;
		}
		//����λͼ�ļ�ͷ
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
		//�豸������
		int iBits;
		//��ǰ��ʾ�ֱ�����ÿ��������ռ�ֽ���
		WORD wBitCount;
		//λͼ��ÿ��������ռ�ֽ���
		//�����ɫ���С�� λͼ�������ֽڴ�С ��  λͼ�ļ���С �� д���ļ��ֽ���
		DWORD  dwPaletteSize = 0, dwBmBitsSize, dwDIBSize, dwWritten;
		BITMAP  Bitmap;
		//λͼ���Խṹ
		BITMAPFILEHEADER   bmfHdr;
		//λͼ�ļ�ͷ�ṹ
		BITMAPINFOHEADER   bi;
		//λͼ��Ϣͷ�ṹ
		LPBITMAPINFOHEADER lpbi;
		//ָ��λͼ��Ϣͷ�ṹ
		HANDLE  fh, hDib, hPal;
		HPALETTE  hOldPal = NULL;

		//�����ļ��������ڴ�������ɫ����
		//����λͼ�ļ�ÿ��������ռ�ֽ���
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

		//�����ɫ���С
		if (wBitCount <= 8)
		{
			dwPaletteSize = (1 << wBitCount)*sizeof(RGBQUAD);
		}

		//����λͼ��Ϣͷ�ṹ
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

		//Ϊλͼ���ݷ����ڴ�
		hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
		*lpbi = bi;
		// �����ɫ��
		hPal = GetStockObject(DEFAULT_PALETTE);
		if (hPal)
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}
		// ��ȡ�õ�ɫ�����µ�����ֵ
		GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);
		//�ָ���ɫ��   
		if (hOldPal)
		{
			SelectPalette(hDC, hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//����λͼ�ļ�    
		fh = CreateFile(ptFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (fh == INVALID_HANDLE_VALUE)
		{
			GlobalUnlock(hDib);
			GlobalFree(hDib);
			return FALSE;
		}
		//����λͼ�ļ�ͷ
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

		//�����豸������(HDC)
		hMemoryDC = CreateCompatibleDC(hDC);

		//����HBITMAP
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

		//�õ�λͼ������
		StretchBlt(hMemoryDC, rcMemory.left, rcMemory.top, rcMemory.right - rcMemory.left, rcMemory.bottom - rcMemory.top,
			hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SRCCOPY);

		//�õ����յ�λͼ��Ϣ
		hBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmapTemp);

		//�ͷ��ڴ�
		DeleteObject(hBitmapTemp);
		hBitmapTemp = NULL;
		::DeleteDC(hMemoryDC);
		hMemoryDC = NULL;

		return hBitmap;
	}

	__inline static TSTRING HResultToTString(HRESULT hInResult)
	{
		HRESULT hResult = S_FALSE;
		TSTRING tsDescription(_T(""));
		IErrorInfo * pErrorInfo = NULL;
		BSTR pbstrDescription = NULL;
		hResult = ::GetErrorInfo(NULL, &pErrorInfo);
		if (SUCCEEDED(hResult) && pErrorInfo)
		{
			hResult = pErrorInfo->GetDescription(&pbstrDescription);
			if (SUCCEEDED(hResult) && pbstrDescription)
			{
				tsDescription = Convert::WToT(pbstrDescription);
			}
		}
		return tsDescription;
	}
	///////////////////////////////////////////////////////////
	//���´����ʵ�ֵĹ����Ǵ�ָ����·���ж�ȡͼƬ������ʾ����
	//
	__inline static void ImagesRenderDisplay(HDC hDC, RECT * pRect, const _TCHAR * tImagePath)
	{
		HANDLE hFile = NULL;
		HRESULT hResult = S_FALSE;
		DWORD dwReadedSize = 0; //����ʵ�ʶ�ȡ���ļ���С
		IStream *pIStream = NULL;//����һ��IStream�ӿ�ָ�룬��������ͼƬ��
		IPicture *pIPicture = NULL;//����һ��IPicture�ӿ�ָ�룬��ʾͼƬ����
		VOID * pImageMemory = NULL;
		HGLOBAL hImageMemory = NULL;
		OLE_XSIZE_HIMETRIC hmWidth = 0;
		OLE_YSIZE_HIMETRIC hmHeight = 0;
		LARGE_INTEGER liFileSize = { 0, 0 };

		hFile = ::CreateFile(tImagePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//��ָ����·��szImagePath�ж�ȡ�ļ����
		if (hFile != INVALID_HANDLE_VALUE)
		{
			liFileSize.LowPart = ::GetFileSize(hFile, (DWORD *)&liFileSize.HighPart); //���ͼƬ�ļ��Ĵ�С����������ȫ���ڴ�
			hImageMemory = ::GlobalAlloc(GMEM_MOVEABLE, liFileSize.QuadPart); //��ͼƬ����ȫ���ڴ�
			if (hImageMemory)
			{
				pImageMemory = ::GlobalLock(hImageMemory); //�����ڴ�

				::ReadFile(hFile, pImageMemory, liFileSize.QuadPart, &dwReadedSize, NULL); //��ȡͼƬ��ȫ���ڴ浱��
				
				hResult = ::CreateStreamOnHGlobal(hImageMemory, FALSE, &pIStream); //��ȫ���ڴ��ʹ��IStream�ӿ�ָ��
				if (SUCCEEDED(hResult) && pIStream)
				{
					//��OleLoadPicture���IPicture�ӿ�ָ��
					hResult = ::OleLoadPicture(pIStream, 0, FALSE, IID_IPicture, (LPVOID*)&(pIPicture));
					//TSTRING tsError = HResultToTString(hResult);
					//�õ�IPicture COM�ӿڶ���󣬾Ϳ��Խ��л��ͼƬ��Ϣ����ʾͼƬ�Ȳ���
					if (SUCCEEDED(hResult) && pIPicture)
					{
						pIPicture->get_Width(&hmWidth); //�ýӿڷ������ͼƬ�Ŀ�͸�
						pIPicture->get_Height(&hmHeight); //�ýӿڷ������ͼƬ�Ŀ�͸�
						pIPicture->Render(hDC, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, 0, hmHeight, hmWidth, -hmHeight, NULL); //��ָ����DC�ϻ��ͼƬ
					}
				}
			}
		}

		if (hImageMemory)
		{
			::GlobalUnlock(hImageMemory); //�����ڴ�

			::GlobalFree(hImageMemory); //�ͷ�ȫ���ڴ�
			hImageMemory = NULL;
		}
		if (pIPicture)
		{
			pIPicture->Release(); //�ͷ�pIPicture
			pIPicture = NULL;
		}
		if (pIStream)
		{
			pIStream->Release(); //�ͷ�pIStream
			pIStream = NULL;
		}
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hFile); //�ر��ļ����
			hFile = INVALID_HANDLE_VALUE;
		}
	}
	///////////////////////////////////////////////////////////
	//���´����ʵ�ֵĹ����Ǵ�ָ����·���ж�ȡͼƬ������ʾ����
	//
	__inline static void ImagesDisplayScreen(SIZE & szImageSize, const _TCHAR * tImagePath)
	{
		HANDLE hFile = NULL;

		DWORD dwReadedSize = 0; //����ʵ�ʶ�ȡ���ļ���С
		IStream *pIStream = NULL;//����һ��IStream�ӿ�ָ�룬��������ͼƬ��
		IPicture *pIPicture = NULL;//����һ��IPicture�ӿ�ָ�룬��ʾͼƬ����
		VOID * pImageMemory = NULL;
		HGLOBAL hImageMemory = NULL;
		OLE_XSIZE_HIMETRIC hmWidth = 0;
		OLE_YSIZE_HIMETRIC hmHeight = 0;
		LARGE_INTEGER liFileSize = { 0, 0 };

		hFile = ::CreateFile(tImagePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//��ָ����·��szImagePath�ж�ȡ�ļ����
		if (hFile != INVALID_HANDLE_VALUE)
		{
			liFileSize.LowPart = ::GetFileSize(hFile, (DWORD *)&liFileSize.HighPart); //���ͼƬ�ļ��Ĵ�С����������ȫ���ڴ�
			hImageMemory = ::GlobalAlloc(GMEM_MOVEABLE, liFileSize.QuadPart); //��ͼƬ����ȫ���ڴ�
			if (hImageMemory)
			{
				pImageMemory = ::GlobalLock(hImageMemory); //�����ڴ�

				::ReadFile(hFile, pImageMemory, liFileSize.QuadPart, &dwReadedSize, NULL); //��ȡͼƬ��ȫ���ڴ浱��

				::CreateStreamOnHGlobal(hImageMemory, false, &pIStream); //��ȫ���ڴ��ʹ��IStream�ӿ�ָ��
				if (pIStream)
				{
					::OleLoadPicture(pIStream, 0, false, IID_IPicture, (LPVOID*)&(pIPicture));//��OleLoadPicture���IPicture�ӿ�ָ��
					if (pIPicture)														  //�õ�IPicture COM�ӿڶ���󣬾Ϳ��Խ��л��ͼƬ��Ϣ����ʾͼƬ�Ȳ���
					{
						pIPicture->get_Width(&hmWidth); //�ýӿڷ������ͼƬ�Ŀ�͸�
						pIPicture->get_Height(&hmHeight); //�ýӿڷ������ͼƬ�Ŀ�͸�
						szImageSize.cx = hmWidth;
						szImageSize.cy = hmHeight;
					}
				}
			}
		}

		if (hImageMemory)
		{
			::GlobalUnlock(hImageMemory); //�����ڴ�

			::GlobalFree(hImageMemory); //�ͷ�ȫ���ڴ�
			hImageMemory = NULL;
		}
		if (pIPicture)
		{
			pIPicture->Release(); //�ͷ�pIPicture
			pIPicture = NULL;
		}
		if (pIStream)
		{
			pIStream->Release(); //�ͷ�pIStream
			pIStream = NULL;
		}
		if (hFile != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(hFile); //�ر��ļ����
			hFile = INVALID_HANDLE_VALUE;
		}
	}
	__inline static
		void DrawMemoryBitmap(HDC &dc, HWND hWnd, LONG lWidth, LONG lHeight, HBITMAP hBitmap)
	{
		RECT rect;
		HBITMAP hOldBitmap;
		int disHeight, disWidth;

		GetClientRect(hWnd, &rect);//��ȡ�ͻ�����С
		disHeight = rect.bottom - rect.top;
		disWidth = rect.right - rect.left;

		HDC mDc = ::CreateCompatibleDC(dc);//������ǰ�����ĵļ���dc(�ڴ�DC)
		hOldBitmap = (HBITMAP)::SelectObject(mDc, hBitmap);//��λͼ���ص��ڴ�DC

		//�����ڴ�DC���ݿ鵽��ǰDC���Զ�����
		::StretchBlt(dc, 0, 0, disWidth, disHeight, mDc, 0, 0, lWidth, lHeight, SRCCOPY);

		//�ָ��ڴ�ԭʼ����
		::SelectObject(mDc, hOldBitmap);

		//ɾ����Դ����ֹй©
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

		GetClientRect(hWnd, &rect);//��ȡ�ͻ�����С
		disHeight = rect.bottom - rect.top;
		disWidth = rect.right - rect.left;

		//����ͼƬ
		hOrgBitmap = (HBITMAP)::LoadImage(GetModuleHandle(NULL), ptFileName, IMAGE_BITMAP, lWidth, lHeight, LR_LOADFROMFILE);

		HDC mDc = ::CreateCompatibleDC(dc);//������ǰ�����ĵļ���dc(�ڴ�DC)
		hOldBitmap = (HBITMAP)::SelectObject(mDc, hOrgBitmap);//��λͼ���ص��ڴ�DC

		//�����ڴ�DC���ݿ鵽��ǰDC���Զ�����
		::StretchBlt(dc, 0, 0, disWidth, disHeight, mDc, 0, 0, lWidth, lHeight, SRCCOPY);

		//�ָ��ڴ�ԭʼ����
		::SelectObject(mDc, hOldBitmap);

		//ɾ����Դ����ֹй©
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
		HFONT CreatePaintFont(LPCTSTR ptszFaceName = _T("����"),
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

		//�����øղŵõ���pBuffer��dwSize����һЩ��Ҫ�����顣����ֱ�����ڴ���ʹ
		//�ã�Ҳ����д�뵽Ӳ���ļ����������Ǽ򵥵�д�뵽Ӳ���ļ���������ǵ��Զ�
		//����Դ����ΪǶ��DLL��Ӧ�ã��������Ҫ����һЩ��
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
	__inline static
		BOOL DragMoveFull(HWND hWnd)
	{
#ifndef _SYSCOMMAND_SC_DRAGMOVE
#define _SYSCOMMAND_SC_DRAGMOVE  0xF012
#endif // !_SYSCOMMAND_SC_DRAGMOVE
		::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, NULL, 0);
		::SendMessage(hWnd, WM_SYSCOMMAND, _SYSCOMMAND_SC_DRAGMOVE, 0);
		//RECT rc = { 0 };
		//::GetClientRect(hWnd, &rc);
		//::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, FALSE, NULL, 0);
		//::PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(rc.left, rc.top));
#ifdef _SYSCOMMAND_SC_DRAGMOVE
#undef _SYSCOMMAND_SC_DRAGMOVE  
#endif // !_SYSCOMMAND_SC_DRAGMOVE

		return TRUE;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	//�������ܣ�����Բ�Ǵ���
	//����������
	//	hWnd		Ҫ���õĴ��ھ��
	//	pstEllipse	Ҫ����Բ�ǵĺ���뾶������뾶
	//	prcExcepted	Ҫ�ų�Բ�ǵ��������²��С
	//����ֵ���޷���
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
	// ����˵������ʧ��ģʽ��ȡHWND��HBITMAP
	// ��    �������ھ��
	// �� �� ֵ������HBITMAP
	// �� д ��: ppshuai 20141126
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

		// Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
		hMemDC = CreateCompatibleDC(hWndDC);

		// ����һ������Ļ�豸��������ݵ�λͼ
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// ����λͼѡ���ڴ��豸��������
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// ���ò�ʧ��ģʽ
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// ����Ļ�豸�����������ڴ��豸��������
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// ���ò�ʧ��ģʽ
		SetStretchBltMode(hMemDC, nMode);

		// �õ���Ļλͼ�ľ��
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// ���DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// �ͷ�DC
		ReleaseDC(hWnd, hWndDC);

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// ����˵������ʧ��ģʽ��ȡHWND��HBITMAP
	// ��    �������ھ����ָ������λ�ü���С
	// �� �� ֵ������HBITMAP
	// �� д ��: ppshuai 20141126
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

		// Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
		hMemDC = CreateCompatibleDC(hWndDC);

		// ����һ������Ļ�豸��������ݵ�λͼ
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// ����λͼѡ���ڴ��豸��������
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// ���ò�ʧ��ģʽ
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// ����Ļ�豸�����������ڴ��豸��������
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// ���ò�ʧ��ģʽ
		SetStretchBltMode(hMemDC, nMode);

		// �õ���Ļλͼ�ľ��
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// ���DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// �ͷ�DC
		ReleaseDC(hWnd, hWndDC);

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// ����˵������ʧ��ģʽ��ȡHWND��HBITMAP
	// ��    �������ھ����ָ������λ�ü���С
	// �� �� ֵ������HBITMAP
	// �� д ��: ppshuai 20141126
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

		// Ϊ��Ļ�豸�����������ݵ��ڴ��豸������
		hMemDC = CreateCompatibleDC(hWndDC);

		// ����һ������Ļ�豸��������ݵ�λͼ
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// ����λͼѡ���ڴ��豸��������
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// ���ò�ʧ��ģʽ
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// ����Ļ�豸�����������ڴ��豸��������
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// ���ò�ʧ��ģʽ
		SetStretchBltMode(hMemDC, nMode);

		// �õ���Ļλͼ�ľ��
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// ���DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// �ͷ�DC
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
	// ����˵������ʧ��ģʽ��ȡHDC��HBITMAP
	// ��    �����豸DC����С
	// �� �� ֵ������HBITMAP
	// �� д ��: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBitmapFromHdc(HDC hDC, DWORD BitWidth, DWORD BitHeight)
	{
		HDC hMemDC;
		INT nMode = 0;
		HBITMAP hBitmap, hBitTemp;
		//�����豸������(HDC)
		hMemDC = CreateCompatibleDC(hDC);
		//����HBITMAP
		hBitmap = CreateCompatibleBitmap(hDC, BitWidth, BitHeight);
		hBitTemp = (HBITMAP)SelectObject(hMemDC, hBitmap);

		nMode = SetStretchBltMode(hDC, COLORONCOLOR);//���ò�ʧ��ģʽ
		//�õ�λͼ������
		StretchBlt(hMemDC, 0, 0, BitWidth, BitHeight, hDC, 0, 0, BitWidth, BitHeight, SRCCOPY);
		SetStretchBltMode(hDC, nMode);
		//�õ����յ�λͼ��Ϣ
		hBitmap = (HBITMAP)SelectObject(hMemDC, hBitTemp);
		//�ͷ��ڴ�
		DeleteObject(hBitTemp);
		DeleteDC(hMemDC);

		return hBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// ����˵������ȡHWND��IStream������,�����ڴ���(ʹ��������ֶ��ͷ�ReleaseGlobalHandle)
	// ��    �������ھ��,���IStream��
	// �� �� ֵ������HANDLE.(ʹ��������ֶ��ͷ�ReleaseGlobalHandle)
	// �� д ��: ppshuai 20141126
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
	// ����˵������ȡHWND��IStream������,�����ڴ���(ʹ��������ֶ��ͷ�ReleaseGlobalHandle)
	// ��    �������ھ��,ָ�����꼰��С�����IStream��
	// �� �� ֵ������HANDLE.(ʹ��������ֶ��ͷ�ReleaseGlobalHandle)
	// �� д ��: ppshuai 20141126
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
	// ����˵�����ͷ��ڴ���
	// ��    �������ھ��,���IStream��
	// �� �� ֵ���޷���ֵ
	// �� д ��: ppshuai 20141126
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
	// ����˵��������HBITMAP��BMPͼƬ�ļ�
	// ��    �������ھ��,Ҫ�����ļ�����
	// �� �� ֵ��bool���͡��ɹ�����true;ʧ�ܷ���false;
	// �� д ��: ppshuai 20141126
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

		//����λͼ�ļ�ÿ��������ռ�ֽ���
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
	// ����˵��������HBITMAP��BMPͼƬ�ļ�
	// ��    ����λͼ���,Ҫ�����ļ�����
	// �� �� ֵ��bool���͡��ɹ�����true;ʧ�ܷ���false;
	// �� д ��: ppshuai 20141126
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

		//����λͼ�ļ�ÿ��������ռ�ֽ���
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
	// ����˵��������HWND��BMPͼƬ�ļ�
	// ��    �������ھ��,Ҫ�����ļ�����
	// �� �� ֵ��bool���͡��ɹ�����true;ʧ�ܷ���false;
	// �� д ��: ppshuai 20141126
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
	// ����˵������ʾHBITMAP��HWND��
	// ��    �������ھ��,HBITMAPλͼ���
	// �� �� ֵ��bool���͡��ɹ�����true;ʧ�ܷ���false;
	// �� д ��: ppshuai 20141126
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

		//��ȡͼƬ��ʾ��Ĵ�С
		GetClientRect(hStaticWnd, &rect);

		//��ȡλͼ�Ĵ�С��Ϣ
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

		//���ò�ʧ��ģʽ
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
	// ����˵������ʾHBITMAP��HWND��
	// ��    ���������ھ��,Դ�ӿؼ�ID��Ŀ���ӿؼ�ID
	// �� �� ֵ���޷���ֵ;
	// �� д ��: ppshuai 20141126
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
	// ����˵������ʾHBITMAP��HWND��
	// ��    ���������ھ��,HBITMAP�����Ŀ���ӿؼ�ID
	// �� �� ֵ���޷���ֵ;
	// �� д ��: ppshuai 20141126
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
		//��ǰ�ֱ�����ÿ������ռ�ֽ���
		WORD wDPIBits = 0;
		//λͼ��ÿ������ռ�ֽ���
		WORD wBitCount = 0;
		//�����ɫ���С
		DWORD dwPaletteSize = 0;
		//λͼ�������ֽڴ�С
		DWORD dwBmBitsSize = 0;
		//λͼ�ļ���С
		DWORD dwDibBitSize = 0;
		//д���ļ��ֽ���
		DWORD dwWritten = 0;
		//λͼ���Խṹ
		BITMAP bmp = { 0 };
		//λͼ�ļ�ͷ�ṹ
		BITMAPFILEHEADER bmpfh = { 0 };
		//λͼ��Ϣͷ�ṹ
		BITMAPINFOHEADER bmpih = { 0 };
		//ָ��λͼ��Ϣͷ�ṹ
		BITMAPINFOHEADER * pbmpih = NULL;
		//�����ļ�
		HANDLE hFile = NULL;
		//�����ڴ���
		HGLOBAL hDibBit = NULL;
		//��ǰ��ɫ����
		HPALETTE hPaltte = NULL;
		//���ݵ�ɫ����
		HPALETTE hPaltteBackup = NULL;

		//����λͼ�ļ�ÿ��������ռ�ֽ���
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

		//Ϊλͼ���ݷ����ڴ�
		hDibBit = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		pbmpih = (LPBITMAPINFOHEADER)GlobalLock(hDibBit);
		*pbmpih = bmpih;

		//�����ɫ��
		hPaltte = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
		if (hPaltte)
		{
			hDC = ::GetDC(NULL);
			hPaltteBackup = ::SelectPalette(hDC, (HPALETTE)hPaltte, FALSE);
			::RealizePalette(hDC);
		}

		//��ȡ�õ�ɫ�����µ�����ֵ
		GetDIBits(hDC,
			hBitmap,
			0,
			(UINT)bmp.bmHeight,
			((BYTE *)pbmpih + sizeof(BITMAPINFOHEADER) + dwPaletteSize),
			(BITMAPINFO *)pbmpih,
			DIB_RGB_COLORS);

		//�ָ���ɫ��
		if (hPaltteBackup)
		{
			::SelectPalette(hDC, (HPALETTE)hPaltteBackup, TRUE);
			::RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//����λͼ�ļ�
		hFile = CreateFile((LPCTSTR)pFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			//����λͼ�ļ�ͷ
			bmpfh.bfType = 0x4D42;   //"BM"
			bmpfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
			bmpfh.bfReserved1 = 0;
			bmpfh.bfReserved2 = 0;
			bmpfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
			dwDibBitSize = bmpfh.bfSize;

			//д��λͼ�ļ�ͷ
			WriteFile(hFile, (void *)&bmpfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

			//д��λͼ�ļ���������
			WriteFile(hFile, (void *)pbmpih, dwDibBitSize, &dwWritten, NULL);

			//���
			GlobalUnlock(hDibBit);
			GlobalFree(hDibBit);
			hDibBit = NULL;

			//�ر��ļ�
			CloseHandle(hFile);
			hFile = NULL;
		}

		DeleteDC(hDC);
		hDC = NULL;

		return TRUE;
	}
	
	///////////////////////////////////////////////////////////////////////////////
	
	__inline static INT_PTR CALLBACK DlgWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
		{

		}
		break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
			{
				EndDialog(hWnd, LOWORD(wParam));
			}
			break;
			case IDCANCEL:
			{
				EndDialog(hWnd, LOWORD(wParam));
			}
			break;
			default:
				break;
			}
		}
		break;
		default:
			break;
		}
		return FALSE;
	}

	class CDlgItemTemplate
	{
	public:
		CDlgItemTemplate()
		{

		}
		CDlgItemTemplate(CDlgItemTemplate & dit)
		{
			this->Initialize(dit.dwStyle, dit.dwExStyle, dit.wX, dit.wY, dit.wCX, dit.wCY, dit.wID, dit.tCN.c_str(), dit.tTN.c_str(), dit.wCDAIT);
		}
		CDlgItemTemplate(DWORD _dwStyle, DWORD _dwExStyle, WORD _wX, WORD _wY, WORD _wCX, WORD _wCY, WORD _wID, LPCTSTR _tCN, LPCTSTR _tTN, WORD _wCDAIT)
		{
			this->Initialize(_dwStyle, _dwExStyle, _wX, _wY, _wCX, _wCY, _wID, _tCN, _tTN, _wCDAIT);
		}

		void Initialize(DWORD _dwStyle, DWORD _dwExStyle, WORD _wX, WORD _wY, WORD _wCX, WORD _wCY, WORD _wID, LPCTSTR _tCN, LPCTSTR _tTN, WORD _wCDAIT)
		{
			this->dwStyle = _dwStyle;
			this->dwExStyle = _dwExStyle;
			this->wX = _wX;
			this->wY = _wY;
			this->wCX = _wCX;
			this->wCY = _wCY;
			this->wID = _wID;
			this->tCN = _tCN;
			this->tTN = _tTN;
			this->wCDAIT = _wCDAIT;
		}

	public:
		DWORD dwStyle;
		DWORD dwExStyle;
		WORD wX;
		WORD wY;
		WORD wCX;
		WORD wCY;
		WORD wID;
		TSTRING tCN;
		TSTRING tTN;
		WORD wCDAIT;
	};

	class CDlgTemplate
	{
	public:
		CDlgTemplate()
		{

		}
		CDlgTemplate(CDlgTemplate & cdt)
		{
			this->Initialize(cdt.dwStyle, cdt.dwExStyle, cdt.wCDIT, cdt.wX, cdt.wY, cdt.wCX, cdt.wCY, cdt.wMENU, cdt.tCN.c_str(), cdt.tTN.c_str(), cdt.wFS, cdt.tFN.c_str(), &cdt.SDITMAP);
		}
		CDlgTemplate(DWORD _dwStyle, DWORD _dwExStyle, WORD _wCDIT, WORD _wX, WORD _wY, WORD _wCX, WORD _wCY, WORD _wMENU, LPCTSTR _tCN, LPCTSTR _tTN, WORD _wFS, LPCTSTR _tFN, std::map<SIZE_T, CDlgItemTemplate> * _SDITMAP)
		{
			this->Initialize(_dwStyle, _dwExStyle, _wCDIT, _wX, _wY, _wCX, _wCY, _wMENU, _tCN, _tTN, _wFS, _tFN, _SDITMAP);
		}

		void Initialize(DWORD _dwStyle, DWORD _dwExStyle, WORD _wCDIT, WORD _wX, WORD _wY, WORD _wCX, WORD _wCY, WORD _wMENU, LPCTSTR _tCN, LPCTSTR _tTN, WORD _wFS, LPCTSTR _tFN, std::map<SIZE_T, CDlgItemTemplate> * _SDITMAP)
		{
			this->dwStyle = _dwStyle;
			this->dwExStyle = _dwExStyle;
			this->wCDIT = _wCDIT;
			this->wX = _wX;
			this->wY = _wY;
			this->wCX = _wCX;
			this->wCY = _wCY;
			this->wMENU = _wMENU;
			this->tCN = _tCN;
			this->tTN = _tTN;
			this->wFS = _wFS;
			this->tFN = _tFN;
			if (_SDITMAP)
			{
				SDITMAP.insert(_SDITMAP->begin(), _SDITMAP->end());
			}
		}

	public:
		DWORD dwStyle;
		DWORD dwExStyle;
		WORD wCDIT;
		WORD wX;
		WORD wY;
		WORD wCX;
		WORD wCY;
		WORD wMENU;
		TSTRING tCN;
		TSTRING tTN;
		WORD wFS;
		TSTRING tFN;

		std::map<SIZE_T, CDlgItemTemplate> SDITMAP;
	};
	
	__inline static void * InitDlgData(SIZE_T * pstSize, std::map<SIZE_T, CDlgTemplate> * pSDTMAP)
	{
		BYTE * pbData = NULL;
		SIZE_T stPlusSize = 0L;
		std::map<SIZE_T, CDlgTemplate>::iterator itSDTEnd;
		std::map<SIZE_T, CDlgTemplate>::iterator itSDTIdx;
		std::map<SIZE_T, CDlgItemTemplate>::iterator itSDITEnd;
		std::map<SIZE_T, CDlgItemTemplate>::iterator itSDITIdx;

		itSDTEnd = pSDTMAP->end();
		itSDTIdx = pSDTMAP->begin();
		for (; itSDTIdx != itSDTEnd; itSDTIdx++)
		{
			stPlusSize = sizeof(itSDTIdx->second.dwStyle);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.dwStyle, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = sizeof(itSDTIdx->second.dwExStyle);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.dwExStyle, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = sizeof(itSDTIdx->second.wCDIT);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.wCDIT, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = sizeof(itSDTIdx->second.wX);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.wX, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = sizeof(itSDTIdx->second.wY);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.wY, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = sizeof(itSDTIdx->second.wCX);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.wCX, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = sizeof(itSDTIdx->second.wCY);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.wCY, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = sizeof(itSDTIdx->second.wMENU);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.wMENU, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = (Convert::TToW(itSDTIdx->second.tCN).length() + 1) * sizeof(WCHAR);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), Convert::TToW(itSDTIdx->second.tCN).c_str(), stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = (Convert::TToW(itSDTIdx->second.tTN).length() + 1) * sizeof(WCHAR);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), Convert::TToW(itSDTIdx->second.tTN).c_str(), stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = sizeof(itSDTIdx->second.wFS);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), &itSDTIdx->second.wFS, stPlusSize);
			(*pstSize) += stPlusSize;

			stPlusSize = (Convert::TToW(itSDTIdx->second.tFN).length() + 1) * sizeof(WCHAR);
			pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
			memcpy(pbData + (*pstSize), Convert::TToW(itSDTIdx->second.tFN).c_str(), stPlusSize);
			(*pstSize) += stPlusSize;

			itSDITEnd = itSDTIdx->second.SDITMAP.end();
			itSDITIdx = itSDTIdx->second.SDITMAP.begin();
			for (; itSDITIdx != itSDITEnd; itSDITIdx++)
			{
				stPlusSize = sizeof(itSDITIdx->second.dwStyle);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), &itSDITIdx->second.dwStyle, stPlusSize);
				(*pstSize) += stPlusSize;

				stPlusSize = sizeof(itSDITIdx->second.dwExStyle);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), &itSDITIdx->second.dwExStyle, stPlusSize);
				(*pstSize) += stPlusSize;

				stPlusSize = sizeof(itSDITIdx->second.wX);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), &itSDITIdx->second.wX, stPlusSize);
				(*pstSize) += stPlusSize;

				stPlusSize = sizeof(itSDITIdx->second.wY);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), &itSDITIdx->second.wY, stPlusSize);
				(*pstSize) += stPlusSize;

				stPlusSize = sizeof(itSDITIdx->second.wCX);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), &itSDITIdx->second.wCX, stPlusSize);
				(*pstSize) += stPlusSize;

				stPlusSize = sizeof(itSDITIdx->second.wCY);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), &itSDITIdx->second.wCY, stPlusSize);
				(*pstSize) += stPlusSize;

				stPlusSize = sizeof(itSDITIdx->second.wID);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), &itSDITIdx->second.wID, stPlusSize);
				(*pstSize) += stPlusSize;

				////////////////////////////////////////////////////////////////////
				stPlusSize = (Convert::TToW(itSDITIdx->second.tCN).length() + 1) * sizeof(WCHAR);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), Convert::TToW(itSDITIdx->second.tCN).c_str(), stPlusSize);
				(*pstSize) += stPlusSize;

				stPlusSize = (Convert::TToW(itSDITIdx->second.tTN).length() + 1) * sizeof(WCHAR);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), Convert::TToW(itSDITIdx->second.tTN).c_str(), stPlusSize);
				(*pstSize) += stPlusSize;

				stPlusSize = sizeof(itSDITIdx->second.wCDAIT);
				pbData = (BYTE *)realloc(pbData, ((*pstSize) + stPlusSize) * sizeof(BYTE));
				memcpy(pbData + (*pstSize), &itSDITIdx->second.wCDAIT, stPlusSize);
				(*pstSize) += stPlusSize;
			}
		}
		return pbData;
	}
	__inline static std::string InitDlgData(std::map<SIZE_T, CDlgTemplate> * pSDTMAP)
	{
		std::string strData("");
		std::map<SIZE_T, CDlgTemplate>::iterator itSDTEnd;
		std::map<SIZE_T, CDlgTemplate>::iterator itSDTIdx;
		std::map<SIZE_T, CDlgItemTemplate>::iterator itSDITIEnd;
		std::map<SIZE_T, CDlgItemTemplate>::iterator itSDITIdx;

		itSDTEnd = pSDTMAP->end();
		itSDTIdx = pSDTMAP->begin();
		for (; itSDTIdx != itSDTEnd; itSDTIdx++)
		{
			strData.append((const char *)&itSDTIdx->second.dwStyle, sizeof(itSDTIdx->second.dwStyle));
			strData.append((const char *)&itSDTIdx->second.dwExStyle, sizeof(itSDTIdx->second.dwExStyle));
			strData.append((const char *)&itSDTIdx->second.wCDIT, sizeof(itSDTIdx->second.wCDIT));
			strData.append((const char *)&itSDTIdx->second.wX, sizeof(itSDTIdx->second.wX));
			strData.append((const char *)&itSDTIdx->second.wY, sizeof(itSDTIdx->second.wY));
			strData.append((const char *)&itSDTIdx->second.wCX, sizeof(itSDTIdx->second.wCX));
			strData.append((const char *)&itSDTIdx->second.wCY, sizeof(itSDTIdx->second.wCY));
			strData.append((const char *)&itSDTIdx->second.wMENU, sizeof(itSDTIdx->second.wMENU));
			strData.append((const char *)Convert::TToW(itSDTIdx->second.tCN).c_str(), (Convert::TToW(itSDTIdx->second.tCN).length() + 1) * sizeof(WCHAR) * sizeof(BYTE));
			strData.append((const char *)Convert::TToW(itSDTIdx->second.tTN).c_str(), (Convert::TToW(itSDTIdx->second.tTN).length() + 1) * sizeof(WCHAR) * sizeof(BYTE));
			strData.append((const char *)&itSDTIdx->second.wFS, sizeof(itSDTIdx->second.wFS) * sizeof(BYTE));
			strData.append((const char *)Convert::TToW(itSDTIdx->second.tFN).c_str(), (Convert::TToW(itSDTIdx->second.tFN).length() + 1) * sizeof(WCHAR) * sizeof(BYTE));
			
			itSDITIEnd = itSDTIdx->second.SDITMAP.end();
			itSDITIdx = itSDTIdx->second.SDITMAP.begin();
			for (; itSDITIdx != itSDITIEnd; itSDITIdx++)
			{
				strData.append((const char *)&itSDITIdx->second.dwStyle, sizeof(itSDITIdx->second.dwStyle) * sizeof(BYTE));
				strData.append((const char *)&itSDITIdx->second.dwExStyle, sizeof(itSDITIdx->second.dwExStyle) * sizeof(BYTE));
				strData.append((const char *)&itSDITIdx->second.wX, sizeof(itSDITIdx->second.wX) * sizeof(BYTE));
				strData.append((const char *)&itSDITIdx->second.wY, sizeof(itSDITIdx->second.wY) * sizeof(BYTE));
				strData.append((const char *)&itSDITIdx->second.wCX, sizeof(itSDITIdx->second.wCX) * sizeof(BYTE));
				strData.append((const char *)&itSDITIdx->second.wCY, sizeof(itSDITIdx->second.wCY) * sizeof(BYTE));
				strData.append((const char *)&itSDITIdx->second.wID, sizeof(itSDITIdx->second.wID) * sizeof(BYTE));
				strData.append((const char *)Convert::TToW(itSDITIdx->second.tCN).c_str(), (Convert::TToW(itSDITIdx->second.tCN).length() + 1) * sizeof(WCHAR) * sizeof(BYTE));
				strData.append((const char *)Convert::TToW(itSDITIdx->second.tTN).c_str(), (Convert::TToW(itSDITIdx->second.tTN).length() + 1) * sizeof(WCHAR) * sizeof(BYTE));
				strData.append((const char *)&itSDITIdx->second.wCDAIT, sizeof(itSDITIdx->second.wCDAIT) * sizeof(BYTE));
			}
		}
		return strData;
	}
	__inline static void * InitParams()
	{		
		DLGTEMPLATE dt = { 0 };
		DLGITEMTEMPLATE dit = { 0 };
		BYTE * pbData = NULL;
		SIZE_T stSize = 0L;
		SIZE_T stPlusSize = 0L;
		WORD wMenu = 0;
		WORD wFontSize = 0;
		WORD wCdit = 0;
		WCHAR wClassName[MAX_PATH] = { 0 };
		WCHAR wTitleName[MAX_PATH] = { 0 };
		WCHAR wFontName[MAX_PATH] = { 0 };
		SIZE_T stChildControlsNum = 2;
		
		stSize = 0;
		stPlusSize = sizeof(DLGTEMPLATE);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		dt.style = DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
		dt.dwExtendedStyle = 0;
		dt.cdit = stChildControlsNum;
		dt.x = 0;
		dt.y = 0;
		dt.cx = 278;
		dt.cy = 54;
		memcpy(pbData + stSize, &dt, stPlusSize);
		stSize += stPlusSize;

		wMenu = 0;
		stPlusSize = sizeof(wMenu);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &wMenu, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wClassName, L"");
		stPlusSize = (wcslen(wClassName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wClassName, stPlusSize);
		stSize += stPlusSize;
		
		wcscpy(wTitleName, L"Zipping");
		stPlusSize = (wcslen(wTitleName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wTitleName, stPlusSize);
		stSize += stPlusSize;

		wFontSize = 8;
		stPlusSize = sizeof(wFontSize);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &wFontSize, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wFontName, L"MS Sans Serif");
		stPlusSize = (wcslen(wFontName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wFontName, stPlusSize);
		stSize += stPlusSize;

		dit.style = BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE;
		dit.dwExtendedStyle = 0;
		dit.x = 113;
		dit.y = 32;
		dit.cx = 50;
		dit.cy = 14;
		dit.id = IDCANCEL;
		stPlusSize = sizeof(dit);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &dit, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wClassName, L"Button");
		stPlusSize = (wcslen(wClassName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wClassName, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wTitleName, L"Cancel");
		stPlusSize = (wcslen(wTitleName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wTitleName, stPlusSize);
		stSize += stPlusSize;

		wCdit = 0;
		stPlusSize = sizeof(wCdit);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &wCdit, stPlusSize);
		stSize += stPlusSize;

		dit.style = WS_CHILD | WS_VISIBLE;
		dit.dwExtendedStyle = 0;
		dit.x = 7;
		dit.y = 7;
		dit.cx = 264;
		dit.cy = 18;
		dit.id = 1;
		stPlusSize = sizeof(dit);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &dit, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wClassName, L"msctls_progress32");
		stPlusSize = (wcslen(wClassName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wClassName, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wTitleName, L"");
		stPlusSize = (wcslen(wTitleName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wTitleName, stPlusSize);
		stSize += stPlusSize;

		wCdit = 0;
		stPlusSize = sizeof(wCdit);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &wCdit, stPlusSize);
		stSize += stPlusSize;

		return pbData;
	}

	__inline static INT_PTR CreateDialogBoxTTT()
	{
		HINSTANCE hInstance = NULL;
#pragma pack(push,1)
		struct TDlgItemTemplate { DWORD s, ex; short x, y, cx, cy; WORD id; };
		struct TDlgTemplate { DWORD s, ex; WORD cdit; short x, y, cx, cy; };
		struct TDlgItem1 { TDlgItemTemplate dli; WCHAR wclass[7]; WCHAR title[7]; WORD cdat; };
		struct TDlgItem2 { TDlgItemTemplate dli; WCHAR wclass[18]; WCHAR title[1]; WORD cdat; };
		struct TDlgData  { TDlgTemplate dlt; WORD menu; WCHAR wclass[1]; WCHAR title[8]; WORD fontsize; WCHAR font[14]; TDlgItem1 i1; TDlgItem2 i2; };
		TDlgData dtp = {
			{ DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, 2, 0, 0, 278, 54 },
			0, L"", L"Zipping", 8, L"MS Sans Serif",
			{ { BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0, 113, 32, 50, 14, IDCANCEL }, L"BUTTON", L"Cancel", 0 },
			{ { WS_CHILD | WS_VISIBLE, 0, 7, 7, 264, 18, 1 }, L"msctls_progress32", L"", 0 } };
#pragma pack(pop)

		hInstance = GetModuleHandle(NULL);

		InitCommonControls();

		int res = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)&dtp, 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);
		if (res == IDCANCEL) return 0;
		return DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)&dtp, 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);
	}

	__inline static INT_PTR CreateDialogBox()
	{
		INT_PTR nRet = 0;
		SIZE_T stSize = 0;
		VOID * pbData = NULL;
		HINSTANCE hInstance = NULL;
		std::map<SIZE_T, CDlgTemplate> sdtmap;
		std::map<SIZE_T, CDlgItemTemplate> sditmap;
		std::map<SIZE_T, CDlgTemplate>::iterator itSDTEnd;
		std::map<SIZE_T, CDlgTemplate>::iterator itSDTIdx;

		sditmap.insert(std::map<SIZE_T, CDlgItemTemplate>::value_type(sditmap.size(),
			CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0, 
			113, 32, 50, 14, IDCANCEL, WC_BUTTON, _T("Cancel"), 0)));
		sditmap.insert(std::map<SIZE_T, CDlgItemTemplate>::value_type(sditmap.size(),
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE, 0, 
			7, 7, 264, 18, 1, PROGRESS_CLASS, _T(""), 0)));
		sdtmap.insert(std::map<SIZE_T, CDlgTemplate>::value_type(sdtmap.size(),
			CDlgTemplate(DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_VISIBLE, 0, 
			2, 0, 0, 278, 54, 0, _T(""), _T("Zipping"), 8, _T("MS Sans Serif"), &sditmap)));
		
		itSDTEnd = sdtmap.end();
		itSDTIdx = sdtmap.begin();
		for (; itSDTIdx != itSDTEnd; itSDTIdx++)
		{
			itSDTIdx->second.wCDIT = itSDTIdx->second.SDITMAP.size();
		}

		pbData = InitDlgData(&stSize, &sdtmap);

		hInstance = GetModuleHandle(NULL);

		InitCommonControls();

		nRet = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)pbData, 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);

		free(pbData);
		pbData = NULL;

		return nRet;
	}

	__inline static INT_PTR CreateDialogBoxEx()
	{
		INT_PTR nResult = 0;
		HINSTANCE hInstance = NULL;
		std::string strDlgData((""));
		std::map<SIZE_T, CDlgTemplate> sdtmap;
		std::map<SIZE_T, CDlgItemTemplate> sditmap;
		std::map<SIZE_T, CDlgTemplate>::iterator itSDTEnd;
		std::map<SIZE_T, CDlgTemplate>::iterator itSDTIdx;

		sditmap.insert(std::map<SIZE_T, CDlgItemTemplate>::value_type(sditmap.size(),
			CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0, 
			63, 32, 50, 14, IDOK, WC_BUTTON, _T("Ok"), 0)));
		sditmap.insert(std::map<SIZE_T, CDlgItemTemplate>::value_type(sditmap.size(),
			CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0, 
			113, 32, 50, 14, IDCANCEL, WC_BUTTON, _T("Cancel"), 0)));
		sditmap.insert(std::map<SIZE_T, CDlgItemTemplate>::value_type(sditmap.size(),
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE, 0, 
			7, 7, 264, 18, 1, PROGRESS_CLASS, _T(""), 0)));
		sdtmap.insert(std::map<SIZE_T, CDlgTemplate>::value_type(sdtmap.size(),
			CDlgTemplate(DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, 
			2, 0, 0, 278, 54, 0, _T(""), _T("Zipping"), 8, _T("MS Sans Serif"), &sditmap)));

		itSDTEnd = sdtmap.end();
		itSDTIdx = sdtmap.begin();
		for (; itSDTIdx != itSDTEnd; itSDTIdx++)
		{
			itSDTIdx->second.wCDIT = itSDTIdx->second.SDITMAP.size();
		}

		strDlgData = InitDlgData(&sdtmap);

		hInstance = GetModuleHandle(NULL);

		InitCommonControls();

		nResult = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)strDlgData.c_str(), 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);
		if (nResult != IDCANCEL)
		{
			//nResult = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)strDlgData.c_str(), 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);
		}

		return nResult;
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
	
	typedef struct tagEnumWindowInfo
	{
		HWND hWnd;
		DWORD dwPid;
	}ENUMWINDOWINFO, *PENUMWINDOWINFO;

	__inline static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
	{
		ENUMWINDOWINFO* pInfo = (ENUMWINDOWINFO*)lParam;
		DWORD dwProcessId = 0;
		GetWindowThreadProcessId(hWnd, &dwProcessId);

		if (dwProcessId == pInfo->dwPid)
		{
			pInfo->hWnd = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	/*******************************************************
	*��������:���ս���ID��ȡ�����ھ��
	*��������:����1������ID
	*��������:HWND
	*ע������:��
	*����޸�ʱ��:2017/5/13
	*******************************************************/
	__inline static HWND GetHwndByProcessId(DWORD dwProcessId)
	{
		ENUMWINDOWINFO info = { 0 };
		info.hWnd = NULL;
		info.dwPid = dwProcessId;
		EnumWindows(EnumWindowsProc, (LPARAM)&info);
		return info.hWnd;
	}
}
}

