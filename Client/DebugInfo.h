//-----------------------------------------------------------------------------
// DebugInfo.h
//-----------------------------------------------------------------------------
// CMessageArray를 통한..
// Debug정보를 표시해주는가?
//-----------------------------------------------------------------------------

#ifdef PLATFORM_WINDOWS
#include "MinTr.h"
#endif

#ifndef	__DEBUGINFO_H__
#define	__DEBUGINFO_H__

//#ifdef __GAME_CLIENT__
//	#ifdef _DEBUG
//		#define	OUTPUT_DEBUG
//	#else
//		#define	OUTPUT_DEBUG
//	#endif
//#endif

	#ifdef	OUTPUT_DEBUG
		
		//------------------------------------------
		// g_DebugMessage
		//------------------------------------------		
		#include "CMessageArray.h"


		extern CMessageArray*	g_pDebugMessage;
				

	//#endif


	//#ifdef	_DEBUG
		#ifdef PLATFORM_USE_SDL
#include <Platform.h>
#else
#include <Windows.h>
#endif
		#include <WinBase.h>
		#include <stdio.h> 
		#include <stdarg.h>

		extern char	g_StringBuffer[256];

		//------------------------------------------
		// debug message
		//------------------------------------------
		#define	DEBUG_MESSAGE(debugMessage)					\
			{												\
				va_list		vl;								\
				va_start(vl, debugMessage);					\
				vsprintf(g_StringBuffer, debugMessage, vl);	\
				va_end(vl);									\
				OutputDebugString(g_StringBuffer);			\
			}
		
//		void* PASCAL operator new( unsigned int cb, LPCSTR lpszFileName, int nLine);
//		void PASCAL operator delete(void *p, LPCSTR lpszFileName, int nLine);
		//void PASCAL operator delete [] (void *p, LPCSTR lpszFileName, int nLine);
		
//		#define	DEBUG_NEW			new (__FILE__, __LINE__)

		#define DEBUG_NEW new

		
//		#define	DEBUG_ADD( message )		\
//				if (g_pDebugMessage!=NULL)	\
//					g_pDebugMessage->Add( message );
//
		void	DEBUG_ADD_FORMAT(const char* format, ...);
		void	DEBUG_ADD_FORMAT_ERR(const char* format, ...);
		void	DEBUG_ADD_FORMAT_WAR(const char* format, ...);

		void	DEBUG_ADD(const char* message);
		void	DEBUG_ADD_ERR(const char* message);
		void	DEBUG_ADD_WAR(const char* message);
		void	DEBUG_CMD(int cmd, const char* message);

	// debug가 아닌 경우..
	#else
		// If DebugLog.h was already included in this translation unit, its
		// real DEBUG_ADD (routes to log_write/the file logger) is already
		// defined - don't clobber it with the no-op stub below. This used to
		// unconditionally stub DEBUG_ADD et al. to ((void)0) in Release
		// builds, silently swallowing every diagnostic in any .cpp that
		// includes DebugInfo.h before (or instead of) DebugLog.h - e.g.
		// UIMessageManager.cpp's Execute_UI_LOGIN - making login/socket
		// failures impossible to trace. Files that need real logging but
		// don't already get DebugLog.h transitively should #include
		// "DebugLog.h" directly rather than relying on this fallback.
		#define	DEBUG_MESSAGE(debugMessage)	((void)0)

		#ifndef DEBUG_ADD
		#define	DEBUG_ADD(message)				((void)0)
		#define	DEBUG_ADD_ERR(message)			((void)0)
		#define	DEBUG_ADD_WAR(message)			((void)0)
		#define	DEBUG_CMD(cmd, message)			((void)0)
		#define	DEBUG_ADD_FORMAT(format, ...)	((void)0)
		#define	DEBUG_ADD_FORMAT_ERR(format, ...)	((void)0)
		#define	DEBUG_ADD_FORMAT_WAR(format, ...)	((void)0)
		#endif

//		#define	DEBUG_NEW			new
	#endif


#endif