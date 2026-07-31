/*-----------------------------------------------------------------------------

	Platform.h

	Cross-platform abstraction layer for Dark Eden client.
	Provides unified API for Windows, Linux, and macOS.

	Original Windows API dependencies are abstracted here.

	2025.01.14

-----------------------------------------------------------------------------*/

#ifndef __DARKEDEN_CLIENT_PLATFORM_H__
#define __DARKEDEN_CLIENT_PLATFORM_H__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <type_traits>
#include <vector>
#include <list>
#include <map>

template <typename A, typename B>
static inline typename std::common_type<A, B>::type platform_max_compat(A a, B b) {
	typedef typename std::common_type<A, B>::type R;
	return (static_cast<R>(a) > static_cast<R>(b)) ? static_cast<R>(a) : static_cast<R>(b);
}

template <typename A, typename B>
static inline typename std::common_type<A, B>::type platform_min_compat(A a, B b) {
	typedef typename std::common_type<A, B>::type R;
	return (static_cast<R>(a) < static_cast<R>(b)) ? static_cast<R>(a) : static_cast<R>(b);
}

#ifndef max
#define max(a, b) platform_max_compat((a), (b))
#endif
#ifndef min
#define min(a, b) platform_min_compat((a), (b))
#endif
#endif

/* Define assert macro for non-Windows platforms */
#ifndef PLATFORM_WINDOWS
#ifndef assert
#define assert(e) ((void)(e))
#endif
#endif

/* ============================================================================
 * Platform Detection
 * ============================================================================ */

/* Detect platform */
#if defined(_WIN32) || defined(_WIN64)
	#define PLATFORM_WIN32_HOST
	#ifndef PLATFORM_USE_SDL
		#define PLATFORM_WINDOWS
	#endif
#elif defined(__linux__)
	#define PLATFORM_LINUX
#elif defined(__APPLE__)
	#include <TargetConditionals.h>
	#if TARGET_OS_MAC
		#ifndef PLATFORM_MACOS
			#define PLATFORM_MACOS
		#endif
	#endif
#else
	#define PLATFORM_UNKNOWN
#endif

/* ============================================================================
 * Platform Selection
 * ============================================================================ */

/* Force SDL backend on non-Windows platforms */
#ifndef PLATFORM_WINDOWS
	#ifndef PLATFORM_USE_SDL
		#define PLATFORM_USE_SDL
	#endif
#endif

/* Allow explicit SDL selection on Windows */
#ifdef PLATFORM_USE_SDL
	#if __has_include(<SDL2/SDL.h>)
		#include <SDL2/SDL.h>
	#elif __has_include(<SDL.h>)
		#include <SDL.h>
	#else
		#include <SDL2/SDL.h>  // Try anyway - include path should be set via CMake
	#endif
#endif

/* ============================================================================
 * Calling Conventions
 * ============================================================================ */

/* Define calling conventions only for true non-Windows compilers.
   MSVC needs __cdecl/__stdcall intact even when the SDL backend is selected. */
#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_WIN32_HOST)
	#ifndef __cdecl
		#define __cdecl
	#endif
	#ifndef __stdcall
		#define __stdcall
	#endif
	#ifndef WINAPI
		#define WINAPI
	#endif
	#ifndef APIENTRY
		#define APIENTRY
	#endif
	#ifndef CALLBACK
		#define CALLBACK
	#endif
	#ifndef INLINE
		#define INLINE inline
	#endif
#endif

/* ============================================================================
 * Basic Type Definitions (from Typedef.h)
 * ============================================================================ */

#ifndef NULL
	#define NULL 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif

#define NOT_SELECTED						-1

/* Type definitions (same as original Typedef.h) */
typedef uint8_t			BYTE;
typedef uint16_t		WORD;
typedef uint32_t		UINT;
typedef uint32_t		DWORD;
typedef uint32_t		ULONG;
typedef uint64_t		QWORD;
typedef uint64_t		DWORD64;
typedef uint64_t		ULONGLONG;
typedef int64_t			LONGLONG;
typedef void*			PVOID;
typedef void*			LPVOID;
typedef void*			ADDRESS_MODE;
typedef uintptr_t		ULONG_PTR;
typedef intptr_t		LONG_PTR;
typedef uintptr_t		DWORD_PTR;
typedef int32_t			LONG;
typedef int				BOOL;
typedef const char*		LPCSTR;
typedef char*			LPSTR;
typedef const char*		LPCTSTR;
typedef char*			LPTSTR;
typedef void			VOID;
typedef int32_t			HRESULT;
typedef intptr_t		LRESULT;
typedef uintptr_t		UINT_PTR;
typedef intptr_t		LPARAM;
typedef intptr_t		WPARAM;
typedef void*			HWND;
typedef void*			HDC;
typedef void*			HFONT;
typedef void*			HINSTANCE;
typedef void*			HANDLE;
typedef void*			HMODULE;
typedef void*			HMENU;
typedef void*			HBRUSH;
typedef void*			HICON;
typedef void*			HCURSOR;
typedef void*			HINTERNET;
typedef DWORD*			LPDWORD;
typedef unsigned char*	LPBYTE;
typedef const wchar_t*	LPCWSTR;
typedef wchar_t*		LPWSTR;

#ifndef PLATFORM_TCHAR_COMPAT_DEFINED
#define PLATFORM_TCHAR_COMPAT_DEFINED
typedef char			TCHAR;
typedef char			_TCHAR;
#endif

#ifndef _T
#define _T(x) x
#endif
#ifndef TEXT
#define TEXT(x) x
#endif
#ifndef _stscanf
#define _stscanf sscanf
#endif

