///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_GUIAPI_H
#define INCLUDED_GUIAPI_H

/// @file guiapi.h
/// Interface definitions for graphical user interface elements plus the 
/// core system itself

#include "enginedll.h"

#include "comtools.h"

#include "guitypes.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IGUIElement);
F_DECLARE_INTERFACE(IGUIStyle);
F_DECLARE_INTERFACE(IGUIFont);
F_DECLARE_INTERFACE(IGUIElementFactory);
F_DECLARE_INTERFACE(IGUIElementRenderer);
F_DECLARE_INTERFACE(IGUIElementRendererFactory);
F_DECLARE_INTERFACE(IGUIElementEnum);
F_DECLARE_INTERFACE(IGUIEvent);
F_DECLARE_INTERFACE(IGUIContainerElement);
F_DECLARE_INTERFACE(IGUILayoutManager);
F_DECLARE_INTERFACE(IGUIGridLayoutManager);
F_DECLARE_INTERFACE(IGUIFlowLayoutManager);
F_DECLARE_INTERFACE(IGUIPanelElement);
F_DECLARE_INTERFACE(IGUIDialogElement);
F_DECLARE_INTERFACE(IGUIButtonElement);
F_DECLARE_INTERFACE(IGUILabelElement);
F_DECLARE_INTERFACE(IGUITextEditElement);
F_DECLARE_INTERFACE(IGUIEventRouter);
F_DECLARE_INTERFACE(IGUIEventListener);
F_DECLARE_INTERFACE(IGUIFactory);
F_DECLARE_INTERFACE(IGUIContext);

class TiXmlElement;
class cGUIFontDesc;

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElement
//
/// @interface IGUIElement
/// @brief The base GUI element interface provides access to properties and 
/// methods common to all GUI widgets.

interface IGUIElement : IUnknown
{
   /// @return The semi-unique identifier assigned to this element
   virtual tResult GetId(tGUIString * pId) const = 0;
   virtual void SetId(const char * pszId) = 0;

   /// @return A boolean value representing whether the element presently has input focus
   virtual bool HasFocus() const = 0;
   /// @brief Called by the GUI system to tell the element that it has input 
   /// focus and can render an indicator of that if desired
   /// @internal
   /// @remarks Calling this function will not actually obtain input focus.
   virtual void SetFocus(bool bFocus) = 0;

   /// @return A boolean value representing whether the mouse is presently over the element
   virtual bool IsMouseOver() const = 0;
   /// @brief Called by the GUI system to tell the element that the mouse is 
   /// presently over it
   /// @internal
   virtual void SetMouseOver(bool bMouseOver) = 0;

   virtual bool IsVisible() const = 0;
   virtual void SetVisible(bool bVisible) = 0;

   virtual bool IsEnabled() const = 0;
   virtual void SetEnabled(bool bEnabled) = 0;

   virtual tResult GetParent(IGUIElement * * ppParent) = 0;
   virtual tResult SetParent(IGUIElement * pParent) = 0;

   /// @return The element's position relative to it's parent element
   /// @remarks Use GUIElementAbsolutePosition to calculate the element's 
   /// position in screen coordinates.
   /// @see GUIElementAbsolutePosition
   virtual tGUIPoint GetPosition() const = 0;
   /// @internal
   /// @remarks Element positions are set by the GUI system using parameters 
   /// from the style along with context information from the parent 
   /// element, if any. Use caution when calling SetPosition directly.
   virtual void SetPosition(const tGUIPoint & point) = 0;

   virtual tGUISize GetSize() const = 0;
   virtual void SetSize(const tGUISize & size) = 0;

   virtual bool Contains(const tGUIPoint & point) const = 0;

   virtual tResult OnEvent(IGUIEvent * pEvent) = 0;

   /// @brief The renderer "class" names the type of GUI element renderer object
   /// that will be instantiated for this element by default.
   /// @return The name of the default renderer for this type of GUI element
   /// @remarks Override the default renderer by instantiating an alternative
   /// one and using SetRenderer to replace the default.
   /// @see IGUIElementRenderer
   virtual tResult GetRendererClass(tGUIString * pRendererClass) = 0;
   virtual tResult SetRendererClass(const tChar * pszRendererClass) = 0;

