///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIBUTTON_H
#define INCLUDED_GUIBUTTON_H

#include "guielementbase.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIButtonElement
//

class cGUIButtonElement : public cComObject<cGUIElementBase<IGUIButtonElement>, &IID_IGUIButtonElement>
{
public:
   cGUIButtonElement();
   ~cGUIButtonElement();

   virtual tResult OnEvent(IGUIEvent * pEvent);

   virtual const tGUIChar * GetText() const;
   virtual tResult GetText(tGUIString * pText) const;
   virtual tResult SetText(const char * pszText);

   virtual tResult GetOnClick(tGUIString * pOnClick) const;
   virtual tResult SetOnClick(const char * pszOnClick);

private:
   tGUIString m_text;
   tGUIString m_onClick;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIBUTTON_H
