///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIEVENTROUTERTEM_H
#define INCLUDED_GUIEVENTROUTERTEM_H

/// @file guieventroutertem.h
/// Contains the member function implementations of the cGUIEventRouter class.
/// This file is intended to be included only by cpp files, not other headers.

#include "guiapi.h"
#include "guielementenum.h"
#include "guielementtools.h"

#include "inputapi.h"

#include "techtime.h"

#include <algorithm>
#include <queue>

#include "dbgalloc.h" // must be last header

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////

class cSameAs
{
public:
   cSameAs(IUnknown * pUnknown) : m_pUnknown(CTAddRef(pUnknown))
   {
   }

   bool operator()(IUnknown * pUnknown2)
   {
      return CTIsSameObject(m_pUnknown, pUnknown2);
   }

private:
   cAutoIPtr<IUnknown> m_pUnknown;
};

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cGUIEventRouter
//

///////////////////////////////////////

template <typename INTRFC>
cGUIEventRouter<INTRFC>::cGUIEventRouter()
{
}

///////////////////////////////////////

template <typename INTRFC>
cGUIEventRouter<INTRFC>::~cGUIEventRouter()
{
   RemoveAllElements();
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::AddEventListener(IGUIEventListener * pListener)
{
   return Connect(pListener);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::RemoveEventListener(IGUIEventListener * pListener)
{
   return Disconnect(pListener);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetFocus(IGUIElement * * ppElement)
{
   Assert(!m_pFocus || m_pFocus->HasFocus());
   return m_pFocus.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetFocus(IGUIElement * pElement)
{
   if (!!m_pFocus)
   {
      m_pFocus->SetFocus(false);
   }
   SafeRelease(m_pFocus);
   m_pFocus = CTAddRef(pElement);
   if (pElement != NULL)
   {
      pElement->SetFocus(true);
   }
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetMouseOver(IGUIElement * * ppElement)
{
   return m_pMouseOver.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetMouseOver(IGUIElement * pElement)
{
   SafeRelease(m_pMouseOver);
   m_pMouseOver = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetDrag(IGUIElement * * ppElement)
{
   return m_pDrag.GetPointer(ppElement);
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::SetDrag(IGUIElement * pElement)
{
   SafeRelease(m_pDrag);
   m_pDrag = CTAddRef(pElement);
   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetElement(const tChar * pszId, IGUIElement * * ppElement)
{
   if (pszId == NULL || ppElement == NULL)
   {
      return E_POINTER;
   }

   // TODO: construct a map to do this
   tGUIElementList::iterator iter;
   for (iter = m_elements.begin(); iter != m_elements.end(); iter++)
   {
      if (GUIElementIdMatch(*iter, pszId))
      {
         *ppElement = CTAddRef(*iter);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::AddElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   if (HasElement(pElement) == S_OK)
   {
      return S_FALSE;
   }

   m_elements.push_back(CTAddRef(pElement));

   cAutoIPtr<IGUIDialogElement> pDialog;
   if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialog) == S_OK)
   {
      m_dialogs.push_back(CTAddRef(pDialog));
   }

   return S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::RemoveElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tResult result = S_FALSE;

   // Remove from element list
   {
      tGUIElementList::iterator f = std::find_if(m_elements.begin(), m_elements.end(), cSameAs(pElement));
      if (f != m_elements.end())
      {
         (*f)->Release();
         m_elements.erase(f);
      }
   }

   // Remove from dialog list too if necessary
   {
      tGUIDialogList::iterator f = std::find_if(m_dialogs.begin(), m_dialogs.end(), cSameAs(pElement));
      if (f != m_dialogs.end())
      {
         (*f)->Release();
         m_dialogs.erase(f);
      }
   }

   return result;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::HasElement(IGUIElement * pElement) const
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList::const_iterator f = std::find_if(m_elements.begin(), m_elements.end(), cSameAs(pElement));
   return (f != m_elements.end()) ? S_OK : S_FALSE;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIEventRouter<INTRFC>::RemoveAllElements()
{
   SafeRelease(m_pFocus);
   SafeRelease(m_pMouseOver);
   SafeRelease(m_pDrag);
   std::for_each(m_elements.begin(), m_elements.end(), CTInterfaceMethod(&IGUIElement::Release));
   m_elements.clear();
   std::for_each(m_dialogs.begin(), m_dialogs.end(), CTInterfaceMethod(&IGUIDialogElement::Release));
   m_dialogs.clear();
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetHitElements(const tGUIPoint & point,
                                                std::list<IGUIElement*> * pElements) const
{
   if (pElements == NULL)
   {
      return E_POINTER;
   }

   std::queue<IGUIElement*> q;

   tGUIElementList::const_iterator iter = m_elements.begin();
   tGUIElementList::const_iterator end = m_elements.end();
   for (; iter != end; iter++)
   {
      q.push(CTAddRef(*iter));
   }

   while (!q.empty())
   {
      cAutoIPtr<IGUIElement> pElement(q.front());
      q.pop();

      tGUIPoint pos(GUIElementAbsolutePosition(pElement));
      tGUIPoint relative(point - pos);

      if (pElement->Contains(relative))
      {
         cAutoIPtr<IGUIContainerElement> pContainer;
         if (pElement->QueryInterface(IID_IGUIContainerElement, (void**)&pContainer) == S_OK)
         {
            cAutoIPtr<IGUIElementEnum> pEnum;
            if (pContainer->GetElements(&pEnum) == S_OK)
            {
               IGUIElement * pChildren[32];
               ulong count = 0;
               while (SUCCEEDED(pEnum->Next(_countof(pChildren), &pChildren[0], &count)) && (count > 0))
               {
                  for (ulong i = 0; i < count; i++)
                  {
                     q.push(pChildren[i]);
                  }
                  count = 0;
               }
            }
         }

         pElements->push_front(CTAddRef(pElement));
      }
   }

   return pElements->empty() ? S_FALSE : S_OK;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetHitElement(const tGUIPoint & point, 
                                               IGUIElement * * ppElement) const
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   std::list<IGUIElement*> hitElements;
   if (GetHitElements(point, &hitElements) == S_OK)
   {
      Assert(!hitElements.empty());
      *ppElement = CTAddRef(hitElements.front());
      std::for_each(hitElements.begin(), hitElements.end(), CTInterfaceMethod(&IGUIElement::Release));
      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////
// Similar to BubbleEvent but doesn't walk up the parent chain

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::DoEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   typename cConnectionPoint<INTRFC, IGUIEventListener>::tSinksIterator iter = BeginSinks();
   typename cConnectionPoint<INTRFC, IGUIEventListener>::tSinksIterator end = EndSinks();
   for (; iter != end; iter++)
   {
      if ((*iter)->OnEvent(pEvent) != S_OK)
      {
         return true;
      }
   }

   cAutoIPtr<IGUIElement> pDispatchTo;
   if (pEvent->GetSourceElement(&pDispatchTo) == S_OK)
   {
      if (!!pDispatchTo)
      {
         if (pDispatchTo->OnEvent(pEvent) != S_OK)
         {
            return true;
         }
      }
   }

   return false;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::BubbleEvent(IGUIEvent * pEvent)
{
   Assert(pEvent != NULL);

   cAutoIPtr<IGUIElement> pEventSrc;
   if (pEvent->GetSourceElement(&pEventSrc) == S_OK)
   {
      return BubbleEvent(pEventSrc, pEvent);
   }

   return false;
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::BubbleEvent(IGUIElement * pStartElement, IGUIEvent * pEvent)
{
   Assert(pStartElement != NULL);
   Assert(pEvent != NULL);

   typename cConnectionPoint<INTRFC, IGUIEventListener>::tSinksIterator iter = BeginSinks();
   typename cConnectionPoint<INTRFC, IGUIEventListener>::tSinksIterator end = EndSinks();
   for (; iter != end; iter++)
   {
      if ((*iter)->OnEvent(pEvent) != S_OK)
      {
         return true;
      }
   }

   cAutoIPtr<IGUIElement> pDispatchTo(CTAddRef(pStartElement));
   while (!!pDispatchTo)
   {
      if (pDispatchTo->OnEvent(pEvent) != S_OK)
      {
         return true;
      }

      if (pEvent->GetCancelBubble() == S_OK)
      {
         break;
      }

      cAutoIPtr<IGUIElement> pNext;
      if (pDispatchTo->GetParent(&pNext) != S_OK)
      {
         SafeRelease(pDispatchTo);
      }
      else
      {
         pDispatchTo = pNext;
      }
   }

   return false;
}

///////////////////////////////////////

template <typename INTRFC>
tResult cGUIEventRouter<INTRFC>::GetActiveModalDialog(IGUIDialogElement * * ppModalDialog)
{
   if (ppModalDialog == NULL)
   {
      return E_POINTER;
   }

   if (m_dialogs.empty())
   {
      return S_FALSE;
   }

   *ppModalDialog = CTAddRef(m_dialogs.back());
   return S_OK;
}

///////////////////////////////////////

inline bool IsDescendant(IGUIContainerElement * pParent, IGUIElement * pElement)
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

///////////////////////////////////////

inline tGUIPoint ScreenToElement(IGUIElement * pGUIElement, const tGUIPoint & point)
{
   return point - GUIElementAbsolutePosition(pGUIElement);
}

///////////////////////////////////////

template <typename INTRFC>
bool cGUIEventRouter<INTRFC>::HandleInputEvent(const sInputEvent * pInputEvent)
{
   tGUIEventCode eventCode = GUIEventCode(pInputEvent->key, pInputEvent->down);
   if (eventCode == kGUIEventNone)
   {
      WarnMsg("Invalid event code\n");
      return false;
   }

   cAutoIPtr<IGUIElement> pMouseOver;
   if (GetHitElement(pInputEvent->point, &pMouseOver) != S_OK)
   {
      Assert(!pMouseOver);
   }

   bool bEatInputEvent = false;

   cAutoIPtr<IGUIElement> pDrag;
   if (GetDrag(&pDrag) == S_OK)
   {
      DoDragDrop(pInputEvent, eventCode, pMouseOver, pDrag);
   }

   cAutoIPtr<IGUIElement> pFocus;
   GetFocus(&pFocus);

   // If a modal dialog is active, restrict input to the dialog and its descendants
   cAutoIPtr<IGUIDialogElement> pModalDialog;
   if (GetActiveModalDialog(&pModalDialog) == S_OK)
   {
      if (KeyIsMouse(pInputEvent->key))
      {
         if (!!pMouseOver && !CTIsSameObject(pModalDialog, pMouseOver) && !IsDescendant(pModalDialog, pMouseOver))
         {
            SafeRelease(pMouseOver);
         }
      }
      else
      {
         if (!!pFocus && !CTIsSameObject(pModalDialog, pFocus) && !IsDescendant(pModalDialog, pFocus))
         {
            SafeRelease(pFocus);
         }

         if (!pFocus)
         {
            pFocus = CTAddRef(pModalDialog);
         }
      }
   }

   if (KeyIsMouse(pInputEvent->key) && !!pMouseOver)
   {
      if (eventCode == kGUIEventMouseDown)
      {
         SetFocus(pMouseOver);

         cAutoIPtr<IGUIEvent> pDragStartEvent;
         if (GUIEventCreate(kGUIEventDragStart, pInputEvent->point, 
            pInputEvent->key, pMouseOver, true, &pDragStartEvent) == S_OK)
         {
            BubbleEvent(pDragStartEvent);
            SetDrag(pMouseOver);
            return true;
         }
      }
      else if (eventCode == kGUIEventMouseMove)
      {
         DoMouseEnterExit(pInputEvent, pMouseOver, NULL);
      }

      cAutoIPtr<IGUIEvent> pEvent;
      if (GUIEventCreate(eventCode, pInputEvent->point, 
         pInputEvent->key, pMouseOver, true, &pEvent) == S_OK)
      {
         return BubbleEvent(pEvent);
      }
   }
   else
   {
      if (!!pFocus)
      {
         cAutoIPtr<IGUIEvent> pEvent;
         if (GUIEventCreate(eventCode, pInputEvent->point, 
            pInputEvent->key, pFocus, true, &pEvent) == S_OK)
         {
            BubbleEvent(pEvent);
         }
      }

      if (!!pModalDialog)
      {
         bEatInputEvent = true;
      }
   }

   return bEatInputEvent;
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIEventRouter<INTRFC>::DoDragDrop(const sInputEvent * pInputEvent, 
                                         tGUIEventCode eventCode, 
                                         IGUIElement * pMouseOver, 
                                         IGUIElement * pDrag)
{
   Assert(pInputEvent != NULL);
   Assert(pDrag != NULL);

   if (eventCode == kGUIEventMouseMove)
   {
      DoMouseEnterExit(pInputEvent, pMouseOver, pDrag);

      // Send drag move to dragging element
      cAutoIPtr<IGUIEvent> pDragMoveEvent;
      if (GUIEventCreate(kGUIEventDragMove, pInputEvent->point, 
         pInputEvent->key, pDrag, true, &pDragMoveEvent) == S_OK)
      {
         if (!BubbleEvent(pDragMoveEvent))
         {
            // Send drag over to moused-over element
            if (!!pMouseOver)
            {
               cAutoIPtr<IGUIEvent> pDragOverEvent;
               if (GUIEventCreate(kGUIEventDragOver, pInputEvent->point, 
                  pInputEvent->key, pMouseOver, true, &pDragOverEvent) == S_OK)
               {
                  BubbleEvent(pDragOverEvent);
               }
            }
         }
      }
   }
   else if (eventCode == kGUIEventMouseUp)
   {
      SetDrag(NULL);

      cAutoIPtr<IGUIEvent> pDragEndEvent;
      if (GUIEventCreate(kGUIEventDragEnd, pInputEvent->point, 
         pInputEvent->key, pDrag, true, &pDragEndEvent) == S_OK)
      {
         BubbleEvent(pDragEndEvent);
      }

      if (!!pMouseOver)
      {
         // If moused-over same as dragging element
         if (CTIsSameObject(pMouseOver, pDrag))
         {
            // Send click to moused-over/dragging element
            // TODO: Doing this here, the click event will occur before the mouse up event
            cAutoIPtr<IGUIEvent> pClickEvent;
            if (GUIEventCreate(kGUIEventClick, pInputEvent->point, 
               pInputEvent->key, pMouseOver, true, &pClickEvent) == S_OK)
            {
               BubbleEvent(pClickEvent);
            }
         }
         else
         {
            // Send drop to moused-over element
            cAutoIPtr<IGUIEvent> pDropEvent;
            if (GUIEventCreate(kGUIEventDrop, pInputEvent->point, 
               pInputEvent->key, pMouseOver, true, &pDropEvent) == S_OK)
            {
               BubbleEvent(pDropEvent);
            }
         }
      }
   }
   else if (eventCode == kGUIEventKeyDown)
   {
      // If key is escape stop dragging
      if (pInputEvent->key == kEscape)
      {
         SetDrag(NULL);
         cAutoIPtr<IGUIEvent> pDragEndEvent;

         if (GUIEventCreate(kGUIEventDragEnd, pInputEvent->point, 
            pInputEvent->key, pDrag, true, &pDragEndEvent) == S_OK)
         {
            BubbleEvent(pDragEndEvent);
         }
      }
   }
}

///////////////////////////////////////

template <typename INTRFC>
void cGUIEventRouter<INTRFC>::DoMouseEnterExit(const sInputEvent * pInputEvent, 
                                               IGUIElement * pMouseOver, 
                                               IGUIElement * pRestrictTo)
{
   if (pMouseOver != NULL)
   {
      bool bMouseOverSame = false;

      cAutoIPtr<IGUIElement> pOldMouseOver;
      if (GetMouseOver(&pOldMouseOver) == S_OK)
      {
         bMouseOverSame = CTIsSameObject(pMouseOver, pOldMouseOver);

         if (!bMouseOverSame)
         {
            SetMouseOver(NULL);
            pOldMouseOver->SetMouseOver(false);

            cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
            if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, 
               pOldMouseOver, true, &pMouseLeaveEvent) == S_OK)
            {
               DoEvent(pMouseLeaveEvent);
            }
         }
      }

      if (!bMouseOverSame 
         && ((pRestrictTo == NULL) || (CTIsSameObject(pMouseOver, pRestrictTo))))
      {
         SetMouseOver(pMouseOver);
         pMouseOver->SetMouseOver(true);

         cAutoIPtr<IGUIEvent> pMouseEnterEvent;
         if (GUIEventCreate(kGUIEventMouseEnter, pInputEvent->point, pInputEvent->key, 
            pMouseOver, true, &pMouseEnterEvent) == S_OK)
         {
            DoEvent(pMouseEnterEvent);
         }
      }
   }
   else
   {
      cAutoIPtr<IGUIElement> pOldMouseOver;
      if (GetMouseOver(&pOldMouseOver) == S_OK)
      {
         Assert(!pOldMouseOver->Contains(ScreenToElement(pOldMouseOver, pInputEvent->point)));

         SetMouseOver(NULL);
         pOldMouseOver->SetMouseOver(false);

         cAutoIPtr<IGUIEvent> pMouseLeaveEvent;
         if (GUIEventCreate(kGUIEventMouseLeave, pInputEvent->point, pInputEvent->key, 
            pOldMouseOver, true, &pMouseLeaveEvent) == S_OK)
         {
            DoEvent(pMouseLeaveEvent);
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

#include "undbgalloc.h"

#endif // !INCLUDED_GUIEVENTROUTERTEM_H