#if defined(PLATFORM_USE_SDL) && defined(PLATFORM_WIN32_HOST)
#ifndef PLATFORM_WIN32_SDL_MESSAGE_COMPAT_DEFINED
#define PLATFORM_WIN32_SDL_MESSAGE_COMPAT_DEFINED
#ifndef S_OK
#define S_OK 0
#endif
#ifndef S_FALSE
#define S_FALSE 1
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif
#ifndef MB_OK
#define MB_OK 0x00000000L
#endif
#ifndef WM_USER
#define WM_USER 0x0400
#endif
#ifndef WM_TIMER
#define WM_TIMER 0x0113
#endif
#ifndef WM_CHAR
#define WM_CHAR 0x0102
#endif
#ifndef WM_KEYDOWN
#define WM_KEYDOWN 0x0100
#endif
#ifndef WM_KEYUP
#define WM_KEYUP 0x0101
#endif
#ifndef WM_DESTROY
#define WM_DESTROY 0x0002
#endif
#ifndef WM_CLOSE
#define WM_CLOSE 0x0010
#endif
#ifndef WM_QUIT
#define WM_QUIT 0x0012
#endif
#ifndef WM_SYSCOMMAND
#define WM_SYSCOMMAND 0x0112
#endif
#ifndef WM_MOVE
#define WM_MOVE 0x0003
#endif
#ifndef WM_GETMINMAXINFO
#define WM_GETMINMAXINFO 0x0024
#endif
#ifndef WM_ACTIVATEAPP
#define WM_ACTIVATEAPP 0x001C
#endif
#ifndef WM_IME_COMPOSITION
#define WM_IME_COMPOSITION 0x010F
#endif
#ifndef WM_IME_STARTCOMPOSITION
#define WM_IME_STARTCOMPOSITION 0x010D
#endif
#ifndef WM_IME_ENDCOMPOSITION
#define WM_IME_ENDCOMPOSITION 0x010E
#endif
#ifndef MM_MCINOTIFY
#define MM_MCINOTIFY 0x3D9
#endif
#ifndef MCI_NOTIFY_SUCCESSFUL
#define MCI_NOTIFY_SUCCESSFUL 0x0001
#endif
#ifndef SC_HOTKEY
#define SC_HOTKEY 0xF150
#endif
#ifndef SC_KEYMENU
#define SC_KEYMENU 0xF100
#endif
#ifndef SC_TASKLIST
#define SC_TASKLIST 0xF140
#endif
#ifndef SC_PREVWINDOW
#define SC_PREVWINDOW 0xF050
#endif
#ifndef SC_NEXTWINDOW
#define SC_NEXTWINDOW 0xF040
#endif
#ifndef SC_CLOSE
#define SC_CLOSE 0xF060
#endif
#ifndef SC_MOVE
#define SC_MOVE 0xF010
#endif
#ifndef SC_SIZE
#define SC_SIZE 0xF000
#endif
#ifndef SC_MAXIMIZE
#define SC_MAXIMIZE 0xF030
#endif
#ifndef SC_MONITORPOWER
#define SC_MONITORPOWER 0xF170
#endif
#ifndef VK_SCROLL
#define VK_SCROLL 0x91
#endif
#ifndef CP_UTF8
#define CP_UTF8 65001
#endif
#ifndef CP_OEMCP
#define CP_OEMCP 1
#endif
#ifndef WC_COMPOSITECHECK
#define WC_COMPOSITECHECK 0x00000200
#endif
#ifndef PM_NOREMOVE
#define PM_NOREMOVE 0x0000
#endif
#ifndef SM_CXSIZEFRAME
#define SM_CXSIZEFRAME 32
#endif
#ifndef SM_CYSIZEFRAME
#define SM_CYSIZEFRAME 33
#endif
#ifndef SM_CYMENU
#define SM_CYMENU 15
#endif
#ifndef __builtin_expect
#define __builtin_expect(expr, expected) (expr)
#endif
#ifndef MAKEINTRESOURCE
#define MAKEINTRESOURCE(i) ((LPCTSTR)((DWORD_PTR)((WORD)(i))))
#endif
#ifndef IDC_ARROW
#define IDC_ARROW ((LPCTSTR)"MAKEINTRESOURCE(32512)")
#endif
#ifndef BLACK_BRUSH
#define BLACK_BRUSH 4
#endif
#ifndef CS_HREDRAW
#define CS_HREDRAW 0x0001
#endif
#ifndef CS_VREDRAW
#define CS_VREDRAW 0x0002
#endif
#ifndef CS_DBLCLKS
#define CS_DBLCLKS 0x0008
#endif
#ifndef WS_EX_APPWINDOW
#define WS_EX_APPWINDOW 0x00040000L
#endif
#ifndef WS_POPUP
#define WS_POPUP 0x80000000L
#endif
#ifndef WS_OVERLAPPED
#define WS_OVERLAPPED 0x00000000L
#endif
#ifndef WS_CLIPCHILDREN
#define WS_CLIPCHILDREN 0x02000000L
#endif
#ifndef WS_THICKFRAME
#define WS_THICKFRAME 0x00040000L
#endif
#ifndef WS_MINIMIZEBOX
#define WS_MINIMIZEBOX 0x00020000L
#endif
#ifndef WS_SYSMENU
#define WS_SYSMENU 0x00080000L
#endif
#ifndef ENUM_CURRENT_SETTINGS
#define ENUM_CURRENT_SETTINGS ((DWORD)-1)
#endif
#ifndef CDS_RESET
#define CDS_RESET 0x40000000
#endif
#ifndef PROCESS_ALL_ACCESS
#define PROCESS_ALL_ACCESS 0x001F0FFF
#endif
#ifndef TH32CS_SNAPPROCESS
#define TH32CS_SNAPPROCESS 0x00000002
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif
#ifndef INTERNET_OPEN_TYPE_PRECONFIG
#define INTERNET_OPEN_TYPE_PRECONFIG 0
#endif
#ifndef INTERNET_FLAG_RESYNCHRONIZE
#define INTERNET_FLAG_RESYNCHRONIZE 0x00000800
#endif
#ifndef INTERNET_OPTION_DATAFILE_NAME
#define INTERNET_OPTION_DATAFILE_NAME 33
#endif
#ifndef SPI_SETSCREENSAVERRUNNING
#define SPI_SETSCREENSAVERRUNNING 97
#endif
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS 1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT 2
#endif
#ifndef STILL_ACTIVE
#define STILL_ACTIVE ((DWORD)-1)
#endif
#ifndef THREAD_PRIORITY_NORMAL
#define THREAD_PRIORITY_NORMAL 0
#endif
#ifndef THREAD_PRIORITY_LOWEST
#define THREAD_PRIORITY_LOWEST -2
#endif
#ifndef IWebBrowser2_DEFINED
#define IWebBrowser2_DEFINED
typedef void* IWebBrowser2;
#endif
typedef long (__cdecl *WNDPROC)(void*, unsigned int, unsigned long, long long);
typedef struct tagWNDCLASS {
	UINT style;
	void* lpfnWndProc;
	int cbClsExtra;
	int cbWndExtra;
	HINSTANCE hInstance;
	HICON hIcon;
	HCURSOR hCursor;
	HBRUSH hbrBackground;
	LPCTSTR lpszMenuName;
	LPCTSTR lpszClassName;
} WNDCLASS, *PWNDCLASS, *LPWNDCLASS;
typedef struct tagPOINT {
	LONG x;
	LONG y;
} POINT, *PPOINT, *LPPOINT;
#ifndef POINT_DEFINED
#define POINT_DEFINED
#endif
typedef struct tagMINMAXINFO {
	POINT ptReserved;
	POINT ptMaxSize;
	POINT ptMaxPosition;
	POINT ptMinTrackSize;
	POINT ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;
typedef struct _devicemode {
	char dmDeviceName[32];
	WORD dmSpecVersion;
	WORD dmDriverVersion;
	WORD dmSize;
	WORD dmDriverExtra;
	DWORD dmFields;
	LONG dmPositionX;
	LONG dmPositionY;
	DWORD dmDisplayOrientation;
	DWORD dmDisplayFixedOutput;
	short dmColor;
	short dmDuplex;
	short dmYResolution;
	short dmTTOption;
	short dmCollate;
	char dmFormName[32];
	WORD dmLogPixels;
	DWORD dmBitsPerPel;
	DWORD dmPelsWidth;
	DWORD dmPelsHeight;
	DWORD dmDisplayFlags;
	DWORD dmDisplayFrequency;
} DEVMODE, *PDEVMODE, *LPDEVMODE;
typedef struct _SECURITY_ATTRIBUTES {
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
typedef struct tagMSG {
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD time;
	POINT pt;
} MSG, *PMSG, *LPMSG;
typedef struct _OSVERSIONINFO {
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	char szCSDVersion[128];
} OSVERSIONINFO, *POSVERSIONINFO, *LPOSVERSIONINFO;
typedef struct tagPROCESSENTRY32 {
	DWORD dwSize;
	DWORD cntUsage;
	DWORD th32ProcessID;
	ULONG_PTR th32DefaultHeapID;
	DWORD th32ModuleID;
	DWORD cntThreads;
	DWORD th32ParentProcessID;
	LONG pcPriClassBase;
	DWORD dwFlags;
	char szExeFile[260];
} PROCESSENTRY32, *PPROCESSENTRY32, *LPPROCESSENTRY32;
typedef LONG (__stdcall *LPTOP_LEVEL_EXCEPTION_FILTER)(struct _EXCEPTION_POINTERS*);
static inline LRESULT DefWindowProc(void* hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	(void)hWnd; (void)Msg; (void)wParam; (void)lParam;
	return 0;
}
static inline void PostQuitMessage(int nExitCode) {
	(void)nExitCode;
}
static inline HICON LoadIcon(HINSTANCE hInstance, LPCTSTR lpIconName) {
	(void)hInstance; (void)lpIconName;
	return NULL;
}
static inline HCURSOR LoadCursor(HINSTANCE hInstance, LPCTSTR lpCursorName) {
	(void)hInstance; (void)lpCursorName;
	return NULL;
}
static inline HBRUSH GetStockObject(int fnObject) {
	(void)fnObject;
	return NULL;
}
static inline int RegisterClass(const WNDCLASS* lpWndClass) {
	(void)lpWndClass;
	return 1;
}
static inline void SetCursor(HCURSOR hCursor) {
	(void)hCursor;
}
static inline BOOL UpdateWindow(HWND hWnd) {
	(void)hWnd;
	return TRUE;
}
static inline HWND SetFocus(HWND hWnd) {
	return hWnd;
}
static inline BOOL EnumDisplaySettings(LPCTSTR lpszDeviceName, DWORD iModeNum, DEVMODE* lpDevMode) {
	(void)lpszDeviceName; (void)iModeNum;
	if (lpDevMode != NULL) {
		lpDevMode->dmPelsWidth = 1024;
		lpDevMode->dmPelsHeight = 768;
		lpDevMode->dmBitsPerPel = 16;
		lpDevMode->dmDisplayFrequency = 60;
	}
	return TRUE;
}
static inline LONG ChangeDisplaySettings(DEVMODE* lpDevMode, DWORD dwFlags) {
	(void)lpDevMode; (void)dwFlags;
	return 0;
}
static inline HANDLE CreateMutex(SECURITY_ATTRIBUTES* lpMutexAttributes, BOOL bInitialOwner, LPCTSTR lpName) {
	(void)lpMutexAttributes; (void)bInitialOwner; (void)lpName;
	return (HANDLE)1;
}
static inline BOOL ReleaseMutex(HANDLE hMutex) {
	(void)hMutex;
	return TRUE;
}
static inline HWND FindWindow(LPCTSTR lpClassName, LPCTSTR lpWindowName) {
	(void)lpClassName; (void)lpWindowName;
	return NULL;
}
static inline DWORD GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize) {
	(void)hModule;
	if (lpFilename != NULL && nSize > 0) {
		const char* name = "DarkEden.exe";
		size_t len = strlen(name);
		if (len >= nSize) len = nSize - 1;
		memcpy(lpFilename, name, len);
		lpFilename[len] = '\0';
		return (DWORD)len;
	}
	return 0;
}
static inline BOOL SetCurrentDirectory(LPCTSTR lpPathName) {
	(void)lpPathName;
	return TRUE;
}
static inline DWORD GetWindowThreadProcessId(HWND hWnd, LPDWORD lpdwProcessId) {
	(void)hWnd;
	if (lpdwProcessId != NULL) *lpdwProcessId = 0;
	return 0;
}
static inline HANDLE OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId) {
	(void)dwDesiredAccess; (void)bInheritHandle; (void)dwProcessId;
	return NULL;
}
static inline BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode) {
	(void)hProcess; (void)uExitCode;
	return FALSE;
}
static inline void Sleep(DWORD dwMilliseconds) {
	SDL_Delay(dwMilliseconds);
}
static inline BOOL TerminateThread(HANDLE thread, DWORD exitCode) {
	(void)thread; (void)exitCode; return TRUE;
}
static inline BOOL GetExitCodeThread(HANDLE thread, LPDWORD lpExitCode) {
	(void)thread; if (lpExitCode) *lpExitCode = STILL_ACTIVE; return TRUE;
}
static inline HANDLE GetCurrentThread(void) {
	return NULL;
}
#ifndef PLATFORM_SET_THREAD_PRIORITY_DEFINED
#define PLATFORM_SET_THREAD_PRIORITY_DEFINED
static inline BOOL SetThreadPriority(HANDLE thread, int priority) {
	(void)thread; (void)priority; return TRUE;
}
#endif
static inline BOOL DeleteFile(LPCTSTR lpFileName) {
	return (remove(lpFileName) == 0) ? TRUE : FALSE;
}
static inline BOOL CopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL bFailIfExists) {
	(void)bFailIfExists;
	FILE* src = fopen(lpExistingFileName, "rb");
	if (src == NULL) return FALSE;
	FILE* dst = fopen(lpNewFileName, "wb");
	if (dst == NULL) {
		fclose(src);
		return FALSE;
	}
	char buffer[8192];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
		fwrite(buffer, 1, n, dst);
	}
	fclose(src);
	fclose(dst);
	return TRUE;
}
static inline LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
	(void)lpTopLevelExceptionFilter;
	return NULL;
}
static inline BOOL PeekMessage(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
	(void)lpMsg; (void)hWnd; (void)wMsgFilterMin; (void)wMsgFilterMax; (void)wRemoveMsg;
	return FALSE;
}
static inline BOOL GetMessage(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
	(void)lpMsg; (void)hWnd; (void)wMsgFilterMin; (void)wMsgFilterMax;
	return FALSE;
}
static inline BOOL TranslateMessage(const MSG* lpMsg) {
	(void)lpMsg;
	return TRUE;
}
static inline LRESULT DispatchMessage(const MSG* lpMsg) {
	(void)lpMsg;
	return 0;
}
static inline BOOL WaitMessage(void) {
	SDL_Delay(1);
	return TRUE;
}
static inline BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, void* pvParam, UINT fWinIni) {
	(void)uiAction; (void)uiParam; (void)pvParam; (void)fWinIni;
	return FALSE;
}
static inline BOOL GetVersionEx(OSVERSIONINFO* lpVersionInformation) {
	if (lpVersionInformation != NULL) {
		lpVersionInformation->dwMajorVersion = 10;
		lpVersionInformation->dwMinorVersion = 0;
		lpVersionInformation->dwBuildNumber = 19045;
		lpVersionInformation->dwPlatformId = VER_PLATFORM_WIN32_NT;
		lpVersionInformation->szCSDVersion[0] = '\0';
	}
	return TRUE;
}
static inline BOOL GetCursorPos(POINT* lpPoint) {
	if (lpPoint != NULL) {
		int x = 0;
		int y = 0;
		SDL_GetMouseState(&x, &y);
		lpPoint->x = x;
		lpPoint->y = y;
		return TRUE;
	}
	return FALSE;
}
static inline BOOL ScreenToClient(HWND hWnd, POINT* lpPoint) {
	(void)hWnd;
	return (lpPoint != NULL) ? TRUE : FALSE;
}
static inline HANDLE CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID) {
	(void)dwFlags; (void)th32ProcessID;
	return INVALID_HANDLE_VALUE;
}
static inline BOOL Process32First(HANDLE hSnapshot, PROCESSENTRY32* lppe) {
	(void)hSnapshot; (void)lppe;
	return FALSE;
}
static inline BOOL Process32Next(HANDLE hSnapshot, PROCESSENTRY32* lppe) {
	(void)hSnapshot; (void)lppe;
	return FALSE;
}
static inline BOOL CloseHandle(HANDLE hObject) {
	(void)hObject;
	return TRUE;
}
static inline BOOL InternetCloseHandle(HINTERNET hInternet) {
	(void)hInternet;
	return TRUE;
}
static inline HINTERNET InternetOpen(LPCTSTR lpszAgent, DWORD dwAccessType, LPCTSTR lpszProxyName, LPCTSTR lpszProxyBypass, DWORD dwFlags) {
	(void)lpszAgent; (void)dwAccessType; (void)lpszProxyName; (void)lpszProxyBypass; (void)dwFlags;
	return (HINTERNET)1;
}
static inline BOOL InternetSetOption(HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength) {
	(void)hInternet; (void)dwOption; (void)lpBuffer; (void)dwBufferLength;
	return TRUE;
}
static inline BOOL InternetQueryOption(HINTERNET hInternet, DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufferLength) {
	(void)hInternet; (void)dwOption; (void)lpBuffer; (void)lpdwBufferLength;
	return FALSE;
}
static inline BOOL InternetGetLastResponseInfo(LPDWORD lpdwError, LPSTR lpszBuffer, LPDWORD lpdwBufferLength) {
	if (lpdwError != NULL) *lpdwError = 0;
	if (lpszBuffer != NULL && lpdwBufferLength != NULL && *lpdwBufferLength > 0) lpszBuffer[0] = '\0';
	return FALSE;
}
static inline BOOL PathIsURL(LPCTSTR pszPath) {
	if (pszPath == NULL) return FALSE;
	return (strncmp(pszPath, "http://", 7) == 0 || strncmp(pszPath, "https://", 8) == 0) ? TRUE : FALSE;
}
static inline HINTERNET InternetOpenUrl(HINTERNET hInternet, LPCTSTR lpszUrl, LPCTSTR lpszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext) {
	(void)hInternet; (void)lpszUrl; (void)lpszHeaders; (void)dwHeadersLength; (void)dwFlags; (void)dwContext;
	return NULL;
}
static inline BOOL InternetReadFile(HINTERNET hFile, LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead) {
	(void)hFile; (void)lpBuffer; (void)dwNumberOfBytesToRead;
	if (lpdwNumberOfBytesRead != NULL) *lpdwNumberOfBytesRead = 0;
	return FALSE;
}
#endif
#endif

