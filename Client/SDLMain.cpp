/*-----------------------------------------------------------------------------

	SDLMain.cpp

	SDL2-based main entry point for Dark Eden client on macOS/Linux.
	Based on WinMain from Client.cpp (lines 3020-4500).

	This file handles:
	- SDL initialization
	- Window creation
	- Event loop
	- Game loop integration
	- Clean shutdown

	2025.01.18 - Rewritten based on Client.cpp WinMain

-----------------------------------------------------------------------------*/

#ifndef PLATFORM_WINDOWS

#define SDL_MAIN_HANDLED
#include "Client_PCH.h"
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#include <SDL_ttf.h>
#include <string.h>
#include <stdio.h>
#if defined(PLATFORM_WIN32_HOST)
#include <direct.h>
#define getcwd _getcwd
#endif

#ifndef SDLMAIN_VERBOSE_LOG
#define SDLMAIN_VERBOSE_LOG 0
#endif

#if SDLMAIN_VERBOSE_LOG
#define SDLMAIN_LOG(...) do { printf(__VA_ARGS__); } while (0)
#define SDLMAIN_DEBUG(...) do { fprintf(stderr, __VA_ARGS__); fflush(stderr); } while (0)
#else
#define SDLMAIN_LOG(...) do {} while (0)
#define SDLMAIN_DEBUG(...) do {} while (0)
#endif

// Client.h - must be included to access g_pUpdate declaration
#include "Client.h"

// VS_UI headers
#include "../VS_UI/src/hangul/Ci.h"
#include "../VS_UI/src/header/VS_UI.h"

// Game headers
#include "UserInformation.h"
#include "SpriteLib/SpriteLibBackend.h"
#include "CGameUpdate.h"
#include "CSpriteSurface.h"
#include "ClientDef.h"
#include "MTopView.h"
#include "MPlayer.h"
#include <Platform.h>
#include "Packet/Exception.h"  // For NoSuchElementException, Throwable
#include "DXLib/DXLibBackend.h"  // For dxlib_input_update
// SEH (Structured Exception Handling) is a Windows-only OS/MSVC feature -
// eh.h, _set_se_translator, and the raw x86 Win32 CONTEXT-struct offsets
// this section relies on don't exist on macOS/Linux. Everything from here
// through the SEHTranslator/GetModuleHandleA declarations below, plus the
// _set_se_translator() call and catch(SEHException&) block further down in
// this file, is compiled only on a real Windows host; other platforms fall
// through to the generic catch(...) block instead.
#ifdef PLATFORM_WIN32_HOST
#include <eh.h>  // _set_se_translator - see SEHException below

// Structured exceptions (access violation, etc.) surface here as an
// unhelpful "Unknown exception" with no message, since catch(...) also
// swallows SEH under this project's exception-handling mode. Translating
// them into a real C++ exception lets us at least log the exception code
// and faulting address before the process exits, instead of a silent close.
//
// Minimal ABI-compatible mirror of the real Win32 EXCEPTION_RECORD/
// EXCEPTION_POINTERS (stable, documented layout) - avoids pulling in a full
// <windows.h> here (that previously broke GetLastError visibility
// elsewhere in this codebase), same pattern as basic/Platform.h's other
// hand-rolled Win32 struct mirrors (WSADATA, DDGAMMARAMP, ...).
struct DE_EXCEPTION_RECORD
{
	unsigned long ExceptionCode;
	unsigned long ExceptionFlags;
	void* ExceptionRecordPtr;
	void* ExceptionAddress;
	unsigned long NumberParameters;
	unsigned long ExceptionInformation[15];
};
struct DE_EXCEPTION_POINTERS
{
	DE_EXCEPTION_RECORD* ExceptionRecord;
	void* ContextRecord;
};

class SEHException : public std::exception
{
public:
	unsigned int m_code;
	void* m_exceptionAddress;
	bool m_hasAccessInfo;
	unsigned long m_accessType;   // 0=read, 1=write, 8=DEP/execute
	unsigned long m_faultAddress;
	unsigned long m_callStack[16];
	int m_callStackCount;

