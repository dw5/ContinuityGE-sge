////////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_ENTITY_H
#define INCLUDED_ENTITY_H

#include "engine/entityapi.h"

#include "tech/techstring.h"

#include <map>

#ifdef _MSC_VER
#pragma once
#endif


////////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntity
//

class cEntity : public cComObject<IMPLEMENTS(IEntity)>
{
public:
   cEntity(const tChar * pszTypeName, tEntityId id);
   ~cEntity();

   virtual tResult GetTypeName(cStr * pTypeName) const;

   virtual tEntityId GetId() const;

   virtual tResult SetComponent(tEntityComponentID cid, IEntityComponent * pComponent);
   virtual tResult GetComponent(tEntityComponentID cid, IEntityComponent * * ppComponent);

   virtual tResult RemoveComponent(tEntityComponentID cid);

   virtual tResult EnumComponents(REFGUID iid, IEnumEntityComponents * * ppEnum);

private:
   void RemoveAllComponents();

   cStr m_typeName;
   tEntityId m_id;

   typedef std::map<tEntityComponentID, IEntityComponent*> tEntityComponentMap;
   tEntityComponentMap m_entityComponentMap;
};


////////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_ENTITY_H
