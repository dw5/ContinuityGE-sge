///////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "entitymanager.h"

#include "renderapi.h"
#include "terrainapi.h"

#include "color.h"
#include "ray.h"
#include "resourceapi.h"

#include <algorithm>

#include <GL/glew.h>

#include "dbgalloc.h" // must be last header

#define IsFlagSet(f, b) (((f)&(b))==(b))

extern tResult ModelEntityCreate(tEntityId id, const tChar * pszModel, const tVec3 & position, IEntity * * ppEntity);

///////////////////////////////////////////////////////////////////////////////

const sVertexElement g_modelVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   0, 0 },
   { kVEU_Normal,    kVET_Float3,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 5 * sizeof(float) },
   { kVEU_Index,     kVET_Float1,   0, 8 * sizeof(float) },
};

const sVertexElement g_blendedVert[] =
{
   { kVEU_TexCoord,  kVET_Float2,   0, 0 },
   { kVEU_Normal,    kVET_Float3,   0, 2 * sizeof(float) },
   { kVEU_Position,  kVET_Float3,   0, 5 * sizeof(float) },
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cEntityManager
//

///////////////////////////////////////

cEntityManager::cEntityManager()
 : m_nextId(0)
{
}

///////////////////////////////////////

cEntityManager::~cEntityManager()
{
}

////////////////////////////////////////

// {DC738464-A124-4dc2-88A5-54619E5D026F}
static const GUID SAVELOADID_EntityManager = 
{ 0xdc738464, 0xa124, 0x4dc2, { 0x88, 0xa5, 0x54, 0x61, 0x9e, 0x5d, 0x2, 0x6f } };

static const int g_entityManagerVer = 1;

///////////////////////////////////////

tResult cEntityManager::Init()
{
   UseGlobal(Sim);
   pSim->Connect(static_cast<ISimClient*>(this));

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RegisterSaveLoadParticipant(SAVELOADID_EntityManager,
      g_entityManagerVer, static_cast<ISaveLoadParticipant*>(this));

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::Term()
{
   UseGlobal(Sim);
   pSim->Disconnect(static_cast<ISimClient*>(this));

   UseGlobal(SaveLoadManager);
   pSaveLoadManager->RevokeSaveLoadParticipant(SAVELOADID_EntityManager, g_entityManagerVer);

   DeselectAll();

   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      (*iter)->Release();
   }
   m_entities.clear();

   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::SpawnEntity(const tChar * pszMesh, float nx, float nz)
{
   tVec3 location;
   UseGlobal(TerrainModel);
   if (pTerrainModel->GetPointOnTerrain(nx, nz, &location) == S_OK)
   {
      return SpawnEntity(pszMesh, location);
   }
   return E_FAIL;
}

///////////////////////////////////////

tResult cEntityManager::SpawnEntity(const tChar * pszMesh, const tVec3 & position)
{
   uint oldNextId = m_nextId;
   cAutoIPtr<IEntity> pEntity;
   if (ModelEntityCreate(m_nextId++, pszMesh, position, &pEntity) != S_OK)
   {
      m_nextId = oldNextId;
      return E_OUTOFMEMORY;
   }

   m_entities.push_back(CTAddRef(pEntity));
   return S_OK;
}

///////////////////////////////////////

tResult cEntityManager::RemoveEntity(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return E_POINTER;
   }

   bool bFound = false;
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      if (CTIsSameObject(pEntity, *iter))
      {
         bFound = true;
         (*iter)->Release();
         m_entities.erase(iter);
         // TODO: return entity's id to a pool?
         break;
      }
   }

   size_t nErasedFromSelected = m_selected.erase(pEntity);
   while (nErasedFromSelected-- > 0)
   {
      pEntity->Release();
   }

   return bFound ? S_OK : S_FALSE;
}

///////////////////////////////////////

void cEntityManager::RenderAll()
{
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      uint flags = (*iter)->GetFlags();
      if (IsFlagSet(flags, kEF_Hidden))
      {
         continue;
      }

      glPushMatrix();
      glMultMatrixf((*iter)->GetWorldTransform().m);

      (*iter)->Render();

      if (IsSelected(*iter))
      {
         RenderWireFrame((*iter)->GetBoundingBox(), cColor(1,1,0));
      }

      glPopMatrix();
   }
}

