///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_D3DGUIRENDER_H
#define INCLUDED_D3DGUIRENDER_H

#include "guiapi.h"

#include <stack>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IDirect3DDevice9);

////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cGUIRenderDeviceD3D
//

class cGUIRenderDeviceD3D : public cComObject<IMPLEMENTS(IGUIRenderDeviceContext)>
{
   cGUIRenderDeviceD3D(const cGUIRenderDeviceD3D &);
   void operator =(const cGUIRenderDeviceD3D &);

public:
   cGUIRenderDeviceD3D(IDirect3DDevice9 * pD3dDevice);
   ~cGUIRenderDeviceD3D();

   virtual void PushScissorRect(const tGUIRect & rect);
   virtual void PopScissorRect();

   virtual void RenderSolidRect(const tGUIRect & rect, const tGUIColor & color);
   virtual void RenderBeveledRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                                  const tGUIColor & bottomRight, const tGUIColor & face);

   virtual void FlushQueue();

   virtual void Begin2D();
   virtual void End2D();

private:
   cAutoIPtr<IDirect3DDevice9> m_pD3dDevice;
   typedef std::stack<tGUIRect> tGUIRectStack;
   tGUIRectStack m_scissorRectStack;
   int m_viewport[4];
};

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_D3DGUIRENDER_H
