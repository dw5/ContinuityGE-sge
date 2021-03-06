///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "LogWnd.h"

#include "platform/sys.h"

#include <afxole.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int kDefaultMaxLines = 1000;

///////////////////////////////////////////////////////////////////////////////

template <class T>
void Swap(T & a, T & b)
{
   T temp;
   temp = a;
   a = b;
   b = temp;
}

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cTextDataSource
//

class cTextDataSource : public COleDataSource
{
public:
   bool SetText(const tChar * pszText);
};

///////////////////////////////////////

bool cTextDataSource::SetText(const tChar * pszText)
{
   if (m_nSize > 0)
   {
      DebugMsg1("Already set text for OLE data source at 0x%08X\n", (long)this);
      return false;
   }

   Assert(AfxIsValidString(pszText, 1));

   HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, _tcslen(pszText) + 1);
   if (hMem == NULL)
   {
      return false;
   }

   char * pMem = (char *)GlobalLock(hMem);
   if (pMem == NULL)
   {
      GlobalFree(hMem);
      return false;
   }

   _tcscpy(pMem, pszText);
   GlobalUnlock(hMem);

   CacheGlobalData(CF_TEXT, hMem);

   return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndLine
//

///////////////////////////////////////

cLogWndLine::cLogWndLine(LPCTSTR pszText, int textLength, COLORREF textColor)
 : m_text(pszText, textLength),
   m_textColor(textColor),
   m_horizontalExtent(0)
{
}

///////////////////////////////////////

cLogWndLine::cLogWndLine(const cLogWndLine & other)
 : m_text(other.m_text),
   m_textColor(other.m_textColor),
   m_horizontalExtent(other.m_horizontalExtent)
{
}

///////////////////////////////////////

cLogWndLine::~cLogWndLine()
{
}

///////////////////////////////////////

const cLogWndLine & cLogWndLine::operator =(const cLogWndLine & other)
{
   m_text = other.m_text;
   m_textColor = other.m_textColor;
   m_horizontalExtent = other.m_horizontalExtent;
   return *this;
}

///////////////////////////////////////

void cLogWndLine::UpdateHorizontalExtent(HDC hDC)
{
   if (hDC != NULL)
   {
      SIZE extent;
      if (::GetTextExtentPoint32(hDC, m_text, m_text.GetLength(), &extent))
      {
         m_horizontalExtent = extent.cx;
      }
   }
}

///////////////////////////////////////

