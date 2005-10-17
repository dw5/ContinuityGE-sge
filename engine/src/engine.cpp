///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "engine.h"
#include "model.h"

#include "resourceapi.h"
#include "globalobj.h"

#include <tinyxml.h>

#include <GL/glew.h>

#ifdef HAVE_CG
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include <CgFX/ICgFX.h>
#include <CgFX/ICgFXEffect.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

void * TiXmlDocumentFromText(void * pData, int dataLength, void * param)
{
   char * psz = reinterpret_cast<char*>(pData);
   if (psz != NULL && strlen(psz) > 0)
   {
      TiXmlDocument * pDoc = new TiXmlDocument;
      if (pDoc != NULL)
      {
         pDoc->Parse(psz);
         bool bError = pDoc->Error();
         if (bError)
         {
            delete pDoc;
            pDoc = NULL;
         }
         return pDoc;
      }
   }

   return NULL;
}

void TiXmlDocumentUnload(void * pData)
{
   TiXmlDocument * pDoc = reinterpret_cast<TiXmlDocument*>(pData);
   delete pDoc;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CG

CGcontext g_CgContext = NULL;
ulong g_CgContextRef = 0;
CGerrorCallbackFunc g_CgOldErrorCallback = NULL;

void CgErrorCallback()
{
   CGerror lastError = cgGetError();
   if (lastError)
   {
      DebugMsg(cgGetErrorString(lastError));
      const char * pszListing = cgGetLastListing(g_CgContext);
      if (pszListing != NULL)
      {
         DebugMsg1("   %s\n", pszListing);
      }
   }
}

CGcontext CgGetContext()
{
   if (g_CgContext == NULL)
   {
      g_CgContext = cgCreateContext();
      Verify(++g_CgContextRef == 1);
      g_CgOldErrorCallback = cgGetErrorCallback();
      cgSetErrorCallback(CgErrorCallback);
   }
   else
   {
      ++g_CgContextRef;
   }

   return g_CgContext;
}

void CgReleaseContext()
{
   if (g_CgContextRef > 0)
   {
      if (--g_CgContextRef == 0 && g_CgContext != NULL)
      {
         cgSetErrorCallback(g_CgOldErrorCallback);
         g_CgOldErrorCallback = NULL;
         cgDestroyContext(g_CgContext);
         g_CgContext = NULL;
      }
   }
}

#endif // HAVE_CG

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CG

CGprofile g_CgProfile = CG_PROFILE_UNKNOWN;

void * CgProgramFromText(void * pData, int dataLength, void * param)
{
   // Must get the Cg context first
   CGcontext cgContext = CgGetContext();
   if (cgContext == NULL)
   {
      return NULL;
   }

   if (g_CgProfile == CG_PROFILE_UNKNOWN)
   {
      g_CgProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
      if (g_CgProfile == CG_PROFILE_UNKNOWN)
      {
         return NULL;
      }
   }

   char * psz = reinterpret_cast<char*>(pData);
   if (psz != NULL && strlen(psz) > 0)
   {
      CGprogram program = cgCreateProgram(cgContext, CG_SOURCE, psz, g_CgProfile, NULL, NULL);
      if (program != NULL)
      {
         cgGLLoadProgram(program);
         return program;
      }
   }

   return NULL;
}

void CgProgramUnload(void * pData)
{
   CGprogram program = reinterpret_cast<CGprogram>(pData);
   if (program != NULL)
   {
      cgDestroyProgram(program);
   }
}

///////////////////////////////////////////////////////////////////////////////

void * CgEffectFromText(void * pData, int dataLength, void * param)
{
   char * psz = reinterpret_cast<char*>(pData);
   if (psz != NULL && strlen(psz) > 0)
   {
   }

   return NULL;
}

void CgEffectUnload(void * pData)
{
//   TiXmlDocument * pDoc = reinterpret_cast<TiXmlDocument*>(pData);
//   delete pDoc;
}

#endif // HAVE_CG

///////////////////////////////////////////////////////////////////////////////

extern tResult GlTextureResourceRegister(); // gltexture.cpp
extern tResult SoundResourceRegister(); // soundwin.cpp

tResult EngineRegisterResourceFormats()
{
   UseGlobal(ResourceManager);
   if (!!pResourceManager)
   {
      if (cModel::RegisterResourceFormat() == S_OK
         && GlTextureResourceRegister() == S_OK
         && SoundResourceRegister() == S_OK
#ifdef HAVE_CG
         && TextFormatRegister("cg") == S_OK
         && TextFormatRegister("fx") == S_OK
         && pResourceManager->RegisterFormat(kRT_CgProgram, kRT_AsciiText, NULL, NULL, CgProgramFromText, CgProgramUnload) == S_OK
         && pResourceManager->RegisterFormat(kRT_CgEffect, kRT_AsciiText, NULL, NULL, CgEffectFromText, CgEffectUnload) == S_OK
#endif
         && pResourceManager->RegisterFormat(kRT_TiXml, kRT_AsciiText, _T("xml"), NULL, TiXmlDocumentFromText, TiXmlDocumentUnload) == S_OK)
      {
         return S_OK;
      }
   }
   return E_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
