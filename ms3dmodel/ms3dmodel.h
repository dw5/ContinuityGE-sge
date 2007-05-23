///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MS3DMODEL_H
#define INCLUDED_MS3DMODEL_H

#include "tech/comtools.h"

#include "ms3dgroup.h"
#include "ms3dmaterial.h"
#include "ms3dtriangle.h"
#include "ms3dvertex.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IReader);

///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cMs3dModel
//

class cMs3dModel
{
   cMs3dModel(const cMs3dModel &);
   const cMs3dModel & operator =(const cMs3dModel &);

public:
   cMs3dModel();
   ~cMs3dModel();

   void * Read(IReader * pReader);

   static void * Load(IReader * pReader);
   static void Unload(void * pData);

private:
   std::vector<cMs3dVertex> ms3dVerts;
   std::vector<cMs3dTriangle> ms3dTris;
   std::vector<cMs3dGroup> ms3dGroups;
   std::vector<cMs3dMaterial> ms3dMaterials;
};

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MS3DMODEL_H
