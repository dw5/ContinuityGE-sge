///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODELREADWRITE_H
#define INCLUDED_MODELREADWRITE_H

#include "enginedll.h"

#include "modeltypes.h"

#include "tech/readwriteapi.h"

#ifdef _MSC_VER
#pragma once
#endif


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<sModelVertex>
//

template <>
class ENGINE_API cReadWriteOps<sModelVertex>
{
public:
   static tResult Read(IReader * pReader, sModelVertex * pModelVertex);
   static tResult Write(IWriter * pWriter, const sModelVertex & modelVertex);
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<sModelMaterial>
//

template <>
class ENGINE_API cReadWriteOps<sModelMaterial>
{
public:
   static tResult Read(IReader * pReader, sModelMaterial * pModelMaterial);
   static tResult Write(IWriter * pWriter, const sModelMaterial & modelMaterial);
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<sModelMesh>
//

template <>
class ENGINE_API cReadWriteOps<sModelMesh>
{
public:
   static tResult Read(IReader * pReader, sModelMesh * pModelMesh);
   static tResult Write(IWriter * pWriter, const sModelMesh & modelMesh);
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<sModelJoint>
//

template <>
class ENGINE_API cReadWriteOps<sModelJoint>
{
public:
   static tResult Read(IReader * pReader, sModelJoint * pModelJoint);
   static tResult Write(IWriter * pWriter, const sModelJoint & modelJoint);
};


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cReadWriteOps<sModelKeyFrame>
//

template <>
class ENGINE_API cReadWriteOps<sModelKeyFrame>
{
public:
   static tResult Read(IReader * pReader, sModelKeyFrame * pModelKeyFrame);
   static tResult Write(IWriter * pWriter, const sModelKeyFrame & modelKeyFrame);
};


///////////////////////////////////////////////////////////////////////////////

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
   ((uint)(byte)(ch0) | ((uint)(byte)(ch1) << 8) | \
   ((uint)(byte)(ch2) << 16) | ((uint)(byte)(ch3) << 24 ))
#endif

#define MODEL_FILE_ID_CHUNK                  MAKEFOURCC('S','G','M','D')
#define MODEL_VERSION_CHUNK                  MAKEFOURCC('V','R','S','N')
#define MODEL_VERTEX_ARRAY_CHUNK             MAKEFOURCC('V','R','T','A')
#define MODEL_INDEX16_ARRAY_CHUNK            MAKEFOURCC('I','1','6','A')
#define MODEL_MESH_ARRAY_CHUNK               MAKEFOURCC('M','S','H','A')
#define MODEL_MATERIAL_ARRAY_CHUNK           MAKEFOURCC('M','T','L','A')
#define MODEL_SKELETON_CHUNK                 MAKEFOURCC('S','K','E','L')
#define MODEL_JOINT_ARRAY_CHUNK              MAKEFOURCC('J','N','T','A')
#define MODEL_ANIMATION_SEQUENCE_CHUNK       MAKEFOURCC('A','N','I','M')

const uint kModelChunkHeaderSize = (2 * sizeof(uint)); // chunk type and length


////////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cModelChunk
//

struct sModelChunkHeader
{
   uint chunkId, chunkLength;
};

typedef void * NoChunkData;

template <typename T>
class cModelChunk
{
   friend class cReadWriteOps< cModelChunk<T> >;

public:
   cModelChunk();
   cModelChunk(uint chunkId);
   cModelChunk(uint chunkId, const T & chunkData);
   cModelChunk(const cModelChunk & other);

   uint GetChunkId() const { return m_chunkHeader.chunkId; }
   uint GetChunkLength() const { return m_chunkHeader.chunkLength; }
   bool NoChunkData() const { return m_bNoChunkData; }
   const T & GetChunkData() const { return m_chunkData; }

private:
   sModelChunkHeader m_chunkHeader;
   bool m_bNoChunkData;
   T m_chunkData;
};

////////////////////////////////////////

template <typename T>
cModelChunk<T>::cModelChunk()
 : m_bNoChunkData(true)
{
   m_chunkHeader.chunkId = 0;
   m_chunkHeader.chunkLength = 0;
}

////////////////////////////////////////

template <typename T>
cModelChunk<T>::cModelChunk(uint chunkId)
 : m_bNoChunkData(true)
{
   m_chunkHeader.chunkId = chunkId;
   m_chunkHeader.chunkLength = 0;
}

////////////////////////////////////////

template <typename T>
cModelChunk<T>::cModelChunk(uint chunkId, const T & chunkData)
 : m_bNoChunkData(false)
 , m_chunkData(chunkData)
{
   m_chunkHeader.chunkId = chunkId;
   m_chunkHeader.chunkLength = 0;
}

////////////////////////////////////////

template <typename T>
cModelChunk<T>::cModelChunk(const cModelChunk & other)
 : m_bNoChunkData(other.m_bNoChunkData)
 , m_chunkData(other.m_chunkData)
{
   memcpy(&m_chunkHeader, &other.m_chunkHeader, sizeof(m_chunkHeader));
}


////////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cReadWriteOps< cModelChunk<T> >
//

template <typename T>
class cReadWriteOps< cModelChunk<T> >
{
public:
   static tResult Read(IReader * pReader, cModelChunk<T> * pModelChunk);
   static tResult Write(IWriter * pWriter, const cModelChunk<T> & modelChunk);
};

////////////////////////////////////////

template <typename T>
tResult cReadWriteOps< cModelChunk<T> >::Read(IReader * pReader, cModelChunk<T> * pModelChunk)
{
   if (pReader == NULL || pModelChunk == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   if (pReader->Read(&pModelChunk->m_chunkHeader, sizeof(pModelChunk->m_chunkHeader)) == S_OK)
   {
      if (pModelChunk->GetChunkLength() == kModelChunkHeaderSize)
      {
         pModelChunk->m_bNoChunkData = true;
         result = S_OK;
      }
      else if (pReader->Read(&pModelChunk->m_chunkData) == S_OK)
      {
         pModelChunk->m_bNoChunkData = false;
         result = S_OK;
      }
   }

   return result;
}

////////////////////////////////////////

template <typename T>
tResult cReadWriteOps< cModelChunk<T> >::Write(IWriter * pWriter, const cModelChunk<T> & modelChunk)
{
   if (pWriter == NULL)
   {
      return E_POINTER;
   }

   tResult result = E_FAIL;

   ulong start = 0, end = 0;
   if (pWriter->Tell(&start) == S_OK                                 // get the chunk's start position
      && pWriter->Write(modelChunk.GetChunkId()) == S_OK             // write the chunk identifier
      && pWriter->Write(static_cast<uint>(0)) == S_OK                // zero will be replaced by chunk size
      && (modelChunk.NoChunkData()                                   // possibly no data, or
         || pWriter->Write(modelChunk.GetChunkData()) == S_OK)       // write the chunk data
      && pWriter->Tell(&end) == S_OK                                 // get the file position after the data
      && pWriter->Seek(start + sizeof(uint), kSO_Set) == S_OK        // seek back to the chunk size (the zero)
      && pWriter->Write(static_cast<uint>(end - start)) == S_OK      // replace the zero with the chunk length
      && pWriter->Seek(end, kSO_Set) == S_OK)                        // seek back to the end of the chunk
   {
      result = S_OK;
   }

   return result;
}


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODELREADWRITE_H
