///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITYAPI_H
#define INCLUDED_ENTITYAPI_H

#include "enginedll.h"
#include "comtools.h"
#include "matrix4.h"
#include "techstring.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

F_DECLARE_INTERFACE(IEntity);
F_DECLARE_INTERFACE(IEntityComponent);
F_DECLARE_INTERFACE(IEntitySpawnComponent);
F_DECLARE_INTERFACE(IEntityEnum);
F_DECLARE_INTERFACE(IEntityManager);
F_DECLARE_INTERFACE(IEntityManagerListener);

class cRay;
class TiXmlElement;

F_DECLARE_INTERFACE(IDictionary);

template <typename T> class cAxisAlignedBox;
typedef class cAxisAlignedBox<float> tAxisAlignedBox;


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntity
//

typedef ulong tEntityId;

////////////////////////////////////////

enum eEntityFlags
{
   kEF_None                   = 0,
   kEF_Hidden                 = (1 << 0),    // Don't render
   kEF_Disabled               = (1 << 1),    // Don't update
   kEF_All                    = 0xFFFFFFFF,
};

////////////////////////////////////////

interface IEntity : IUnknown
{
   virtual tEntityId GetId() const = 0;

   virtual uint GetFlags() const = 0;
   virtual uint SetFlags(uint flags, uint mask) = 0;

   virtual tResult GetModel(cStr * pModel) const = 0;

   virtual tResult GetPosition(tVec3 * pPosition) const = 0;

   virtual const tMatrix4 & GetWorldTransform() const = 0;

   virtual const tAxisAlignedBox & GetBoundingBox() const = 0;

   virtual void Update(double elapsedTime) = 0;
   virtual void Render() = 0;

   virtual tResult AddComponent(REFGUID guid, IEntityComponent * pComponent) = 0;
   virtual tResult FindComponent(REFGUID guid, IEntityComponent * * ppComponent) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityComponent
//

interface IEntityComponent : IUnknown
{
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntitySpawnComponent
//

interface IEntitySpawnComponent : IEntityComponent
{
   virtual uint GetQueueSize() const = 0;

   virtual tResult SetRallyPoint(const tVec3 & rallyPoint) = 0;
   virtual tResult GetRallyPoint(tVec3 * pRallyPoint) const = 0;

   virtual tResult Spawn(const tChar * pszEntity) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityEnum
//

interface IEntityEnum : IUnknown
{
   virtual tResult Next(ulong count, IEntity * * ppEntities, ulong * pnEntities) = 0;
   virtual tResult Skip(ulong count) = 0;
   virtual tResult Reset() = 0;
   virtual tResult Clone(IEntityEnum * * ppEnum) = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityManager
//

typedef tResult (* tEntityComponentFactoryFn)(const TiXmlElement * pTiXmlElement,
                                              IEntity * pEntity, IEntityComponent * * ppComponent);

interface IEntityManager : IUnknown
{
   virtual tResult AddEntityManagerListener(IEntityManagerListener * pListener) = 0;
   virtual tResult RemoveEntityManagerListener(IEntityManagerListener * pListener) = 0;

   virtual tResult SpawnEntity(const tChar * pszEntity, const tVec3 & position) = 0;

   virtual tResult RemoveEntity(IEntity * pEntity) = 0;
   virtual void RemoveAll() = 0;

   virtual void RenderAll() = 0;

   virtual tResult RayCast(const cRay & ray, IEntity * * ppEntity) const = 0;
   virtual tResult BoxCast(const tAxisAlignedBox & box, IEntityEnum * * ppEnum) const = 0;

   virtual tResult Select(IEntity * pEntity) = 0;
   virtual tResult SelectBoxed(const tAxisAlignedBox & box) = 0;
   virtual tResult DeselectAll() = 0;
   virtual uint GetSelectedCount() const = 0;
   virtual tResult GetSelected(IEntityEnum * * ppEnum) const = 0;

   virtual tResult RegisterComponentFactory(const tChar * pszComponent,
                                            tEntityComponentFactoryFn pfnFactory) = 0;
   virtual tResult RevokeComponentFactory(const tChar * pszComponent) = 0;
   virtual tResult CreateComponent(const TiXmlElement * pTiXmlElement, IEntity * pEntity,
                                   IEntityComponent * * ppComponent) = 0;
};

////////////////////////////////////////

ENGINE_API tResult EntityManagerCreate();


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityManagerListener
//

interface IEntityManagerListener : IUnknown
{
   virtual void OnEntitySelectionChange() = 0;
};


///////////////////////////////////////////////////////////////////////////////
//
// INTERFACE: IEntityDefinition
//

interface IEntityDefinition : IUnknown
{
   virtual tResult CreateInstance(IEntity * * ppEntity) = 0;
};


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITYAPI_H
