///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_LUAINTERP_H
#define INCLUDED_LUAINTERP_H

#include "scriptapi.h"
#include "globalobj.h"

#ifdef _MSC_VER
#pragma once
#endif

typedef struct lua_State lua_State;

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLuaInterpreter
//

class cLuaInterpreter : public cGlobalObject<IMPLEMENTS(IScriptInterpreter)>
{
public:
   cLuaInterpreter();
   ~cLuaInterpreter();

   virtual tResult Init();
   virtual tResult Term();

   tResult ExecFile(const char * pszFile);
   tResult ExecString(const char * pszCode);
   void CallFunction(const char * pszName, const char * pszArgDesc = NULL, ...);
   void CallFunction(const char * pszName, const char * pszArgDesc, va_list args);
   tResult AddFunction(const char * pszName, tScriptFn pfn);
   tResult RemoveFunction(const char * pszName);

   tResult GetGlobal(const char * pszName, cScriptVar * pValue);
   tResult GetGlobal(const char * pszName, double * pValue);
   tResult GetGlobal(const char * pszName, char * pValue, int cbMaxValue);
   void SetGlobal(const char * pszName, double value);
   void SetGlobal(const char * pszName, const char * pszValue);

   tResult RegisterCustomClass(const tChar * pszClassName, IScriptableFactory * pFactory);
   tResult RevokeCustomClass(const tChar * pszClassName);

private:
   lua_State * m_L;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_LUAINTERP_H