int cLogWndLine::GetHorizontalExtent() const
{
   return m_horizontalExtent;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndLocation
//

///////////////////////////////////////

cLogWndLocation::cLogWndLocation()
 : iLine(-1),
   iChar(-1)
{
}

///////////////////////////////////////

cLogWndLocation::cLogWndLocation(const cLogWndLocation & other)
 : iLine(other.iLine),
   iChar(other.iChar)
{
}

///////////////////////////////////////

cLogWndLocation::~cLogWndLocation()
{
}

///////////////////////////////////////

const cLogWndLocation & cLogWndLocation::operator =(const cLogWndLocation & other)
{
   iLine = other.iLine;
   iChar = other.iChar;
   return *this;
}

///////////////////////////////////////

bool cLogWndLocation::operator <(const cLogWndLocation & other) const
{
   return iLine < other.iLine || (iLine == other.iLine && iChar < other.iChar);
}

///////////////////////////////////////

bool cLogWndLocation::operator >(const cLogWndLocation & other) const
{
   return iLine > other.iLine || (iLine == other.iLine && iChar > other.iChar);
}

///////////////////////////////////////

bool cLogWndLocation::operator <=(const cLogWndLocation & other) const
{
   return iLine < other.iLine || (iLine == other.iLine && iChar <= other.iChar);
}

///////////////////////////////////////

bool cLogWndLocation::operator >=(const cLogWndLocation & other) const
{
   return iLine > other.iLine || (iLine == other.iLine && iChar >= other.iChar);
}

///////////////////////////////////////

bool cLogWndLocation::operator ==(const cLogWndLocation & other) const
{
   return iLine == other.iLine && iChar == other.iChar;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWndSelection
//

///////////////////////////////////////

cLogWndSelection::cLogWndSelection()
 : m_pSelAnchor(NULL)
 , m_pSelDrag(NULL)
 , m_bDragGTEAnchor(false)
{
}

///////////////////////////////////////

cLogWndSelection::~cLogWndSelection()
{
}

///////////////////////////////////////

void cLogWndSelection::Clear()
{
   m_startSel = cLogWndLocation();
   m_endSel = cLogWndLocation();
   m_pSelAnchor = NULL;
   m_pSelDrag = NULL;
}

///////////////////////////////////////

bool cLogWndSelection::IsEmpty() const
{
   return (m_startSel.iLine == -1) && (m_endSel.iLine == -1);
}

///////////////////////////////////////

bool cLogWndSelection::IsWithinSelection(const cLogWndLocation & loc) const
{
   return (loc >= m_startSel) && (loc < m_endSel);
}

///////////////////////////////////////

void cLogWndSelection::DragSelectBegin(const cLogWndLocation & loc)
{
   m_savedStartSel = m_startSel;
   m_savedEndSel = m_endSel;
   m_startSel = loc;
   m_endSel = loc;
   m_bDragGTEAnchor = true;
   m_pSelAnchor = &m_startSel;
   m_pSelDrag = &m_endSel;
}

///////////////////////////////////////

void cLogWndSelection::DragSelectUpdate(const cLogWndLocation & loc)
{
   *m_pSelDrag = loc;

   int iAnchorEntry = m_pSelAnchor->iLine;
   int iDragEntry = m_pSelDrag->iLine;

   // is the drag pos less than the anchor pos?
   bool bDragLTAnchor = (iDragEntry < iAnchorEntry) || 
      ((iDragEntry == iAnchorEntry) && (m_pSelDrag->iChar < m_pSelAnchor->iChar));

   // is the drag pos equal to the anchor pos?
   bool bDragEqAnchor = (iDragEntry == iAnchorEntry) && (m_pSelDrag->iChar == m_pSelAnchor->iChar);

   // is the drag pos greater than the anchor pos?
   bool bDragGTAnchor = (iDragEntry > iAnchorEntry) || 
      ((iDragEntry == iAnchorEntry) && (m_pSelDrag->iChar > m_pSelAnchor->iChar));

   if ((m_bDragGTEAnchor && bDragLTAnchor) ||
         (!m_bDragGTEAnchor && (bDragGTAnchor || bDragEqAnchor)))
   {
      Swap(*m_pSelAnchor, *m_pSelDrag);
      Swap(m_pSelAnchor, m_pSelDrag);
      m_bDragGTEAnchor = !m_bDragGTEAnchor;
   }
}

///////////////////////////////////////

void cLogWndSelection::DragSelectEnd()
{
   if (m_startSel.iLine == -1)
   {
      m_startSel.iLine = 0;
      m_startSel.iChar = 0;
   }

   if (m_endSel.iLine == -1)
   {
      Clear();
   }

   m_pSelAnchor = NULL;
   m_pSelDrag = NULL;
}

///////////////////////////////////////

void cLogWndSelection::DragSelectCancel()
{
   m_startSel = m_savedStartSel;
   m_endSel = m_savedEndSel;
   m_pSelAnchor = NULL;
   m_pSelDrag = NULL;
}

///////////////////////////////////////

bool cLogWndSelection::GetSelection(const tLogWndLines & lines, CString * pSel) const
{
   return false;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cLogWnd
//

///////////////////////////////////////

cLogWnd::cLogWnd()
 : m_nMaxLines(kDefaultMaxLines)
 , m_lineHeight(0)
 , m_maxLineHorizontalExtent(0)
 , m_bAtEnd(true)
 , m_nAddsSinceLastPaint(0)
{
}

///////////////////////////////////////

cLogWnd::~cLogWnd()
{
   Clear();
}

///////////////////////////////////////

void cLogWnd::AddText(LPCTSTR pszText, int textLength, COLORREF textColor)
{
   Assert(AfxIsValidString(pszText, lstrlen(pszText)));
   Assert((textLength < 0) || (textLength <= lstrlen(pszText)));

   if (!IsWindow())
   {
      return;
   }

   static const tChar szLineBreakChars[] = _T("\r\n");

   if (textLength < 0)
   {
      textLength = lstrlen(pszText);
   }

   if (textColor == CLR_INVALID)
   {
      textColor = GetTextColor();
   }

   WTL::CDC dc(::GetDC(m_hWnd));
   HFONT hOldFont = dc.SelectFont(GetFont());

   int breakLength = _tcscspn(pszText, szLineBreakChars);
   while ((breakLength <= textLength) && (breakLength > 0))
   {
      cLogWndLine * pLine = new cLogWndLine(pszText, breakLength, textColor);
      pLine->UpdateHorizontalExtent(dc);
      m_lines.push_back(pLine);

      if (pLine->GetHorizontalExtent() > m_maxLineHorizontalExtent)
      {
         m_maxLineHorizontalExtent = pLine->GetHorizontalExtent();
      }

      pszText += breakLength;
      while ((*pszText != 0) && (_tcschr(szLineBreakChars, *pszText) != NULL))
      {
         pszText++;
      }
      breakLength = _tcscspn(pszText, szLineBreakChars);
   }

   dc.SelectFont(hOldFont);

   EnforceMaxLines();

   UpdateScrollInfo();

   // update the scroll position only if it's already at the very end
   if (m_bAtEnd)
   {
      ScrollBottom();
   }

   if (++m_nAddsSinceLastPaint > 15)
   {
      UpdateWindow();
   }
   else
   {
      Invalidate();
   }
}

///////////////////////////////////////

void cLogWnd::Clear()
{
   m_selection.Clear();

   m_maxLineHorizontalExtent = 0;

   for (tLogWndLines::iterator iter = m_lines.begin(); iter != m_lines.end(); iter++)
   {
      delete *iter;
   }
   m_lines.clear();

   // Normally, when we clear the contents we want to update the scrollbar
   // calibration. However, we don't want to do this if the window has been
   // destroyed (i.e., Clear() is being called from the destructor).
   if (IsWindow())
   {
      m_bAtEnd = true;
      UpdateScrollInfo();
      Invalidate();
   }
}

///////////////////////////////////////

void cLogWnd::SetMaxLines(int nMaxLines)
{
   m_nMaxLines = nMaxLines;
   m_lines.reserve(nMaxLines);
   EnforceMaxLines();
}

///////////////////////////////////////

bool cLogWnd::CopySelection()
{
   bool result = false; // assume failure

   CString sel;
   if (GetSelection(&sel) && sel.GetLength() > 0)
   {
      cTextDataSource tds;
      if (result = tds.SetText(sel))
      {
         tds.SetClipboard();
      }
   }

   return result;
}

///////////////////////////////////////

bool cLogWnd::GetSelection(CString * pSel)
{
   if (!m_selection.IsEmpty())
   {
      Assert(pSel != NULL);
      pSel->Empty();

      for (uint i = m_selection.GetStart().iLine; i < m_lines.size(); i++)
      {
         int iStart, iEnd;

         if (i == m_selection.GetStart().iLine)
         {
            iStart = m_selection.GetStart().iChar;
         }
         else
         {
            iStart = 0;
         }

         if (i == m_selection.GetEnd().iLine)
         {
            iEnd = m_selection.GetEnd().iChar;
         }
         else
         {
            iEnd = m_lines[i]->GetTextLen();
         }

         CString s = m_lines[i]->GetText().Mid(iStart, iEnd - iStart);

         if (pSel->GetLength())
         {
            *pSel += '\n';
         }
         *pSel += s;

         // if we just did the last entry in the selection, break out of the loop
         if (i == m_selection.GetEnd().iLine)
         {
            break;
         }
      }

      return true;
   }

   return false;
}

///////////////////////////////////////

void cLogWnd::UpdateScrollInfo()
{
   CRect rect;
   GetClientRect(&rect);

   if (!rect.IsRectEmpty() && !m_lines.empty())
   {
      SetScrollSize(m_maxLineHorizontalExtent, m_lines.size() * m_lineHeight);
      SetScrollLine(0, m_lineHeight);
      SetScrollPage(0, m_lineHeight * 4);
   }
}

///////////////////////////////////////

void cLogWnd::UpdateFontMetrics()
{
   WTL::CDC dc(::GetDC(m_hWnd));
   HFONT hOldFont = dc.SelectFont(GetFont());

   tLogWndLines::iterator iter = m_lines.begin();
   for (; iter != m_lines.end(); iter++)
   {
      (*iter)->UpdateHorizontalExtent(dc);
   }

   TEXTMETRIC tm;
   Verify(dc.GetTextMetrics(&tm));

   m_lineHeight = tm.tmHeight + tm.tmExternalLeading;

   dc.SelectFont(hOldFont);
}

///////////////////////////////////////

void cLogWnd::EnforceMaxLines()
{
   bool bUpdateMaxEntrySize = false;

   while (!m_lines.empty() && (m_lines.size() > GetMaxLines()))
   {
      Assert(m_lines[0] != NULL);
      cLogWndLine * pRemoved = m_lines[0];
      m_lines.erase(m_lines.begin());

      // if the entry removed was the biggest one, need to find the new biggest entry
      if (pRemoved->GetHorizontalExtent() == m_maxLineHorizontalExtent)
      {
         bUpdateMaxEntrySize = true;
      }

      delete pRemoved;
   }

   if (bUpdateMaxEntrySize)
   {
      m_maxLineHorizontalExtent = 0;

      WTL::CDC dc(::GetDC(m_hWnd));
      HFONT hOldFont = dc.SelectFont(GetFont());

      tLogWndLines::iterator iter;
      for (iter = m_lines.begin(); iter != m_lines.end(); iter++)
      {
         if ((*iter)->GetHorizontalExtent() > m_maxLineHorizontalExtent)
         {
            m_maxLineHorizontalExtent = (*iter)->GetHorizontalExtent();
         }
      }

      dc.SelectFont(hOldFont);
   }
}

///////////////////////////////////////

bool cLogWnd::GetHitLocation(const CPoint & point, cLogWndLocation * pHitTest)
{
   return GetHitLocation(point, &pHitTest->iLine, &pHitTest->iChar);
}

///////////////////////////////////////

bool cLogWnd::GetHitLocation(const CPoint & point, int * piLine, int * piChar) const
{
   bool bHit = false;

   int iLine;
   if (GetHitLine(point, &iLine))
   {
      WTL::CDC dc(::GetDC(m_hWnd));
      HFONT hOldFont = dc.SelectFont(GetFont());

      DWORD gcpFlags = GetFontLanguageInfo(dc);
      gcpFlags |= GCP_MAXEXTENT;

      GCP_RESULTS gcpResults = {0};
      gcpResults.lStructSize = sizeof(GCP_RESULTS);
      gcpResults.nGlyphs = m_lines[iLine]->GetTextLen();

      if (GetCharacterPlacement(dc, m_lines[iLine]->GetText(),
         m_lines[iLine]->GetTextLen(), point.x, &gcpResults, gcpFlags))
      {
         bHit = true;

         if (piLine != NULL)
         {
            *piLine = iLine;
         }

         if (piChar != NULL)
         {
            *piChar = gcpResults.nMaxFit;
         }
      }

      dc.SelectFont(hOldFont);
   }

   return bHit;
}

///////////////////////////////////////

bool cLogWnd::GetHitLine(const CPoint & point, int * piLine) const
{
   int iHitLine = point.y / m_lineHeight;
   if (iHitLine >= 0 && iHitLine < static_cast<int>(m_lines.size()))
   {
      if (piLine != NULL)
      {
         *piLine = iHitLine;
      }
      return true;
   }
   return false;
}

///////////////////////////////////////

void cLogWnd::GetVisibleRange(int * pStart, int * pEnd)
{
   Assert(m_lineHeight > 0);
   if (pStart != NULL)
   {
      *pStart = m_ptOffset.y / m_lineHeight;
   }
   if (pEnd != NULL)
   {
      CRect rect;
      GetClientRect(rect);
      *pEnd = (m_ptOffset.y + rect.Height()) / m_lineHeight;
   }
}

///////////////////////////////////////

void cLogWnd::AddContextMenuItems(WTL::CMenu* pMenu)
{
   if (pMenu != NULL)
   {
      static const UINT stdCmdIds[] = { ID_EDIT_COPY, ID_EDIT_CLEAR_ALL };
      for (UINT i = 0; i < _countof(stdCmdIds); i++)
      {
         CString str;
         if (str.LoadString(stdCmdIds[i]))
         {
            int index = str.ReverseFind('\n');
            if (index != -1)
            {
               str.Delete(0, index + 1);
            }
            if (i > 0 && pMenu->GetMenuItemCount() > 0)
            {
               Verify(pMenu->AppendMenu(MF_SEPARATOR));
            }
            Verify(pMenu->AppendMenu(MF_STRING, stdCmdIds[i], str));
         }
      }
   }
}

///////////////////////////////////////

void cLogWnd::DoPaint(WTL::CDCHandle dc)
{
   if (m_lines.empty())
   {
      return;
   }

   int savedDC = dc.SaveDC();

   HFONT hOldFont = dc.SelectFont(GetFont());

   int iStart, iEnd;
   GetVisibleRange(&iStart, &iEnd);

   CRect client;
   GetClientRect(client);

   CRect r(client.left, iStart * m_lineHeight, client.right, 0);
   r.bottom = r.top + m_lineHeight;

   bool bInSel = false;

   tLogWndLines::iterator iter = m_lines.begin() + iStart;
   for (int index = 0; (iter != m_lines.end()) && (index <= iEnd); index++, iter++)
   {
      if ((index == m_selection.GetStart().iLine) && (index == m_selection.GetEnd().iLine))
      {
         CSize startSelExtent;
         dc.GetTextExtent(m_lines[m_selection.GetStart().iLine]->GetText(), m_selection.GetStart().iChar, &startSelExtent);

         CRect clip(r);
         clip.right = startSelExtent.cx;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         CSize endSelExtent;
         dc.GetTextExtent(m_lines[m_selection.GetEnd().iLine]->GetText(), m_selection.GetEnd().iChar, &endSelExtent);

         clip.left = clip.right;
         clip.right = endSelExtent.cx;

         dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
         dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         clip.left = clip.right;
         clip.right = r.right;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);
      }
      else if (index == m_selection.GetStart().iLine)
      {
         bInSel = true;

         CSize startSelExtent;
         dc.GetTextExtent(m_lines[m_selection.GetStart().iLine]->GetText(), m_selection.GetStart().iChar, &startSelExtent);

         CRect clip(r);
         clip.right = startSelExtent.cx;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         CSize textSize;
         dc.GetTextExtent((*iter)->GetText(), (*iter)->GetTextLen(), &textSize);

         clip = r;
         clip.left = startSelExtent.cx;
         clip.right = textSize.cx;

         dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
         dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         clip.left = clip.right;
         clip.right = r.right;

         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, NULL, 0, NULL);
      }
      else if (index == m_selection.GetEnd().iLine)
      {
         bInSel = false;

         CSize endSelExtent;
         dc.GetTextExtent(m_lines[m_selection.GetEnd().iLine]->GetText(), m_selection.GetEnd().iChar, &endSelExtent);

         CRect clip(r);
         clip.right = endSelExtent.cx;

         dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
         dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

         clip = r;
         clip.left = endSelExtent.cx;

         dc.SetBkColor(GetBkColor());
         dc.SetTextColor((*iter)->GetTextColor());
         dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);
      }
      else
      {
         if (bInSel)
         {
            CSize textSize;
            dc.GetTextExtent((*iter)->GetText(), (*iter)->GetTextLen(), &textSize);

            CRect clip(r);
            clip.right = textSize.cx;

            dc.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
            dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
            dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);

            clip.left = clip.right;
            clip.right = r.right;
            dc.ExtTextOut(r.left, r.top, ETO_OPAQUE | ETO_CLIPPED, &clip, NULL, 0, NULL);
         }
         else
         {
            dc.SetBkColor(GetBkColor());
            dc.SetTextColor((*iter)->GetTextColor());
            dc.ExtTextOut(r.left, r.top, ETO_OPAQUE, &r, (*iter)->GetText(), (*iter)->GetTextLen(), NULL);
         }
      }

      // Prepare to draw the next entry
      r.OffsetRect(0, r.Height());
   }

   if (r.top < client.bottom)
   {
      r.bottom = client.bottom;
      dc.FillSolidRect(r, GetBkColor());
   }

   dc.RestoreDC(savedDC);

   dc.SelectFont(hOldFont);

   m_nAddsSinceLastPaint = 0;
}