	SEHException(unsigned int code, struct _EXCEPTION_POINTERS* pExpRaw)
		: m_code(code), m_exceptionAddress(NULL), m_hasAccessInfo(false), m_accessType(0), m_faultAddress(0), m_callStackCount(0)
	{
		DE_EXCEPTION_POINTERS* pExp = reinterpret_cast<DE_EXCEPTION_POINTERS*>(pExpRaw);
		if (pExp && pExp->ExceptionRecord)
		{
			m_exceptionAddress = pExp->ExceptionRecord->ExceptionAddress;
			if (pExp->ExceptionRecord->NumberParameters >= 2)
			{
				m_hasAccessInfo = true;
				m_accessType = pExp->ExceptionRecord->ExceptionInformation[0];
				m_faultAddress = pExp->ExceptionRecord->ExceptionInformation[1];
			}
		}

		// Walk the EBP chain from the crash's own CONTEXT (x86 only - this
		// project builds Win32). Reading raw offsets into the x86 CONTEXT
		// struct instead of the real type for the same reason as
		// DE_EXCEPTION_POINTERS above (avoid a full <windows.h> here):
		// Ebp is at byte offset 180, Eip at 184 in the real CONTEXT layout.
		// Only the leaf frame's address (Ebp+4 = return address) was ever
		// visible before this - with several possible unguarded-pack call
		// sites already patched, walking the full chain is what actually
		// tells us WHICH caller still has an unguarded/null access, instead
		// of guessing site by site again.
		if (pExp && pExp->ContextRecord)
		{
			unsigned char* ctx = (unsigned char*)pExp->ContextRecord;
			unsigned long ebp = *(unsigned long*)(ctx + 180);

			for (int i = 0; i < 16 && ebp != 0; i++)
			{
				// Sanity-check before dereferencing: must look like a real
				// stack address, monotonically increasing (stack grows down,
				// so caller frames have higher Ebp than callee frames).
				if (ebp < 0x10000 || ebp > 0x7FFFFFFFUL)
					break;

				unsigned long retAddr = *(unsigned long*)(ebp + 4);
				m_callStack[m_callStackCount++] = retAddr;

				unsigned long nextEbp = *(unsigned long*)ebp;
				if (nextEbp <= ebp)
					break;
				ebp = nextEbp;
			}
		}
	}
	const char* what() const noexcept override { return "structured exception"; }
};

void SEHTranslator(unsigned int code, struct _EXCEPTION_POINTERS* pExp)
{
	throw SEHException(code, pExp);
}

// For logging the module base only (see the SEHException catch block) -
// avoids pulling in a full <windows.h> just for this one call.
extern "C" __declspec(dllimport) HMODULE __stdcall GetModuleHandleA(const char* lpModuleName);
#endif /* PLATFORM_WIN32_HOST */

// Language detection
enum DARKEDEN_LANGUAGE
{
	DARKEDEN_KOREAN = 0,
	DARKEDEN_CHINESE,
	DARKEDEN_JAPANESE,
	DARKEDEN_ENGLISH,
	DARKEDEN_TAIWAN,
	DARKEDEN_LANGUAGE_MAX
};
extern DARKEDEN_LANGUAGE CheckDarkEdenLanguage();

// Client mode and UI
extern enum CLIENT_MODE g_Mode;
extern C_VS_UI gC_vs_ui;

#ifdef main
#undef main
#endif

//-----------------------------------------------------------------------------
// SDL-specific globals
//-----------------------------------------------------------------------------
SDL_Window* g_pSDLWindow = NULL;
SDL_Renderer* g_pSDLRenderer = NULL;
bool g_bRunning = true;

