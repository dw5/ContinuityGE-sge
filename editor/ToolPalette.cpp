/////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "ToolPalette.h"
#include "BitmapUtils.h"
#include "DynamicLink.h"

#ifdef HAVE_CPPUNIT
#include <cppunit/extensions/HelperMacros.h>
#endif

#include "dbgalloc.h" // must be last header

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////

static const int kTextGap = 2;
static const int kImageGap = 1;
static const int kCheckedItemTextOffset = 2;

///////////////////////////////////////////////////////////////////////////////

static cDynamicLink g_msimg32("msimg32");

typedef BOOL (WINAPI * tGradientFillFn)(HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);

BOOL WINAPI DynamicGradientFill(HDC hdc,
                                PTRIVERTEX pVertex,
                                ULONG dwNumVertex,
                                PVOID pMesh,
                                ULONG dwNumMesh,
                                ULONG dwMode)
{
   static bool bTriedAndFailed = false;
   static tGradientFillFn pfnGradientFill = NULL;

   if (pfnGradientFill == NULL && !bTriedAndFailed)
   {
      pfnGradientFill = reinterpret_cast<tGradientFillFn>(g_msimg32.GetProcAddress("GradientFill"));
      if (pfnGradientFill == NULL)
      {
         bTriedAndFailed = true;
      }
   }

   if (pfnGradientFill != NULL)
   {
      return (*pfnGradientFill)(hdc, pVertex, dwNumVertex, pMesh, dwNumMesh, dwMode);
   }
   else
   {
      return FALSE;
   }
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolItem
//

////////////////////////////////////////

cToolItem::cToolItem(cToolGroup * pGroup, const tChar * pszName, int iImage, void * pUserData)
 : m_pGroup(pGroup),
   m_name(pszName != NULL ? pszName : ""),
   m_state(kTPTS_None),
   m_iImage(iImage),
   m_pUserData(pUserData)
{
}

////////////////////////////////////////

cToolItem::~cToolItem()
{
}

////////////////////////////////////////

void cToolItem::SetState(uint mask, uint state)
{
   m_state = (m_state & ~mask) | (state & mask);
}

////////////////////////////////////////

void cToolItem::ToggleChecked()
{
   SetState(kTPTS_Checked, IsChecked() ? 0 : kTPTS_Checked);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolGroup
//

////////////////////////////////////////

cToolGroup::cToolGroup(const tChar * pszName, HIMAGELIST hImageList)
 : m_name(pszName != NULL ? pszName : ""),
   m_hImageList(hImageList),
   m_bCollapsed(false)
{
}

////////////////////////////////////////

cToolGroup::~cToolGroup()
{
   if (m_hImageList != NULL)
   {
      ImageList_Destroy(m_hImageList);
      m_hImageList = NULL;
   }

   Clear();
}

////////////////////////////////////////

HTOOLITEM cToolGroup::AddTool(const tChar * pszTool, int iImage, void * pUserData)
{
   if (pszTool != NULL)
   {
      HTOOLITEM hTool = FindTool(pszTool);
      if (hTool != NULL)
      {
         return hTool;
      }

      cToolItem * pTool = new cToolItem(this, pszTool, iImage, pUserData);
      if (pTool != NULL)
      {
         m_tools.push_back(pTool);
         return reinterpret_cast<HTOOLITEM>(pTool);
      }
   }

   return NULL;
}

////////////////////////////////////////

bool cToolGroup::RemoveTool(HTOOLITEM hTool)
{
   if (hTool != NULL)
   {
      cToolItem * pRmTool = reinterpret_cast<cToolItem *>(hTool);
      tTools::iterator iter = m_tools.begin();
      tTools::iterator end = m_tools.end();
      for (; iter != end; iter++)
      {
         if (*iter == pRmTool)
         {
            delete pRmTool;
            m_tools.erase(iter);
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

HTOOLITEM cToolGroup::FindTool(const tChar * pszTool)
{
   if (pszTool != NULL)
   {
      tTools::iterator iter = m_tools.begin();
      tTools::iterator end = m_tools.end();
      for (; iter != end; iter++)
      {
         if (lstrcmp((*iter)->GetName(), pszTool) == 0)
         {
            return reinterpret_cast<HTOOLITEM>(*iter);
         }
      }
   }
   return NULL;
}

////////////////////////////////////////

bool cToolGroup::IsTool(HTOOLITEM hTool) const
{
   if (hTool != NULL)
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      tTools::const_iterator iter = m_tools.begin();
      tTools::const_iterator end = m_tools.end();
      for (; iter != end; iter++)
      {
         if (*iter == pTool)
         {
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

void cToolGroup::Clear()
{
   tTools::iterator iter = m_tools.begin();
   tTools::iterator end = m_tools.end();
   for (; iter != end; iter++)
   {
      delete *iter;
   }
   m_tools.clear();
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPaletteRenderer
//

////////////////////////////////////////

cToolPaletteRenderer::cToolPaletteRenderer()
 : m_bHaveMousePos(false),
   m_totalHeight(0)
{
}

////////////////////////////////////////

cToolPaletteRenderer::cToolPaletteRenderer(const cToolPaletteRenderer & other)
 : m_dc(other.m_dc),
   m_rect(other.m_rect),
   m_mousePos(other.m_mousePos),
   m_bHaveMousePos(other.m_bHaveMousePos),
   m_totalHeight(other.m_totalHeight)
{
}

////////////////////////////////////////

cToolPaletteRenderer::~cToolPaletteRenderer()
{
}

////////////////////////////////////////

const cToolPaletteRenderer & cToolPaletteRenderer::operator =(const cToolPaletteRenderer & other)
{
   m_dc = other.m_dc;
   m_rect = other.m_rect;
   m_mousePos = other.m_mousePos;
   m_bHaveMousePos = other.m_bHaveMousePos;
   m_totalHeight = other.m_totalHeight;
   return *this;
}

////////////////////////////////////////

bool cToolPaletteRenderer::Begin(HDC hDC, LPCRECT pRect, const POINT * pMousePos)
{
   if (m_checkedItemBrush.IsNull())
   {
      static const WORD patternBits[] =
      {
         0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,
      };

      HBITMAP hPatternBm = CreateBitmap(8, 8, 1, 1, patternBits);
      if (hPatternBm != NULL)
      {
         m_checkedItemBrush.CreatePatternBrush(hPatternBm);
         DeleteObject(hPatternBm);
      }
   }

   if (m_dc.IsNull() && (hDC != NULL) && (pRect != NULL))
   {
      m_dc = hDC;
      m_rect = pRect;
      if (pMousePos != NULL)
      {
         m_mousePos = *pMousePos;
         m_bHaveMousePos = true;
      }
      else
      {
         m_bHaveMousePos = false;
      }
      m_totalHeight = 0;
      return true;
   }

   return false;
}

////////////////////////////////////////

bool cToolPaletteRenderer::End()
{
   if (!m_dc.IsNull())
   {
      m_dc = NULL;
      m_bHaveMousePos = false;
      return true;
   }
   return false;
}

////////////////////////////////////////

HANDLE cToolPaletteRenderer::GetHitItem(const CPoint & point, RECT * pRect)
{
   tCachedRects::const_iterator iter = m_cachedRects.begin();
   tCachedRects::const_iterator end = m_cachedRects.end();
   for (; iter != end; iter++)
   {
      if (iter->second.PtInRect(point))
      {
         if (pRect != NULL)
         {
            *pRect = iter->second;
         }
         return iter->first;
      }
   }
   return NULL;
}

////////////////////////////////////////

bool cToolPaletteRenderer::GetItemRect(HANDLE hItem, RECT * pRect)
{
   tCachedRects::const_iterator iter = m_cachedRects.find(hItem);
   if (iter != m_cachedRects.end())
   {
      if (pRect != NULL)
      {
         *pRect = iter->second;
      }
      return true;
   }
   return false;
}

////////////////////////////////////////

void cToolPaletteRenderer::Render(tGroups::const_iterator from,
                                  tGroups::const_iterator to)
{
   if (m_dc.IsNull())
   {
      return;
   }

   // Not using std::for_each() here because the functor is passed by
   // value which instantiates a temporary object and the renderer class
   // is a little too heavy to copy on every paint.
   tGroups::const_iterator iter = from;
   for (; iter != to; iter++)
   {
      Render(*iter);
   }
}

////////////////////////////////////////

void cToolPaletteRenderer::Render(const cToolGroup * pGroup)
{
   if (m_dc.IsNull() || (pGroup == NULL))
   {
      return;
   }

   CRect r(m_rect);
   r.top += m_totalHeight;

   int headingHeight = RenderGroupHeading(m_dc, &r, pGroup);

   if (headingHeight > 0)
   {
      m_cachedRects[reinterpret_cast<HANDLE>(const_cast<cToolGroup *>(pGroup))] = r;
   }

   if (pGroup->IsCollapsed())
   {
      m_totalHeight += r.Height();
      return;
   }

   HIMAGELIST hNormalImages = pGroup->GetImageList();

   CRect toolRect(m_rect);
   toolRect.top += m_totalHeight + headingHeight;

   uint nTools = pGroup->GetToolCount();
   for (uint i = 0; i < nTools; i++)
   {
      cToolItem * pTool = pGroup->GetTool(i);
      if (pTool != NULL)
      {
         CSize extent;
         m_dc.GetTextExtent(pTool->GetName(), -1, &extent);

         extent.cy += (2 * kTextGap);

         toolRect.bottom = toolRect.top + extent.cy;

         int iImage = pTool->GetImageIndex();

         CRect textRect(toolRect);
         CPoint imageOffset(0,0);

         if ((hNormalImages != NULL) && (iImage > -1))
         {
            IMAGEINFO imageInfo;
            if (ImageList_GetImageInfo(hNormalImages, iImage, &imageInfo))
            {
               int imageHeight = (imageInfo.rcImage.bottom - imageInfo.rcImage.top);
               int imageWidth = (imageInfo.rcImage.right - imageInfo.rcImage.left);
               if ((imageHeight + (2 * kImageGap)) > toolRect.Height())
               {
                  toolRect.bottom = toolRect.top + imageHeight + (2 * kImageGap);
               }
               imageOffset.x = kImageGap;
               imageOffset.y = ((toolRect.Height() - imageHeight) / 2);
               textRect.left = toolRect.left + imageOffset.x + imageWidth;
            }
            else
            {
               // ImageList_GetImageInfo() failed so don't draw the image
               iImage = -1;
            }
         }

         m_cachedRects[reinterpret_cast<HANDLE>(pTool)] = toolRect;

         textRect.left += kTextGap;

         // All size/position calculation done, now draw

         m_dc.FillSolidRect(toolRect, GetSysColor(COLOR_3DFACE));

         if (!pTool->IsDisabled())
         {
            if (pTool->IsChecked())
            {
               m_dc.Draw3dRect(toolRect, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));

               textRect.left += kCheckedItemTextOffset;
               textRect.top += kCheckedItemTextOffset;

               imageOffset.x += kCheckedItemTextOffset;
               imageOffset.y += kCheckedItemTextOffset;

               CRect tr2(toolRect);
               tr2.DeflateRect(2, 2);

               if (!m_checkedItemBrush.IsNull())
               {
                  COLORREF oldTextColor = m_dc.SetTextColor(GetSysColor(COLOR_3DHILIGHT));
                  COLORREF oldBkColor = m_dc.SetBkColor(GetSysColor(COLOR_3DFACE));
                  m_dc.FillRect(tr2, m_checkedItemBrush);
                  m_dc.SetTextColor(oldTextColor);
                  m_dc.SetBkColor(oldBkColor);
               }
            }
            else if (m_bHaveMousePos && toolRect.PtInRect(m_mousePos))
            {
               m_dc.Draw3dRect(toolRect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));
            }
         }

         if ((hNormalImages != NULL) && (iImage > -1))
         {
            // TODO: clip the image in case it is offset by being a checked item
            BOOL (STDCALL *pfnILDraw)(HIMAGELIST, int, HDC, int, int, uint) = pTool->IsDisabled()
               ? ImageList_DrawDisabled
               : ImageList_Draw;

            (*pfnILDraw)(hNormalImages, iImage, m_dc,
               toolRect.left + imageOffset.x,
               toolRect.top + imageOffset.y,
               ILD_NORMAL);
         }

         COLORREF oldTextColor = m_dc.SetTextColor(pTool->IsDisabled()
            ? GetSysColor(COLOR_GRAYTEXT)
            : GetSysColor(COLOR_WINDOWTEXT));
         int oldBkMode = m_dc.SetBkMode(TRANSPARENT);
         m_dc.DrawText(pTool->GetName(), -1, &textRect, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);
         m_dc.SetTextColor(oldTextColor);
         m_dc.SetBkMode(oldBkMode);

         toolRect.OffsetRect(0, toolRect.Height());
      }
   }

   m_totalHeight = toolRect.top;

   if (m_totalHeight < m_rect.Height())
   {
      m_dc.FillSolidRect(m_rect.left, m_totalHeight, m_rect.right, m_rect.bottom, GetSysColor(COLOR_3DFACE));
   }
}

////////////////////////////////////////

int cToolPaletteRenderer::RenderGroupHeading(WTL::CDCHandle dc, LPRECT pRect,
                                             const cToolGroup * pGroup)
{
   if ((pGroup == NULL) || (lstrlen(pGroup->GetName()) == 0))
   {
      return 0;
   }

   Assert(!dc.IsNull());
   Assert(pRect != NULL);

   CSize extent;
   dc.GetTextExtent(pGroup->GetName(), -1, &extent);

   extent.cy += (2 * kTextGap);

   pRect->bottom = pRect->top + extent.cy;

   dc.Draw3dRect(pRect, GetSysColor(COLOR_3DHILIGHT), GetSysColor(COLOR_3DSHADOW));

   CRect textRect(*pRect);
   textRect.DeflateRect(1, 1);

   int halfWidth = textRect.Width() / 2;

   COLORREF leftColor = GetSysColor(COLOR_3DSHADOW);
   COLORREF rightColor = GetSysColor(COLOR_3DFACE);

   TRIVERTEX gv[2];
   gv[0].x = textRect.left;
   gv[0].y = textRect.top;
   gv[0].Red = GetRValue16(leftColor);
   gv[0].Green = GetGValue16(leftColor);
   gv[0].Blue = GetBValue16(leftColor);
   gv[0].Alpha = 0;
   gv[1].x = textRect.left + halfWidth;
   gv[1].y = textRect.bottom; 
   gv[1].Red = GetRValue16(rightColor);
   gv[1].Green = GetGValue16(rightColor);
   gv[1].Blue = GetBValue16(rightColor);
   gv[1].Alpha = 0;

   GRADIENT_RECT gr;
   gr.UpperLeft = 0;
   gr.LowerRight = 1;
   if (DynamicGradientFill(dc, gv, _countof(gv), &gr, 1, GRADIENT_FILL_RECT_H))
   {
      dc.FillSolidRect(textRect.left + halfWidth, textRect.top, halfWidth, textRect.Height(), rightColor);
   }
   else
   {
      dc.FillSolidRect(textRect.left, textRect.top, textRect.Width(), textRect.Height(), rightColor);
   }

   textRect.left += kTextGap;

   dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
   dc.SetBkMode(TRANSPARENT);
   dc.DrawText(pGroup->GetName(), -1, &textRect, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

   return extent.cy;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cToolPalette
//

////////////////////////////////////////

cToolPalette::cToolPalette()
 : m_bUpdateScrollInfo(false),
   m_hMouseOverItem(NULL),
   m_hClickCandidateItem(NULL)
{
}

////////////////////////////////////////

cToolPalette::~cToolPalette()
{
}

////////////////////////////////////////

LRESULT cToolPalette::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   return 0;
}

////////////////////////////////////////

void cToolPalette::OnDestroy()
{
   Clear();
   SetFont(NULL, FALSE);
   m_renderer.FlushCachedRects();
}

////////////////////////////////////////

void cToolPalette::OnSize(UINT nType, CSize size)
{
   m_bUpdateScrollInfo = true;
   m_renderer.FlushCachedRects();
}

////////////////////////////////////////

void cToolPalette::OnSetFont(HFONT hFont, BOOL bRedraw)
{
   if (!m_font.IsNull())
   {
      Verify(m_font.DeleteObject());
   }

   if (hFont != NULL)
   {
      LOGFONT logFont = {0};
      if (GetObject(hFont, sizeof(LOGFONT), &logFont))
      {
         m_font.CreateFontIndirect(&logFont);
      }
   }

   if (bRedraw)
   {
      Invalidate();
      UpdateWindow();
   }
}

////////////////////////////////////////

LRESULT cToolPalette::OnEraseBkgnd(WTL::CDCHandle dc)
{
   return TRUE;
}

////////////////////////////////////////

void cToolPalette::OnMouseLeave()
{
   SetMouseOverItem(NULL);
}

////////////////////////////////////////

void cToolPalette::OnMouseMove(UINT flags, CPoint point)
{
   SetMouseOverItem(m_renderer.GetHitItem(point));
}

////////////////////////////////////////

void cToolPalette::OnLButtonDown(UINT flags, CPoint point)
{
   Assert(m_hClickCandidateItem == NULL);
   m_hClickCandidateItem = m_renderer.GetHitItem(point);
   if (m_hClickCandidateItem != NULL)
   {
      SetCapture();
   }
}

////////////////////////////////////////

void cToolPalette::OnLButtonUp(UINT flags, CPoint point)
{
   if (m_hClickCandidateItem != NULL)
   {
      ReleaseCapture();
      if (m_hClickCandidateItem == m_renderer.GetHitItem(point))
      {
         DoClick(m_hClickCandidateItem, point);
      }
      m_hClickCandidateItem = NULL;
   }
}

////////////////////////////////////////

bool cToolPalette::GetMousePos(LPPOINT pMousePos) const
{
   if (pMousePos != NULL)
   {
      if (GetCursorPos(pMousePos) && ::MapWindowPoints(NULL, m_hWnd, pMousePos, 1))
      {
         return true;
      }
   }
   return false;
}

////////////////////////////////////////

void cToolPalette::DoPaint(WTL::CDCHandle dc)
{
   CRect rect;
   GetClientRect(rect);

   const POINT * pMousePos = NULL;
   CPoint mousePos;
   if (GetMousePos(&mousePos))
   {
      pMousePos = &mousePos;
   }

   HFONT hOldFont = dc.SelectFont(!m_font.IsNull() ? m_font : WTL::AtlGetDefaultGuiFont());

   Verify(m_renderer.Begin(dc, rect, pMousePos));
   m_renderer.Render(m_groups.begin(), m_groups.end());
   Verify(m_renderer.End());

   dc.SelectFont(hOldFont);

   if (m_bUpdateScrollInfo)
   {
      m_bUpdateScrollInfo = false;
      SetScrollSize(1, m_renderer.GetTotalHeight());
   }
}

////////////////////////////////////////

bool cToolPalette::ExclusiveCheck() const
{
   return (GetStyle() & kTPS_ExclusiveCheck) == kTPS_ExclusiveCheck;
}

////////////////////////////////////////

HTOOLGROUP cToolPalette::AddGroup(const tChar * pszGroup, HIMAGELIST hImageList)
{
   if (pszGroup != NULL)
   {
      HTOOLGROUP hGroup = FindGroup(pszGroup);
      if (hGroup != NULL)
      {
         return hGroup;
      }

      cToolGroup * pGroup = new cToolGroup(pszGroup, hImageList);
      if (pGroup != NULL)
      {
         m_bUpdateScrollInfo = true;
         m_groups.push_back(pGroup);
         return reinterpret_cast<HTOOLGROUP>(pGroup);
      }
   }

   return NULL;
}

////////////////////////////////////////

bool cToolPalette::RemoveGroup(HTOOLGROUP hGroup)
{
   if (hGroup != NULL)
   {
      cToolGroup * pRmGroup = reinterpret_cast<cToolGroup *>(hGroup);
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (*iter == pRmGroup)
         {
            uint nTools = pRmGroup->GetToolCount();
            for (uint i = 0; i < nTools; i++)
            {
               DoNotify(pRmGroup->GetTool(i), kTPN_ItemDestroy);
            }
            delete pRmGroup;
            m_groups.erase(iter);
            m_renderer.FlushCachedRects();
            m_bUpdateScrollInfo = true;
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

uint cToolPalette::GetGroupCount() const
{
   return m_groups.size();
}

////////////////////////////////////////

HTOOLGROUP cToolPalette::FindGroup(const tChar * pszGroup)
{
   if (pszGroup != NULL)
   {
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (lstrcmp((*iter)->GetName(), pszGroup) == 0)
         {
            return reinterpret_cast<HTOOLGROUP>(*iter);
         }
      }
   }
   return NULL;
}

////////////////////////////////////////

bool cToolPalette::IsGroup(HTOOLGROUP hGroup)
{
   if (hGroup != NULL)
   {
      cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if (*iter == pGroup)
         {
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

bool cToolPalette::IsTool(HTOOLITEM hTool)
{
   if (hTool != NULL)
   {
      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         if ((*iter)->IsTool(hTool))
         {
            return true;
         }
      }
   }
   return false;
}

////////////////////////////////////////

void cToolPalette::Clear()
{
   // Call RemoveGroup() for each so that all the item destroy notifications happen
   while (!m_groups.empty())
   {
      RemoveGroup(reinterpret_cast<HTOOLGROUP>(m_groups.front()));
   }

   // Still important to let the STL container do any internal cleanup
   m_groups.clear();

   m_bUpdateScrollInfo = true;
}

////////////////////////////////////////

HTOOLITEM cToolPalette::AddTool(HTOOLGROUP hGroup, const tChar * pszTool, int iImage, void * pUserData)
{
   sToolPaletteItem tpi = {0};
   lstrcpyn(tpi.szName, pszTool, _countof(tpi.szName));
   tpi.iImage = iImage;
   tpi.pUserData = pUserData;
   return AddTool(hGroup, &tpi);
}

////////////////////////////////////////

HTOOLITEM cToolPalette::AddTool(HTOOLGROUP hGroup, const sToolPaletteItem * pTPI)
{
   if (IsGroup(hGroup) && (pTPI != NULL))
   {
      cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
      HTOOLITEM hItem = pGroup->AddTool(pTPI->szName, pTPI->iImage, pTPI->pUserData);
      if (hItem != NULL)
      {
         m_bUpdateScrollInfo = true;
         Invalidate();
         return hItem;
      }
   }

   return NULL;
}

////////////////////////////////////////

bool cToolPalette::GetToolText(HTOOLITEM hTool, std::string * pText)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      if (pText != NULL)
      {
         *pText = pTool->GetName();
      }

      return true;
   }

   return false;
}

////////////////////////////////////////

bool cToolPalette::GetTool(HTOOLITEM hTool, sToolPaletteItem * pTPI)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      if (pTPI != NULL)
      {
         lstrcpyn(pTPI->szName, pTool->GetName(), _countof(pTPI->szName));
         pTPI->iImage = pTool->GetImageIndex();
         pTPI->state = 0;
         pTPI->pUserData = pTool->GetUserData();
         return true;
      }
   }

   return false;
}

////////////////////////////////////////

bool cToolPalette::RemoveTool(HTOOLITEM hTool)
{
   // TODO
   WarnMsg("Unsupported function called: cToolPalette::RemoveTool\n");
   return false;
}

////////////////////////////////////////

bool cToolPalette::EnableTool(HTOOLITEM hTool, bool bEnable)
{
   if (IsTool(hTool))
   {
      cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
      Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));

      pTool->SetState(kTPTS_Disabled, bEnable ? 0 : kTPTS_Disabled);

      return true;
   }

   return false;
}

////////////////////////////////////////

void cToolPalette::SetMouseOverItem(HANDLE hItem)
{
   if (hItem != m_hMouseOverItem)
   {
      // remove this if-statement if group headings ever have a mouse-over effect
      if (!IsGroup(reinterpret_cast<HTOOLGROUP>(m_hMouseOverItem)))
      {
         CRect oldItemRect;
         if ((m_hMouseOverItem != NULL) && m_renderer.GetItemRect(m_hMouseOverItem, &oldItemRect))
         {
            InvalidateRect(oldItemRect, TRUE);
         }
      }

      m_hMouseOverItem = hItem;

      // remove this if-statement if group headings ever have a mouse-over effect
      if (!IsGroup(reinterpret_cast<HTOOLGROUP>(hItem)))
      {
         CRect itemRect;
         if ((hItem != NULL) && m_renderer.GetItemRect(hItem, &itemRect))
         {
            InvalidateRect(itemRect, FALSE);
         }
      }
   }
}

////////////////////////////////////////

void cToolPalette::GetCheckedItems(std::vector<HTOOLITEM> * pCheckedItems)
{
   if (pCheckedItems != NULL)
   {
      pCheckedItems->clear();

      tGroups::iterator iter = m_groups.begin();
      tGroups::iterator end = m_groups.end();
      for (; iter != end; iter++)
      {
         uint nTools = (*iter)->GetToolCount();
         for (uint i = 0; i < nTools; i++)
         {
            cToolItem * pTool = (*iter)->GetTool(i);
            if (pTool->IsChecked())
            {
               pCheckedItems->push_back(reinterpret_cast<HTOOLITEM>(pTool));
            }
         }
      }
   }
}

////////////////////////////////////////

void cToolPalette::DoClick(HANDLE hItem, CPoint point)
{
   if (hItem != NULL)
   {
      if (IsGroup(reinterpret_cast<HTOOLGROUP>(hItem)))
      {
         DoGroupClick(reinterpret_cast<HTOOLGROUP>(hItem), point);
      }
      else
      {
         DoItemClick(reinterpret_cast<HTOOLITEM>(hItem), point);
      }
   }
}

////////////////////////////////////////

void cToolPalette::DoGroupClick(HTOOLGROUP hGroup, CPoint point)
{
   cToolGroup * pGroup = reinterpret_cast<cToolGroup *>(hGroup);
   pGroup->ToggleExpandCollapse();
   m_bUpdateScrollInfo = true;
   RECT rg;
   if (m_renderer.GetItemRect(hGroup, &rg))
   {
      m_renderer.FlushCachedRects();
      RECT rc;
      GetClientRect(&rc);
      rg.bottom = rc.bottom;
      InvalidateRect(&rg);
   }
   else
   {
      m_renderer.FlushCachedRects();
      Invalidate();
   }
}

////////////////////////////////////////

void cToolPalette::DoItemClick(HTOOLITEM hTool, CPoint point)
{
   cToolItem * pTool = reinterpret_cast<cToolItem *>(hTool);
   Assert(IsGroup(reinterpret_cast<HTOOLGROUP>(pTool->GetGroup())));
   if (!pTool->IsDisabled())
   {
      DoNotify(pTool, kTPN_ItemClick, point);
      if (ExclusiveCheck())
      {
         std::vector<HTOOLITEM> checkedItems;
         GetCheckedItems(&checkedItems);
         Assert(checkedItems.size() <= 1);
         if (!checkedItems.empty())
         {
            if (checkedItems[0] != hTool)
            {
               cToolItem * pUncheckTool = reinterpret_cast<cToolItem *>(checkedItems[0]);

               DoNotify(pUncheckTool, kTPN_ItemUncheck);
               pUncheckTool->SetState(kTPTS_Checked, 0);

               RECT r;
               if (m_renderer.GetItemRect(checkedItems[0], &r))
               {
                  InvalidateRect(&r);
               }
               else
               {
                  Invalidate();
               }
            }
         }

         if (!pTool->IsChecked())
         {
            DoNotify(pTool, kTPN_ItemCheck);
            pTool->SetState(kTPTS_Checked, kTPTS_Checked);
         }
      }
      else
      {
         DoNotify(pTool, pTool->IsChecked()? kTPN_ItemUncheck : kTPN_ItemCheck);
         pTool->ToggleChecked();
      }
      RECT rt;
      if (m_renderer.GetItemRect(hTool, &rt))
      {
         InvalidateRect(&rt);
      }
      else
      {
         Invalidate();
      }
   }
}

////////////////////////////////////////

LRESULT cToolPalette::DoNotify(cToolItem * pTool, int code, CPoint point)
{
   Assert(code == kTPN_ItemClick || code == kTPN_ItemDestroy
      || code == kTPN_ItemCheck || code == kTPN_ItemUncheck);
   if (pTool != NULL)
   {
      HWND hWndParent = GetParent();
      if (::IsWindow(hWndParent))
      {
         sNMToolPaletteItem nm = {0};
         nm.hdr.hwndFrom = m_hWnd;
         nm.hdr.code = code;
         nm.hdr.idFrom = GetDlgCtrlID();
         nm.pt = point;
         nm.hTool = reinterpret_cast<HTOOLITEM>(pTool);
         nm.pUserData = pTool->GetUserData();
         return ::SendMessage(hWndParent, WM_NOTIFY, nm.hdr.idFrom, reinterpret_cast<LPARAM>(&nm));
      }
   }
   return -1;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CPPUNIT

class cToolPaletteTests : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(cToolPaletteTests);
      CPPUNIT_TEST(TestCallsWithBadHandles);
      CPPUNIT_TEST(TestAddRemoveGroups);
      CPPUNIT_TEST(Test1);
   CPPUNIT_TEST_SUITE_END();

   void TestCallsWithBadHandles();
   void TestAddRemoveGroups();
   void Test1();

public:
   cToolPaletteTests();
   ~cToolPaletteTests();

   virtual void setUp();
   virtual void tearDown();

private:
   typedef CWinTraits<WS_POPUP, 0> tDummyWinTraits;
   class cDummyWindow : public CWindowImpl<cDummyWindow, CWindow, tDummyWinTraits>
   {
   public:
      DECLARE_WND_CLASS(NULL);
      DECLARE_EMPTY_MSG_MAP()
   };

   cDummyWindow * m_pDummyWnd;
   cToolPalette * m_pToolPalette;
};

////////////////////////////////////////

CPPUNIT_TEST_SUITE_REGISTRATION(cToolPaletteTests);

////////////////////////////////////////

void cToolPaletteTests::TestCallsWithBadHandles()
{
   if (m_pToolPalette == NULL || !m_pToolPalette->IsWindow())
   {
      return;
   }

   std::string s;
   sToolPaletteItem tpi;

   HTOOLGROUP hBadGroup = (HTOOLGROUP)0xDEADBEEF;
   HTOOLITEM hBadTool = (HTOOLITEM)0xDEADBEEF;

   CPPUNIT_ASSERT(!m_pToolPalette->RemoveGroup(hBadGroup));
   CPPUNIT_ASSERT(!m_pToolPalette->IsGroup(hBadGroup));
   CPPUNIT_ASSERT(!m_pToolPalette->IsTool(hBadTool));
   CPPUNIT_ASSERT(!m_pToolPalette->AddTool(hBadGroup, "foo", -1));
   CPPUNIT_ASSERT(!m_pToolPalette->AddTool(hBadGroup, &tpi));
   CPPUNIT_ASSERT(!m_pToolPalette->GetToolText(hBadTool, &s));
   CPPUNIT_ASSERT(!m_pToolPalette->GetTool(hBadTool, &tpi));
   CPPUNIT_ASSERT(!m_pToolPalette->RemoveTool(hBadTool));
   CPPUNIT_ASSERT(!m_pToolPalette->EnableTool(hBadTool, true));
}

////////////////////////////////////////

void cToolPaletteTests::TestAddRemoveGroups()
{
   if (m_pToolPalette == NULL || !m_pToolPalette->IsWindow())
   {
      return;
   }

   uint i, nGroupsAdd = 10 + (rand() % 90);

   std::vector<HTOOLGROUP> groups;

   for (i = 0; i < nGroupsAdd; i++)
   {
      tChar szTemp[200];
      wsprintf(szTemp, "Group %d", i);
      HTOOLGROUP hToolGroup = m_pToolPalette->AddGroup(szTemp, NULL);
      CPPUNIT_ASSERT(hToolGroup != NULL);
      groups.push_back(hToolGroup);
   }

   CPPUNIT_ASSERT(m_pToolPalette->GetGroupCount() == nGroupsAdd);

   uint nGroupsRemove = nGroupsAdd - (rand() % (nGroupsAdd / 2));
   CPPUNIT_ASSERT(nGroupsRemove < nGroupsAdd);

   for (i = 0; i < nGroupsRemove; i++)
   {
      int iRemove = rand() % groups.size();
      CPPUNIT_ASSERT(m_pToolPalette->RemoveGroup(groups[iRemove]));
      groups.erase(groups.begin() + iRemove);
   }

   CPPUNIT_ASSERT(m_pToolPalette->GetGroupCount() == (nGroupsAdd - nGroupsRemove));
}

////////////////////////////////////////

void cToolPaletteTests::Test1()
{
   if (m_pToolPalette == NULL || !m_pToolPalette->IsWindow())
   {
      return;
   }

   for (int i = 0; i < 3; i++)
   {
      tChar szTemp[200];
      wsprintf(szTemp, "Group %c", 'A' + i);
      HTOOLGROUP hToolGroup = m_pToolPalette->AddGroup(szTemp, NULL);
      if (hToolGroup != NULL)
      {
         int nTools = 3 + (rand() & 3);
         for (int j = 0; j < nTools; j++)
         {
            wsprintf(szTemp, "Tool %d", j);
            HTOOLITEM hTool = m_pToolPalette->AddTool(hToolGroup, szTemp, -1);
            if (hTool != NULL)
            {
               if (j == 1)
               {
                  CPPUNIT_ASSERT(m_pToolPalette->EnableTool(hTool, false));
               }
            }
         }
      }
   }
}

////////////////////////////////////////

cToolPaletteTests::cToolPaletteTests()
 : m_pDummyWnd(NULL),
   m_pToolPalette(NULL)
{
}

////////////////////////////////////////

cToolPaletteTests::~cToolPaletteTests()
{
}

////////////////////////////////////////

void cToolPaletteTests::setUp()
{
   CPPUNIT_ASSERT(m_pDummyWnd == NULL);
   m_pDummyWnd = new cDummyWindow;
   CPPUNIT_ASSERT(m_pDummyWnd != NULL);
   HWND hDummyWnd = m_pDummyWnd->Create(NULL, CWindow::rcDefault);
   CPPUNIT_ASSERT(IsWindow(hDummyWnd));
   if (!IsWindow(hDummyWnd))
   {
      delete m_pDummyWnd;
      m_pDummyWnd = NULL;
   }
   else
   {
      CPPUNIT_ASSERT(m_pToolPalette == NULL);
      m_pToolPalette = new cToolPalette;
      CPPUNIT_ASSERT(m_pToolPalette != NULL);
      HWND hWndToolPalette = m_pToolPalette->Create(m_pDummyWnd->m_hWnd, CWindow::rcDefault);
      CPPUNIT_ASSERT(IsWindow(hWndToolPalette));
      if (!IsWindow(hWndToolPalette))
      {
         delete m_pToolPalette;
         m_pToolPalette = NULL;
      }
   }
}

////////////////////////////////////////

void cToolPaletteTests::tearDown()
{
   if (m_pToolPalette != NULL && m_pToolPalette->IsWindow())
   {
      CPPUNIT_ASSERT(m_pToolPalette->DestroyWindow());
   }
   delete m_pToolPalette;
   m_pToolPalette = NULL;

   if (m_pDummyWnd != NULL && m_pDummyWnd->IsWindow())
   {
      CPPUNIT_ASSERT(m_pDummyWnd->DestroyWindow());
   }
   delete m_pDummyWnd;
   m_pDummyWnd = NULL;
}

///////////////////////////////////////////////////////////////////////////////

#endif // HAVE_CPPUNIT

///////////////////////////////////////////////////////////////////////////////