///////////////////////////////////////

void cLogWnd::DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine)
{
   tBase::DoScroll(nType, nScrollCode, cxyOffset, cxySizeAll, cxySizePage, cxySizeLine);

   SCROLLINFO info;
   info.cbSize = sizeof(SCROLLINFO);
   info.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
   Verify(GetScrollInfo(SB_VERT, &info));

   if (info.nPos >= (int)(info.nMax - info.nPage))
   {
      m_bAtEnd = true;
   }
   else
   {
      m_bAtEnd = false;
   }
}

///////////////////////////////////////

LRESULT cLogWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   UpdateFontMetrics();
   Verify(AfxOleInit());
   return 0;
}

///////////////////////////////////////

void cLogWnd::OnDestroy()
{
   AfxOleTerm();
}

///////////////////////////////////////

LRESULT cLogWnd::OnEraseBkgnd(WTL::CDCHandle dc)
{
   if (m_lines.empty())
   {
      CRect rect;
      GetClientRect(&rect);
      dc.FillSolidRect(rect, GetBkColor());
   }
   return TRUE;
}

///////////////////////////////////////

void cLogWnd::OnContextMenu(HWND hWnd, CPoint point)
{
   WTL::CMenu contextMenu;
   Verify(contextMenu.CreatePopupMenu());

   AddContextMenuItems(&contextMenu);

   if (!m_selection.IsEmpty())
   {
      contextMenu.EnableMenuItem(ID_EDIT_COPY, MF_BYCOMMAND | MF_ENABLED);
   }
   else
   {
      contextMenu.EnableMenuItem(ID_EDIT_COPY, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
   }

   contextMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, m_hWnd);
}

