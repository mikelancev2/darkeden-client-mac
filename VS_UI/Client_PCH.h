////////////////////////////////////////////////////////////////////////////////
//	created:	2004/12/22
//	file base:	client_pch.h
//
//	Modified for cross-platform support (macOS/Linux)
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef PLATFORM_WINDOWS
	#pragma warning(disable:4290)
	#pragma warning(disable:4018)
	#pragma warning(disable:4244)
	#pragma warning(disable:4018)
	#pragma warning(disable:4786)
	#pragma warning(push)
#endif

#include <string>
#include <cassert>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdarg>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(PLATFORM_WINDOWS) && !defined(PLATFORM_USE_SDL)
	#ifdef PLATFORM_USE_SDL
#include <Platform.h>
#else
#include <windows.h>
#endif
	#include <MMSystem.h>
	#include <Digitalv.h>
	#include <DDraw.h>
	#include <io.h>
	#include <fcntl.h>
	#pragma warning(pop)
#else
	/* Use platform abstraction layer */
	#include <Platform.h>
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
	#if !defined(_WIN32) && !defined(_WIN64)
		#include <unistd.h>
	#endif
#endif

//#include "GAME1024.h"

using std::string;
using std::vector;
using std::map;
using std::list;
using std::deque;
using std::bitset;

extern BOOL g_MyFull;
extern RECT g_GameRect;
//extern GAME1024 g_NewMode;
extern	LONG g_SECTOR_WIDTH;
extern	LONG g_SECTOR_HEIGHT;
extern	LONG g_SECTOR_WIDTH_HALF;
extern	LONG g_SECTOR_HEIGHT_HALF;
extern	LONG g_SECTOR_SKIP_PLAYER_LEFT;
extern	LONG g_SECTOR_SKIP_PLAYER_UP;

extern	LONG g_TILESURFACE_SECTOR_WIDTH;
extern	LONG g_TILESURFACE_SECTOR_HEIGHT;
extern	LONG g_TILESURFACE_SECTOR_OUTLINE_RIGHT;
extern	LONG g_TILESURFACE_SECTOR_OUTLINE_DOWN;
extern	LONG g_TILESURFACE_WIDTH;
extern	LONG g_TILESURFACE_HEIGHT;
extern	LONG g_TILESURFACE_OUTLINE_RIGHT;
extern	LONG g_TILESURFACE_OUTLINE_DOWN;
extern	LONG g_TILE_X_HALF;
extern	LONG g_TILE_Y_HALF;