///////////////////////////////////////

tResult cEntityManager::RayCast(const cRay & ray, IEntity * * ppEntity) const
{
   tEntityList::const_iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));
      const tMatrix4 & worldTransform = pEntity->GetWorldTransform();
      tVec3 position(worldTransform.m[12], worldTransform.m[13], worldTransform.m[14]);
      tAxisAlignedBox bbox(pEntity->GetBoundingBox());
      bbox.Offset(position);
      if (ray.IntersectsAxisAlignedBox(bbox))
      {
         *ppEntity = CTAddRef(pEntity);
         return S_OK;
      }
   }

   return S_FALSE;
}

///////////////////////////////////////

tResult cEntityManager::BoxCast(const tAxisAlignedBox & box, IEntityEnum * * ppEnum) const
{
   return E_NOTIMPL;
}

///////////////////////////////////////

tResult cEntityManager::Select(IEntity * pEntity)
{
   if (pEntity == NULL)
   {
      return E_POINTER;
   }

   std::pair<tEntitySet::iterator, bool> result = m_selected.insert(pEntity);
   if (result.second)
   {
      pEntity->AddRef();
      return S_OK;
   }
   else
   {
      return S_FALSE;
   }
}

///////////////////////////////////////

tResult cEntityManager::SelectBoxed(const tAxisAlignedBox & box)
{
   int nSelected = 0;
   tEntityList::const_iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      cAutoIPtr<IEntity> pEntity(CTAddRef(*iter));
      const tMatrix4 & worldTransform = pEntity->GetWorldTransform();
      tVec3 position(worldTransform.m[12], worldTransform.m[13], worldTransform.m[14]);
      tAxisAlignedBox bbox(pEntity->GetBoundingBox());
      bbox.Offset(position);
      if (bbox.Intersects(box))
      {
         m_selected.insert(CTAddRef(pEntity));
         nSelected++;
      }
   }
   return (nSelected > 0) ? S_OK : S_FALSE;
}

///////////////////////////////////////

tResult cEntityManager::DeselectAll()
{
   if (m_selected.empty())
   {
      return S_FALSE;
   }
   std::for_each(m_selected.begin(), m_selected.end(), CTInterfaceMethod(&IEntity::Release));
   m_selected.clear();
   return S_OK;
}

///////////////////////////////////////

uint cEntityManager::GetSelectedCount() const
{
   return m_selected.size();
}

///////////////////////////////////////

tResult cEntityManager::GetSelected(IEntityEnum * * ppEnum) const
{
   if (ppEnum == NULL)
   {
      return E_POINTER;
   }
   if (m_selected.empty())
   {
      return S_FALSE;
   }
   return tEntitySetEnum::Create(m_selected, ppEnum);
}

///////////////////////////////////////

void cEntityManager::OnSimFrame(double elapsedTime)
{
   tEntityList::iterator iter = m_entities.begin();
   for (; iter != m_entities.end(); iter++)
   {
      uint flags = (*iter)->GetFlags();
      if (IsFlagSet(flags, kEF_Disabled))
      {
         continue;
      }
      (*iter)->Update(elapsedTime);
   }
}

///////////////////////////////////////

tResult cEntityManager::Save(IWriter * pWriter)
{
   return S_FALSE;
}

///////////////////////////////////////

tResult cEntityManager::Load(IReader * pReader, int version)
{
   return S_FALSE;
}

///////////////////////////////////////

bool cEntityManager::IsSelected(IEntity * pEntity) const
{
   return (m_selected.find(pEntity) != m_selected.end());
}

///////////////////////////////////////

tResult EntityManagerCreate()
{
   cAutoIPtr<IEntityManager> p(static_cast<IEntityManager*>(new cEntityManager));
   if (!p)
   {
      return E_OUTOFMEMORY;
   }
   return RegisterGlobalObject(IID_IEntityManager, p);
}

///////////////////////////////////////////////////////////////////////////////