   virtual tResult GetRenderer(IGUIElementRenderer * * ppRenderer) = 0;
   virtual tResult SetRenderer(IGUIElementRenderer * pRenderer) = 0;

   virtual tResult GetStyle(IGUIStyle * * ppStyle) = 0;
   virtual tResult SetStyle(IGUIStyle * pStyle) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIStyle
//

enum eGUIAlignment
{
   kGUIAlignLeft,
   kGUIAlignRight,
   kGUIAlignCenter,
};

enum eGUIVerticalAlignment
{
   kGUIVertAlignTop,
   kGUIVertAlignBottom,
   kGUIVertAlignCenter,
};

enum eGUIDimensionSpec
{
   kGUIDimensionPixels,
   kGUIDimensionPercent,
};

interface IGUIStyle : IUnknown
{
   virtual tResult GetAttribute(const char * pszAttribute, tGUIString * pValue) = 0;
   virtual tResult GetAttribute(const char * pszAttribute, uint * pValue) = 0;
   virtual tResult GetAttribute(const char * pszAttribute, tGUIColor * pValue) = 0;
   virtual tResult SetAttribute(const char * pszAttribute, const char * pszValue) = 0;

   virtual tResult GetAlignment(uint * pAlignment) = 0;
   virtual tResult SetAlignment(uint alignment) = 0;

   virtual tResult GetVerticalAlignment(uint * pAlignment) = 0;
   virtual tResult SetVerticalAlignment(uint alignment) = 0;

   virtual tResult GetBackgroundColor(tGUIColor * pColor) = 0;
   virtual tResult SetBackgroundColor(const tGUIColor & color) = 0;

   virtual tResult GetForegroundColor(tGUIColor * pColor) = 0;
   virtual tResult SetForegroundColor(const tGUIColor & color) = 0;

   virtual tResult GetTextAlignment(uint * pAlignment) = 0;
   virtual tResult SetTextAlignment(uint alignment) = 0;

   virtual tResult GetTextVerticalAlignment(uint * pAlignment) = 0;
   virtual tResult SetTextVerticalAlignment(uint alignment) = 0;

   virtual tResult GetFontName(tGUIString * pFontName) = 0;
   virtual tResult SetFontName(const char * pszFontName) = 0;

   virtual tResult GetFontPointSize(uint * pFontPointSize) = 0;
   virtual tResult SetFontPointSize(uint fontPointSize) = 0;

   virtual tResult GetFontBold(uint * pB) = 0;
   virtual tResult SetFontBold(bool b) = 0;

   virtual tResult GetFontItalic(uint * pB) = 0;
   virtual tResult SetFontItalic(bool b) = 0;

   virtual tResult GetFontShadow(uint * pB) = 0;
   virtual tResult SetFontShadow(bool b) = 0;

   virtual tResult GetFontOutline(uint * pB) = 0;
   virtual tResult SetFontOutline(bool b) = 0;

   virtual tResult GetFontDesc(cGUIFontDesc * pFontDesc) = 0;
   virtual tResult GetFont(IGUIFont * * ppFont) = 0;

   virtual tResult GetWidth(uint * pWidth, uint * pSpec) = 0;
   virtual tResult SetWidth(uint width, uint spec) = 0;

   virtual tResult GetHeight(uint * pHeight, uint * pSpec) = 0;
   virtual tResult SetHeight(uint height, uint spec) = 0;
};

///////////////////////////////////////
/// Attempt to parse a color value from the given string. Valid strings are
/// of the form "(R,G,B,A)". The alpha component is optional. Color components 
/// can be in the range [0,255] or [0,1]. Certain standard colors are 
/// supported, too. For example, "red", "green", "blue", etc.

ENGINE_API tResult GUIStyleParseColor(const char * psz, tGUIColor * pColor);

///////////////////////////////////////
/// Parse a CSS-like string to produce a GUI style object.

ENGINE_API tResult GUIStyleParse(const char * pszStyle, IGUIStyle * * ppStyle);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFont
//

enum eGUIFontRenderTextFlags
{
   kRT_Center        = (1<<0),
   kRT_VCenter       = (1<<1),
   kRT_NoClip        = (1<<2),
   kRT_CalcRect      = (1<<3),
   kRT_SingleLine    = (1<<4),
   kRT_Bottom        = (1<<5),
   kRT_NoBlend       = (1<<6),
   kRT_DropShadow    = (1<<7),
};

interface IGUIFont : IUnknown
{
   virtual tResult RenderText(const char * pszText, int textLength, tRect * pRect,
                              uint flags, const cColor & color) const = 0;
   virtual tResult RenderText(const wchar_t * pszText, int textLength, tRect * pRect,
                              uint flags, const cColor & color) const = 0;
};

///////////////////////////////////////

ENGINE_API tResult GUIFontCreate(const cGUIFontDesc & fontDesc, IGUIFont * * ppFont);

ENGINE_API tResult GUIFontGetDefault(IGUIFont * * ppFont);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElementFactory
//

interface IGUIElementFactory : IUnknown
{
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElementRenderer
//

interface IGUIElementRenderer : IUnknown
{
   virtual tResult Render(IGUIElement * pElement) = 0;

