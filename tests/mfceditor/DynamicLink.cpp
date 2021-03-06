/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "DynamicLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cDynamicLink
//

////////////////////////////////////////

cDynamicLink::cDynamicLink(const tChar * pszLibrary)
 : m_name(pszLibrary != NULL ? pszLibrary : ""),
   m_hLibrary(NULL)
{
}

////////////////////////////////////////

cDynamicLink::~cDynamicLink()
{
   if (m_hLibrary != NULL)
   {
      FreeLibrary(m_hLibrary);
      m_hLibrary = NULL;
   }
}

////////////////////////////////////////

FARPROC cDynamicLink::GetProcAddress(const tChar * pszProc)
{
   if (m_hLibrary == NULL)
   {
      if (m_name.empty())
      {
         return NULL;
      }

      m_hLibrary = LoadLibrary(m_name.c_str());
      if (m_hLibrary == NULL)
      {
         return NULL;
      }
   }

   return ::GetProcAddress(m_hLibrary, pszProc);
}

/////////////////////////////////////////////////////////////////////////////