#ifndef FAR
#define FAR
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef LOWORD
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif
#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#endif
#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | (((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16)))
#endif
#ifndef MAKEWPARAM
#define MAKEWPARAM(l, h) ((WPARAM)(DWORD)MAKELONG(l, h))
#endif
#ifndef MAKELPARAM
#define MAKELPARAM(l, h) ((LPARAM)(DWORD)MAKELONG(l, h))
#endif
#ifndef MAKELRESULT
#define MAKELRESULT(l, h) ((LRESULT)(DWORD)MAKELONG(l, h))
#endif
#ifndef TEXT
#define TEXT(x) x
#endif
#ifndef _T
#define _T(x) x
#endif
#ifndef _stscanf
#define _stscanf sscanf
#endif
#ifndef GetDoubleClickTime
#define GetDoubleClickTime() 500
#endif
#ifndef DDSCAPS_SYSTEMMEMORY
#define DDSCAPS_SYSTEMMEMORY 0x00000800
#endif
#ifndef SW_HIDE
#define SW_HIDE 0
#endif
#ifndef SM_CYVSCROLL
#define SM_CYVSCROLL 20
#endif
#ifndef SM_CXSCREEN
#define SM_CXSCREEN 0
#endif
#ifndef SM_CYSCREEN
#define SM_CYSCREEN 1
#endif
#ifndef WS_EX_TOPMOST
#define WS_EX_TOPMOST 0x00000008L
#endif
#ifndef WS_VISIBLE
#define WS_VISIBLE 0x10000000L
#endif
#ifndef WM_USER
#define WM_USER 0x0400
#endif
#ifndef PBS_SMOOTH
#define PBS_SMOOTH 0x01
#endif
#ifndef PBM_SETRANGE
#define PBM_SETRANGE (WM_USER + 1)
#endif
#ifndef PBM_SETPOS
#define PBM_SETPOS (WM_USER + 2)
#endif
#ifndef PBM_STEPIT
#define PBM_STEPIT (WM_USER + 5)
#endif
#ifndef PBM_SETSTEP
#define PBM_SETSTEP (WM_USER + 4)
#endif
#ifndef PROGRESS_CLASS
#define PROGRESS_CLASS "msctls_progress32"
#endif
typedef void* HMENU;
static inline int ShowCursor(BOOL bShow) { (void)bShow; return 0; }
static inline BOOL ShowWindow(HWND hWnd, int nCmdShow) { (void)hWnd; (void)nCmdShow; return TRUE; }
static inline void InitCommonControls(void) {}
static inline int GetSystemMetrics(int nIndex) { (void)nIndex; return 0; }
static inline HWND CreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	(void)dwExStyle; (void)lpClassName; (void)lpWindowName; (void)dwStyle; (void)X; (void)Y; (void)nWidth; (void)nHeight; (void)hWndParent; (void)hMenu; (void)hInstance; (void)lpParam;
	return NULL;
}
static inline LRESULT SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	(void)hWnd; (void)Msg; (void)wParam; (void)lParam;
	return 0;
}
static inline BOOL SetWindowText(HWND hWnd, LPCSTR lpString) {
	(void)hWnd; (void)lpString;
	return TRUE;
}
#ifndef PLATFORM_WIDECHAR_TO_MULTIBYTE_DEFINED
#define PLATFORM_WIDECHAR_TO_MULTIBYTE_DEFINED
static inline int WideCharToMultiByte(UINT CodePage, DWORD dwFlags,
	LPCWSTR lpWideCharStr, int cchWideChar,
	LPSTR lpMultiByteStr, int cbMultiByte,
	LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar) {
	(void)CodePage; (void)dwFlags; (void)lpDefaultChar; (void)lpUsedDefaultChar;
	if (lpWideCharStr == NULL || lpMultiByteStr == NULL || cbMultiByte <= 0) return 0;
	if (cchWideChar == -1) {
		int len = 0;
		while (lpWideCharStr[len]) len++;
		cchWideChar = len;
	}
	int n = (cchWideChar < cbMultiByte - 1) ? cchWideChar : cbMultiByte - 1;
	for (int i = 0; i < n; ++i) {
		wchar_t ch = lpWideCharStr[i];
		lpMultiByteStr[i] = (ch <= 0x7f) ? (char)ch : '?';
	}
	lpMultiByteStr[n] = '\0';
	return n;
}
#endif

#ifndef PLATFORM_SYSTEMTIME_DEFINED
#define PLATFORM_SYSTEMTIME_DEFINED
typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif

#ifndef PLATFORM_GET_LOCAL_TIME_STUB_DEFINED
#define PLATFORM_GET_LOCAL_TIME_STUB_DEFINED
static inline void GetLocalTime(LPSYSTEMTIME lpSystemTime) {
	if (lpSystemTime) {
		time_t aclock;
		time(&aclock);
		struct tm* now = localtime(&aclock);
		lpSystemTime->wYear = now ? (WORD)(now->tm_year + 1900) : 1970;
		lpSystemTime->wMonth = now ? (WORD)(now->tm_mon + 1) : 1;
		lpSystemTime->wDayOfWeek = now ? (WORD)now->tm_wday : 0;
		lpSystemTime->wDay = now ? (WORD)now->tm_mday : 1;
		lpSystemTime->wHour = now ? (WORD)now->tm_hour : 0;
		lpSystemTime->wMinute = now ? (WORD)now->tm_min : 0;
		lpSystemTime->wSecond = now ? (WORD)now->tm_sec : 0;
		lpSystemTime->wMilliseconds = 0;
	}
}
#endif

#if defined(PLATFORM_USE_SDL) && defined(PLATFORM_WIN32_HOST) && !defined(PLATFORM_WINSOCK_COMPAT_DEFINED)
#define PLATFORM_WINSOCK_COMPAT_DEFINED
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef UINT_PTR SOCKET;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET)(~0))
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#ifndef AF_INET
#define AF_INET 2
#endif
#ifndef SOCK_STREAM
#define SOCK_STREAM 1
#endif
#ifndef SOCK_DGRAM
#define SOCK_DGRAM 2
#endif
#ifndef IPPROTO_TCP
#define IPPROTO_TCP 6
#endif
#ifndef IPPROTO_UDP
#define IPPROTO_UDP 17
#endif
#ifndef SOL_SOCKET
#define SOL_SOCKET 0xffff
#endif
#ifndef SO_REUSEADDR
#define SO_REUSEADDR 0x0004
#endif
#ifndef SO_LINGER
#define SO_LINGER 0x0080
#endif
#ifndef SO_SNDBUF
#define SO_SNDBUF 0x1001
#endif
#ifndef SO_RCVBUF
#define SO_RCVBUF 0x1002
#endif
#ifndef FIONREAD
#define FIONREAD 0x4004667f
#endif
#ifndef FIONBIO
#define FIONBIO 0x8004667e
#endif
#ifndef INADDR_ANY
#define INADDR_ANY 0x00000000
#endif
#ifndef SD_RECEIVE
#define SD_RECEIVE 0
#endif
#ifndef SD_SEND
#define SD_SEND 1
#endif
#ifndef SD_BOTH
#define SD_BOTH 2
#endif
#ifndef WSAAPI
#define WSAAPI __stdcall
#endif
#ifndef WSABASEERR
#define WSABASEERR 10000
#endif
#ifndef WSANOTINITIALISED
#define WSANOTINITIALISED (WSABASEERR + 93)
#endif
#ifndef WSAENETDOWN
#define WSAENETDOWN (WSABASEERR + 50)
#endif
#ifndef WSAEAFNOSUPPORT
#define WSAEAFNOSUPPORT (WSABASEERR + 47)
#endif
#ifndef WSAEINPROGRESS
#define WSAEINPROGRESS (WSABASEERR + 36)
#endif
#ifndef WSAEMFILE
#define WSAEMFILE (WSABASEERR + 24)
#endif
#ifndef WSAENOBUFS
#define WSAENOBUFS (WSABASEERR + 55)
#endif
#ifndef WSAEPROTONOSUPPORT
#define WSAEPROTONOSUPPORT (WSABASEERR + 43)
#endif
#ifndef WSAEPROTOTYPE
#define WSAEPROTOTYPE (WSABASEERR + 41)
#endif
#ifndef WSAESOCKTNOSUPPORT
#define WSAESOCKTNOSUPPORT (WSABASEERR + 44)
#endif
#ifndef WSAEADDRINUSE
#define WSAEADDRINUSE (WSABASEERR + 48)
#endif
#ifndef WSAEADDRNOTAVAIL
#define WSAEADDRNOTAVAIL (WSABASEERR + 49)
#endif
#ifndef WSAEFAULT
#define WSAEFAULT (WSABASEERR + 14)
#endif
#ifndef WSAEINVAL
#define WSAEINVAL (WSABASEERR + 22)
#endif
#ifndef WSAENOTSOCK
#define WSAENOTSOCK (WSABASEERR + 38)
#endif
#ifndef WSAEINTR
#define WSAEINTR (WSABASEERR + 4)
#endif
#ifndef WSAEALREADY
#define WSAEALREADY (WSABASEERR + 37)
#endif
#ifndef WSAECONNREFUSED
#define WSAECONNREFUSED (WSABASEERR + 61)
#endif
#ifndef WSAEISCONN
#define WSAEISCONN (WSABASEERR + 56)
#endif
#ifndef WSAENETUNREACH
#define WSAENETUNREACH (WSABASEERR + 51)
#endif
#ifndef WSAETIMEDOUT
#define WSAETIMEDOUT (WSABASEERR + 60)
#endif
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK (WSABASEERR + 35)
#endif
#ifndef WSAEOPNOTSUPP
#define WSAEOPNOTSUPP (WSABASEERR + 45)
#endif
#ifndef WSAENOPROTOOPT
#define WSAENOPROTOOPT (WSABASEERR + 42)
#endif
#ifndef WSAENETRESET
#define WSAENETRESET (WSABASEERR + 52)
#endif
#ifndef WSAENOTCONN
#define WSAENOTCONN (WSABASEERR + 57)
#endif
#ifndef WSAEACCES
#define WSAEACCES (WSABASEERR + 13)
#endif
#ifndef WSAESHUTDOWN
#define WSAESHUTDOWN (WSABASEERR + 58)
#endif
#ifndef WSAEMSGSIZE
#define WSAEMSGSIZE (WSABASEERR + 40)
#endif
#ifndef WSAEHOSTUNREACH
#define WSAEHOSTUNREACH (WSABASEERR + 65)
#endif
#ifndef WSAECONNABORTED
#define WSAECONNABORTED (WSABASEERR + 53)
#endif
#ifndef WSAECONNRESET
#define WSAECONNRESET (WSABASEERR + 54)
#endif

struct in_addr {
	u_long s_addr;
};

struct sockaddr {
	u_short sa_family;
	char sa_data[14];
};

struct sockaddr_in {
	short sin_family;
	u_short sin_port;
	struct in_addr sin_addr;
	char sin_zero[8];
};

struct linger {
	u_short l_onoff;
	u_short l_linger;
};

struct hostent {
	char* h_name;
	char** h_aliases;
	short h_addrtype;
	short h_length;
	char** h_addr_list;
};

struct timeval {
	long tv_sec;
	long tv_usec;
};

typedef struct fd_set {
	u_int fd_count;
	SOCKET fd_array[64];
} fd_set;

typedef struct WSAData {
	WORD wVersion;
	WORD wHighVersion;
	char szDescription[257];
	char szSystemStatus[129];
	unsigned short iMaxSockets;
	unsigned short iMaxUdpDg;
	char* lpVendorInfo;
} WSADATA, *LPWSADATA;