   virtual tGUISize GetPreferredSize(IGUIElement * pElement) = 0;

   virtual tResult GetFont(IGUIElement * pElement, IGUIFont * * ppFont) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElementRendererFactory
//

interface IGUIElementRendererFactory : IUnknown
{
   virtual tResult CreateRenderer(IGUIElement * pElement, IGUIElementRenderer * * ppRenderer) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIElementEnum
//

interface IGUIElementEnum : IUnknown
{
   virtual tResult Next(ulong count, IGUIElement * * ppElements, ulong * pnElements) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(IGUIElementEnum * * ppEnum) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIEvent
//

enum eGUIEventCode
{
   kGUIEventNone,
   kGUIEventFocus,
   kGUIEventBlur,
   kGUIEventDestroy,
   kGUIEventMouseMove,
   kGUIEventMouseEnter,
   kGUIEventMouseLeave,
   kGUIEventMouseUp,
   kGUIEventMouseDown,
   kGUIEventMouseWheelUp,
   kGUIEventMouseWheelDown,
   kGUIEventKeyUp,
   kGUIEventKeyDown,
   kGUIEventClick,
   kGUIEventHover,
   kGUIEventDragStart,
   kGUIEventDragEnd,
   kGUIEventDragMove,
   kGUIEventDragOver,
   kGUIEventDrop,
};

typedef enum eGUIEventCode tGUIEventCode;

interface IGUIEvent : IUnknown
{
   virtual tResult GetEventCode(tGUIEventCode * pEventCode) = 0;
   virtual tResult GetMousePosition(tGUIPoint * pMousePos) = 0;
   virtual tResult GetKeyCode(long * pKeyCode) = 0;
   virtual tResult GetSourceElement(IGUIElement * * ppElement) = 0;

   virtual bool IsCancellable() const = 0;

   virtual tResult GetCancelBubble() = 0;
   virtual tResult SetCancelBubble(bool bCancel) = 0;
};

///////////////////////////////////////

ENGINE_API tResult GUIEventCreate(tGUIEventCode eventCode, 
                                  tGUIPoint mousePos, 
                                  long keyCode, 
                                  IGUIElement * pSource, 
                                  bool bCancellable, 
                                  IGUIEvent * * ppEvent);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIContainerElement
//

interface IGUIContainerElement : IGUIElement
{
   virtual tResult AddElement(IGUIElement * pElement) = 0;
   virtual tResult RemoveElement(IGUIElement * pElement) = 0;
   virtual tResult GetElements(IGUIElementEnum * * ppElements) = 0;
   virtual tResult HasElement(IGUIElement * pElement) const = 0;
   virtual tResult GetElement(const tChar * pszId, IGUIElement * * ppElement) const = 0;

   virtual tResult GetLayout(IGUILayoutManager * * ppLayout) = 0;
   virtual tResult SetLayout(IGUILayoutManager * pLayout) = 0;

   virtual tResult GetInsets(tGUIInsets * pInsets) = 0;
   virtual tResult SetInsets(const tGUIInsets & insets) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUILayoutManager
//

interface IGUILayoutManager : IUnknown
{
   virtual tResult Layout(IGUIContainerElement * pContainer) = 0;
   virtual tResult GetPreferredSize(IGUIContainerElement * pContainer, tGUISize * pSize) = 0;

