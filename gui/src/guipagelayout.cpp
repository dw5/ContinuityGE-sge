///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guipagelayout.h"
#include "guielementtools.h"
#include "guistyleapi.h"

#include "dbgalloc.h" // must be last header


///////////////////////////////////////////////////////////////////////////////

static tResult GUIElementSize(IGUIElement * pElement, IGUIElementRenderer * pRenderer,
                              const tGUISize & baseSize, tGUISize * pSize)
{
   tGUISize preferredSize;
   bool bHavePreferredSize = (pRenderer->GetPreferredSize(pElement, &preferredSize) == S_OK);

   cAutoIPtr<IGUIStyle> pStyle;
   if (pElement->GetStyle(&pStyle) == S_OK)
   {
      tGUISize styleSize(0,0);

      if (GUIStyleWidth(pStyle, baseSize.width, &styleSize.width) != S_OK)
      {
         if (bHavePreferredSize)
         {
            styleSize.width = preferredSize.width;
         }
         else
         {
            return S_FALSE;
         }
      }

      if (GUIStyleHeight(pStyle, baseSize.height, &styleSize.height) != S_OK)
      {
         if (bHavePreferredSize)
         {
            styleSize.height = preferredSize.height;
         }
         else
         {
            return S_FALSE;
         }
      }

      *pSize = styleSize;
      return S_OK;
   }
   else if (bHavePreferredSize)
   {
      *pSize = preferredSize;
      return S_OK;
   }

   return S_FALSE;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPageLayoutFlow
//

///////////////////////////////////////

cGUIPageLayoutFlow::cGUIPageLayoutFlow()
{
}

///////////////////////////////////////

cGUIPageLayoutFlow::cGUIPageLayoutFlow(const tGUIRect & rect)
 : m_rect(rect)
 , m_pos(static_cast<float>(rect.left), static_cast<float>(rect.top))
 , m_rowHeight(0)
{
}

///////////////////////////////////////

cGUIPageLayoutFlow::cGUIPageLayoutFlow(const cGUIPageLayoutFlow & other)
 : m_rect(other.m_rect)
 , m_pos(other.m_pos)
 , m_rowHeight(other.m_rowHeight)
{
}

///////////////////////////////////////

cGUIPageLayoutFlow::~cGUIPageLayoutFlow()
{
}

///////////////////////////////////////

const cGUIPageLayoutFlow & cGUIPageLayoutFlow::operator =(const cGUIPageLayoutFlow & other)
{
   m_rect = other.m_rect;
   m_pos = other.m_pos;
   m_rowHeight = other.m_rowHeight;
   return *this;
}

///////////////////////////////////////

void cGUIPageLayoutFlow::PlaceElement(IGUIElement * pElement)
{
   if (pElement != NULL)
   {
      pElement->SetPosition(m_pos);
      tGUISize elementSize(pElement->GetSize());
      m_pos.x += elementSize.width;
      if (elementSize.height > m_rowHeight)
      {
         m_rowHeight = elementSize.height;
      }
      if (m_pos.x >= m_rect.right)
      {
         m_pos.x = static_cast<float>(m_rect.left);
         m_pos.y += m_rowHeight;
         m_rowHeight = 0;
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIPageLayout
//

///////////////////////////////////////

cGUIPageLayout::cGUIPageLayout(const tGUIRect & rect)
 : m_topLevelRect(rect)
 , m_topLevelSize(static_cast<tGUISizeType>(rect.GetWidth()), static_cast<tGUISizeType>(rect.GetHeight()))
 , m_topLevelFlow(rect)
{
}

///////////////////////////////////////

cGUIPageLayout::cGUIPageLayout(const cGUIPageLayout & other)
 : m_topLevelRect(other.m_topLevelRect)
 , m_topLevelSize(other.m_topLevelSize)
 , m_topLevelFlow(other.m_topLevelFlow)
{
}

///////////////////////////////////////

cGUIPageLayout::~cGUIPageLayout()
{
   while (!m_layoutQueue.empty())
   {
      cAutoIPtr<IGUIContainerElement> pContainer(m_layoutQueue.front());
      m_layoutQueue.pop();

      cAutoIPtr<IGUILayoutManager> pLayout;
      if (pContainer->GetLayout(&pLayout) == S_OK)
      {
         pLayout->Layout(pContainer);
      }
   }
}

///////////////////////////////////////

tResult cGUIPageLayout::operator ()(IGUIElement * pElement, IGUIElementRenderer * pRenderer, void *)
{
   Assert(pElement != NULL);
   Assert(pRenderer != NULL);

   tGUISize parentSize(0,0);
   cAutoIPtr<IGUIElement> pParent;
   if (pElement->GetParent(&pParent) == S_OK)
   {
      parentSize = pParent->GetSize();
   }
   else
   {
      parentSize = m_topLevelSize;
   }

   tGUISize elementSize;
   if (GUIElementSize(pElement, pRenderer, parentSize, &elementSize) != S_OK)
   {
      return S_FALSE;
   }

   if (AlmostEqual(elementSize.width, 0) || AlmostEqual(elementSize.height, 0))
   {
      return S_FALSE;
   }

   pElement->SetSize(elementSize);

   tGUIRect clientArea(0,0,0,0);
   pRenderer->ComputeClientArea(pElement, &clientArea);

   pElement->SetClientArea(clientArea);

   if (!pParent)
   {
      m_topLevelFlow.PlaceElement(pElement);
   }

   cAutoIPtr<IGUIContainerElement> pContainer;
   if (pElement->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
   {
      m_layoutQueue.push(CTAddRef(pContainer));
   }

   return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
