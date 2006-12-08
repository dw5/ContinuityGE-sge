///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORAPPFORM_H
#define INCLUDED_EDITORAPPFORM_H

#include "EditorDocument.h"
#include "EditorForm.h"
#include "EditorTools.h"
#include "GlControl.h"
#include "ToolPalette.h"

#include "tech/comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

///////////////////////////////////////////////////////////////////////////////
//
// NAMESPACE: ManagedEditor
//

namespace ManagedEditor
{

   ///////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: EditorAppForm
   //

   ref class EditorAppForm sealed : public EditorForm
   {
   public:
      EditorAppForm();
      ~EditorAppForm();

      void OnIdle(System::Object ^ sender, System::EventArgs ^ e);

      void OnToolSelect(System::Object ^ sender, ToolSelectEventArgs ^ e);

      virtual void OnResize(System::EventArgs ^ e) override;

      void glControl_OnMouseDown(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void glControl_OnMouseUp(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void glControl_OnMouseClick(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void glControl_OnMouseMove(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);
      void glControl_OnMouseHover(System::Object ^ sender, System::EventArgs ^ e);
      void glControl_OnMouseWheel(System::Object ^ sender, System::Windows::Forms::MouseEventArgs ^ e);

   protected:
      virtual void NewDocument() override;
      virtual void OpenDocument() override;
      virtual void SaveDocument() override;
      virtual void SaveDocumentAs() override;

      void CreateEditorTools();

   private:
      GlControl ^ m_glControl;
      ToolPalette ^ m_toolPalette;
      System::Windows::Forms::PropertyGrid ^ m_propertyGrid;

      EditorDocument ^ m_document;
   };

} // namespace ManagedEditor

#endif // INCLUDED_EDITORAPPFORM_H