//-----------------------------------------------------------------------------
// SDL_PresentGameBackBuffer
//
// Actually pushes g_pBack to the screen. CSDLGraphics::Flip() is a no-op
// stub left over from the DirectDraw port - the real presentation only
// happened here, inline in the main loop. Blocking modal loops elsewhere
// (e.g. GameMain.cpp's "disconnected, press any key" dialog) call
// CSDLGraphics::Flip() expecting a frame to be shown and then wait for
// input, so without this they spin forever on a frame that was never
// presented, looking like a frozen window.
//-----------------------------------------------------------------------------
void SDL_PresentGameBackBuffer()
{
	if (g_pLast != NULL && g_pBack != NULL) {
		POINT origin = {0, 0};
		g_pBack->Blt(&origin, g_pLast, NULL);
	}

	if (g_pBack != NULL) {
		spritectl_surface_t backend_surface = g_pBack->GetBackendSurface();
		if (backend_surface != SPRITECTL_INVALID_SURFACE) {
			spritectl_present_surface(backend_surface, g_pSDLRenderer);
		}
	}

	SDL_RenderPresent(g_pSDLRenderer);
}

//-----------------------------------------------------------------------------
// Stub implementations for functions not available on non-Windows platforms
//-----------------------------------------------------------------------------

// Stub for ExecuteActionInfoFromMainNode
void ExecuteActionInfoFromMainNode(
	unsigned short, unsigned short, unsigned short, int, int, unsigned int,
	unsigned short, unsigned short, int, unsigned long,
	struct MActionResult*, bool, int, int)
{
	// Stub - do nothing on SDL platform
}

// g_Print / FillRect / rectangle are provided by RenderingFunctions.cpp

/*-----------------------------------------------------------------------------
 *
 * SDL version of InitApp - Creates window and initializes game
 *
 *-----------------------------------------------------------------------------*/
