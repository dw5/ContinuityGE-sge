///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "gcommon.h"
#include "uiwidgets.h"
#include "uiwidgutilstem.h"
#include "uirender.h"

#include "inputapi.h"

#include "font.h"
#include "render.h"
#include "material.h"
#include "image.h"

#include "keys.h"
#include "vec3.h"
#include "resmgr.h"
#include "globalobj.h"
#include "techtime.h"

#include <locale>

// @HACK @TODO: only needed while cUIBitmapButton moves to new
// rendering interfaces (use to use cImage::Blt)
#include "ggl.h"

#include "dbgalloc.h" // must be last header

static const int g_3dEdge = 2;

static const cUIColor g_hideousErrorColor(0,1,0,1);

///////////////////////////////////////////////////////////////////////////////

struct sUIVertex
{
   float u, v;
   byte r, g, b, a;
   tVec3 pos;
};

sVertexElement g_UIVertexDecl[] =
{
   { kVDU_TexCoord, kVDT_Float2 },
   { kVDU_Color, kVDT_UnsignedByte4 },
   { kVDU_Position, kVDT_Float3 }
};

cAutoIPtr<IVertexDeclaration> g_pUIVertexDecl;

IVertexDeclaration * UIAccessVertexDecl()
{
   if (!g_pUIVertexDecl)
   {
      Assert(AccessRenderDevice() != NULL);
      AccessRenderDevice()->CreateVertexDeclaration(g_UIVertexDecl, _countof(g_UIVertexDecl), &g_pUIVertexDecl);
   }

   return g_pUIVertexDecl;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIDialog
//

///////////////////////////////////////

cUIDialog::cUIDialog()
{
   SetMargins(
      cUIMargins(
         kDefaultMargin,
         kDefaultMargin + kCaptionHeight + kCaptionGap,
         kDefaultMargin,
         kDefaultMargin));
}

///////////////////////////////////////

cUIDialog::~cUIDialog()
{
}

///////////////////////////////////////

void cUIDialog::Render(IRenderDevice * pRenderDevice)
{
   UIDraw3dRect(GetScreenRect(), 2, AccessStyle()->GetHighlight(), AccessStyle()->GetShadow(), AccessStyle()->GetFace());

   cUIRect captionRect = GetCaptionRect();

   UIDrawGradientRect(captionRect, AccessStyle()->GetCaption(), AccessStyle()->GetCaption(),
      AccessStyle()->GetCaptionGradient(), AccessStyle()->GetCaptionGradient());

   if (!m_title.empty())
   {
      UIDrawText(m_title.c_str(), m_title.length(), &captionRect,
         kTextCenter | kTextVCenter, AccessStyle()->AccessFont(), AccessStyle()->GetCaptionText());
   }

   tBase::Render(pRenderDevice);
}

///////////////////////////////////////

bool cUIDialog::OnEvent(const cUIEvent * pEvent)
{
   if ((pEvent->code == kEventKeyDown) && (pEvent->keyCode == kEscape))
   {
      cUIEvent destroyEvent(kEventDestroy, cUIPoint(0,0), 0, this);
      UIBubbleEvent(this, &destroyEvent);
      delete this;
      return true;
   }
   return tBase::OnEvent(pEvent);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUILabel
//

cUILabel::cUILabel()
{
   NoFocus();
}

///////////////////////////////////////

void cUILabel::Render(IRenderDevice * pRenderDevice)
{
   cUIRect rect = GetScreenRect();
   UIDrawText(m_text.c_str(), m_text.length(), &rect,
      kTextCenter | kTextVCenter, AccessStyle()->AccessFont(), AccessStyle()->GetText());
}

///////////////////////////////////////

cUISize cUILabel::GetPreferredSize() const
{
   float textWidth, textHeight;
   AccessStyle()->AccessFont()->MeasureText(m_text.c_str(), m_text.length(), &textWidth, &textHeight);

   return cUISize(textWidth, textHeight);
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIImage
//

///////////////////////////////////////

cUIImage::cUIImage()
{
   NoFocus();
}

///////////////////////////////////////

cUIImage::~cUIImage()
{
}

///////////////////////////////////////

void cUIImage::Render(IRenderDevice * pRenderDevice)
{
   if (m_pTex != NULL)
   {
      UIDrawTextureRect(GetScreenRect(), m_pTex);
   }
   else
   {
      // draw a hideous color to indicate error
      UIDrawSolidRect(GetScreenRect(), g_hideousErrorColor);
   }
}

///////////////////////////////////////

cUISize cUIImage::GetPreferredSize() const
{
   return m_size;
}

///////////////////////////////////////

bool cUIImage::LoadImage(const char * pszFilename)
{
   UseGlobal(ResourceManager);

   cImage * pImage = ImageLoad(pResourceManager, pszFilename);
   if (pImage != NULL)
   {
      m_size = cUISize(pImage->GetWidth(), pImage->GetHeight());
      AccessRenderDevice()->CreateTexture(pImage, &m_pTex);
      delete pImage;
   }
   // return true unconditionally so that this component will
   // be a placeholder at least if the image fails to load
   return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIButton
//

cUIButton::cUIButton()
 : m_bPressed(false),
   m_bMouseOver(false)
{
}

///////////////////////////////////////

void cUIButton::Render(IRenderDevice * pRenderDevice)
{
   cUIRect screenRect = GetScreenRect();

   cUIPoint offset(0,0);

   if (IsMouseOver() && IsPressed())
   {
      UIDraw3dRect(screenRect, g_3dEdge, AccessStyle()->GetShadow(), AccessStyle()->GetHighlight(), AccessStyle()->GetFace());
      offset = cUIPoint(g_3dEdge, g_3dEdge);
   }
   else
   {
      UIDraw3dRect(screenRect, g_3dEdge, AccessStyle()->GetHighlight(), AccessStyle()->GetShadow(), AccessStyle()->GetFace());
   }

   if (!m_text.empty())
   {
      screenRect.left += offset.x;
      screenRect.top += offset.y;
      UIDrawText(m_text.c_str(), 
         m_text.length(), 
         &screenRect,
         kTextCenter | kTextVCenter, 
         AccessStyle()->AccessFont(), 
         IsMouseOver() ? AccessStyle()->GetHot() : AccessStyle()->GetText());
   }
}

///////////////////////////////////////

bool cUIButton::OnEvent(const cUIEvent * pEvent)
{
   if (cUIScriptEventHandler<cUIButton>::FilterEvent(pEvent))
      return true;
   if (pEvent->code == kEventMouseDown)
   {
      m_bPressed = true;
   }
   else if (pEvent->code == kEventMouseUp || pEvent->code == kEventBlur)
   {
      m_bPressed = false;
   }
   else if (pEvent->code == kEventMouseEnter)
   {
      m_bMouseOver = true;
   }
   else if (pEvent->code == kEventMouseLeave)
   {
      m_bMouseOver = false;
   }
   return Base::OnEvent(pEvent);
}

///////////////////////////////////////

cUISize cUIButton::GetPreferredSize() const
{
   float textWidth, textHeight;
   AccessStyle()->AccessFont()->MeasureText(GetText(), -1, &textWidth, &textHeight);

   return cUISize(textWidth + (2 * kTextBorderHorz),
                  textHeight + (2 * kTextBorderVert));
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIBitmapButton
//

static const int kUIBitmapButtonIndices = 4;

///////////////////////////////////////

cUIBitmapButton::cUIBitmapButton()
 : m_size(0,0),
   m_bPressed(false),
   m_bMouseOver(false)
{
}

///////////////////////////////////////

cUIBitmapButton::~cUIBitmapButton()
{
}

///////////////////////////////////////

void cUIBitmapButton::Render(IRenderDevice * pRenderDevice)
{
   uint buttonState = kBS_Normal;
   if (IsMouseOver())
   {
      buttonState = IsPressed() ? kBS_Pressed : kBS_Hover;
   }

   cUIRect screenRect = GetScreenRect();

   glPushMatrix();
   glTranslatef(screenRect.left, screenRect.top, 0);

   AccessRenderDevice()->Render(kRP_TriangleFan, 
      m_pMat,
      kUIBitmapButtonIndices, 
      m_pIB,
      buttonState * 4, 
      m_pVB);

   glPopMatrix();
}

///////////////////////////////////////

bool cUIBitmapButton::OnEvent(const cUIEvent * pEvent)
{
   if (pEvent->code == kEventMouseDown)
   {
      m_bPressed = true;
   }
   else if (pEvent->code == kEventMouseUp || pEvent->code == kEventBlur)
   {
      m_bPressed = false;
   }
   else if (pEvent->code == kEventMouseEnter)
   {
      m_bMouseOver = true;
   }
   else if (pEvent->code == kEventMouseLeave)
   {
      m_bMouseOver = false;
   }

   if (cUIScriptEventHandler<cUIBitmapButton>::FilterEvent(pEvent))
      return true;

   return cUIWidget::OnEvent(pEvent);
}

///////////////////////////////////////

cUISize cUIBitmapButton::GetPreferredSize() const
{
   return cUISize(m_size.width, m_size.height / 4);
}

///////////////////////////////////////

bool cUIBitmapButton::SetBitmap(const char * pszName)
{
   UseGlobal(ResourceManager);

   cImage * pImage = ImageLoad(pResourceManager, pszName);
   if (pImage != NULL)
   {
      m_size = cUISize(pImage->GetWidth(), pImage->GetHeight());

      // @TODO: Check for errors here
      cAutoIPtr<ITexture> pTex;
      AccessRenderDevice()->CreateTexture(pImage, &pTex);

      delete pImage;

      m_pMat = MaterialCreate();
      if (m_pMat != NULL)
      {
         m_pMat->SetTexture(0, pTex);
      }

      if (AccessRenderDevice()->CreateIndexBuffer(kUIBitmapButtonIndices, kBU_Default, kIBF_16Bit, kBP_Auto, &m_pIB) == S_OK)
      {
         uint16 * pIndexData;
         if (m_pIB->Lock(kBL_Discard, (void * *)&pIndexData) == S_OK)
         {
            pIndexData[0] = 0;
            pIndexData[1] = 1;
            pIndexData[2] = 2;
            pIndexData[3] = 3;
            m_pIB->Unlock();
         }
      }

      if (AccessRenderDevice()->CreateVertexBuffer(kNumVerts, kBU_Default, UIAccessVertexDecl(), kBP_Auto, &m_pVB) == S_OK)
      {
         float w = m_size.width;
         float h = m_size.height / 4;

         sUIVertex verts[kNumVerts];

         for (int i = 0; i < _countof(verts); i++)
         {
            verts[i].r = verts[i].g = verts[i].b = verts[i].a = 255;
         }

         // steady state
         verts[0].u = 0;
         verts[0].v = 0.5;
         verts[0].pos = tVec3(0,0,0);
         verts[1].u = 0;
         verts[1].v = 0.25;
         verts[1].pos = tVec3(0,h,0);
         verts[2].u = 1;
         verts[2].v = 0.25;
         verts[2].pos = tVec3(w,h,0);
         verts[3].u = 1;
         verts[3].v = 0.5;
         verts[3].pos = tVec3(w,0,0);

         // hovered
         verts[4].u = 0;
         verts[4].v = 0.75;
         verts[4].pos = tVec3(0,0,0);
         verts[5].u = 0;
         verts[5].v = 0.5;
         verts[5].pos = tVec3(0,h,0);
         verts[6].u = 1;
         verts[6].v = 0.5;
         verts[6].pos = tVec3(w,h,0);
         verts[7].u = 1;
         verts[7].v = 0.75;
         verts[7].pos = tVec3(w,0,0);

         // pressed
         verts[8].u = 0;
         verts[8].v = 1;
         verts[8].pos = tVec3(0,0,0);
         verts[9].u = 0;
         verts[9].v = 0.75;
         verts[9].pos = tVec3(0,h,0);
         verts[10].u = 1;
         verts[10].v = 0.75;
         verts[10].pos = tVec3(w,h,0);
         verts[11].u = 1;
         verts[11].v = 1;
         verts[11].pos = tVec3(w,0,0);

         // disabled
         verts[12].u = 0;
         verts[12].v = 0.25;
         verts[12].pos = tVec3(0,0,0);
         verts[13].u = 0;
         verts[13].v = 0;
         verts[13].pos = tVec3(0,h,0);
         verts[14].u = 1;
         verts[14].v = 0;
         verts[14].pos = tVec3(w,h,0);
         verts[15].u = 1;
         verts[15].v = 0.25;
         verts[15].pos = tVec3(w,0,0);

         void * pVertexData;
         if (m_pVB->Lock(kBL_Discard, &pVertexData) == S_OK)
         {
            memcpy(pVertexData, verts, sizeof(verts));
            m_pVB->Unlock();
         }
      }
   }
   // return true unconditionally so that this component will
   // be a placeholder at least if the image fails to load
   return true;
}


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cUIEdit
//

///////////////////////////////////////

cUIEdit::cUIEdit()
 : m_size(kDefaultSize),
   m_selection(&m_text),
   m_timeLastBlink(0),
   m_bCursorBlinkOn(true),
   m_bCursorForceOn(false)
{
}

///////////////////////////////////////

cUIEdit::~cUIEdit()
{
}

///////////////////////////////////////

void cUIEdit::Render(IRenderDevice * pRenderDevice)
{
   cUIRect rect = GetScreenRect();

   UIDraw3dRect(rect, g_3dEdge, AccessStyle()->GetShadow(), AccessStyle()->GetFace(), AccessStyle()->GetBackground());

   rect.left += g_3dEdge + kHorzInset;
   rect.top += kVertInset;
   rect.right -= g_3dEdge + kHorzInset;
   rect.bottom -= kVertInset;

   UIPushClipRect(rect);

   static const float kCursorWidth = 1.f;

//   cUISize size = UIMeasureText(m_text.c_str(), m_selection.GetCursorIndex(), kTextNoClip, AccessStyle()->GetFont());
   cUISize size = UIMeasureText(m_text.c_str(), m_selection.GetCursorIndex(), AccessStyle()->AccessFont());
   if (size.width >= rect.GetWidth())
   {
      rect.left -= size.width - rect.GetWidth() + kCursorWidth;
   }

   // don't do clipping in the UIDrawText call because it's already done (above)
   UIDrawText(m_text.c_str(), m_text.length(), &rect, kTextNoClip, AccessStyle()->AccessFont(), AccessStyle()->GetText());

   // render the cursor if this widget has focus and it's 'on' in its blink cycle
   if (IsFocussed() && (m_bCursorBlinkOn || m_bCursorForceOn))
   {
      cUISize textSize = UIMeasureText(m_text.c_str(), m_selection.GetCursorIndex(), AccessStyle()->AccessFont());

      cUIRect cursorRect(
         rect.left + textSize.width,
         rect.top + kCursorWidth,
         rect.left + textSize.width + kCursorWidth,
         rect.bottom - kCursorWidth);

      static const cUIColor BLACK(0,0,0,1);

      UIDrawSolidRect(cursorRect, BLACK);
   }

   static const int kCursorBlinkFreq = 2; // blink this many times per second
   static const float kCursorBlinkPeriod = 1.f / kCursorBlinkFreq;

   float time = TimeGetSecs();

   if (m_timeLastBlink == 0)
   {
      m_timeLastBlink = time;
   }
   else if ((time - m_timeLastBlink) > kCursorBlinkPeriod)
   {
      m_bCursorBlinkOn = !m_bCursorBlinkOn;

      m_timeLastBlink = time;
   }

   UIPopClipRect();
}

///////////////////////////////////////

bool cUIEdit::OnEvent(const cUIEvent * pEvent)
{
   if (pEvent->code == kEventMouseDown)
   {
      m_selection.SetCursorFromPoint(pEvent->mousePos - GetScreenRect().GetTopLeft());
   }
   else if (pEvent->code == kEventKeyUp)
   {
      m_bCursorForceOn = false;
   }
   else if (pEvent->code == kEventKeyDown)
   {
      // attempt to keep the cursor visible if the user is going to
      // hold down any key for a number of repeat characters
      m_bCursorForceOn = true;

      switch (pEvent->keyCode)
      {
         case kBackspace:
         {
            m_selection.Backspace();
            break;
         }
         case kDelete:
         {
            m_selection.Delete();
            break;
         }
         case kLeft:
         {
            UseGlobal(Input);
            if (pInput->KeyIsDown(kCtrl))
            {
               m_selection.WordLeft();
            }
            else
            {
               m_selection.CharLeft();
            }
            break;
         }
         case kRight:
         {
            UseGlobal(Input);
            if (pInput->KeyIsDown(kCtrl))
            {
               m_selection.WordRight();               
            }
            else
            {
               m_selection.CharRight();
            }
            break;
         }
         case kHome:
         {
            m_selection.Start();
            break;
         }
         case kEnd:
         {
            m_selection.End();
            break;
         }
         case kEscape:
         {
            // don't eat escape keypresses
            return false;
         }
         default:
         {
            if (isprint(pEvent->keyCode))
            {
               m_selection.ReplaceSel((char)pEvent->keyCode);
            }
            break;
         }
      }

      return true;
   }

   return cUIWidget::OnEvent(pEvent);
}

///////////////////////////////////////

cUISize cUIEdit::GetPreferredSize() const
{
   char * psz = reinterpret_cast<char *>(alloca(m_size * sizeof(char)));
   memset(psz, 'M', m_size);
   cUISize size;
   AccessStyle()->AccessFont()->MeasureText(psz, m_size, &size.width, &size.height);
   size.width += kHorzInset * 2;
   size.height += kVertInset * 2;
   return size;
}

///////////////////////////////////////////////////////////////////////////////