   virtual tResult GetHGap(uint * pHGap) = 0;
   virtual tResult SetHGap(uint hGap) = 0;

   virtual tResult GetVGap(uint * pVGap) = 0;
   virtual tResult SetVGap(uint vGap) = 0;
};

///////////////////////////////////////
// Create a layout manager from an XML fragment

ENGINE_API tResult GUILayoutManagerCreate(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout);

///////////////////////////////////////

typedef tResult (* tGUILayoutManagerFactoryFn)(const TiXmlElement * pXmlElement, IGUILayoutManager * * ppLayout);
ENGINE_API tResult GUILayoutManagerRegister(const tChar * pszName, tGUILayoutManagerFactoryFn pfn);
ENGINE_API void GUILayoutManagerRegisterBuiltInTypes();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIGridLayoutManager
//

interface IGUIGridLayoutManager : IGUILayoutManager
{
   virtual tResult GetColumns(uint * pColumns) = 0;
   virtual tResult SetColumns(uint columns) = 0;

   virtual tResult GetRows(uint * pRows) = 0;
   virtual tResult SetRows(uint rows) = 0;
};

///////////////////////////////////////

ENGINE_API tResult GUIGridLayoutManagerCreate(IGUIGridLayoutManager * * ppLayout);
ENGINE_API tResult GUIGridLayoutManagerCreate(uint columns, uint rows, IGUIGridLayoutManager * * ppLayout);
ENGINE_API tResult GUIGridLayoutManagerCreate(uint columns, uint rows, uint hGap, uint vGap, IGUIGridLayoutManager * * ppLayout);


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFlowLayoutManager
//

interface IGUIFlowLayoutManager : IGUILayoutManager
{
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIPanelElement
//

interface IGUIPanelElement : IGUIContainerElement
{
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIDialogElement
//

interface IGUIDialogElement : IGUIContainerElement
{
   virtual tResult GetTitle(tGUIString * pTitle) = 0;
   virtual tResult SetTitle(const char * pszTitle) = 0;

   virtual tResult GetCaptionHeight(uint * pHeight) = 0;
   virtual tResult SetCaptionHeight(uint height) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIButtonElement
//

interface IGUIButtonElement : IGUIElement
{
   virtual bool IsArmed() const = 0;
   virtual void SetArmed(bool bArmed) = 0;

   virtual tResult GetText(tGUIString * pText) const = 0;
   virtual tResult SetText(const char * pszText) = 0;

   virtual tResult GetOnClick(tGUIString * pOnClick) const = 0;
   virtual tResult SetOnClick(const char * pszOnClick) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUILabelElement
//

interface IGUILabelElement : IGUIElement
{
   virtual tResult GetText(tGUIString * pText) = 0;
   virtual tResult SetText(const char * pszText) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUITextEditElement
//

interface IGUITextEditElement : IGUIElement
{
   // The number of characters that the element will be sized to accomodate
   virtual tResult GetEditSize(uint * pEditSize) = 0;
   virtual tResult SetEditSize(uint editSize) = 0;

   virtual tResult GetSelection(uint * pStart, uint * pEnd) = 0;
   virtual tResult SetSelection(uint start, uint end) = 0;

   virtual const char * GetText() const = 0;
   virtual void SetText(const char * pszText) = 0;

   virtual void UpdateBlinkingCursor() = 0;
   virtual bool ShowBlinkingCursor() const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIEventRouter
//

interface IGUIEventRouter : IUnknown
{
   virtual tResult AddEventListener(IGUIEventListener * pListener) = 0;
   virtual tResult RemoveEventListener(IGUIEventListener * pListener) = 0;

   virtual tResult GetFocus(IGUIElement * * ppElement) = 0;
   virtual tResult SetFocus(IGUIElement * pElement) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIEventListener
//

interface IGUIEventListener : IUnknown
{
   virtual tResult OnEvent(IGUIEvent * pEvent) = 0;
};

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIFactory
//

interface IGUIFactory : IUnknown
{
   virtual tResult CreateElement(const TiXmlElement * pXmlElement, IGUIElement * * ppElement) = 0;

