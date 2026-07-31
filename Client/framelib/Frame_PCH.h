////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#pragma once


#pragma warning(push)
#pragma warning(disable:4018)
#pragma warning(disable:4786)
#include <string>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <bitset>
#include <algorithm>
#include <iostream>
#include <fstream>
#ifdef PLATFORM_WINDOWS
#ifdef PLATFORM_USE_SDL
#include <Platform.h>
#else
#include <Windows.h>
#endif
#else
#include <Platform.h>
#endif
#pragma warning(pop)

using std::string;
using std::vector;
using std::map;
using std::list;
using std::deque;
using std::bitset;
using std::ifstream;
using std::ofstream;

