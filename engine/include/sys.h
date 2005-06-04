///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_SYS_H
#define INCLUDED_SYS_H

#include "enginedll.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
// implemented in syswin.cpp, syslinux.cpp, or other OS-specific implementation file

ENGINE_API void SysAppActivate(bool active);
ENGINE_API void SysQuit();
ENGINE_API bool SysGetClipboardString(char * psz, int max);
ENGINE_API bool SysSetClipboardString(const char * psz);
ENGINE_API HANDLE SysCreateWindow(const tChar * pszTitle, int width, int height);
ENGINE_API void SysSwapBuffers();
ENGINE_API int SysEventLoop(void (* pfnFrameHandler)(), void (* pfnResizeHack)(int, int));

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_SYS_H
