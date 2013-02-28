/////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_STDAFX_H
#define INCLUDED_STDAFX_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////

#include "tech/techtypes.h"
#include "tech/techassert.h"
#include "tech/techlog.h"

#define QI_TEMPLATE_METHOD_FOR_ATL
#include "tech/combase.h"

/////////////////////////////////////////////////////////////////////////////

#define _WTL_NO_AUTOMATIC_NAMESPACE
#include <atlbase.h>
#include <atlapp.h>

extern WTL::CAppModule _Module;

#include <atlwin.h>

/////////////////////////////////////////////////////////////////////////////

F_DECLARE_HANDLE(HINSTANCE);
extern HINSTANCE g_hInstance;

/////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_STDAFX_H
