///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUILABEL_H
#define INCLUDED_GUILABEL_H

#include "guielementbase.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IRenderFont);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElement
//

class cGUILabelElement : public cComObject<cGUIElementBase<IGUILabelElement>, &IID_IGUILabelElement>
{
public:
   cGUILabelElement();
   ~cGUILabelElement();

   virtual tResult GetRendererClass(tGUIString * pRendererClass);

   virtual const char * GetText() const;
   virtual void SetText(const char * pszText);

private:
   tGUIString m_text;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelElementFactory
//

class cGUILabelElementFactory : public cComObject<IMPLEMENTS(IGUIElementFactory)>
{
public:
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement);
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelStatelessRenderer
//

class cGUILabelStatelessRenderer : public cComObject<IMPLEMENTS(IGUIElementRenderer)>
{
public:
   cGUILabelStatelessRenderer();
   ~cGUILabelStatelessRenderer();

   virtual tResult Render(IGUIElement * pElement, IRenderDevice * pRenderDevice);

   virtual tGUISize GetPreferredSize(IGUIElement * pElement);

private:
   cAutoIPtr<IRenderFont> m_pFont;
};

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUILabelStatelessRendererFactory
//

class cGUILabelStatelessRendererFactory : public cComObject<IMPLEMENTS(IGUIElementRendererFactory)>
{
public:
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer);

private:
   cAutoIPtr<IGUIElementRenderer> m_pStatelessLabelRenderer;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUILABEL_H
