///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX34_H
#define INCLUDED_MATRIX34_H

#include "techdll.h"
#include "matrix3.h"
#include "quat.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

#ifndef NO_DEFAULT_MATRIX34
template <typename T> class cMatrix34;
typedef cMatrix34<float> tMatrix34;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMatrix34
//
// A 3x3 matrix (rotation) with an extra column (to represent a translation)

template <typename T>
class cMatrix34
{
public:
   cMatrix34();
   cMatrix34(T m00, T m10, T m20, T m01, T m11, T m21, T m02, T m12, T m22, T m03, T m13, T m23);
   cMatrix34(const cMatrix34 & other);

   const cMatrix34 & operator =(const cMatrix34 & other);

   static const cMatrix34 & GetIdentity();

   void SetRotation(const cQuat<T> & q);
   void SetRotation(const cMatrix3<T> & r);
   void SetTranslation(const cVec3<T> & t);

   void Compose(const cMatrix34 & other, cMatrix34 * pResult) const;

   void Transform(const cVec3<T> & v, cVec3<T> * pResult) const;
   void Transform3fv(const float * pV, float * pDest) const;

   // [ m00 m01 m02 m03 ]
   // [ m10 m11 m12 m13 ]
   // [ m20 m21 m22 m23 ]

   union
   {
      struct
      {
         T m00,m10,m20,m01,m11,m21,m02,m12,m22,m03,m13,m23;
      };

      T m[12];
   };
};

///////////////////////////////////////

template <typename T>
cMatrix34<T>::cMatrix34()
{
}

///////////////////////////////////////

template <typename T>
cMatrix34<T>::cMatrix34(T m00, T m10, T m20,
                        T m01, T m11, T m21,
                        T m02, T m12, T m22,
                        T m03, T m13, T m23)
{
   this->m00 = m00;
   this->m10 = m10;
   this->m20 = m20;
   this->m01 = m01;
   this->m11 = m11;
   this->m21 = m21;
   this->m02 = m02;
   this->m12 = m12;
   this->m22 = m22;
   this->m03 = m03;
   this->m13 = m13;
   this->m23 = m23;
}

///////////////////////////////////////

template <typename T>
cMatrix34<T>::cMatrix34(const cMatrix34 & other)
{
   operator =(other);
}

///////////////////////////////////////

template <typename T>
const cMatrix34<T> & cMatrix34<T>::operator =(const cMatrix34 & other)
{
   for (int i = 0; i < _countof(m); i++)
   {
      m[i] = other.m[i];
   }
   return *this;
}

///////////////////////////////////////

template <typename T>
const cMatrix34<T> & cMatrix34<T>::GetIdentity()
{
   static const cMatrix34<T> identity(1,0,0,0,1,0,0,0,1,0,0,0);
   return identity;
}

///////////////////////////////////////

template <typename T>
void cMatrix34<T>::SetRotation(const cQuat<T> & q)
{
   cMatrix3<T> m;
   q.ToMatrix(&m);
   SetRotation(m);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix34<T>::SetRotation(const cMatrix3<T> & r)
{
   m00 = r.m00;
   m10 = r.m10;
   m20 = r.m20;
   m01 = r.m01;
   m11 = r.m11;
   m21 = r.m21;
   m02 = r.m02;
   m12 = r.m12;
   m22 = r.m22;
}

///////////////////////////////////////

template <typename T>
inline void cMatrix34<T>::SetTranslation(const cVec3<T> & t)
{
   m03 = t.x;
   m13 = t.y;
   m23 = t.z;
}

///////////////////////////////////////

template <typename T>
void cMatrix34<T>::Compose(const cMatrix34 & other, cMatrix34 * pResult) const
{
   Assert(pResult != NULL);
   pResult->m00 = m00*other.m00 + m01*other.m10 + m02*other.m20;
   pResult->m10 = m10*other.m00 + m11*other.m10 + m12*other.m20;
   pResult->m20 = m20*other.m00 + m21*other.m10 + m22*other.m20;
   pResult->m01 = m00*other.m01 + m01*other.m11 + m02*other.m21;
   pResult->m11 = m10*other.m01 + m11*other.m11 + m12*other.m21;
   pResult->m21 = m20*other.m01 + m21*other.m11 + m22*other.m21;
   pResult->m02 = m00*other.m02 + m01*other.m12 + m02*other.m22;
   pResult->m12 = m10*other.m02 + m11*other.m12 + m12*other.m22;
   pResult->m22 = m20*other.m02 + m21*other.m12 + m22*other.m22;
   pResult->m03 = m00*other.m03 + m01*other.m13 + m02*other.m23 + m03;
   pResult->m13 = m10*other.m03 + m11*other.m13 + m12*other.m23 + m13;
   pResult->m23 = m20*other.m03 + m21*other.m13 + m22*other.m23 + m23;
}

///////////////////////////////////////

template <typename T>
inline void cMatrix34<T>::Transform(const cVec3<T> & v, cVec3<T> * pResult) const
{
   Assert(pResult != NULL);
   // [ m00 m01 m02 m03 ]   [v0]
   // [ m10 m11 m12 m13 ] * [v1] = [v0' v1' v2' v3']
   // [ m20 m21 m22 m23 ]   [v2]
   //                       [v3]
   pResult->x = (v.x * m00) + (v.y * m01) + (v.z * m02) + m03;
   pResult->y = (v.x * m10) + (v.y * m11) + (v.z * m12) + m13;
   pResult->z = (v.x * m20) + (v.y * m21) + (v.z * m22) + m23;
}

///////////////////////////////////////

template <typename T>
inline void cMatrix34<T>::Transform3fv(const float * pV, float * pDest) const
{
   pDest[0] = (pV[0] * m00) + (pV[1] * m01) + (pV[2] * m02) + m03;
   pDest[1] = (pV[0] * m10) + (pV[1] * m11) + (pV[2] * m12) + m13;
   pDest[2] = (pV[0] * m20) + (pV[1] * m21) + (pV[2] * m22) + m23;
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX34_H