///////////////////////////////////////

LRESULT cLogWnd::OnSetCursor(HWND hWnd, UINT hitTest, UINT message)
{
   if (hitTest == HTCLIENT)
   {
      CPoint point(GetMessagePos());
      ScreenToClient(&point);

      cLogWndLocation hitLoc;
      if (GetHitLocation(point, &hitLoc)
         && m_selection.IsWithinSelection(hitLoc))
      {
         SetCursor(LoadCursor(NULL, IDC_ARROW));
      }
      else
      {
         SetCursor(LoadCursor(NULL, IDC_IBEAM));
      }

      return TRUE;
   }

   return FALSE;
}

///////////////////////////////////////

void cLogWnd::OnMouseMove(UINT flags, CPoint point)
{
   if (GetCapture() == m_hWnd)
   {
      CRect rect;
      GetClientRect(&rect);

      if (point.y < rect.top)
      {
         ScrollLineUp();
      }
      else if (point.y > rect.bottom)
      {
         ScrollLineDown();
      }

      if (point.x > rect.right)
      {
         ScrollLineRight();
      }
      else if (point.x < rect.left)
      {
         ScrollLineLeft();
      }

      cLogWndLocation loc;
      if (GetHitLocation(point, &loc))
      {
         m_selection.DragSelectUpdate(loc);

         Invalidate(FALSE);
         UpdateWindow();
      }
   }
}

