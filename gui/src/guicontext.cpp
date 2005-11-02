///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "guicontext.h"
#include "guievent.h"
#include "guielementtools.h"
#include "guieventroutertem.h"
#include "guipage.h"

#include "scriptvar.h"
#include "sys.h"
#include "engineapi.h"

#include "keys.h"
#include "resourceapi.h"
#include "readwriteapi.h"
#include "configapi.h"

#include <tinyxml.h>

#include <vector>

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

///////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(GUIContext);

#define LocalMsg(msg)                  DebugMsgEx(GUIContext,(msg))
#define LocalMsg1(msg,a1)              DebugMsgEx1(GUIContext,(msg),(a1))
#define LocalMsg2(msg,a1,a2)           DebugMsgEx2(GUIContext,(msg),(a1),(a2))

#define LocalMsgIf(cond,msg)           DebugMsgIfEx(GUIContext,(cond),(msg))
#define LocalMsgIf1(cond,msg,a1)       DebugMsgIfEx1(GUIContext,(cond),(msg),(a1))
#define LocalMsgIf2(cond,msg,a1,a2)    DebugMsgIfEx2(GUIContext,(cond),(msg),(a1),(a2))

///////////////////////////////////////////////////////////////////////////////

template <typename CONTAINER>
static tResult LoadElements(TiXmlDocument * pTiXmlDoc, CONTAINER * pElements)
{
   if (pTiXmlDoc == NULL || pElements == NULL)
   {
      return E_POINTER;
   }

   UseGlobal(GUIFactory);

   uint nCreated = 0;
   TiXmlElement * pXmlElement;
   for (pXmlElement = pTiXmlDoc->FirstChildElement(); pXmlElement != NULL;
        pXmlElement = pXmlElement->NextSiblingElement())
   {
      if (pXmlElement->Type() == TiXmlNode::ELEMENT)
      {
         cAutoIPtr<IGUIElement> pGuiElement;
         if (pGUIFactory->CreateElement(pXmlElement, &pGuiElement) == S_OK)
         {
            pElements->push_back(CTAddRef(pGuiElement));
            nCreated++;
         }
      }
   }

   return nCreated;
}

///////////////////////////////////////////////////////////////////////////////

template <typename CONTAINER>
static tResult LoadElements(const char * psz, CONTAINER * pElements)
{
   if (psz == NULL || pElements == NULL)
   {
      return E_POINTER;
   }

   TiXmlBase::SetCondenseWhiteSpace(false);

   TiXmlDocument doc;
   doc.Parse(psz);

   int errorId = doc.ErrorId();
   if (errorId != TiXmlBase::TIXML_NO_ERROR && errorId != TiXmlBase::TIXML_ERROR_DOCUMENT_EMPTY)
   {
      ErrorMsg1("TiXml parse error: %s\n", doc.ErrorDesc());
      return E_FAIL;
   }

   uint nCreated = LoadElements(&doc, pElements);
   if (nCreated > 0)
   {
      return S_OK;
   }
   else
   {
      TiXmlDocument * pTiXmlDoc = NULL;
      UseGlobal(ResourceManager);
      if (pResourceManager->Load(psz, kRT_TiXml, NULL, (void**)&pTiXmlDoc) == S_OK)
      {
         nCreated = LoadElements(pTiXmlDoc, pElements);
      }
   }

   return (nCreated > 0) ? S_OK : S_FALSE;
}

///////////////////////////////////////////////////////////////////////////////

static bool g_bExitModalLoop = false;