extern "C" {
	int WSAAPI WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
	int WSAAPI WSACleanup(void);
	int WSAAPI WSAGetLastError(void);
	SOCKET WSAAPI socket(int af, int type, int protocol);
	int WSAAPI bind(SOCKET s, const struct sockaddr* name, int namelen);
	int WSAAPI connect(SOCKET s, const struct sockaddr* name, int namelen);
	int WSAAPI listen(SOCKET s, int backlog);
	SOCKET WSAAPI accept(SOCKET s, struct sockaddr* addr, int* addrlen);
	int WSAAPI getsockopt(SOCKET s, int level, int optname, char* optval, int* optlen);
	int WSAAPI setsockopt(SOCKET s, int level, int optname, const char* optval, int optlen);
	int WSAAPI send(SOCKET s, const char* buf, int len, int flags);
	int WSAAPI sendto(SOCKET s, const char* buf, int len, int flags, const struct sockaddr* to, int tolen);
	int WSAAPI recv(SOCKET s, char* buf, int len, int flags);
	int WSAAPI recvfrom(SOCKET s, char* buf, int len, int flags, struct sockaddr* from, int* fromlen);
	int WSAAPI closesocket(SOCKET s);
	int WSAAPI ioctlsocket(SOCKET s, long cmd, u_long* argp);
	int WSAAPI shutdown(SOCKET s, int how);
	u_long WSAAPI inet_addr(const char* cp);
	char* WSAAPI inet_ntoa(struct in_addr in);
	u_short WSAAPI htons(u_short hostshort);
	u_long WSAAPI htonl(u_long hostlong);
	u_short WSAAPI ntohs(u_short netshort);
	u_long WSAAPI ntohl(u_long netlong);
	int WSAAPI gethostname(char* name, int namelen);
	struct hostent* WSAAPI gethostbyname(const char* name);
	int WSAAPI select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const struct timeval* timeout);
}
#endif
#ifndef PLATFORM_CHAR_T_DEFINED
#define PLATFORM_CHAR_T_DEFINED
typedef WORD			char_t;
#endif

#ifndef PLATFORM_COLORREF_DEFINED
#define PLATFORM_COLORREF_DEFINED
typedef DWORD			COLORREF;
#endif
#ifndef RGB
#define RGB(r,g,b)		((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif

#ifndef FW_NORMAL
#define FW_NORMAL 400
#endif
#ifndef FW_BOLD
#define FW_BOLD 700
#endif
#ifndef FW_THIN
#define FW_THIN 100
#endif
#ifndef FW_MEDIUM
#define FW_MEDIUM 500
#endif
#ifndef FW_HEAVY
#define FW_HEAVY 900
#endif
#ifndef FW_EXTRABOLD
#define FW_EXTRABOLD 800
#endif
#ifndef FW_LIGHT
#define FW_LIGHT 300
#endif

#ifndef LOGFONT_DEFINED
#define LOGFONT_DEFINED
typedef struct tagLOGFONT {
	long lfHeight;
	long lfWidth;
	long lfEscapement;
	long lfOrientation;
	long lfWeight;
	unsigned char lfItalic;
	unsigned char lfUnderline;
	unsigned char lfStrikeOut;
	unsigned char lfCharSet;
	unsigned char lfOutPrecision;
	unsigned char lfClipPrecision;
	unsigned char lfQuality;
	unsigned char lfPitchAndFamily;
	char lfFaceName[32];
} LOGFONT, *PLOGFONT, *LPLOGFONT;
#endif

#ifndef TRANSPARENT
#define TRANSPARENT 1
#endif
#ifndef OPAQUE
#define OPAQUE 2
#endif
#ifndef TA_NOUPDATECP
#define TA_NOUPDATECP 0
#endif
#ifndef TA_LEFT
#define TA_LEFT 0
#endif
#ifndef TA_TOP
#define TA_TOP 0
#endif
#ifndef TA_UPDATECP
#define TA_UPDATECP 1
#endif
#ifndef TA_RIGHT
#define TA_RIGHT 2
#endif
#ifndef TA_CENTER
#define TA_CENTER 6
#endif
#ifndef TA_BASELINE
#define TA_BASELINE 24
#endif

#ifndef ANSI_CHARSET
#define ANSI_CHARSET 0
#endif
#ifndef DEFAULT_CHARSET
#define DEFAULT_CHARSET 1
#endif
#ifndef SYMBOL_CHARSET
#define SYMBOL_CHARSET 2
#endif
#ifndef SHIFTJIS_CHARSET
#define SHIFTJIS_CHARSET 128
#endif
#ifndef HANGUL_CHARSET
#define HANGUL_CHARSET 129
#endif
#ifndef GB2312_CHARSET
#define GB2312_CHARSET 134
#endif
#ifndef OEM_CHARSET
#define OEM_CHARSET 255
#endif
#ifndef OUT_DEFAULT_PRECIS
#define OUT_DEFAULT_PRECIS 0
#endif
#ifndef CLIP_DEFAULT_PRECIS
#define CLIP_DEFAULT_PRECIS 0
#endif
#ifndef DEFAULT_QUALITY
#define DEFAULT_QUALITY 0
#endif
#ifndef DEFAULT_PITCH
#define DEFAULT_PITCH 0
#endif
#ifndef FF_DONTCARE
#define FF_DONTCARE 0
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef MAX_PATH
#define MAX_PATH _MAX_PATH
#endif
#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#endif

#ifndef WIN32_FIND_DATA_DEFINED
#define WIN32_FIND_DATA_DEFINED
typedef struct _WIN32_FIND_DATAA {
	DWORD dwFileAttributes;
	char cFileName[MAX_PATH];
	char cAlternateFileName[14];
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;
#endif

#ifndef RECT_DEFINED
#define RECT_DEFINED
typedef struct tagRECT {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, *PRECT, *LPRECT;
#endif

static inline BOOL SetRect(RECT* rect, int left, int top, int right, int bottom) {
	if (!rect) {
		return FALSE;
	}
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
	return TRUE;
}

#ifndef PLATFORM_WSPRINTF_STUB_DEFINED
#define PLATFORM_WSPRINTF_STUB_DEFINED
#include <stdarg.h>
static inline int wsprintf(char* buf, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = vsprintf(buf, fmt, args);
	va_end(args);
	return ret;
}
#endif

#ifndef FindFirstFileA
static inline HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData) {
	(void)lpFileName;
	(void)lpFindFileData;
	return INVALID_HANDLE_VALUE;
}
#define FindFirstFileA FindFirstFileA
#define FindFirstFile FindFirstFileA
#endif
#ifndef FindNextFileA
static inline BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData) {
	(void)hFindFile;
	(void)lpFindFileData;
	return FALSE;
}
#define FindNextFileA FindNextFileA
#define FindNextFile FindNextFileA
#endif
#ifndef FindClose
static inline BOOL FindClose(HANDLE hFindFile) {
	(void)hFindFile;
	return TRUE;
}
#endif

#ifndef GetLogicalDrives
static inline DWORD GetLogicalDrives() {
	return 0;
}
#endif
#ifndef GetCurrentDirectory
static inline DWORD GetCurrentDirectory(DWORD nBufferLength, LPSTR lpBuffer) {
	if (lpBuffer && nBufferLength > 0) {
		lpBuffer[0] = '\0';
	}
	return 0;
}
#endif

#ifndef S_OK
#define S_OK 0
#endif
#ifndef S_FALSE
#define S_FALSE 1
#endif
#ifndef MB_OK
#define MB_OK 0x00000000L
#endif
#ifndef MB_ICONERROR
#define MB_ICONERROR 0x00000010L
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

#ifndef WM_USER
#define WM_USER 0x0400
#endif
#ifndef WM_TIMER
#define WM_TIMER 0x0113
#endif
#ifndef WM_CHAR
#define WM_CHAR 0x0102
#endif
#ifndef WM_KEYDOWN
#define WM_KEYDOWN 0x0100
#endif
#ifndef WM_KEYUP
#define WM_KEYUP 0x0101
#endif
#ifndef WM_TEXTINPUT
#define WM_TEXTINPUT 0x0111
#endif
#ifndef WM_TEXTEDITING
#define WM_TEXTEDITING 0x0110
#endif

#ifndef VK_BACK
#define VK_BACK 0x08
#endif
#ifndef VK_TAB
#define VK_TAB 0x09
#endif
#ifndef VK_RETURN
#define VK_RETURN 0x0D
#endif
#ifndef VK_SHIFT
#define VK_SHIFT 0x10
#endif
#ifndef VK_CONTROL
#define VK_CONTROL 0x11
#endif
#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif
#ifndef VK_SPACE
#define VK_SPACE 0x20
#endif
#ifndef VK_LEFT
#define VK_LEFT 0x25
#endif
#ifndef VK_UP
#define VK_UP 0x26
#endif
#ifndef VK_RIGHT
#define VK_RIGHT 0x27
#endif
#ifndef VK_DOWN
#define VK_DOWN 0x28
#endif
#ifndef VK_HOME
#define VK_HOME 0x24
#endif
#ifndef VK_END
#define VK_END 0x23
#endif
#ifndef VK_INSERT
#define VK_INSERT 0x2D
#endif
#ifndef VK_DELETE
#define VK_DELETE 0x2E
#endif

#ifndef POINT_DEFINED
#define POINT_DEFINED
typedef struct tagPOINT {
	LONG x;
	LONG y;
} POINT, *PPOINT, *LPPOINT;
#endif

#ifndef SIZE_DEFINED
#define SIZE_DEFINED
typedef struct tagSIZE {
	LONG cx;
	LONG cy;
} SIZE, *PSIZE, *LPSIZE;
#endif

#ifndef PLATFORM_MESSAGEBOX_STUB_DEFINED
#define PLATFORM_MESSAGEBOX_STUB_DEFINED
static inline int MessageBox(void* hWnd, const char* lpText, const char* lpCaption, unsigned int uType) {
	(void)hWnd;
	(void)uType;
	fprintf(stderr, "[%s] %s\n", lpCaption ? lpCaption : "Message", lpText ? lpText : "");
	return 1;
}
#endif

	/* Define id_t for cross-platform compatibility (unsigned int on all platforms) */
	typedef unsigned int   id_t;

#if defined(PLATFORM_USE_SDL) && defined(PLATFORM_WIN32_HOST)
typedef SDL_Thread* platform_thread_t;
typedef SDL_mutex* platform_mutex_t;
typedef struct platform_event_s* platform_event_t;
typedef void* platform_lib_t;

#ifndef PLATFORM_INVALID_THREAD
#define PLATFORM_INVALID_THREAD NULL
#endif
#ifndef PLATFORM_INVALID_MUTEX
#define PLATFORM_INVALID_MUTEX NULL
#endif
#ifndef PLATFORM_INVALID_EVENT
#define PLATFORM_INVALID_EVENT NULL
#endif
#ifndef PLATFORM_INVALID_LIB
#define PLATFORM_INVALID_LIB NULL
#endif
#ifndef PLATFORM_INFINITE
#define PLATFORM_INFINITE ((DWORD)-1)
#endif

#ifndef GetTickCount
#define GetTickCount() platform_get_ticks()
#endif

// NOTE: g_CurrentTime is fed from timeGetTime() throughout Client/ (see
// CWaitPacketUpdate.cpp, GCUpdateInfoHandler.cpp) and is expected to be real
// system uptime (original Win32 timeGetTime semantics), NOT process/SDL
// uptime - CGameUpdate.cpp's anti-cheat check (g_CurrentTime < 60000) assumes
// system uptime and misfires within the first real minute of every process
// if this aliases to platform_get_ticks()/SDL_GetTicks() like GetTickCount()
// does. Capture the real winmm import under a distinct name *before* the
// macro below rewrites the bare "timeGetTime" token, then route the macro
// through that real call instead of platform_get_ticks().
#if !defined(PLATFORM_REAL_TIMEGETTIME_DECLARED)
#define PLATFORM_REAL_TIMEGETTIME_DECLARED
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
extern "C" __declspec(dllimport) DWORD __stdcall timeGetTime(void);
static inline DWORD platform_get_system_time(void) { return timeGetTime(); }
#endif
#ifndef timeGetTime
#define timeGetTime() platform_get_system_time()
#endif

typedef DWORD (*platform_thread_func_t)(void* param);