static BOOL InitApp(int nCmdShow)
{
	int cx = 800, cy = 600;
	// Created hidden and only revealed once InitGame() below has drawn a
	// real frame (see SDL_ShowWindow near the end of this function). While
	// shown-but-idle, InitGame()'s long synchronous load (no message pump,
	// no repeated presents for ~1s+) leaves the swapchain's other backbuffer(s)
	// exactly as the GPU driver initialized them - some drivers fill unused
	// VRAM with a debug pattern (magenta/pink is a common one) - and since
	// nothing re-presents during that window, Windows/DWM can show that
	// stale buffer instead of the one black frame we already presented.
	// That's the "flashes pink after the black frame, before the title art"
	// startup glitch. Not showing the window until content is ready sidesteps
	// this regardless of the exact GPU-driver behavior behind it.
	Uint32 flags = SDL_WINDOW_HIDDEN;

	// Determine window size and flags based on command line
	extern bool g_bFullScreen;
	extern BOOL g_MyFull;

	if (g_bFullScreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
		flags |= SDL_WINDOW_BORDERLESS;

		// Get screen dimensions
		SDL_DisplayMode dm;
		if (SDL_GetCurrentDisplayMode(0, &dm) == 0) {
			cx = dm.w;
			cy = dm.h;
		}
	} else {
		flags |= SDL_WINDOW_RESIZABLE;

		// Check for 1024x768 mode
		extern RECT g_GameRect;
		if (g_MyFull) {
			cx = 1024;
			cy = 768;
		} else {
			cx = g_GameRect.right;
			cy = g_GameRect.bottom;
		}
	}

	// Create SDL window
	g_pSDLWindow = SDL_CreateWindow(
		"Dark Eden",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		cx, cy,
		flags
	);

	if (!g_pSDLWindow) {
		fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
		return FALSE;
	}

	// Prefer the Direct3D11 SDL2 render backend - matches
	// Data/Info/ClientModern.ini's SDL2RenderDriver=direct3d11 from the
	// legacy client's own experimental SDL2 bridge. Without this hint SDL2
	// auto-picks a driver (order varies by SDL2 build/GPU/drivers - can land
	// on direct3d9 or opengl instead), so this is what actually pins down
	// which backend gets used instead of leaving it to chance. Harmless if
	// unavailable: SDL_CreateRenderer falls back to auto-selection below.
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");

	// Create SDL renderer
	// NOTE: no SDL_RENDERER_PRESENTVSYNC - the original client never vsync-
	// locked either (players routinely saw 200+ FPS), and capping to the
	// monitor's refresh rate (~58-60fps here) was a regression vs that.
	g_pSDLRenderer = SDL_CreateRenderer(
		g_pSDLWindow, -1,
		SDL_RENDERER_ACCELERATED
	);

	if (!g_pSDLRenderer) {
		fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
		return FALSE;
	}

	// Set the renderer's draw color to black for SDL_RenderClear
	// This ensures the screen is cleared to black each frame
	if (SDL_SetRenderDrawColor(g_pSDLRenderer, 0, 0, 0, 255) != 0) {
		fprintf(stderr, "Failed to set render draw color: %s\n", SDL_GetError());
	}

	// Present one black frame immediately. Without this, the window is
	// shown (SDL_WINDOW_SHOWN) with its back buffer never cleared/presented
	// yet, so the OS displays whatever was already in that GPU memory until
	// InitGame() finishes its (lengthy) loading and the first real frame is
	// presented - visible as a brief flash of garbage/pink on startup.
	SDL_RenderClear(g_pSDLRenderer);
	SDL_RenderPresent(g_pSDLRenderer);

	// Hide cursor
	SDL_ShowCursor(0);

	// Initialize SpriteLib SDL backend
	if (spritectl_init() != 0) {
		fprintf(stderr, "Failed to initialize SpriteLib backend\n");
		return FALSE;
	}

	// Initialize game
	if (!InitGame()) {
		fprintf(stderr, "Failed to initialize game\n");
		return FALSE;
	}

	// GameInit.cpp's DrawTitleLoading() runs several times during the loading
	// steps above and draws a "please wait" sprite straight onto g_pBack;
	// whatever it left there when InitGame() returned would otherwise be the
	// very first thing shown (that transient loading-sprite content, not the
	// real title screen). Clear BOTH buffers - SDL_PresentGameBackBuffer()
	// below copies g_pLast over g_pBack unconditionally, so clearing g_pBack
	// alone would just get overwritten again by g_pLast's own leftovers.
	if (g_pLast != NULL) {
		g_pLast->FillSurface(0);
	}
	if (g_pBack != NULL) {
		g_pBack->FillSurface(0);
	}

	// Push whatever InitGame() has drawn by now (the title screen) to the
	// backbuffer and only THEN reveal the window, so the very first frame
	// the user ever sees is real content - see the flags comment above.
	SDL_PresentGameBackBuffer();
	SDL_ShowWindow(g_pSDLWindow);

	return TRUE;
}

/*-----------------------------------------------------------------------------
 *
 * Cleanup SDL resources
 *
 *-----------------------------------------------------------------------------*/
static void CleanupSDL()
{
	if (g_pSDLRenderer) {
		SDL_DestroyRenderer(g_pSDLRenderer);
		g_pSDLRenderer = NULL;
	}
	if (g_pSDLWindow) {
		SDL_DestroyWindow(g_pSDLWindow);
		g_pSDLWindow = NULL;
	}
	spritectl_shutdown();
}

