////////////////////////////////////////////////////////////////////////////////
// $Id$

#include "stdhdr.h"

#include "model.h"

#include "renderapi.h"

#include "globalobj.h"
#include "resourceapi.h"
#include "readwriteapi.h"
#include "readwriteutils.h"

#include "dbgalloc.h" // must be last header


////////////////////////////////////////////////////////////////////////////////

LOG_DEFINE_CHANNEL(ModelSgem);

#define LocalMsg(msg)            DebugMsgEx(ModelSgem,msg)
#define LocalMsg1(msg,a)         DebugMsgEx1(ModelSgem,msg,(a))
#define LocalMsg2(msg,a,b)       DebugMsgEx2(ModelSgem,msg,(a),(b))
#define LocalMsg3(msg,a,b,c)     DebugMsgEx3(ModelSgem,msg,(a),(b),(c))

////////////////////////////////////////////////////////////////////////////////


static const byte g_sgemId[] = "MeGs";

void * ModelSgemLoad(IReader * pReader)
{
   if (pReader == NULL)
   {
      return NULL;
   }

   LocalMsg("Loading SGEM file...\n");

   //////////////////////////////
   // Read the header

   byte id[4];
   uint32 version;
   if (pReader->Read(id, sizeof(id)) != S_OK
      || pReader->Read(&version) != S_OK
      || memcmp(id, g_sgemId, 4) != 0)
   {
      ErrorMsg("Bad SGEM file header\n");
      return NULL;
   }

   if (version != 1)
   {
      ErrorMsg1("SGEM version %d not supported\n", version);
      return NULL;
   }

   //////////////////////////////

   uint nMeshes = 0;
   if (pReader->Read(&nMeshes, sizeof(nMeshes)) != S_OK
      || nMeshes == 0)
   {
      return NULL;
   }

   LocalMsg1("%d Meshes\n", nMeshes);

   tModelMeshes meshes(nMeshes);

   for (uint i = 0; i < nMeshes; ++i)
   {
      std::vector<sModelVertex> verts;
      if (pReader->Read(&verts) != S_OK)
      {
         return NULL;
      }

      static const ePrimitiveType primTypes[] = { kPT_Triangles, kPT_TriangleStrip, kPT_TriangleFan };

      int primType = -1;
      if (pReader->Read(&primType) != S_OK
         || primType < 0 || primType > 2)
      {
         return NULL;
      }

      std::vector<uint16> indices;
      if (pReader->Read(&indices) != S_OK)
      {
         return NULL;
      }

      meshes[i] = cModelMesh(primTypes[primType], indices, -1);
   }

   //////////////////////////////

   std::vector<sModelMaterial> materials;
   if (pReader->Read(&materials) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Materials\n", materials.size());

   //////////////////////////////

   std::vector<sModelJoint> joints;
   if (pReader->Read(&joints) != S_OK)
   {
      return NULL;
   }

   LocalMsg1("%d Joints\n", joints.size());

   cAutoIPtr<IModelSkeleton> pSkeleton;
   if (ModelSkeletonCreate(&joints[0], joints.size(), &pSkeleton) == S_OK)
   {
   }

   //////////////////////////////

   uint nAnims = 0;
   if (pReader->Read(&nAnims, sizeof(nAnims)) != S_OK
      || nAnims == 0)
   {
      return NULL;
   }

   LocalMsg1("%d Anims\n", nAnims);

   for (uint i = 0; i < nAnims; ++i)
   {
      static const eModelAnimationType animTypes[] =
      {
         kMAT_Walk,
         kMAT_Run,
         kMAT_Death,
         kMAT_Attack,
         kMAT_Damage,
         kMAT_Idle,
      };

      int intAnimType = -1;
      if (pReader->Read(&intAnimType) != S_OK
         || intAnimType < kMAT_Walk || intAnimType > kMAT_Idle)
      {
         return NULL;
      }

      eModelAnimationType animType = animTypes[intAnimType];

      std::vector< std::vector<sModelKeyFrame> > animKeyFrameVectors;
      if (pReader->Read(&animKeyFrameVectors) != S_OK)
      {
         return NULL;
      }

      std::vector< std::vector<sModelKeyFrame> >::const_iterator iter = animKeyFrameVectors.begin();
      std::vector< std::vector<sModelKeyFrame> >::const_iterator end = animKeyFrameVectors.end();
      for (; iter != end; ++iter)
      {
         const std::vector<sModelKeyFrame> & keyFrames = *iter;

         cAutoIPtr<IModelKeyFrameInterpolator> pInterp;
         if (ModelKeyFrameInterpolatorCreate(&keyFrames[0], keyFrames.size(), &pInterp) == S_OK)
         {
         }
      }
   }

   return NULL;
}

///////////////////////////////////////

void ModelSgemUnload(void * pData)
{
   cModel * pModel = reinterpret_cast<cModel*>(pData);
   delete pModel;
}


////////////////////////////////////////////////////////////////////////////////

tResult ModelSgemResourceRegister()
{
   UseGlobal(ResourceManager);
   return pResourceManager->RegisterFormat(kRT_Model, _T("sgem"), ModelSgemLoad, NULL, ModelSgemUnload);
}

////////////////////////////////////////////////////////////////////////////////
