/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkITKLevelTracingImageFilter.h $

  Copyright (c) ???
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkITKNumericTraits - Extra itk::NumericTraits instantiations for VTK
// .SECTION Description
// vtkITKNumericTraits provides extra instantiations for itk::NumericTraits for VTK scalar types.
#ifndef __vtkITKNumericTraits_h
#define __vtkITKNumericTraits_h

#include "vtkITK.h"
#include "vtkSystemIncludes.h" 

#include "itkNumericTraits.h"

namespace itk
{

#if defined(VTK_TYPE_USE___INT64)
template <>
class NumericTraits<__int64> : public vcl_numeric_limits<__int64> {
public:
  typedef __int64 ValueType;
  typedef __int64 PrintType;
  typedef unsigned __int64 AbsType;
  typedef __int64 AccumulateType;
  typedef double RealType;
  typedef RealType ScalarRealType;
  typedef float FloatType;
  static const __int64 VTK_ITK_EXPORT Zero;
  static const __int64 VTK_ITK_EXPORT One;

  static __int64 NonpositiveMin() { return min(); }
  static bool IsPositive(__int64 val) { return val > Zero; }
  static bool IsNonpositive(__int64 val) { return val <= Zero; }
  static bool IsNegative(__int64 val) { return val < Zero; }
  static bool IsNonnegative(__int64 val) {return val >= Zero; }
  static __int64  ZeroValue() { return Zero; }
};

template <>
class NumericTraits<unsigned __int64> : public vcl_numeric_limits<unsigned __int64> {
public:
  typedef unsigned __int64 ValueType;
  typedef unsigned __int64 PrintType;
  typedef unsigned __int64 AbsType;
  typedef unsigned __int64 AccumulateType;
  typedef double RealType;
  typedef RealType ScalarRealType;
  typedef float FloatType;
  static const unsigned __int64 VTK_ITK_EXPORT Zero;
  static const unsigned __int64 VTK_ITK_EXPORT One;

  static unsigned __int64 NonpositiveMin() { return min(); }
  static bool IsPositive(unsigned __int64 val) { return val != Zero; }
  static bool IsNonpositive(unsigned __int64 val) { return val == Zero; }
  static bool IsNegative(unsigned __int64) { return false; }
  static bool IsNonnegative(unsigned __int64) {return true; }
  static unsigned __int64  ZeroValue() { return Zero; }
};
#endif

#if defined(VTK_TYPE_USE_LONG_LONG)
template <>
class NumericTraits<long long> : public vcl_numeric_limits<long long> {
public:
  typedef long long ValueType;
  typedef long long PrintType;
  typedef unsigned long long AbsType;
  typedef long long AccumulateType;
  typedef double RealType;
  typedef RealType ScalarRealType;
  typedef float FloatType;
  static const long long VTK_ITK_EXPORT Zero;
  static const long long VTK_ITK_EXPORT One;

  static long long NonpositiveMin() { return min(); }
  static bool IsPositive(long long val) { return val > Zero; }
  static bool IsNonpositive(long long val) { return val <= Zero; }
  static bool IsNegative(long long val) { return val < Zero; }
  static bool IsNonnegative(long long val) {return val >= Zero; }
  static long long  ZeroValue() { return Zero; }
};

template <>
class NumericTraits<unsigned long long> : public vcl_numeric_limits<unsigned long long> {
public:
  typedef unsigned long long ValueType;
  typedef unsigned long long PrintType;
  typedef unsigned long long AbsType;
  typedef unsigned long long AccumulateType;
  typedef double RealType;
  typedef RealType ScalarRealType;
  typedef float FloatType;
  static const unsigned long long VTK_ITK_EXPORT Zero;
  static const unsigned long long VTK_ITK_EXPORT One;

  static unsigned long long NonpositiveMin() { return min(); }
  static bool IsPositive(unsigned long long val) { return val != Zero; }
  static bool IsNonpositive(unsigned long long val) { return val == Zero; }
  static bool IsNegative(unsigned long long) { return false; }
  static bool IsNonnegative(unsigned long long) {return true; }
  static unsigned long long  ZeroValue() { return Zero; }
};
#endif

}

#endif // namespace