DWORD platform_get_ticks(void);
uint64_t platform_get_performance_counter(void);
uint64_t platform_get_performance_frequency(void);
void platform_sleep(DWORD ms);
platform_thread_t platform_thread_create(platform_thread_func_t func, void* param);
int platform_thread_wait(platform_thread_t thread);
void platform_thread_close(platform_thread_t thread);
platform_mutex_t platform_mutex_create(int initial_locked);
int platform_mutex_lock(platform_mutex_t mutex);
int platform_mutex_unlock(platform_mutex_t mutex);
void platform_mutex_close(platform_mutex_t mutex);
platform_event_t platform_event_create(int manual_reset, int initial_state);
int platform_event_wait(platform_event_t event, DWORD timeout);
int platform_event_signal(platform_event_t event);
int platform_event_reset(platform_event_t event);
void platform_event_close(platform_event_t event);
platform_lib_t platform_lib_load(const char* filename);
void* platform_lib_get_symbol(platform_lib_t lib, const char* symbol);
void platform_lib_free(platform_lib_t lib);
char platform_get_path_separator(void);
int platform_file_exists(const char* filename);
int platform_get_executable_dir(char* buffer, size_t size);
int platform_create_directory(const char* path);
int platform_is_ctrl_pressed(void);
BYTE platform_get_scan_code(DWORD lParam);

