// heckSystem.h: interface for the CheckSystem class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __HECKSYSTEM_H__
#define __HECKSYSTEM_H__

#ifdef PLATFORM_WINDOWS
#ifdef PLATFORM_USE_SDL
#include <Platform.h>
#else
#include <Windows.h>
#endif
#else
#include <Platform.h>
#endif


class CheckSystem
{
public:
	BOOL GetSystem();
	CheckSystem();
	virtual ~CheckSystem();

};

#endif // !defined(AFX_HECKSYSTEM_H__6B8B3F6F_FEBE_481A_8C37_32FE368CC667__INCLUDED_)