   virtual tResult CreateRenderer(const tChar * pszRendererClass, IGUIElement * pElement, IGUIElementRenderer * * ppRenderer) = 0;

   virtual tResult RegisterElementFactory(const char * pszType, IGUIElementFactory * pFactory) = 0;
   virtual tResult RevokeElementFactory(const char * pszType) = 0;

   virtual tResult RegisterElementRendererFactory(const char * pszRenderer, IGUIElementRendererFactory * pFactory) = 0;
   virtual tResult RevokeElementRendererFactory(const char * pszRenderer) = 0;
};

///////////////////////////////////////

ENGINE_API void GUIFactoryCreate();

///////////////////////////////////////

tResult RegisterGUIElementFactory(const char * pszType, IGUIElementFactory * pFactory);

struct sAutoRegisterGUIElementFactory
{
   sAutoRegisterGUIElementFactory(const char * pszType, IGUIElementFactory * pFactory)
   {
      RegisterGUIElementFactory(pszType, pFactory);
      SafeRelease(pFactory);
   }
};

#define REFERENCE_GUIELEMENTFACTORY(type) \
   extern IGUIElementFactory * Make##type##ElementFactory(); \
   void * MAKE_UNIQUE(g_pRefSym##type) = (void *)&Make##type##ElementFactory

#define AUTOREGISTER_GUIELEMENTFACTORY(type, factoryClass) \
   IGUIElementFactory * Make##type##ElementFactory() \
   { return static_cast<IGUIElementFactory *>(new (factoryClass)); } \
   static sAutoRegisterGUIElementFactory g_auto##type##Element(#type, Make##type##ElementFactory())

///////////////////////////////////////

tResult RegisterGUIElementRendererFactory(const char * pszRenderer, IGUIElementRendererFactory * pFactory);

struct sAutoRegisterGUIElementRendererFactory
{
   sAutoRegisterGUIElementRendererFactory(const char * pszRenderer, IGUIElementRendererFactory * pFactory)
   {
      RegisterGUIElementRendererFactory(pszRenderer, pFactory);
      SafeRelease(pFactory);
   }
};

#define REFERENCE_GUIELEMENTRENDERERFACTORY(renderer) \
   extern IGUIElementRendererFactory * Make##renderer##RendererFactory(); \
   void * MAKE_UNIQUE(g_pRefSym##renderer) = (void *)&Make##renderer##RendererFactory

#define AUTOREGISTER_GUIELEMENTRENDERERFACTORY(renderer, factoryClass) \
   IGUIElementRendererFactory * Make##renderer##RendererFactory() \
   { return static_cast<IGUIElementRendererFactory *>(new (factoryClass)); } \
   static sAutoRegisterGUIElementRendererFactory g_auto##renderer##Renderer(#renderer, Make##renderer##RendererFactory())

///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IGUIContext
//

interface IGUIContext : IGUIEventRouter
{
   // TODO: needed only for the hacky dialog box implementation
   virtual tResult RemoveElement(IGUIElement * pElement) = 0;

   virtual tResult GetElement(const tChar * pszId, IGUIElement * * ppElement) = 0;

   virtual tResult ShowModalDialog(const tChar * pszDialog) = 0;

   /// @brief Loads GUI elements from either an XML fragment or file
   /// @param pszXmlStringOrFile specifies either an XML fragment or file name
   /// @param bVisible determines whether the loaded elements will be intially visible
   /// @return S_OK, S_FALSE if no error and no elements loaded, or an E_xxx code
   virtual tResult LoadElements(const char * pszXmlStringOrFile, bool bVisible) = 0;

   virtual void ClearGUI() = 0;

   virtual tResult RenderGUI() = 0;

   virtual tResult ShowDebugInfo(const tGUIPoint & placement, const tGUIColor & textColor) = 0;
   virtual tResult HideDebugInfo() = 0;
};

///////////////////////////////////////

ENGINE_API void GUIContextCreate();

///////////////////////////////////////////////////////////////////////////////

ENGINE_API void GlRenderBevelledRect(const tGUIRect & rect, int bevel, const tGUIColor & topLeft,
                                     const tGUIColor & bottomRight, const tGUIColor & face);

ENGINE_API void GlBegin2D();
ENGINE_API void GlEnd2D();

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_GUIAPI_H