#ifndef RECT_DEFINED
#define RECT_DEFINED
typedef struct tagRECT {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, *PRECT, *LPRECT;
#endif
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* WAVEFORMATEX structure for audio format */
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct _WAVEFORMATEX {
	WORD wFormatTag;
	WORD nChannels;
	DWORD nSamplesPerSec;
	DWORD nAvgBytesPerSec;
	WORD nBlockAlign;
	WORD wBitsPerSample;
	WORD cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX, *PWAVEFORMATEX;
#endif

/* WAVE format constants */
#define WAVE_FORMAT_PCM		1
#define WAVE_FORMAT_ADPCM	2

/* DirectSound buffer capabilities */
#ifndef PLATFORM_WINDOWS
#define DSBCAPS_PRIMARYBUFFER		0x00000001
#define DSBCAPS_STATIC			0x00000002
#define DSBCAPS_LOCHARDWARE		0x00000004
#define DSBCAPS_LOCSOFTWARE		0x00000008
#define DSBCAPS_CTRL3D			0x00000010
#define DSBCAPS_CTRLFREQUENCY		0x00000020
#define DSBCAPS_CTRLPAN			0x00000040
#define DSBCAPS_CTRLVOLUME		0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY	0x00000100
#define DSBCAPS_CTRLFX			0x00000200
#define DSBCAPS_STICKYFOCUS		0x00004000
#define DSBCAPS_GLOBALFOCUS		0x00008000
#define DSBCAPS_GETCURRENTPOSITION2	0x00010000
#define DSBCAPS_MUTE3DATMAX		0x00020000
#define DSBCAPS_MIXIN			0x00040000
#define DSBCAPS_TRUEPLAYPOSITION	0x00080000
#endif

/* DirectSound types for non-Windows platforms */
#ifndef PLATFORM_WINDOWS
struct IDirectSound;
struct IDirectSoundBuffer;
struct IDirectSoundNotify;

#ifndef LPDIRECTSOUNDBUFFER
typedef struct IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;
#endif
typedef struct IDirectSound* LPDIRECTSOUND;
typedef struct IDirectSoundNotify* LPDIRECTSOUNDNOTIFY;
#endif

/* CRITICAL_SECTION for thread synchronization */
#if defined(PLATFORM_USE_SDL) && defined(PLATFORM_WIN32_HOST)
#ifndef _CRITICAL_SECTION_DEFINED
#define _CRITICAL_SECTION_DEFINED

typedef struct _CRITICAL_SECTION {
	SDL_mutex* mutex;
	int initialized;
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

static inline void InitializeCriticalSection(CRITICAL_SECTION* cs) {
	if (cs != NULL) {
		cs->mutex = SDL_CreateMutex();
		cs->initialized = (cs->mutex != NULL) ? 1 : 0;
	}
}

static inline void EnterCriticalSection(CRITICAL_SECTION* cs) {
	if (cs != NULL && cs->initialized) {
		SDL_LockMutex(cs->mutex);
	}
}

static inline void LeaveCriticalSection(CRITICAL_SECTION* cs) {
	if (cs != NULL && cs->initialized) {
		SDL_UnlockMutex(cs->mutex);
	}
}

static inline void DeleteCriticalSection(CRITICAL_SECTION* cs) {
	if (cs != NULL && cs->initialized) {
		SDL_DestroyMutex(cs->mutex);
		cs->mutex = NULL;
		cs->initialized = 0;
	}
}
#endif
#elif !defined(PLATFORM_WINDOWS)
#ifndef _CRITICAL_SECTION_DEFINED
#define _CRITICAL_SECTION_DEFINED
#include <pthread.h>

typedef struct _CRITICAL_SECTION {
	pthread_mutex_t mutex;
	int initialized;
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

/* Critical section functions - pthread-based implementations for macOS */
/* Note: Use recursive mutex to match Windows CRITICAL_SECTION behavior */
static inline void InitializeCriticalSection(CRITICAL_SECTION* cs) {
	if (cs != NULL) {
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);  // 递归锁
		pthread_mutex_init(&cs->mutex, &attr);
		pthread_mutexattr_destroy(&attr);
		cs->initialized = 1;
	}
}

static inline void EnterCriticalSection(CRITICAL_SECTION* cs) {
	if (cs != NULL && cs->initialized) {
		pthread_mutex_lock(&cs->mutex);
	}
}

static inline void LeaveCriticalSection(CRITICAL_SECTION* cs) {
	if (cs != NULL && cs->initialized) {
		pthread_mutex_unlock(&cs->mutex);
	}
}

static inline void DeleteCriticalSection(CRITICAL_SECTION* cs) {
	if (cs != NULL && cs->initialized) {
		pthread_mutex_destroy(&cs->mutex);
		cs->initialized = 0;
	}
}

/* GDI object management functions - stub implementations */
static inline int DeleteObject(void* hObject) {
	(void)hObject;
	/* Stub - Windows GDI object deletion */
	return 1; /* Return TRUE */
}

/* Font weight constants */
#define FW_NORMAL 400
#define FW_BOLD 700
#define FW_THIN 100
#define FW_MEDIUM 500
#define FW_HEAVY 900
#define FW_EXTRABOLD 800
#define FW_LIGHT 300

/* LOGFONT structure for font creation */
#ifndef LOGFONT_DEFINED
#define LOGFONT_DEFINED
typedef struct tagLOGFONT {
	long lfHeight;
	long lfWidth;
	long lfEscapement;
	long lfOrientation;
	long lfWeight;
	unsigned char lfItalic;
	unsigned char lfUnderline;
	unsigned char lfStrikeOut;
	unsigned char lfCharSet;
	unsigned char lfOutPrecision;
	unsigned char lfClipPrecision;
	unsigned char lfQuality;
	unsigned char lfPitchAndFamily;
	char lfFaceName[32];
} LOGFONT, *PLOGFONT, *LPLOGFONT;
#endif

/* Character set constants */
#define ANSI_CHARSET 0
#define DEFAULT_CHARSET 1
#define SYMBOL_CHARSET 2
#define SHIFTJIS_CHARSET 128
#define HANGUL_CHARSET 129
#define GB2312_CHARSET 134
#define OEM_CHARSET 255

/* Output precision constants */
#define OUT_DEFAULT_PRECIS 0
#define OUT_STRING_PRECIS 1
#define OUT_CHARACTER_PRECIS 2
#define OUT_STROKE_PRECIS 3
#define OUT_TT_PRECIS 4
#define OUT_DEVICE_PRECIS 5
#define OUT_RASTER_PRECIS 6
#define OUT_TT_ONLY_PRECIS 7
#define OUT_OUTLINE_PRECIS 8
#define OUT_SCREEN_OUTLINE_PRECIS 9
#define OUT_PS_ONLY_PRECIS 10

/* Clip precision constants */
#define CLIP_DEFAULT_PRECIS 0
#define CLIP_CHARACTER_PRECIS 1
#define CLIP_STROKE_PRECIS 2
#define CLIP_MASK 0xf
#define CLIP_LH_ANGLES (1<<4)
#define CLIP_TT_ALWAYS (2<<4)
#define CLIP_EMBEDDED (8<<4)

/* Font quality constants */
#define DEFAULT_QUALITY 0
#define DRAFT_QUALITY 1
#define PROOF_QUALITY 2
#define NONANTIALIASED_QUALITY 3
#define ANTIALIASED_QUALITY 4
#define CLEARTYPE_QUALITY 5

/* Font pitch and family constants */
#define DEFAULT_PITCH 0
#define FIXED_PITCH 1
#define VARIABLE_PITCH 2
#define FF_DONTCARE 0
#define FF_ROMAN 1
#define FF_SWISS 2
#define FF_MODERN 3
#define FF_SCRIPT 4
#define FF_DECORATIVE 5

/* Background mode constants */
#define TRANSPARENT 1
#define OPAQUE 2

/* Text alignment constants */
#define TA_NOUPDATECP 0
#define TA_LEFT 0
#define TA_TOP 0
#define TA_UPDATECP 1
#define TA_RIGHT 2
#define TA_CENTER 6
#define TA_BASELINE 24

/* DirectDraw surface capabilities */
#define DDSCAPS_SYSTEMMEMORY 0x00000800L

/* GDI font creation function - stub implementation */
static inline void* CreateFontIndirect(LOGFONT* lplf) {
	(void)lplf;
	/* Stub - would create a font on Windows */
	return (void*)1; /* Return a non-null handle */
}
#endif

/* Windows path constants */
#ifndef _MAX_PATH
	#define _MAX_PATH	260
#endif

/* Define MAX_PATH without underscore for compatibility */
#ifndef MAX_PATH
	#define MAX_PATH _MAX_PATH
#endif

/* Color type definitions */
#ifndef PLATFORM_COLORREF_DEFINED
#define PLATFORM_COLORREF_DEFINED
typedef DWORD			COLORREF;
#endif
#ifndef RGB
#define RGB(r,g,b)		((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif

/* id_t conflicts with POSIX on macOS/Linux, only define on Windows */
#ifdef PLATFORM_WINDOWS
typedef DWORD			id_t;
#endif

#ifndef PLATFORM_CHAR_T_DEFINED
#define PLATFORM_CHAR_T_DEFINED
typedef WORD			char_t;
#endif

/* ============================================================================
 * Common Windows Type Definitions (for cross-platform compatibility)
 * ============================================================================ */

#ifndef PLATFORM_WINDOWS
	/* Windows-compatible type definitions for non-Windows platforms */
	typedef int				BOOL;
	/* Define id_t for cross-platform compatibility (unsigned int on all platforms) */
	typedef unsigned int   id_t;
	#ifndef TRUE
		#define TRUE	1
	#endif
	#ifndef FALSE
		#define FALSE	0
	#endif

	typedef int32_t			HRESULT;
	typedef intptr_t		LRESULT;
	typedef uintptr_t		UINT_PTR;
	#ifndef S_OK
		#define S_OK		0
	#endif
	#ifndef S_FALSE
		#define S_FALSE		1
	#endif

	/* HRESULT macros */
	#ifndef SUCCEEDED
		#define SUCCEEDED(hr)	(((HRESULT)(hr)) >= 0)
	#endif
	#ifndef FAILED
		#define FAILED(hr)		(((HRESULT)(hr)) < 0)
	#endif

	/* VOID type */
	#ifndef VOID
		typedef void		VOID;
	#endif

	/* Additional Windows types */
	typedef int32_t			LONG;
	typedef void*			LPVOID;
	typedef void*			HWND;
	typedef void*			HDC;
	typedef void*			HFONT;
	typedef void*			HINSTANCE;
	typedef void*			HANDLE;
	typedef DWORD*			LPDWORD;
	typedef const char*		LPCSTR;
	typedef char*			LPSTR;
	typedef const char*		LPCTSTR;
	typedef char*			LPTSTR;
	typedef const wchar_t*	LPCWSTR;
	typedef wchar_t*		LPWSTR;
	typedef unsigned char*	LPBYTE;
	typedef intptr_t		LPARAM;
	typedef intptr_t		WPARAM;
	typedef uint32_t			UINT;

	/* MessageBox constants */
	#define MB_OK			0x00000000L
	#define MB_ICONERROR		0x00000010L

	/* Windows parameter annotation macros (for callback function signatures) */
	#ifndef IN
		#define IN
	#endif
	#ifndef OUT
		#define OUT
	#endif
	#ifndef OPTIONAL
		#define OPTIONAL
	#endif

	/* Character type macros */
	#ifndef _T
		#define _T(x)		x
	#endif
	#ifndef TEXT
		#define TEXT(x)	x
	#endif
	#ifndef _L
		#define _L(x)		x
	#endif

	/* TCHAR and related types */
	#ifndef UNICODE
		typedef char			TCHAR;
		#define _tcscat		strcat
		#define _tcscpy		strcpy
		#define _tcslen		strlen
		#define _tcschr		strchr
		#define _tcsrchr		strrchr
		#define _stprintf	sprintf
		#define _tprintf		printf
		#define _tmain		main
	#else
		typedef wchar_t		TCHAR;
		#define _tcscat		wcscat
		#define _tcscpy		wcscpy
		#define _tcslen		wcslen
		#define _tcschr		wcschr
		#define _tcsrchr		wcsrchr
		#define _stprintf	swprintf
		#define _tprintf		wprintf
		#define _tmain		wmain
	#endif

	typedef TCHAR*			LPTSTR;
	typedef const TCHAR*	LPCTSTR;

	/* _TCHAR alias for compatibility with older code */
	#ifndef _TCHAR
		#define _TCHAR	TCHAR
	#endif

	/* SystemParametersInfo constants */
	#define SPI_GETMOUSE			0x0003
	#define SPI_SETMOUSE			0x0004

	/* String comparison (case-insensitive) - Windows stricmp equivalent */
	#define stricmp strcasecmp

	/* Microsoft-specific string functions - use standard equivalents */
	#define _stscanf sscanf

	/* Process and thread stubs for tlhelp32.h functions */
	#define INVALID_HANDLE_VALUE ((HANDLE)-1)
	#define TH32CS_SNAPPROCESS 0x00000002
	#define CloseHandle(handle) /* No-op on non-Windows */

	typedef struct {
		DWORD dwSize;              // Length of structure in bytes
		DWORD cntUsage;
		DWORD th32ProcessID;
		ULONG_PTR th32DefaultHeapID;
		DWORD th32ModuleID;
		DWORD cntThreads;
		DWORD th32ParentProcessID;
		LONG pcPriClassBase;
		DWORD dwFlags;
		char szExeFile[MAX_PATH];
	} PROCESSENTRY32;

	/* InitCommonControls stub - no-op on non-Windows */
	#define InitCommonControls()

	/* GetSystemMetrics constants */
	#define SM_CXSCREEN 0
	#define SM_CYSCREEN 1
	#define SM_CYVSCROLL 20
	#define SM_CYSIZEFRAME 33
	#define SM_CYMENU 15
	#define SM_CXSIZEFRAME 32

	/* Window style constants */
	#define WS_EX_TOPMOST 0x00000008
	#define WS_EX_APPWINDOW 0x00040000
	#define WS_VISIBLE 0x10000000
	#define WS_POPUP 0x80000000L
	#define WS_OVERLAPPED 0x00000000L
	#define WS_CLIPCHILDREN 0x02000000L
	#define WS_THICKFRAME 0x00040000L
	#define WS_MINIMIZEBOX 0x00020000L
	#define WS_SYSMENU 0x00080000L
	#define SW_HIDE 0

	/* Progress bar constants */
	#define PROGRESS_CLASS "PROGRESS_CLASS"
	#define PBS_SMOOTH 0x01
	#define PBM_SETRANGE (WM_USER+1)
	#define PBM_SETSTEP (WM_USER+4)
	#define PBM_SETPOS (WM_USER+2)
	#define PBM_STEPIT (WM_USER+5)

	/* Window class styles */
	#define CS_HREDRAW 0x0001
	#define CS_VREDRAW 0x0002
	#define CS_DBLCLKS 0x0008

	/* WNDCLASS structure (Windows window class registration) */
	typedef struct tagWNDCLASS {
		UINT style;
		void* lpfnWndProc;
		int cbClsExtra;
		int cbWndExtra;
		void* hInstance;
		void* hIcon;
		void* hCursor;
		void* hbrBackground;
		const char* lpszMenuName;
		const char* lpszClassName;
	} WNDCLASS, *PWNDCLASS, *LPWNDCLASS;

	/* Message macros */
	#define MAKELPARAM(l, h) ((LPARAM)(((DWORD_PTR)(l) & 0xFFFF) | ((DWORD_PTR)(h) << 16)))
	#define WM_USER 0x0400
	#define WM_TIMER 0x0113
	#define WM_CHAR 0x0102
	#define WM_KEYUP 0x0101
	#define WM_IME_COMPOSITION 0x010F
	#define WM_IME_STARTCOMPOSITION 0x010D
	#define WM_IME_ENDCOMPOSITION 0x010E

	/* SDL text input messages */
	#define WM_TEXTINPUT 0x0111  /* SDL text input event (committed text) */
	#define WM_TEXTEDITING 0x0110  /* SDL text editing event (IME composition) */

	/* Window messages */
	#define WM_DESTROY 0x0002
	#define WM_CLOSE 0x0010
	#define WM_QUIT 0x0012
	#define WM_SYSCOMMAND 0x0112
	#define WM_MOVE 0x0003
	#define WM_KEYDOWN 0x0100
	#define WM_GETMINMAXINFO 0x0024
	#define WM_ACTIVATEAPP 0x001C

	/* Virtual key codes */
	#define VK_SPACE 0x20
	#define VK_RETURN 0x0D
	#define VK_ESCAPE 0x1B
	#define VK_SCROLL 0x91

	/* Code page constants */
	#define CP_ACP 0
	#define CP_OEMCP 1
	#define CP_UTF8 65001
	#define WC_COMPOSITECHECK 0x00000200

	/* System command values */
	#define SC_HOTKEY 0xF150
	#define SC_KEYMENU 0xF100
	#define SC_TASKLIST 0xF140
	#define SC_PREVWINDOW 0xF050
	#define SC_NEXTWINDOW 0xF040
	#define SC_CLOSE 0xF060
	#define SC_MOVE 0xF010
	#define SC_SIZE 0xF000
	#define SC_SCREENSAVE 0xF140  // Note: Same value as SC_TASKLIST
	#define SC_MONITORPOWER 0xF170
	#define SC_MAXIMIZE 0xF030

	/* MCI messages - still needed by some code */
	#define MM_MCINOTIFY 0x3D9
	#define MCI_NOTIFY_SUCCESSFUL 0x0001
	#define MCI_NOTIFY_SUPERCEDED 0x0002
	#define MCI_NOTIFY_ABORTED 0x0004
	#define MCI_NOTIFY_FAILURE 0x0008

#ifndef PLATFORM_WIDECHAR_TO_MULTIBYTE_DEFINED
#define PLATFORM_WIDECHAR_TO_MULTIBYTE_DEFINED
	/* WideCharToMultiByte stub - basic conversion for non-Windows */
	static inline int WideCharToMultiByte(UINT CodePage, DWORD dwFlags,
		LPCWSTR lpWideCharStr, int cchWideChar,
		LPSTR lpMultiByteStr, int cbMultiByte,
		LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar) {
		(void)CodePage; (void)dwFlags; (void)lpDefaultChar; (void)lpUsedDefaultChar;
		/* Basic UTF-16 to UTF-8 conversion for non-Windows platforms */
		if (cchWideChar == -1) {
			/* Find null terminator */
			int len = 0;
			while (lpWideCharStr[len]) len++;
			cchWideChar = len;
		}
		/* Simple conversion - just copy lower byte (works for ASCII) */
		for (int i = 0; i < cchWideChar && i < cbMultiByte - 1; i++) {
			lpMultiByteStr[i] = (char)(lpWideCharStr[i] & 0xFF);
		}
		lpMultiByteStr[cchWideChar < cbMultiByte ? cchWideChar : cbMultiByte - 1] = '\0';
		return cchWideChar;
	}
#endif

	/* Stock object constants */
	#define BLACK_BRUSH 4
	#define WHITE_BRUSH 0
	#define DC_BRUSH 18

	/* HMENU typedef */
	typedef void* HMENU;
	typedef void* HBRUSH;
	typedef void* HICON;
	typedef void* HCURSOR;

	/* Resource management macros */
	#define MAKEINTRESOURCE(i) (LPCTSTR)((DWORD_PTR)((WORD)(i)))

	/* Standard cursor */
	#define IDC_ARROW ((LPCTSTR)"MAKEINTRESOURCE(32512)")

	/* FAR PASCAL macros for Windows callback conventions */
	#ifndef FAR
		#define FAR
	#endif

	#ifndef PASCAL
		#define PASCAL
	#endif

	/* FARPROC - pointer to a function (Windows callback type) */
	typedef int (*FARPROC)();

	/* Callback function type */
	typedef long (__cdecl *WNDPROC)(void*, unsigned int, unsigned long, long long);

	/* IWebBrowser2 stub - COM interface for web browser control */
	#ifndef IWebBrowser2_DEFINED
	#define IWebBrowser2_DEFINED
	typedef void* IWebBrowser2;
	#endif

	/* Windows timing functions */
	#ifndef GetTickCount
	#define GetTickCount()		platform_get_ticks()
	#endif
	#ifndef timeGetTime
	#define timeGetTime()		platform_get_ticks()
	#endif
#endif

/* ============================================================================
 * Platform-Specific Types
 * ============================================================================ */

#ifdef PLATFORM_WINDOWS
	/* Windows types */
	#ifndef _WINDOWS_
		#define WIN32_LEAN_AND_MEAN
#include <windows.h>
	#endif

	typedef HANDLE	platform_thread_t;
	typedef HANDLE	platform_mutex_t;
	typedef HANDLE	platform_event_t;
	typedef HMODULE	platform_lib_t;

	#define PLATFORM_INVALID_THREAD	NULL
	#define PLATFORM_INVALID_MUTEX	NULL
	#define PLATFORM_INVALID_EVENT	NULL
	#define PLATFORM_INVALID_LIB		NULL

#else
	/* SDL/POSIX types */
	typedef SDL_Thread*	platform_thread_t;
	typedef SDL_mutex*	platform_mutex_t;

	/* Forward declaration for event structure */
	typedef struct platform_event_s* platform_event_t;

	typedef void*	platform_lib_t;

	#define PLATFORM_INVALID_THREAD	NULL
	#define PLATFORM_INVALID_MUTEX	NULL
	#define PLATFORM_INVALID_EVENT	NULL
	#define PLATFORM_INVALID_LIB		NULL

#endif /* PLATFORM_WINDOWS */

/* ============================================================================
 * Time Functions
 * ============================================================================ */

/**
 * Get current time in milliseconds (like timeGetTime/GetTickCount)
 * @return Time in milliseconds
 */
DWORD platform_get_ticks(void);

/**
 * Get high-performance counter value (like QueryPerformanceCounter)
 * @return Counter value
 */
uint64_t platform_get_performance_counter(void);

/**
 * Get high-performance counter frequency (like QueryPerformanceFrequency)
 * @return Counter frequency (counts per second)
 */
uint64_t platform_get_performance_frequency(void);

/**
 * Sleep for specified milliseconds (like Sleep)
 * @param ms Milliseconds to sleep
 */
void platform_sleep(DWORD ms);

/* ============================================================================
 * Thread Functions
 * ============================================================================ */

/**
 * Thread entry point type
 */
typedef DWORD (*platform_thread_func_t)(void* param);

/**
 * Create a new thread (like CreateThread)
 * @param func Thread function
 * @param param Parameter to pass to thread function
 * @return Thread handle or PLATFORM_INVALID_THREAD on failure
 */
platform_thread_t platform_thread_create(platform_thread_func_t func, void* param);

/**
 * Wait for thread to finish (like WaitForSingleObject on thread)
 * @param thread Thread handle
 * @return Wait result (0 = success, non-zero = failure)
 */
int platform_thread_wait(platform_thread_t thread);

/**
 * Close thread handle (like CloseHandle on thread)
 * @param thread Thread handle
 */
void platform_thread_close(platform_thread_t thread);

/* ============================================================================
 * Mutex Functions
 * ============================================================================ */

/**
 * Create a mutex (like CreateMutex)
 * @param initial_locked Whether mutex starts locked
 * @return Mutex handle or PLATFORM_INVALID_MUTEX on failure
 */
platform_mutex_t platform_mutex_create(int initial_locked);

/**
 * Lock a mutex (like WaitForSingleObject on mutex)
 * @param mutex Mutex handle
 * @return Lock result (0 = success, non-zero = failure)
 */
int platform_mutex_lock(platform_mutex_t mutex);

/**
 * Unlock a mutex (like ReleaseMutex)
 * @param mutex Mutex handle
 * @return Unlock result (0 = success, non-zero = failure)
 */
int platform_mutex_unlock(platform_mutex_t mutex);

/**
 * Close mutex handle (like CloseHandle on mutex)
 * @param mutex Mutex handle
 */
void platform_mutex_close(platform_mutex_t mutex);

/* ============================================================================
 * Event Functions
 * ============================================================================ */

/**
 * Create an event object (like CreateEvent)
 * @param manual_reset Whether event requires manual reset
 * @param initial_state Initial state (TRUE = signaled, FALSE = non-signaled)
 * @return Event handle or PLATFORM_INVALID_EVENT on failure
 */
platform_event_t platform_event_create(int manual_reset, int initial_state);

/**
 * Wait for event to be signaled (like WaitForSingleObject on event)
 * @param event Event handle
 * @param timeout Timeout in milliseconds (or PLATFORM_INFINITE for infinite wait)
 * @return Wait result (0 = success, non-zero = timeout/failure)
 */
int platform_event_wait(platform_event_t event, DWORD timeout);

/**
 * Signal an event (like SetEvent)
 * @param event Event handle
 * @return Signal result (0 = success, non-zero = failure)
 */
int platform_event_signal(platform_event_t event);

/**
 * Reset an event to non-signaled (like ResetEvent)
 * @param event Event handle
 * @return Reset result (0 = success, non-zero = failure)
 */
int platform_event_reset(platform_event_t event);

/**
 * Close event handle (like CloseHandle on event)
 * @param event Event handle
 */
void platform_event_close(platform_event_t event);

#define PLATFORM_INFINITE	((DWORD)-1)

/* ============================================================================
 * Dynamic Library Functions
 * ============================================================================ */

/**
 * Load a dynamic library (like LoadLibrary)
 * @param filename Library file name/path
 * @return Library handle or PLATFORM_INVALID_LIB on failure
 */
platform_lib_t platform_lib_load(const char* filename);

/**
 * Get function address from library (like GetProcAddress)
 * @param lib Library handle
 * @param symbol Function symbol name
 * @return Function pointer or NULL on failure
 */
void* platform_lib_get_symbol(platform_lib_t lib, const char* symbol);

/**
 * Unload a dynamic library (like FreeLibrary)
 * @param lib Library handle
 */
void platform_lib_free(platform_lib_t lib);

/* ============================================================================
 * File/Path Functions
 * ============================================================================ */

/**
 * Get path separator for current platform
 * @return Path separator character ('\\' on Windows, '/' on POSIX)
 */
char platform_get_path_separator(void);

/**
 * Check if file exists
 * @param filename File path to check
 * @return 1 if exists, 0 otherwise
 */
int platform_file_exists(const char* filename);

/**
 * Get current executable directory
 * @param buffer Buffer to store path
 * @param size Buffer size
 * @return 0 on success, non-zero on failure
 */
int platform_get_executable_dir(char* buffer, size_t size);

/**
 * Create directory if it doesn't exist
 * @param path Directory path
 * @return 0 on success, non-zero on failure
 */
int platform_create_directory(const char* path);

/* ============================================================================
 * Keyboard Functions (from PlatformUtil.h)
 * ============================================================================ */

/**
 * Check if Control key is currently pressed
 * @return 1 if pressed, 0 otherwise
 */
int platform_is_ctrl_pressed(void);

/**
 * Get keyboard scan code from lParam (Windows message parameter)
 * @param lParam LPARAM from keyboard message
 * @return Scan code
 */
BYTE platform_get_scan_code(DWORD lParam);

/* ============================================================================
 * Registry/Configuration Functions (Windows-only abstraction)
 * ============================================================================ */

/**
 * Get string value from configuration (replaces RegQueryValueEx)
 * @param key Configuration key name (e.g., "SOFTWARE\\Netmarble\\NetmarbleDarkEden")
 * @param value Value name
 * @param buffer Buffer to store value
 * @param size Buffer size (in/out)
 * @return 0 on success, non-zero on failure
 */
int platform_config_get_string(const char* key, const char* value,
                               char* buffer, DWORD* size);

/**
 * Set string value in configuration (replaces RegSetValueEx)
 * @param key Configuration key name
 * @param value Value name
 * @param data String data to set
 * @return 0 on success, non-zero on failure
 */
int platform_config_set_string(const char* key, const char* value,
                               const char* data);

/* ============================================================================
 * Error Reporting
 * ============================================================================ */

/**
 * Show error message box (like MessageBox)
 * @param title Message box title
 * @param message Error message
 */
void platform_show_error(const char* title, const char* message);

/* ============================================================================
 * Initialization
 * ============================================================================ */

/**
 * Initialize platform abstraction layer
 * Call this at program startup
 * @return 0 on success, non-zero on failure
 */
int platform_init(void);

/**
 * Cleanup platform abstraction layer
 * Call this at program shutdown
 */
void platform_shutdown(void);

/* ============================================================================
 * Windows Compatibility Macros
 * ============================================================================ */

// Define DLLIFC as empty on non-Windows platforms (for Immersion library compatibility)
#ifndef PLATFORM_WINDOWS
#ifndef DLLIFC
#define DLLIFC
#endif
#endif

// Windows constants that may be needed
#ifndef MAXLONG
#define MAXLONG 2147483647L  // 0x7FFFFFFF
#endif

#ifndef MAXDWORD
#define MAXDWORD 0xFFFFFFFF
#endif

/* ============================================================================
 * Rectangle Structure (Windows RECT equivalent)
 * ============================================================================ */

#ifndef RECT_DEFINED
#define RECT_DEFINED

/**
 * Point structure (equivalent to Windows POINT)
 * Used for defining 2D coordinates
 */
#ifndef POINT_DEFINED
#define POINT_DEFINED
typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT, *PPOINT, *LPPOINT;
#endif

/**
 * Rectangle structure (equivalent to Windows RECT)
 * Used for defining rectangular areas
 */
typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *LPRECT;

/**
 * MINMAXINFO structure (used in WM_GETMINMAXINFO)
 * Contains information about a window's maximized size and position
 */
typedef struct tagMINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;

/* SYSTEMTIME structure (date and time) */
#ifndef PLATFORM_SYSTEMTIME_DEFINED
#define PLATFORM_SYSTEMTIME_DEFINED
typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif

#endif /* RECT_DEFINED */

/* DEVMODE structure (display mode settings)
 * Deliberately NOT nested inside the RECT_DEFINED guard above (it used to
 * be, and that was the bug): RECT/POINT/MINMAXINFO are already provided
 * unconditionally earlier in this file, so on non-Windows-host SDL builds
 * RECT_DEFINED is already set by the time the block above is reached and
 * the whole thing - including DEVMODE, the only piece here without a
 * definition anywhere else - gets silently skipped, leaving LPDEVMODE
 * undefined for ChangeDisplaySettingsA/EnumDisplaySettingsA below. Giving
 * it its own top-level guard, outside RECT_DEFINED's scope, fixes that. */
#ifndef DEVMODE_DEFINED
#define DEVMODE_DEFINED
#define ENUM_CURRENT_SETTINGS ((DWORD)-1)
#define DM_BITSPERPEL 0x00040000
#define DM_PELSWIDTH 0x00080000
#define DM_PELSHEIGHT 0x00100000
#define DM_DISPLAYFREQUENCY 0x00400000

typedef struct _devicemode {
    char   dmDeviceName[32];
    WORD   dmSpecVersion;
    WORD   dmDriverVersion;
    WORD   dmSize;
    WORD   dmDriverExtra;
    DWORD  dmFields;
    union {
        struct {
            short dmOrientation;
            short dmPaperSize;
            short dmPaperLength;
            short dmPaperWidth;
            short dmScale;
            short dmCopies;
            short dmDefaultSource;
            short dmPrintQuality;
        };
        POINT dmPosition;
    };
    short  dmColor;
    short  dmDuplex;
    short  dmYResolution;
    short  dmTTOption;
    short  dmCollate;
    char   dmFormName[32];
    WORD   dmLogPixels;
    DWORD  dmBitsPerPel;
    DWORD  dmPelsWidth;
    DWORD  dmPelsHeight;
    DWORD  dmDisplayFlags;
    DWORD  dmDisplayFrequency;
} DEVMODE, *PDEVMODE, *LPDEVMODE;
#endif /* DEVMODE_DEFINED */

/* ============================================================================
 * Windows File and Process API Stubs
 * ============================================================================ */

/* File time structure (must be defined before WIN32_FIND_DATA) */
#ifndef FILETIME_DEFINED
#define FILETIME_DEFINED
typedef struct _FILETIME {
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
#endif

/* Security attributes structure */
#ifndef SECURITY_ATTRIBUTES_DEFINED
#define SECURITY_ATTRIBUTES_DEFINED
typedef struct _SECURITY_ATTRIBUTES {
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
#endif

/* Find file data structure */
#ifndef WIN32_FIND_DATA_DEFINED
#define WIN32_FIND_DATA_DEFINED
typedef struct _WIN32_FIND_DATAA {
	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD nFileSizeHigh;
	DWORD nFileSizeLow;
	DWORD dwReserved0;
	DWORD dwReserved1;
	char cFileName[MAX_PATH];
	char cAlternateFileName[14];
} WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;
#endif

/* Process access rights */
#ifndef PROCESS_ALL_ACCESS
	#define PROCESS_ALL_ACCESS (0xFFFF)
#endif

/* Display settings constants */
#ifndef CDS_RESET
	#define CDS_RESET 0x40000000
#endif
#ifndef CDS_UPDATEREGISTRY
	#define CDS_UPDATEREGISTRY 0x00000001
#endif
#ifndef DISP_CHANGE_SUCCESSFUL
	#define DISP_CHANGE_SUCCESSFUL 0
#endif
#ifndef DISP_CHANGE_RESTART
	#define DISP_CHANGE_RESTART 1
#endif
#ifndef DISP_CHANGE_FAILED
	#define DISP_CHANGE_FAILED -1
#endif

/* Find file handles */
typedef void* HANDLE;
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef void* HKEY;

/* Registry constants */
#ifndef HKEY_LOCAL_MACHINE
	#define HKEY_LOCAL_MACHINE ((HKEY)0x80000002)
#endif

#ifndef KEY_ALL_ACCESS
	#define KEY_ALL_ACCESS (0xF003F)
#endif

#ifndef REG_SZ
	#define REG_SZ 1
#endif

#ifndef ERROR_SUCCESS
	#define ERROR_SUCCESS 0
#endif

/* Stub implementations for file and process operations */
#ifndef DeleteFile
static inline BOOL DeleteFileA(LPCSTR lpFileName) {
	(void)lpFileName;
	return FALSE;
}
#define DeleteFile DeleteFileA
#endif

#ifndef CopyFile
static inline BOOL CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists) {
	(void)lpExistingFileName; (void)lpNewFileName; (void)bFailIfExists;
	return FALSE;
}
#define CopyFile CopyFileA
#endif

#ifndef SetCurrentDirectory
static inline BOOL SetCurrentDirectoryA(LPCSTR lpPathName) {
	(void)lpPathName;
	return FALSE;
}
#define SetCurrentDirectory SetCurrentDirectoryA
#endif

#ifndef GetModuleFileName
static inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) {
	(void)hModule;
	if (lpFilename && nSize > 0) {
		lpFilename[0] = '\0';
		return 0;
	}
	return 0;
}
#define GetModuleFileName GetModuleFileNameA
#endif

#ifndef INVALID_HANDLE_VALUE
	#define INVALID_HANDLE_VALUE ((HANDLE)(-1))
#endif

#ifndef CreateMutexA
static inline HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) {
	(void)lpMutexAttributes; (void)bInitialOwner; (void)lpName;
	return (HANDLE)NULL;
}
#define CreateMutex CreateMutexA
#endif

#ifndef ChangeDisplaySettingsA
static inline LONG ChangeDisplaySettingsA(LPDEVMODE lpDevMode, DWORD dwflags) {
	(void)lpDevMode; (void)dwflags;
	return DISP_CHANGE_FAILED;
}
#define ChangeDisplaySettings ChangeDisplaySettingsA
#endif

#ifndef GetLastError
static inline DWORD GetLastError() {
	return 0;
}
#endif

#ifndef _chdir
	#define _chdir chdir
#endif

/* Unix-style function name mappings for Windows compatibility */
#ifndef _access
	#define _access access
#endif

#ifndef _getcwd
	#define _getcwd getcwd
#endif

#ifndef _rmdir
	#define _rmdir rmdir
#endif

/* _P_OVERLAY for spawn function */
#ifndef _P_OVERLAY
	#define _P_OVERLAY 2
#endif

/* Registry types */
#ifndef REGSAM
typedef DWORD REGSAM;
#endif

#ifndef PHKEY
typedef HKEY* PHKEY;
#endif

/* Registry functions */
#ifndef RegOpenKeyExA
static inline LONG RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) {
	(void)hKey; (void)lpSubKey; (void)ulOptions; (void)samDesired;
	if (phkResult) *phkResult = NULL;
	return ERROR_SUCCESS;
}
#define RegOpenKeyEx RegOpenKeyExA
#endif

#ifndef RegQueryValueExA
static inline LONG RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
	(void)hKey; (void)lpValueName; (void)lpReserved; (void)lpType;
	if (lpData && lpcbData && *lpcbData > 0) {
		lpData[0] = '\0';
		*lpcbData = 1;
	}
	return ERROR_SUCCESS;
}
#define RegQueryValueEx RegQueryValueExA
#endif

