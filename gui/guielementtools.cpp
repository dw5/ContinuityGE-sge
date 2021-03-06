///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guielementtools.h"
#include "gui/guiapi.h"
#include "gui/guistyleapi.h"

#include "render/renderfontapi.h"

#include "tech/configapi.h"
#include "tech/globalobj.h"
#include "tech/point2.inl"

#ifdef HAVE_UNITTESTPP
#include "UnitTest++.h"
#endif

#include "tech/dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

bool IsDescendant(IGUIElement * pParent, IGUIElement * pElement)
{
   Assert(pParent != NULL);
   Assert(pElement != NULL);

   cAutoIPtr<IGUIElement> pTestParent;

   if (pElement->GetParent(&pTestParent) == S_OK)
   {
      do
      {
         if (CTIsSameObject(pTestParent, pParent))
         {
            return true;
         }

         cAutoIPtr<IGUIElement> pNextParent;
         if (pTestParent->GetParent(&pNextParent) != S_OK)
         {
            SafeRelease(pTestParent);
         }
         else
         {
            pTestParent = pNextParent;
         }
      }
      while (!!pTestParent);
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

tGUIString GUIElementType(IUnknown * pUnkElement)
{
   if (pUnkElement == NULL)
   {
      return tGUIString(_T("(null)"));
   }

   static const struct
   {
      const GUID * pIID;
      const tGUIChar * pszType;
   }
   guiElementTypes[] =
   {
      { &IID_IGUIButtonElement,     _T("Button") },
      { &IID_IGUIDialogElement,     _T("Dialog") },
      { &IID_IGUILabelElement,      _T("Label") },
      { &IID_IGUIListBoxElement,    _T("ListBox") },
      { &IID_IGUIPanelElement,      _T("Panel") },
      { &IID_IGUIScriptElement,     _T("Script") },
      { &IID_IGUIScrollBarElement,  _T("ScrollBar") },
      { &IID_IGUIStyleElement,      _T("Style") },
      { &IID_IGUITextEditElement,   _T("TextEdit") },
      { &IID_IGUITitleBarElement,   _T("TitleBar") },
      { &IID_IGUIContainerElement,  _T("Container") }, // Should be at the bottom
   };

   for (int i = 0; i < _countof(guiElementTypes); i++)
   {
      cAutoIPtr<IUnknown> pUnk;
      if (pUnkElement->QueryInterface(*guiElementTypes[i].pIID, (void**)&pUnk) == S_OK)
      {
         return tGUIString(guiElementTypes[i].pszType);
      }
   }

   return tGUIString(_T("(unknown)"));;
}


///////////////////////////////////////////////////////////////////////////////

bool GUIElementIdMatch(IGUIElement * pElement, const tChar * pszId)
{
   if (pElement != NULL && pszId != NULL)
   {
      tGUIString id;
      if (pElement->GetId(&id) == S_OK && id.compare(pszId) == 0)
      {
         return true;
      }
   }
   return false;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIStyleWidth(IGUIStyle * pStyle, tGUISizeType baseWidth, tGUISizeType * pWidth)
{
   if (pStyle == NULL || pWidth == NULL)
   {
      return E_POINTER;
   }

   int styleWidth;
   uint styleWidthSpec;
   if (pStyle->GetWidth(&styleWidth, &styleWidthSpec) == S_OK && styleWidth >= 0)
   {
      if (styleWidthSpec == kGUIDimensionPixels)
      {
         *pWidth = static_cast<tGUISizeType>(styleWidth);
         return S_OK;
      }
      else if (styleWidthSpec == kGUIDimensionPercent)
      {
         *pWidth = static_cast<tGUISizeType>((styleWidth * baseWidth) / 100);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

tResult GUIStyleHeight(IGUIStyle * pStyle, tGUISizeType baseHeight, tGUISizeType * pHeight)
{
   if (pStyle == NULL || pHeight == NULL)
   {
      return E_POINTER;
   }

   int styleHeight;
   uint styleHeightSpec;
   if (pStyle->GetHeight(&styleHeight, &styleHeightSpec) == S_OK && styleHeight >= 0)
   {
      if (styleHeightSpec == kGUIDimensionPixels)
      {
         *pHeight = static_cast<tGUISizeType>(styleHeight);
         return S_OK;
      }
      else if (styleHeightSpec == kGUIDimensionPercent)
      {
         *pHeight = static_cast<tGUISizeType>((styleHeight * baseHeight) / 100);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

void GUIPlaceElement(const tGUIRect & field, IGUIElement * pGUIElement)
{
   Assert(pGUIElement != NULL);

   tGUISize size = pGUIElement->GetSize();

   tGUIPoint pos(static_cast<float>(field.left), static_cast<float>(field.top));

   cAutoIPtr<IGUIStyle> pStyle;
   if (pGUIElement->GetStyle(&pStyle) == S_OK)
   {
      uint align;
      if (pStyle->GetAlignment(&align) == S_OK)
      {
         if (align == kGUIAlignLeft)
         {
            pos.x = static_cast<float>(field.left);
         }
         else if (align == kGUIAlignRight)
         {
            pos.x = static_cast<float>(field.left + field.GetWidth() - size.width);
         }
         else if (align == kGUIAlignCenter)
         {
            pos.x = static_cast<float>(field.left + ((field.GetWidth() - size.width) / 2));
         }
      }

      uint vertAlign;
      if (pStyle->GetVerticalAlignment(&vertAlign) == S_OK)
      {
         if (vertAlign == kGUIVertAlignTop)
         {
            pos.y = static_cast<float>(field.top);
         }
         else if (vertAlign == kGUIVertAlignBottom)
         {
            pos.y = field.top + field.GetHeight() - size.height;
         }
         else if (vertAlign == kGUIVertAlignCenter)
         {
            pos.y = field.top + ((field.GetHeight() - size.height) / 2);
         }
      }
   }

   pGUIElement->SetPosition(pos);
}

///////////////////////////////////////////////////////////////////////////////

tGUIPoint GUIElementAbsolutePosition(IGUIElement * pGUIElement, uint * pnParents)
{
   Assert(pGUIElement != NULL);

   uint nParents = 0;
   tGUIPoint absolutePosition = pGUIElement->GetPosition();

   cAutoIPtr<IGUIElement> pParent;
   if (pGUIElement->GetParent(&pParent) == S_OK)
   {
      do
      {
         absolutePosition += pParent->GetPosition();
         nParents++;

         cAutoIPtr<IGUIElement> pNext;
         if (pParent->GetParent(&pNext) != S_OK)
         {
            SafeRelease(pParent);
         }
         else
         {
            pParent = pNext;
         }
      }
      while (!!pParent);
   }

   if (pnParents != NULL)
   {
      *pnParents = nParents;
   }

   return absolutePosition;
}


///////////////////////////////////////////////////////////////////////////////

tResult GUIElementFont(IGUIElement * pElement, IRenderFont * * ppFont)
{
   if (pElement == NULL || ppFont == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      if (pStyle->GetFont(ppFont) == S_OK)
      {
         return S_OK;
      }
   }

   UseGlobal(GUIContext);
   return pGUIContext->GetDefaultFont(ppFont);
}


///////////////////////////////////////////////////////////////////////////////

tResult GUISetText(const tGUIChar * pszText, tGUIString * pString)
{
   if (pString == NULL)
   {
      return E_POINTER;
   }

   if (pszText == NULL)
   {
      pString->erase();
   }
   else
   {
      static const struct
      {
         const tGUIChar * pszFind;
         const tGUIChar * pszReplace;
      }
      replacements[] =
      {
         { _T("&copy;"), _T("\xa9") }, // copyright symbol
      };

      pString->assign(pszText);

      for (int i = 0; i < _countof(replacements); ++i)
      {
         tGUIString::size_type j = tGUIString::npos;
         while ((j = pString->find(replacements[i].pszFind)) != tGUIString::npos)
         {
            pString->replace(j, _tcslen(replacements[i].pszFind), replacements[i].pszReplace);
         }
      }
   }

   return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_UNITTESTPP

TEST(GUISetText)
{
   tGUIString temp;
   CHECK(GUISetText(_T("Simple Text"), &temp) == S_OK);
   CHECK(temp.compare(_T("Simple Text")) == 0);
   CHECK(GUISetText("Copyright symbol &copy; test", &temp) == S_OK);
   CHECK(temp.compare(_T("Copyright symbol � test")) == 0);
}

#endif

///////////////////////////////////////////////////////////////////////////////
