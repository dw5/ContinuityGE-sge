///////////////////////////////////////////////////////////////////////////////
// $Id$

#ifndef INCLUDED_MATRIX3_H
#define INCLUDED_MATRIX3_H

#include "techdll.h"
#include "vec3.h"

#ifdef _MSC_VER
#pragma once
#endif

#ifndef NO_DEFAULT_MATRIX3
template <typename T> class cMatrix3;
typedef cMatrix3<float> tMatrix3;
#endif

///////////////////////////////////////////////////////////////////////////////
//
// TEMPLATE: cMatrix3
//

template <typename T>
class cMatrix3
{
public:
   cMatrix3();
   cMatrix3(T m00, T m10, T m20, T m01, T m11, T m21, T m02, T m12, T m22);
   cMatrix3(const cMatrix3 & other);

   const cMatrix3 & operator =(const cMatrix3 & other);

   static const cMatrix3 & GetIdentity();

   void Transpose(cMatrix3 * pDest);

   void Multiply(const cMatrix3 & other, cMatrix3 * pResult) const;

   void Transform(const cVec3<T> & v, cVec3<T> * pResult) const;

   void Transform3(const float * pV, float * pDest) const;

   // [ m00 m01 m02 ]
   // [ m10 m11 m12 ]
   // [ m20 m21 m22 ]

   union
   {
      struct
      {
         T m00,m10,m20,m01,m11,m21,m02,m12,m22;
      };

      T m[9];
   };
};

///////////////////////////////////////

template <typename T>
cMatrix3<T>::cMatrix3()
{
}

///////////////////////////////////////

template <typename T>
cMatrix3<T>::cMatrix3(T m00, T m10, T m20, T m01, T m11, T m21, T m02, T m12, T m22)
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
}

///////////////////////////////////////

template <typename T>
cMatrix3<T>::cMatrix3(const cMatrix3 & other)
{
   operator =(other);
}

///////////////////////////////////////

template <typename T>
const cMatrix3<T> & cMatrix3<T>::operator =(const cMatrix3 & other)
{
   for (int i = 0; i < _countof(m); i++)
   {
      m[i] = other.m[i];
   }
   return *this;
}

///////////////////////////////////////

template <typename T>
const cMatrix3<T> & cMatrix3<T>::GetIdentity()
{
   static const cMatrix3<T> identity(1,0,0,0,1,0,0,0,1);
   return identity;
}

///////////////////////////////////////

template <typename T>
void cMatrix3<T>::Transpose(cMatrix3<T> * pDest)
{
   Assert(pDest != NULL);
   pDest->m00 = m00;
   pDest->m10 = m01;
   pDest->m20 = m02;
   pDest->m01 = m10;
   pDest->m11 = m11;
   pDest->m21 = m12;
   pDest->m02 = m20;
   pDest->m12 = m21;
   pDest->m22 = m22;
}

///////////////////////////////////////

template <typename T>
void cMatrix3<T>::Multiply(const cMatrix3 & other, cMatrix3 * pResult) const
{
   Assert(pResult != NULL);
   MatrixMultiply(m, other.m, pResult->m);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix3<T>::Transform(const cVec3<T> & v, cVec3<T> * pResult) const
{
   Assert(pResult != NULL);
   // [ m00 m01 m02 ]   [v0]
   // [ m10 m11 m12 ] * [v1] = [v0' v1' v2']
   // [ m20 m21 m22 ]   [v2]
   pResult[0] = (v[0] * m00) + (v[1] * m01) + (v[2] * m02);
   pResult[1] = (v[0] * m10) + (v[1] * m11) + (v[2] * m12);
   pResult[2] = (v[0] * m20) + (v[1] * m21) + (v[2] * m22);
}

///////////////////////////////////////

template <typename T>
inline void cMatrix3<T>::Transform3(const float * pV, float * pDest) const
{
   pDest[0] = (pV[0] * m00) + (pV[1] * m01) + (pV[2] * m02);
   pDest[1] = (pV[0] * m10) + (pV[1] * m11) + (pV[2] * m12);
   pDest[2] = (pV[0] * m20) + (pV[1] * m21) + (pV[2] * m22);
}

///////////////////////////////////////////////////////////////////////////////

#endif // !INCLUDED_MATRIX3_H