static bool GUIModalLoopFrameHandler()
{
   if (g_bExitModalLoop)
   {
      return false;
   }
   return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIModalLoopEventListener
//

////////////////////////////////////////

class cGUIModalLoopEventListener : public cComObject<IMPLEMENTS(IGUIEventListener)>
{
public:
   cGUIModalLoopEventListener(tResult * pResult);
   ~cGUIModalLoopEventListener();

   virtual tResult OnEvent(IGUIEvent * pEvent);

private:
   tResult * m_pResult;
};

////////////////////////////////////////

cGUIModalLoopEventListener::cGUIModalLoopEventListener(tResult * pResult)
 : m_pResult(pResult)
{
}

////////////////////////////////////////

cGUIModalLoopEventListener::~cGUIModalLoopEventListener()
{
}

////////////////////////////////////////

tResult cGUIModalLoopEventListener::OnEvent(IGUIEvent * pEvent)
{
   bool bEatEvent = false;

   tGUIEventCode eventCode;
   Verify(pEvent->GetEventCode(&eventCode) == S_OK);

   long keyCode;
   Verify(pEvent->GetKeyCode(&keyCode) == S_OK);

   switch (eventCode)
   {
      case kGUIEventKeyUp:
      {
         if (keyCode == kEnter)
         {
            *m_pResult = S_OK;
            g_bExitModalLoop = true;
            bEatEvent = true;
         }
         break;
      }
      case kGUIEventKeyDown:
      {
         if (keyCode == kEscape)
         {
            *m_pResult = S_FALSE;
            g_bExitModalLoop = true;
            bEatEvent = true;
         }
         break;
      }
      case kGUIEventClick:
      {
         cAutoIPtr<IGUIElement> pSrcElement;
         if (pEvent->GetSourceElement(&pSrcElement) == S_OK)
         {
            cAutoIPtr<IGUIButtonElement> pButtonElement;
            if (pSrcElement->QueryInterface(IID_IGUIButtonElement, (void**)&pButtonElement) == S_OK)
            {
               if (GUIElementIdMatch(pButtonElement, "ok"))
               {
                  *m_pResult = S_OK;
                  g_bExitModalLoop = true;
                  bEatEvent = true;
               }
               else if (GUIElementIdMatch(pButtonElement, "cancel"))
               {
                  *m_pResult = S_FALSE;
                  g_bExitModalLoop = true;
                  bEatEvent = true;
               }
            }
         }
         break;
      }
   }

   return bEatEvent ? S_FALSE : S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIContext
//

///////////////////////////////////////

BEGIN_CONSTRAINTS(cGUIContext)
   AFTER_GUID(IID_IInput)
   AFTER_GUID(IID_IScriptInterpreter)
END_CONSTRAINTS()

///////////////////////////////////////

cGUIContext::cGUIContext()
 : m_inputListener(this)
 , m_bShowingModalDialog(false)
#ifdef GUI_DEBUG
 , m_bShowDebugInfo(false)
 , m_debugInfoPlacement(0,0)
 , m_debugInfoTextColor(tGUIColor::White)
 , m_lastMousePos(0,0)
#endif
{
}

///////////////////////////////////////

cGUIContext::~cGUIContext()
{
   Assert(m_pages.empty());
}

///////////////////////////////////////

tResult cGUIContext::Init()
{
   UseGlobal(Input);
   pInput->SetGUIInputListener(&m_inputListener);
   GUILayoutRegisterBuiltInTypes();
   UseGlobal(ScriptInterpreter);
   pScriptInterpreter->AddNamedItem(GetName(), static_cast<IScriptable*>(this));
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Term()
{
   UseGlobal(Input);
   pInput->SetGUIInputListener(NULL);

   std::list<cGUIPage*>::iterator iter = m_pages.begin();
   for (; iter != m_pages.end(); iter++)
   {
      delete *iter;
   }
   m_pages.clear();

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::Invoke(const char * pszMethodName,
                            int argc, const cScriptVar * argv,
                            int nMaxResults, cScriptVar * pResults)
{
   if (pszMethodName == NULL)
   {
      return E_POINTER;
   }

   typedef tResult (cGUIContext::*tInvokeMethod)(int argc, const cScriptVar * argv,
                                                 int nMaxResults, cScriptVar * pResults);

   static const struct
   {
      const tChar * pszMethodName;
      tInvokeMethod pfnMethod;
   }
   invokeMethods[] =
   {
      { "ShowModalDialog",    &cGUIContext::InvokeShowModalDialog },
      { "PushPage",           &cGUIContext::InvokePushPage },
      { "PopPage",            &cGUIContext::InvokePopPage },
      { "ToggleDebugInfo",    &cGUIContext::InvokeToggleDebugInfo },
      { "GetElement",         &cGUIContext::InvokeGetElement },
   };

   for (int i = 0; i < _countof(invokeMethods); i++)
   {
      if (strcmp(invokeMethods[i].pszMethodName, pszMethodName) == 0)
      {
         return (this->*invokeMethods[i].pfnMethod)(argc, argv, nMaxResults, pResults);
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokeShowModalDialog(int argc, const cScriptVar * argv,
                                           int nMaxResults, cScriptVar * pResults)
{
   Assert(nMaxResults >= 1);

   if (argc != 1 || !argv[0].IsString())
   {
      return E_INVALIDARG;
   }

   tResult result = ShowModalDialog(argv[0]);
   if (result == S_OK)
   {
      *pResults = cScriptVar(true);
      result = 1; // # of return values
   }
   else if (result == S_FALSE)
   {
      *pResults = cScriptVar::Nil;
      result = 1; // # of return values
   }
   return result;
}

///////////////////////////////////////

tResult cGUIContext::InvokePushPage(int argc, const cScriptVar * argv,
                                    int nMaxResults, cScriptVar * pResults)
{
   if (argc == 1 && argv[0].IsString())
   {
      if (PushPage(argv[0]) == S_OK)
      {
         LocalMsg1("Loading GUI definitions from %s\n", argv[0].psz);
         return S_OK;
      }
   }
   else
   {
      return E_INVALIDARG;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokePopPage(int argc, const cScriptVar * argv,
                                   int nMaxResults, cScriptVar * pResults)
{
   if (argc != 0)
   {
      return E_INVALIDARG;
   }

   if (PopPage() == S_OK)
   {
      return S_OK;
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::InvokeToggleDebugInfo(int argc, const cScriptVar * argv,
                                           int nMaxResults, cScriptVar * pResults)
{
   tGUIPoint placement(0,0);
   cAutoIPtr<IGUIStyle> pStyle;

   if (argc == 2 
      && argv[0].IsNumber() 
      && argv[1].IsNumber())
   {
      placement = tGUIPoint(argv[0], argv[1]);
   }
   else if (argc == 3 
      && argv[0].IsNumber() 
      && argv[1].IsNumber()
      && argv[2].IsString())
   {
      placement = tGUIPoint(argv[0], argv[1]);
      if (GUIStyleParse(argv[2], &pStyle) != S_OK)
      {
         SafeRelease(pStyle);
      }
   }
   else
   {
      return E_FAIL;
   }

   if (ShowDebugInfo(placement, pStyle) == S_FALSE)
   {
      HideDebugInfo();
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::InvokeGetElement(int argc, const cScriptVar * argv,
                                      int nMaxResults, cScriptVar * pResults)
{
   if (argc != 1 || !argv[0].IsString())
   {
      return E_INVALIDARG;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      cAutoIPtr<IGUIElement> pElement;
      if (pPage->GetElement(argv[0], &pElement) == S_OK)
      {
         cAutoIPtr<IScriptable> pScriptable;
         if (pElement->QueryInterface(IID_IScriptable, (void**)&pScriptable) == S_OK)
         {
            pResults[0] = pScriptable;
            return 1;
         }
      }
   }

   return E_FAIL;
}

///////////////////////////////////////

tResult cGUIContext::ShowModalDialog(const tChar * pszDialog)
{
   if (pszDialog == NULL)
   {
      return E_POINTER;
   }

   // Only one at a time supported now
   if (m_bShowingModalDialog)
   {
      WarnMsg("Attempt to show multiple confirm dialogs\n");
      return E_FAIL;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage == NULL)
   {
      return E_FAIL;
   }

   tResult result = E_FAIL;

   std::vector<IGUIElement*> elements;
   if (::LoadElements(pszDialog, &elements) == S_OK && elements.size() == 1
      && SUCCEEDED(CheckModalDialog(elements.front())))
   {
      m_bShowingModalDialog = true;

      pPage->AddElement(elements.front());

      cGUIModalLoopEventListener listener(&result);
      AddEventListener(&listener);

      g_bExitModalLoop = false;

      // This function won't return until the modal loop is ended by
      // some user action (Enter, Esc, OK button click, etc.)
      SysEventLoop(GUIModalLoopFrameHandler, NULL);

      RemoveEventListener(&listener);

      ElementRemoved(elements.front());
      pPage->RemoveElement(elements.front());

      m_bShowingModalDialog = false;
   }

   std::for_each(elements.begin(), elements.end(), CTInterfaceMethod(&IGUIElement::Release));

   return result;
}

///////////////////////////////////////

static tResult ExecScriptElement(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIScriptElement> pScriptElement;
   if (pElement->QueryInterface(IID_IGUIScriptElement, (void**)&pScriptElement) == S_OK)
   {
      tGUIString script;
      if (pScriptElement->GetScript(&script) == S_OK)
      {
         UseGlobal(ScriptInterpreter);
         if (pScriptInterpreter->ExecString(script.c_str()) != S_OK)
         {
            WarnMsg("An error occured running script element\n");
         }
      }
   }

   return S_OK;
}

tResult cGUIContext::PushPage(const tChar * pszPage)
{
   if (pszPage == NULL)
   {
      return E_POINTER;
   }

   tGUIElementList elements;
   if (::LoadElements(pszPage, &elements) == S_OK)
   {
      cGUIPage * pPage = new cGUIPage(&elements);
      if (pPage == NULL)
      {
         return E_OUTOFMEMORY;
      }

      m_pages.push_back(pPage);

      std::for_each(elements.begin(), elements.end(), CTInterfaceMethod(&IGUIElement::Release));

      SetFocus(NULL);
      SetMouseOver(NULL);
      SetDrag(NULL);

      std::for_each(pPage->BeginElements(), pPage->EndElements(), ExecScriptElement);

      return S_OK;
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::PopPage()
{
   if (m_pages.empty())
   {
      return E_FAIL;
   }

   cGUIPage * pLastPage = m_pages.back();
   m_pages.pop_back();
   delete pLastPage, pLastPage = NULL;

   SetFocus(NULL);
   SetMouseOver(NULL);
   SetDrag(NULL);

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::RenderGUI()
{
   cAutoIPtr<IGUIRenderDeviceContext> pRenderDeviceContext;
   if (GetRenderDeviceContext(&pRenderDeviceContext) != S_OK)
   {
      return E_FAIL;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      uint w, h;
      if (pRenderDeviceContext->GetViewportSize(&w, &h) == S_OK)
      {
         pPage->UpdateLayout(tGUIRect(0,0,w,h));
      }

      pPage->Render(static_cast<IGUIRenderDevice*>(pRenderDeviceContext));
   }

#ifdef GUI_DEBUG
   RenderDebugInfo();
#endif

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetRenderDeviceContext(IGUIRenderDeviceContext * * ppRenderDeviceContext)
{
   // Wait as long as possible to create the GUI rendering device to
   // ensure there is a GL context.
   if (!m_pRenderDeviceContext)
   {
      if (FAILED(GUIRenderDeviceCreateGL(&m_pRenderDeviceContext)))
      {
         return E_FAIL;
      }
   }

   return m_pRenderDeviceContext.GetPointer(ppRenderDeviceContext);
}

///////////////////////////////////////

tResult cGUIContext::SetRenderDeviceContext(IGUIRenderDeviceContext * pRenderDeviceContext)
{
   SafeRelease(m_pRenderDeviceContext);
   m_pRenderDeviceContext = CTAddRef(pRenderDeviceContext);
   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::GetDefaultFont(IGUIFont * * ppFont)
{
   if (!m_pDefaultFont)
   {
      tChar szTypeFace[32];
      memset(szTypeFace, 0, sizeof(szTypeFace));
      if (ConfigGetString("default_font_win32", szTypeFace, _countof(szTypeFace)) != S_OK)
      {
         ConfigGetString("default_font", szTypeFace, _countof(szTypeFace));
      }

      int pointSize = 10;
      if (ConfigGet("default_font_size_win32", &pointSize) != S_OK)
      {
         ConfigGet("default_font_size", &pointSize);
      }

      int effects = kGFE_None;
      if (ConfigGet("default_font_effects_win32", &effects) != S_OK)
      {
         ConfigGet("default_font_effects", &effects);
      }

      UseGlobal(GUIFontFactory);
      pGUIFontFactory->CreateFont(cGUIFontDesc(szTypeFace, pointSize, effects), &m_pDefaultFont);
   }

   return m_pDefaultFont.GetPointer(ppFont);
}

///////////////////////////////////////

tResult cGUIContext::ShowDebugInfo(const tGUIPoint & placement, IGUIStyle * pStyle)
{
#ifdef GUI_DEBUG
   if (!m_bShowDebugInfo)
   {
      m_debugInfoPlacement = placement;

      if (pStyle != NULL)
      {
         pStyle->GetForegroundColor(&m_debugInfoTextColor);

         cGUIFontDesc debugFontDesc;
         if (pStyle->GetFontDesc(&debugFontDesc) == S_OK)
         {
            SafeRelease(m_pDebugFont);
            UseGlobal(GUIFontFactory);
            pGUIFontFactory->CreateFont(debugFontDesc, &m_pDebugFont);
         }
      }

      m_bShowDebugInfo = true;
      return S_OK;
   }
#endif
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::HideDebugInfo()
{
#ifdef GUI_DEBUG
   if (m_bShowDebugInfo)
   {
      m_bShowDebugInfo = false;
      return S_OK;
   }
#endif
   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::GetHitElement(const tGUIPoint & point, IGUIElement * * ppElement) const
{
   if (ppElement == NULL)
   {
      return E_POINTER;
   }

   const cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      std::list<IGUIElement*> hitElements;
      if (pPage->GetHitElements(point, &hitElements) == S_OK)
      {
         Assert(!hitElements.empty());
         *ppElement = CTAddRef(hitElements.front());
         std::for_each(hitElements.begin(), hitElements.end(), CTInterfaceMethod(&IGUIElement::Release));
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::GetActiveModalDialog(IGUIDialogElement * * ppModalDialog)
{
   if (ppModalDialog == NULL)
   {
      return E_POINTER;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage != NULL)
   {
      return pPage->GetActiveModalDialog(ppModalDialog);
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cGUIContext::CheckModalDialog(IGUIElement * pElement)
{
   if (pElement == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIDialogElement> pDialog;
   if (pElement->QueryInterface(IID_IGUIDialogElement, (void**)&pDialog) != S_OK)
   {
      return E_INVALIDARG;
   }

   // TODO: this won't find buttons inside nested containers, like a panel inside a dialog
   if (CheckChild(pDialog, "ok", IID_IGUIButtonElement) != S_OK
      && CheckChild(pDialog, "cancel", IID_IGUIButtonElement) != S_OK)
   {
      WarnMsg("Dialog box has no \"ok\" nor \"cancel\" button\n");
      return S_FALSE;
   }

   return S_OK;
}

///////////////////////////////////////

tResult cGUIContext::CheckChild(IGUIContainerElement * pContainer, const tChar * pszId, REFGUID iid)
{
   if (pContainer == NULL || pszId == NULL)
   {
      return E_POINTER;
   }

   cAutoIPtr<IGUIElement> pElement;
   if (pContainer->GetElement(pszId, &pElement) != S_OK)
   {
      return E_FAIL;
   }

   cAutoIPtr<IUnknown> pUnknown;
   if (pElement->QueryInterface(iid, (void**)&pUnknown) != S_OK)
   {
      return E_FAIL;
   }

   return S_OK;
}

///////////////////////////////////////

#ifdef GUI_DEBUG
tResult cGUIContext::GetDebugFont(IGUIFont * * ppFont)
{
   if (!!m_pDebugFont)
   {
      return m_pDebugFont.GetPointer(ppFont);
   }
   else
   {
      return GetDefaultFont(ppFont);
   }
}
#endif

///////////////////////////////////////

#ifdef GUI_DEBUG
void cGUIContext::RenderDebugInfo()
{
   if (!m_bShowDebugInfo)
   {
      return;
   }

   cGUIPage * pPage = GetCurrentPage();
   if (pPage == NULL)
   {
      return;
   }

   cAutoIPtr<IGUIFont> pFont;
   if (GetDebugFont(&pFont) == S_OK)
   {
      tGUIRect rect(0,0,0,0);
      pFont->RenderText("Xy", -1, &rect, kRT_CalcRect, m_debugInfoTextColor);

      const int lineHeight = rect.GetHeight();

      rect = tGUIRect(Round(m_debugInfoPlacement.x), Round(m_debugInfoPlacement.y), 0, 0);

      cStr temp;
      temp.Format("Mouse: (%d, %d)", Round(m_lastMousePos.x), Round(m_lastMousePos.y));
      pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
      rect.Offset(0, lineHeight);

      tGUIElementList hitElements;
      if (pPage->GetHitElements(m_lastMousePos, &hitElements) == S_OK)
      {
         tGUIElementList::reverse_iterator iter = hitElements.rbegin();
         for (int index = 0; iter != hitElements.rend(); iter++, index++)
         {
            cStr type, temp;
            if (SUCCEEDED(GUIElementType(*iter, &type)))
            {
               temp.Format("Element %d: %s", index, type.c_str());
            }
            else
            {
               temp.Format("Element %d: <unknown type>", index);
            }
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            rect.left += lineHeight;

            const tGUISize size((*iter)->GetSize());
            temp.Format("Size: %d x %d", Round(size.width), Round(size.height));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            const tGUIPoint pos((*iter)->GetPosition());
            temp.Format("Position: (%d, %d)", Round(pos.x), Round(pos.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            uint nParents = 0;
            tGUIPoint absPos(GUIElementAbsolutePosition(*iter, &nParents));
            temp.Format("Absolute Position: (%d, %d)", Round(absPos.x), Round(absPos.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            temp.Format("# Parents: %d", nParents);
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            const tGUIPoint relPoint(m_lastMousePos - absPos);
            Assert((*iter)->Contains(relPoint));
            temp.Format("Mouse (relative): (%d, %d)", Round(relPoint.x), Round(relPoint.y));
            pFont->RenderText(temp.c_str(), temp.length(), &rect, kRT_NoClip, m_debugInfoTextColor);
            rect.Offset(0, lineHeight);

            rect.left -= lineHeight;
         }

         std::for_each(hitElements.begin(), hitElements.end(), CTInterfaceMethod(&IGUIElement::Release));
      }
   }
}
#endif

///////////////////////////////////////

#ifdef GUI_DEBUG
bool cGUIContext::HandleInputEvent(const sInputEvent * pEvent)
{
   Assert(pEvent != NULL);

   if (pEvent->key == kMouseMove)
   {
      m_lastMousePos = pEvent->point;
   }

   return cGUIEventRouter<cGUIContext, IGUIContext>::HandleInputEvent(pEvent);
}
#endif

///////////////////////////////////////

cGUIContext::cInputListener::cInputListener(cGUIContext * pOuter)
 : m_pOuter(pOuter)
{
}

///////////////////////////////////////

bool cGUIContext::cInputListener::OnInputEvent(const sInputEvent * pEvent)
{
   Assert(m_pOuter != NULL);
   return m_pOuter->HandleInputEvent(pEvent);
}

///////////////////////////////////////

tResult GUIContextCreate()
{
   cAutoIPtr<IGUIContext> p(static_cast<IGUIContext*>(new cGUIContext));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IGUIContext, p);
}

///////////////////////////////////////////////////////////////////////////////