///////////////////////////////////////

void cLogWnd::OnLButtonDown(UINT flags, CPoint point)
{
   cLogWndLocation hitLoc;
   if (GetHitLocation(point, &hitLoc))
   {
      if (m_selection.IsWithinSelection(hitLoc))
      {
         CString sel;
         if (GetSelection(&sel) && sel.GetLength() > 0)
         {
            cTextDataSource tds;
            Verify(tds.SetText(sel));
            tds.DoDragDrop();
         }
      }
      else
      {
         m_selection.DragSelectBegin(hitLoc);

         Invalidate(FALSE);
         UpdateWindow();

         SetCapture();
      }
   }
}

///////////////////////////////////////

void cLogWnd::OnLButtonUp(UINT flags, CPoint point)
{
   if (GetCapture() == m_hWnd)
   {
      m_selection.DragSelectEnd();

      Invalidate(FALSE);
      ReleaseCapture();
   }
}

///////////////////////////////////////

void cLogWnd::OnCancelMode()
{
   m_selection.DragSelectCancel();
}

///////////////////////////////////////

void cLogWnd::OnEditCopy(UINT code, int id, HWND hWnd)
{
   CopySelection();
}

///////////////////////////////////////

void cLogWnd::OnEditClearAll(UINT code, int id, HWND hWnd)
{
   Clear();
}

///////////////////////////////////////////////////////////////////////////////