/*-----------------------------------------------------------------------------
 *
 * Main entry point for SDL2 builds
 *
 *-----------------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
#ifdef PLATFORM_WIN32_HOST
	_set_se_translator(SEHTranslator);
#endif

	SDLMAIN_DEBUG("DEBUG: Dark Eden SDL2 starting...\n");

	// Parse command line
	const char* cmdLine = "0000000001";  // Default: window mode
	if (argc > 1 && argv[1] != NULL) {
		cmdLine = argv[1];
	}

	SDLMAIN_DEBUG("Command line: %s\n", cmdLine);
	SDLMAIN_LOG("========================================\n");
	SDLMAIN_LOG("Dark Eden Client (SDL2 Backend)\n");
	SDLMAIN_LOG("Based on Client.cpp WinMain\n");
	SDLMAIN_LOG("========================================\n");
	SDLMAIN_LOG("Command line: %s\n", cmdLine);
	SDLMAIN_LOG("\n");

	// Global variables from WinMain
	extern BOOL g_MyFull;
	extern bool g_bFullScreen;
	extern RECT g_GameRect;
	extern int g_x, g_y;
	extern int g_SECTOR_WIDTH, g_SECTOR_HEIGHT;
	extern int g_SECTOR_WIDTH_HALF, g_SECTOR_HEIGHT_HALF;
	extern int g_SECTOR_SKIP_PLAYER_LEFT, g_SECTOR_SKIP_PLAYER_UP;
	extern int g_TILESURFACE_SECTOR_WIDTH, g_TILESURFACE_SECTOR_HEIGHT;
	extern int g_TILESURFACE_SECTOR_OUTLINE_RIGHT, g_TILESURFACE_SECTOR_OUTLINE_DOWN;
	extern int g_TILESURFACE_WIDTH, g_TILESURFACE_HEIGHT;
	extern int g_TILESURFACE_OUTLINE_RIGHT, g_TILESURFACE_OUTLINE_DOWN;
	extern int g_TILE_X_HALF, g_TILE_Y_HALF;
	extern DWORD g_dwVideoMemory;
	extern MTopView* g_pTopView;

	bool cmpFullScreen = false;

	// Parse command line (from WinMain lines 3047-3115)
	size_t cmdLen = strlen(cmdLine);
	if (cmdLen > 0) {
		char lastChar = cmdLine[cmdLen - 1];

		if (lastChar == '1') {
			g_MyFull = false;
			cmpFullScreen = false;
		}
		else if (lastChar == '2') {
			g_MyFull = false;
			cmpFullScreen = true;
		}
		else if (lastChar == '3') {
			g_MyFull = true;
			cmpFullScreen = false;
			g_GameRect.left = 1023;
			g_GameRect.top = 767;
			g_GameRect.right = 1024;
			g_GameRect.bottom = 768;
			g_x = 512;
			g_y = 384;
			g_SECTOR_WIDTH = 21;
			g_SECTOR_HEIGHT = 32;
			g_SECTOR_WIDTH_HALF = 11;
			g_SECTOR_HEIGHT_HALF = 17;
			g_SECTOR_SKIP_PLAYER_LEFT = -10;
			g_SECTOR_SKIP_PLAYER_UP = -16;
			g_TILESURFACE_SECTOR_WIDTH = 27;
			g_TILESURFACE_SECTOR_HEIGHT = 37;
			g_TILESURFACE_SECTOR_OUTLINE_RIGHT = 24;
			g_TILESURFACE_SECTOR_OUTLINE_DOWN = 34;
			g_TILESURFACE_WIDTH = 1296;
			g_TILESURFACE_HEIGHT = 888;
			g_TILESURFACE_OUTLINE_RIGHT = 1152;
			g_TILESURFACE_OUTLINE_DOWN = 816;
			g_TILE_X_HALF = 24;
			g_TILE_Y_HALF = 12;
		}
		else if (lastChar == '4') {
			g_MyFull = true;
			cmpFullScreen = true;
			g_GameRect.left = 1023;
			g_GameRect.top = 767;
			g_GameRect.right = 1024;
			g_GameRect.bottom = 768;
			g_x = 512;
			g_y = 384;
			g_SECTOR_WIDTH = 21;
			g_SECTOR_HEIGHT = 32;
			g_SECTOR_WIDTH_HALF = 11;
			g_SECTOR_HEIGHT_HALF = 17;
			g_SECTOR_SKIP_PLAYER_LEFT = -10;
			g_SECTOR_SKIP_PLAYER_UP = -16;
			g_TILESURFACE_SECTOR_WIDTH = 27;
			g_TILESURFACE_SECTOR_HEIGHT = 37;
			g_TILESURFACE_SECTOR_OUTLINE_RIGHT = 24;
			g_TILESURFACE_SECTOR_OUTLINE_DOWN = 34;
			g_TILESURFACE_WIDTH = 1296;
			g_TILESURFACE_HEIGHT = 888;
			g_TILESURFACE_OUTLINE_RIGHT = 1152;
			g_TILESURFACE_OUTLINE_DOWN = 816;
			g_TILE_X_HALF = 24;
			g_TILE_Y_HALF = 12;
		}
	}

	// Set full screen flag
	g_bFullScreen = cmpFullScreen;

	//-----------------------------------------------------------------
	// Get current directory (from WinMain lines 3201-3213)
	//-----------------------------------------------------------------
	char g_CWD[_MAX_PATH];

	// On macOS/Linux, use getcwd() to get current directory
	// Note: When running from DarkEden directory, this should already be correct
	if (getcwd(g_CWD, _MAX_PATH) != NULL) {
		SDLMAIN_LOG("Working directory: %s\n", g_CWD);
	} else {
		// Fallback to "." if getcwd fails
		g_CWD[0] = '.';
		g_CWD[1] = '\0';
		SDLMAIN_LOG("Working directory: . (getcwd failed)\n");
	}

	//-----------------------------------------------------------------
	// Initialize SDL2
	//-----------------------------------------------------------------
	SDLMAIN_LOG("Initializing SDL2...\n");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) < 0) {
		fprintf(stderr, "ERROR: SDL_Init failed: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	// Initialize SDL_ttf for font rendering
	SDLMAIN_LOG("Initializing SDL_ttf...\n");
	if (TTF_Init() < 0) {
		fprintf(stderr, "ERROR: TTF_Init failed: %s\n", TTF_GetError());
		return 1;
	}
	atexit(TTF_Quit);

	//-----------------------------------------------------------------
	// Initialize language and character input (IME)
	// This MUST be done before InitGame() - same order as WinMain
	//-----------------------------------------------------------------
	SDLMAIN_LOG("Detecting language and initializing IME...\n");
	DARKEDEN_LANGUAGE Language = CheckDarkEdenLanguage();

	// Override to Chinese for macOS/Linux builds with Chinese resources
	Language = DARKEDEN_CHINESE;

	switch (Language)
	{
	case DARKEDEN_CHINESE:
		gC_ci = new CI_CHINESE;
		SDLMAIN_LOG("Language: Chinese (Simplified)\n");
		break;

	case DARKEDEN_KOREAN:
		gC_ci = new CI_KOREAN;
		SDLMAIN_LOG("Language: Korean\n");
		break;

	default:
		// Default to Chinese for non-Windows platforms
		gC_ci = new CI_CHINESE;
		SDLMAIN_LOG("Language: Chinese (Simplified - default)\n");
		break;
	}

	//-----------------------------------------------------------------
	// Initialize App (create window, init game)
	//-----------------------------------------------------------------
	SDLMAIN_LOG("Initializing application...\n");
	int nCmdShow = 1;  // Default: show window

	// Frame rate tracking variables (declared outside InitApp block for cleanup access)
	const int g_FrameGood = 15;  // Minimum acceptable FPS (from Client.cpp line 154)
	DWORD g_FrameCount = 0;  // Local frame counter

	try {
		if (InitApp(nCmdShow))
		{
			// Set default video memory for non-Windows platforms
			g_dwVideoMemory = 256 * 1024 * 1024;  // 256 MB default

		//-----------------------------------------------------------------
		// Set user language based on detected language
		//-----------------------------------------------------------------
		extern UserInformation* g_pUserInformation;
		if (g_pUserInformation != NULL)
		{
			if (gC_ci->IsKorean())
			{
				g_pUserInformation->SetKorean();
				SDLMAIN_LOG("User language set to: Korean\n");
			}
			else if (gC_ci->IsChinese())
			{
				g_pUserInformation->SetChinese();
				SDLMAIN_LOG("User language set to: Chinese\n");
			}
			else if (gC_ci->IsJapanese())
			{
				g_pUserInformation->SetJapanese();
				SDLMAIN_LOG("User language set to: Japanese\n");
			}
		}

		//-----------------------------------------------------------------
		// Initialize TopView
		//-----------------------------------------------------------------
		SDLMAIN_LOG("Initializing TopView...\n");
		if (!g_pTopView->IsInit())
		{
			SDLMAIN_LOG("TopView not initialized, calling Init()...\n");
			try {
				g_pTopView->Init();
				SDLMAIN_LOG("TopView Init() completed\n");
			} catch (NoSuchElementException& e) {
				fprintf(stderr, "ERROR: NoSuchElementException in TopView::Init(): %s\n", e.toString().c_str());
				// Continue anyway - some resources might be missing
			} catch (Throwable& t) {
				fprintf(stderr, "ERROR: Throwable in TopView::Init(): %s\n", t.toString().c_str());
				// Continue anyway
			}
		}
		SDLMAIN_LOG("TopView initialization complete\n");

		g_bActiveApp = TRUE;
		CheckActivate(TRUE);

		// Ensure window has focus for mouse input
		SDL_RaiseWindow(g_pSDLWindow);

		SDLMAIN_LOG("\n");
		SDLMAIN_LOG("Starting game loop...\n");
		SDLMAIN_LOG("Press Ctrl+C or close window to exit.\n");
		SDLMAIN_LOG("\n");
		SDLMAIN_DEBUG("DEBUG: Entering game loop, g_bActiveApp = %d\n", g_bActiveApp);

		//-----------------------------------------------------------------
		// Main game loop (from WinMain lines 4244-4341)
		//-----------------------------------------------------------------
		// Note: All SDL events are now handled by dxlib_input_update() in DXLibBackendSDL.cpp
		// This prevents event queue conflicts and ensures mouse events are properly processed
		static int loopCount = 0;

		// Frame rate tracking variables
		g_StartTime = timeGetTime();
		g_StartFrameCount = 0;
		g_FrameCount = 0;

		while (g_bRunning)
		{
			// Process SDL events (IMPORTANT: prevents "Not Responding" on macOS)
			// dxlib_input_update handles: SDL_QUIT, SDL_WINDOWEVENT, SDL_KEY*, SDL_MOUSE*, SDL_TEXT*
			dxlib_input_update();

			// Game update when active
			if (g_bActiveApp)
			{
				// Real system uptime (winmm timeGetTime, via Platform.h), matching
				// CWaitPacketUpdate.cpp/GCUpdateInfoHandler.cpp's g_CurrentTime feed.
				// Must NOT be SDL_GetTicks() (process uptime): CGameUpdate.cpp's
				// anti-cheat check treats g_CurrentTime < 60000 as "suspicious",
				// which SDL_GetTicks() satisfies for the first real minute of
				// every single run, auto-quitting almost every session.
				g_CurrentTime = timeGetTime();

				// NOTE: no SDL_RenderClear() here - SDL_PresentGameBackBuffer()
				// below (called unconditionally every active frame, see end of
				// this block) always does a full-window SDL_RenderCopy of
				// g_pBack, so clearing first was pure wasted GPU work: every
				// pixel it touched got immediately overdrawn anyway.

				// Game update (from WinMain lines 4275-4290)
				if (g_pUpdate != NULL)
				{
					g_pUpdate->Update();
				}

				// Frame rate tracking (from WinMain lines 4320-4333)
				DWORD timeGap = g_CurrentTime - g_StartTime;

				if (timeGap > 1000)
				{
					g_FrameRate = (g_FrameCount - g_StartFrameCount) * 1000 / timeGap;
					g_bGoodFPS = (g_FrameRate >= g_FrameGood);

					g_StartTime = g_CurrentTime;
					g_StartFrameCount = g_FrameCount;
				}

				// Present back buffer to screen
				SDL_PresentGameBackBuffer();
				g_FrameCount++;
			}
			else
			{
				// Game inactive - sleep to reduce CPU usage
				SDL_Delay(10);  // Replaces WaitMessage()
			}
		}
	}
	else
	{
		fprintf(stderr, "ERROR: InitApp failed\n");
	}
	}
	catch (FileNotExistException& e) {
		fprintf(stderr, "ERROR: FileNotExistException: %s\n", e.toString().c_str());
		{ FILE* f = fopen("Log/ui_debug.log", "a"); if (f) { fprintf(f, "FATAL: FileNotExistException: %s\n", e.toString().c_str()); fclose(f); } }
		return 1;
	}
	catch (Throwable& t) {
		fprintf(stderr, "ERROR: Throwable: %s\n", t.toString().c_str());
		{ FILE* f = fopen("Log/ui_debug.log", "a"); if (f) { fprintf(f, "FATAL: Throwable: %s\n", t.toString().c_str()); fclose(f); } }
		return 1;
	}
#ifdef PLATFORM_WIN32_HOST
	catch (SEHException& se) {
		fprintf(stderr, "ERROR: structured exception code=0x%08X\n", se.m_code);
		{
			FILE* f = fopen("Log/ui_debug.log", "a");
			if (f)
			{
				fprintf(f, "FATAL: structured exception code=0x%08X (0xC0000005=access violation)\n", se.m_code);
				// Module base lets a later offline lookup (dumpbin /map,
				// addr2line-style tooling against DarkEden.pdb) turn this
				// into a source line - "access violation" alone isn't
				// actionable, this at least pins down WHERE it happened.
				HMODULE hSelf = GetModuleHandleA(NULL);
				fprintf(f, "  exceptionAddress=0x%p moduleBase=0x%p offset=0x%p\n",
					se.m_exceptionAddress, (void*)hSelf,
					(void*)((char*)se.m_exceptionAddress - (char*)hSelf));
				if (se.m_hasAccessInfo)
				{
					fprintf(f, "  accessType=%s faultAddress=0x%08lX\n",
						se.m_accessType == 1 ? "write" : (se.m_accessType == 8 ? "execute(DEP)" : "read"),
						se.m_faultAddress);
				}
				fprintf(f, "  callStack (offsets from moduleBase, resolve each against DarkEden.pdb):\n");
				for (int i = 0; i < se.m_callStackCount; i++)
				{
					fprintf(f, "    [%d] 0x%p offset=0x%p\n", i,
						(void*)se.m_callStack[i],
						(void*)((char*)se.m_callStack[i] - (char*)hSelf));
				}
				fclose(f);
			}
		}
		return 1;
	}
#endif
	catch (std::exception& e) {
		fprintf(stderr, "ERROR: std::exception: %s\n", e.what());
		{ FILE* f = fopen("Log/ui_debug.log", "a"); if (f) { fprintf(f, "FATAL: std::exception: %s\n", e.what()); fclose(f); } }
		return 1;
	}
	catch (...) {
		fprintf(stderr, "ERROR: Unknown exception\n");
		{ FILE* f = fopen("Log/ui_debug.log", "a"); if (f) { fprintf(f, "FATAL: Unknown exception (non-standard, no message available)\n"); fclose(f); } }
		return 1;
	}

	//-----------------------------------------------------------------
	// Cleanup (from WinMain lines 4434-4446)
	//-----------------------------------------------------------------
	SDLMAIN_LOG("\n");
	SDLMAIN_LOG("Shutting down...\n");

	ReleaseAllObjects();

	// Clean up character input object (must be done after ReleaseAllObjects)
	delete gC_ci;
	gC_ci = NULL;

	// Clean up SDL resources
	CleanupSDL();

	SDLMAIN_LOG("Game exited cleanly.\n");
	SDLMAIN_LOG("Total frames rendered: %u\n", g_FrameCount);

	return 0;
}

#endif /* PLATFORM_WINDOWS */