#ifndef RegSetValueExA
static inline LONG RegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) {
	(void)hKey; (void)lpValueName; (void)Reserved; (void)dwType; (void)lpData; (void)cbData;
	return ERROR_SUCCESS;
}
#define RegSetValueEx RegSetValueExA
#endif

#ifndef RegCloseKey
static inline LONG RegCloseKey(HKEY hKey) {
	(void)hKey;
	return ERROR_SUCCESS;
}
#endif

/* Registry access mask type */
#ifndef REGSAM_DEFINED
	#define REGSAM_DEFINED
#endif

#ifndef EnumDisplaySettingsA
static inline BOOL EnumDisplaySettingsA(LPCSTR lpszDeviceName, DWORD iModeNum, LPDEVMODE lpDevMode) {
	(void)lpszDeviceName; (void)iModeNum;
	if (lpDevMode) {
		lpDevMode->dmBitsPerPel = 32;
		lpDevMode->dmPelsWidth = 1024;
		lpDevMode->dmPelsHeight = 768;
		lpDevMode->dmDisplayFrequency = 60;
	}
	return FALSE;
}
#define EnumDisplaySettings EnumDisplaySettingsA
#endif

/* Spawn functions */
#ifndef _spawnl
static inline intptr_t _spawnl(int mode, const char* cmdname, const char* arg0, ...) {
	(void)mode; (void)cmdname; (void)arg0;
	return -1;
}
#endif

