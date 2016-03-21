///  vtkITKNumericTraits - Extra itk::NumericTraits instantiations for VTK
///
/// vtkITKNumericTraits provides extra instantiations for itk::NumericTraits for VTK scalar types.
#ifndef __vtkITKNumericTraits_h
#define __vtkITKNumericTraits_h

#include "vtkITK.h"
#include "vtkSystemIncludes.h"

#include "itkNumericTraits.h"

namespace itk
{

#if defined(VTK_TYPE_USE___INT64)
template <>
class NumericTraits<__int64> : public std::numeric_limits<__int64> {
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

  static __int64 min( ) { return std::numeric_limits<__int64>::min(); }
  static __int64 max( ) { return std::numeric_limits<__int64>::max(); }
  static __int64 min( __int64 ) { return std::numeric_limits<__int64>::min(); }
  static __int64 max( __int64 ) { return std::numeric_limits<__int64>::max(); }
  static __int64 NonpositiveMin() { return min(); }
  static bool IsPositive(__int64 val) { return val > Zero; }
  static bool IsNonpositive(__int64 val) { return val <= Zero; }
  static bool IsNegative(__int64 val) { return val < Zero; }
  static bool IsNonnegative(__int64 val) {return val >= Zero; }
  static __int64  ZeroValue() { return Zero; }
};

template <>
class NumericTraits<unsigned __int64> : public std::numeric_limits<unsigned __int64> {
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

  static unsigned __int64 min( ) { return std::numeric_limits<unsigned __int64>::min(); }
  static unsigned __int64 max( ) { return std::numeric_limits<unsigned __int64>::max(); }
  static unsigned __int64 min( unsigned __int64 ) { return std::numeric_limits<unsigned __int64>::min(); }
  static unsigned __int64 max( unsigned __int64 ) { return std::numeric_limits<unsigned __int64>::max(); }
  static unsigned __int64 NonpositiveMin() { return min(); }
  static bool IsPositive(unsigned __int64 val) { return val != Zero; }
  static bool IsNonpositive(unsigned __int64 val) { return val == Zero; }
  static bool IsNegative(unsigned __int64) { return false; }
  static bool IsNonnegative(unsigned __int64) {return true; }
  static unsigned __int64  ZeroValue() { return Zero; }
};
#endif

}

#endif /// namespace
