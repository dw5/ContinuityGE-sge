///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MODEL_H
#define INCLUDED_MODEL_H

#include "enginedll.h"
#include "comtools.h"

#include "quat.h"
#include "vec3.h"
#include "matrix4.h"
#include "techstring.h"

#include <vector>

#ifdef _MSC_VER
#pragma once
#endif

#if _MSC_VER <= 1300
#pragma warning(push)
#pragma warning(disable:4251)
#endif


F_DECLARE_INTERFACE(IReader);

typedef uint GLenum;

///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1300
template class ENGINE_API cMatrix4<float>;
template class ENGINE_API std::allocator< cMatrix4<float> >;
template class ENGINE_API std::vector< cMatrix4<float> >;
template class ENGINE_API std::allocator<uint16>;
template class ENGINE_API std::vector<uint16>;
#endif

typedef std::vector< cMatrix4<float> > tMatrices;


///////////////////////////////////////////////////////////////////////////////

struct sModelVertex
{
   tVec3::value_type u, v;
   tVec3 normal;
   tVec3 pos;
   float bone;
};


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<sModelVertex>;
template class ENGINE_API std::vector<sModelVertex>;
#endif

typedef std::vector<sModelVertex> tModelVertices;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelMaterial
//

class ENGINE_API cModelMaterial
{
public:
   cModelMaterial();
   cModelMaterial(const cModelMaterial & other);

   // Common case: diffuse and texture only
   cModelMaterial(const float diffuse[4], const tChar * pszTexture);

   // All color components
   cModelMaterial(const float diffuse[4], const float ambient[4],
      const float specular[4], const float emissive[4], float shininess,
      const tChar * pszTexture);

   ~cModelMaterial();

   const cModelMaterial & operator =(const cModelMaterial & other);

   // Apply diffuse color (for glEnable(GL_COLOR_MATERIAL)) and bind the texture
   void GlDiffuseAndTexture();

   // Apply all components with glMaterial and bind the texture
   void GlMaterialAndTexture();

private:
   float m_diffuse[4], m_ambient[4], m_specular[4], m_emissive[4], m_shininess;
   cStr m_texture;
};


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<cModelMaterial>;
template class ENGINE_API std::vector<cModelMaterial>;
#endif

typedef std::vector<cModelMaterial> tModelMaterials;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelMesh
//

class ENGINE_API cModelMesh
{
public:
   cModelMesh();
   cModelMesh(const cModelMesh & other);

   cModelMesh(GLenum glPrimitive, const std::vector<uint16> & indices, int8 materialIndex);

   ~cModelMesh();

   const cModelMesh & operator =(const cModelMesh & other);

private:
   GLenum m_glPrimitive;
   std::vector<uint16> m_indices;
   int8 m_materialIndex;
};

#if _MSC_VER > 1300
template class ENGINE_API std::allocator<cModelMesh>;
template class ENGINE_API std::vector<cModelMesh>;
#endif

typedef std::vector<cModelMesh> tModelMeshes;


///////////////////////////////////////////////////////////////////////////////
//
// STRUCT: sModelKeyFrame
//

struct sModelKeyFrame
{
   double time;
   tVec3 translation;
   tQuat rotation;
};

#if _MSC_VER > 1300
template class ENGINE_API std::allocator<sModelKeyFrame>;
template class ENGINE_API std::vector<sModelKeyFrame>;
#endif

typedef std::vector<sModelKeyFrame> tModelKeyFrames;


///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModelJoint
//


class ENGINE_API cModelJoint
{
public:
   cModelJoint();
   cModelJoint(const cModelJoint & other);

   cModelJoint(int parentIndex, const tMatrix4 & localTransform, const tModelKeyFrames & keyFrames);

   ~cModelJoint();

   const cModelJoint & operator =(const cModelJoint & other);

   int GetParentIndex() const;
   const tMatrix4 & GetLocalTransform() const;

   tResult GetKeyFrame(uint index, sModelKeyFrame * pFrame);

   tResult Interpolate(double time, tVec3 * pTrans, tQuat * pRot);


private:
   int m_parentIndex;
   tMatrix4 m_localTransform;

   std::vector<sModelKeyFrame> m_keyFrames;
};

inline int cModelJoint::GetParentIndex() const
{
   return m_parentIndex;
}

inline const tMatrix4 & cModelJoint::GetLocalTransform() const
{
   return m_localTransform;
}


#if _MSC_VER > 1300
template class ENGINE_API std::allocator<cModelJoint>;
template class ENGINE_API std::vector<cModelJoint>;
#endif

typedef std::vector<cModelJoint> tModelJoints;



///////////////////////////////////////////////////////////////////////////////
//
// CLASS: cModel
//

#define kRT_Model "Model" // resource type

class ENGINE_API cModel
{
   cModel(const cModel &);
   const cModel & operator =(const cModel &);

   cModel();

   cModel(const tModelVertices & verts,
          const tModelMaterials & materials,
          const tModelMeshes & meshes);

   cModel(const tModelVertices & verts,
          const tModelMaterials & materials,
          const tModelMeshes & meshes,
          const tModelJoints & joints);

public:
   virtual ~cModel();

   static tResult Create(const tModelVertices & verts,
                         const tModelMaterials & materials,
                         const tModelMeshes & meshes,
                         cModel * * ppModel);

   static tResult Create(const tModelVertices & verts,
                         const tModelMaterials & materials,
                         const tModelMeshes & meshes,
                         const tModelJoints & joints,
                         cModel * * ppModel);

   bool IsAnimated() const;

   void Animate(double elapsedTime);

   void Render();

   static tResult RegisterResourceFormat();

private:

   // Transform every vertex by the inverse of its affecting
   // joint's absolute transform
   void PreApplyJoints();

   static void * ModelLoadMs3d(IReader * pReader);

   static void ModelUnload(void * pData);


   std::vector<sModelVertex> m_vertices;
   std::vector<cModelMaterial> m_materials;
   std::vector<cModelMesh> m_meshes;
   std::vector<cModelJoint> m_joints;

   double m_animationTime;
   std::vector< cMatrix4<float> > m_blendMatrices;
};

inline bool cModel::IsAnimated() const
{
   return !m_joints.empty();
}


#if _MSC_VER <= 1300
#pragma warning(pop)
#endif


///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MODEL_H