/* _finddata_t structure for file finding */
#ifndef _FINDDATA_T_DEFINED
#define _FINDDATA_T_DEFINED
struct _finddata_t {
	unsigned attrib;
	time_t time_create;
	time_t time_access;
	time_t time_write;
	long size;
	char name[512];
};
#endif

/* Find functions (Unix-style) */
#ifndef _findfirst
static inline long _findfirst(const char* filename, struct _finddata_t* finddata) {
	(void)filename; (void)finddata;
	return -1;
}
#endif

#ifndef _findnext
static inline int _findnext(long handle, struct _finddata_t* finddata) {
	(void)handle; (void)finddata;
	return -1;
}
#endif

#ifndef _findclose
static inline int _findclose(long handle) {
	(void)handle;
	return 0;
}
#endif

/* MSG structure for Windows messages */
#ifndef tagMSG_DEFINED
#define tagMSG_DEFINED
typedef struct tagMSG {
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD time;
	POINT pt;
} MSG, *PMSG, *LPMSG;
#endif

/* PeekMessage flags */
#ifndef PM_NOREMOVE
	#define PM_NOREMOVE 0x0000
#endif

/* Console constants */
#ifndef MIN_CLRSCR
	#define MIN_CLRSCR 0
#endif

#ifndef MIN_SHOWWND
	#define MIN_SHOWWND 1
#endif

#ifndef MIN_HIDEWND
	#define MIN_HIDEWND 2
#endif

/* PeekMessage function */
#ifndef PeekMessageA
static inline BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
	(void)hWnd; (void)wMsgFilterMin; (void)wMsgFilterMax; (void)wRemoveMsg;
	return FALSE;
}
#define PeekMessage PeekMessageA
#endif

/* System parameters info */
#ifndef SPI_SETSCREENSAVERRUNNING
	#define SPI_SETSCREENSAVERRUNNING 17
#endif

/* OS version info */
#ifndef OSVERSIONINFO_DEFINED
#define OSVERSIONINFO_DEFINED
typedef struct _OSVERSIONINFOA {
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	char szCSDVersion[128];
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;
#define OSVERSIONINFO OSVERSIONINFOA
#endif

#ifndef VER_PLATFORM_WIN32_WINDOWS
	#define VER_PLATFORM_WIN32_WINDOWS 1
#endif

#ifndef VER_PLATFORM_WIN32_NT
	#define VER_PLATFORM_WIN32_NT 2
#endif

/* DirectInput key codes */
#ifndef DIK_NUMPADENTER
	#define DIK_NUMPADENTER 0x9C
#endif

/* Version checking */
#ifndef GetVersionExA
static inline BOOL GetVersionExA(LPOSVERSIONINFOA lpVersionInformation) {
	if (lpVersionInformation) {
		lpVersionInformation->dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
		lpVersionInformation->dwMajorVersion = 5;
		lpVersionInformation->dwMinorVersion = 1;
		lpVersionInformation->dwBuildNumber = 2600;
		lpVersionInformation->dwPlatformId = VER_PLATFORM_WIN32_NT;
		lpVersionInformation->szCSDVersion[0] = '\0';
	}
	return TRUE;
}
#define GetVersionEx GetVersionExA
#endif

/* Exception handling */
/* Exception filter type (must be defined before EXCEPTION_POINTERS) */
#ifndef LPTOP_LEVEL_EXCEPTION_FILTER
typedef LONG (*LPTOP_LEVEL_EXCEPTION_FILTER)(struct _EXCEPTION_POINTERS*);
#endif

#ifndef EXCEPTION_POINTERS_DEFINED
#define EXCEPTION_POINTERS_DEFINED
typedef struct _EXCEPTION_POINTERS {
	DWORD ExceptionCode;
	DWORD ExceptionFlags;
	void* ExceptionRecord;
	void* ExceptionAddress;
	DWORD NumberParameters;
	void* ExceptionInformation[15];
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
#endif

/* DirectDraw caps */
#ifndef DDSCAPS_VIDEOMEMORY
	#define DDSCAPS_VIDEOMEMORY 0x00000040
#endif

typedef struct _DDCAPS {
	DWORD dwSize;
	DWORD dwCaps;
	DWORD dwCaps2;
	DWORD dwCKeyCaps;
	DWORD dwFXCaps;
	DWORD dwFXAlphaCaps;
	DWORD dwPalCaps;
	DWORD dwSVCaps;
	DWORD dwAlphaCaps;
	DWORD dwVideoPortCaps;
	DWORD dwVideoPortCaps2;
	DWORD dwVidMemTotal;
	DWORD dwVidMemFree;
	DWORD dwMaxVisibleOverhead;
} DDCAPS;

/* max and min macros for compatibility with Windows code */
#ifndef PLATFORM_WINDOWS
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
/* __int64 Windows type - use long long on macOS */
typedef long long __int64;
/* _atoi64 Windows function - use atoll on macOS */
#define _atoi64(x) atoll(x)
#endif

#ifndef PLATFORM_WINDOWS
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>

/* Windows API stubs for file operations */
#define FILE_ATTRIBUTE_DIRECTORY (0x00000010)

/* Windows Virtual Key Codes for keyboard input */
#ifndef VK_UP
#define VK_UP    0x26
#endif
#ifndef VK_DOWN
#define VK_DOWN  0x28
#endif
#ifndef VK_LEFT
#define VK_LEFT  0x25
#endif
#ifndef VK_RIGHT
#define VK_RIGHT 0x27
#endif
#ifndef VK_RETURN
#define VK_RETURN 0x0D
#endif
#ifndef VK_ESCAPE
#define VK_ESCAPE 0x1B
#endif
#ifndef VK_TAB
#define VK_TAB 0x09
#endif
#ifndef VK_BACK
#define VK_BACK 0x08
#endif
#ifndef VK_SPACE
#define VK_SPACE  0x20
#endif
#ifndef VK_SHIFT
#define VK_SHIFT  0x10
#endif
#ifndef VK_CONTROL
#define VK_CONTROL 0x11
#endif
#ifndef VK_HOME
#define VK_HOME 0x24
#endif
#ifndef VK_END
#define VK_END 0x23
#endif
#ifndef VK_DELETE
#define VK_DELETE 0x2E
#endif
#ifndef VK_INSERT
#define VK_INSERT 0x2D
#endif

/* DirectInput Key Codes */
#ifndef DIK_LCONTROL
#define DIK_LCONTROL 0x1D
#endif
#ifndef DIK_RCONTROL
#define DIK_RCONTROL 0x9D
#endif
#ifndef DIK_LSHIFT
#define DIK_LSHIFT 0x2A
#endif
#ifndef DIK_RSHIFT
#define DIK_RSHIFT 0x36
#endif

/* Windows macros for word manipulation */
#ifndef LOWORD
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif
#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#endif
#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#endif

#endif

/* SetSurfaceInfo for SDL backend - copies S_SURFACEINFO */
#ifndef PLATFORM_WINDOWS
#include "2d.h"
static inline void SetSurfaceInfo(S_SURFACEINFO* dest, const S_SURFACEINFO* src) {
    if (dest && src) {
        dest->p_surface = src->p_surface;
        dest->width = src->width;
        dest->height = src->height;
        dest->pitch = src->pitch;
    }
}
#endif

/* DirectInput key codes for non-Windows platforms */
#ifndef PLATFORM_WINDOWS
/* DIK_LMENU and DIK_RMENU are the DirectInput names for Left/Right ALT */
#define DIK_LMENU           0x38
#define DIK_RMENU           0xB8
/* Alternate names for ALT keys */
#define DIK_LALT            DIK_LMENU
#define DIK_RALT            DIK_RMENU

/* Windows macros for creating LONG/LPARAM from values */
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD_PTR)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#endif
#ifndef MAKEWPARAM
#define MAKEWPARAM(l, h) ((WPARAM)(DWORD)MAKELONG(l, h))
#endif
#ifndef MAKELPARAM
#define MAKELPARAM(l, h) ((LPARAM)(DWORD)MAKELONG(l, h))
#endif
#ifndef MAKELRESULT
#define MAKELRESULT(l, h) ((LRESULT)(DWORD)MAKELONG(l, h))
#endif
#endif
#endif

#endif /* __DARKEDEN_CLIENT_PLATFORM_H__ */
