///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_EDITORDOCCMDS_H
#define INCLUDED_EDITORDOCCMDS_H

#include "EditorDocument.h"

#include "tech/comtools.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IEntity);

F_DECLARE_HANDLE(HTERRAINVERTEX);

namespace ManagedEditor
{
   generic <typename T>
   ref class XYZ sealed
   {
   public:
      XYZ(T x, T y, T z)
      {
         X = x;
         Y = y;
         Z = z;
      }

      property T X;
      property T Y;
      property T Z;
   };

   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: SelectCommand
   //

   ref class SelectCommand sealed : public EditorDocumentCommand
   {
   public:
      SelectCommand();
      SelectCommand(XYZ<float> ^ rayOrigin, XYZ<float> ^ rayDirection);
      ~SelectCommand();
      !SelectCommand();

      virtual void Do() override;
      virtual bool CanUndo() override;
      virtual void Undo() override;
      property System::String ^ Label
      {
         virtual System::String ^ get() override;
      }

   private:
      XYZ<float> ^ m_rayOrigin;
      XYZ<float> ^ m_rayDirection;
      IEnumEntities * m_pOldSelection;
   };


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: PlaceEntityCommand
   //

   ref class PlaceEntityCommand sealed : public EditorDocumentCommand
   {
   public:
      PlaceEntityCommand(System::String ^ entity, float nx, float nz);

      virtual void Do() override;
      virtual bool CanUndo() override;
      virtual void Undo() override;
      property System::String ^ Label
      {
         virtual System::String ^ get() override;
      }

   private:
      System::String ^ m_entity;
      float m_nx, m_nz;
      ulong m_placedEntityId;
   };


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: ChangeTerrainElevationCommand
   //

   ref class ChangeTerrainElevationCommand : public EditorDocumentCommand
   {
   public:
      ChangeTerrainElevationCommand(HTERRAINVERTEX hVertex, float elevDelta);

      virtual void Do() override;
      virtual bool CanUndo() override;
      virtual void Undo() override;
      property System::String ^ Label
      {
         virtual System::String ^ get() override;
      }

   private:
      HTERRAINVERTEX m_hVertex;
      float m_elevDelta, m_oldElevation;
   };


   /////////////////////////////////////////////////////////////////////////////
   //
   // CLASS: SetTerrainElevationCommand
   //

   ref class SetTerrainElevationCommand : public EditorDocumentCommand
   {
   public:
      SetTerrainElevationCommand(HTERRAINVERTEX hVertex, float elevation);

      virtual void Do() override;
      virtual bool CanUndo() override;
      virtual void Undo() override;
      property System::String ^ Label
      {
         virtual System::String ^ get() override;
      }

   private:
      HTERRAINVERTEX m_hVertex;
      float m_elevation, m_oldElevation;
   };

} // namespace ManagedEditor

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_EDITORDOCCMDS_H
