//$$ precisio.h                          floating point constants

#ifndef PRECISION_LIB
#define PRECISION_LIB 0

#define WANT_MATH
#include "include.h"              // in case being used as stand alone

#ifdef _STANDARD_                 // standard library available
#include <limits>
#endif

#ifdef use_namespace
namespace NEWMAT {
#endif

#ifdef _STANDARD_                 // standard library available

#ifdef OPT_COMPATIBLE
#include <cfloat>                 // for FLT_MAX
#endif

using namespace std;
        
class FloatingPointPrecision
{
public:
   static int Dig()              // number of decimal digits or precision
      { return numeric_limits<Real>::digits10 ; }

   static Real Epsilon()         // smallest number such that 1+Eps!=Eps
      { return numeric_limits<Real>::epsilon(); }

   static int Mantissa()         // bits in mantisa
      { return numeric_limits<Real>::digits; }

   static Real Maximum()         // maximum value
      { return numeric_limits<Real>::max(); }

   static int MaximumDecimalExponent()  // maximum decimal exponent
      { return numeric_limits<Real>::max_exponent10; }

   static int MaximumExponent()  // maximum binary exponent
      { return numeric_limits<Real>::max_exponent; }

   static Real LnMaximum()       // natural log of maximum
      { return (Real)log(Maximum()); }

   static Real Minimum()         // minimum positive value
      { return numeric_limits<Real>::min(); } 

   static int MinimumDecimalExponent() // minimum decimal exponent
      { return numeric_limits<Real>::min_exponent10; }

   static int MinimumExponent()  // minimum binary exponent
      { return numeric_limits<Real>::min_exponent; }

   static Real LnMinimum()       // natural log of minimum
      { return (Real)log(Minimum()); }

   static int Radix()            // exponent radix
      { return numeric_limits<Real>::radix; }

   static int Rounds()           // addition rounding (1 = does round)
   {
          return numeric_limits<Real>::round_style ==
                 round_to_nearest ? 1 : 0;
   }

};


#else                              // _STANDARD_ not defined

#ifndef SystemV                    // if there is float.h

#ifdef USING_FLOAT

class FloatingPointPrecision
{
public:
   static int Dig()
      { return FLT_DIG; }        // number of decimal digits or precision

   static Real Epsilon()
      { return FLT_EPSILON; }    // smallest number such that 1+Eps!=Eps

   static int Mantissa()
      { return FLT_MANT_DIG; }   // bits in mantisa

   static Real Maximum()
      { return FLT_MAX; }        // maximum value

   static int MaximumDecimalExponent()
      { return FLT_MAX_10_EXP; } // maximum decimal exponent

   static int MaximumExponent()
      { return FLT_MAX_EXP; }    // maximum binary exponent

   static Real LnMaximum()
      { return (Real)log(Maximum()); } // natural log of maximum

   static Real Minimum()
      { return FLT_MIN; }        // minimum positive value

   static int MinimumDecimalExponent()
      { return FLT_MIN_10_EXP; } // minimum decimal exponent

   static int MinimumExponent()
      { return FLT_MIN_EXP; }    // minimum binary exponent

   static Real LnMinimum()
      { return (Real)log(Minimum()); } // natural log of minimum

   static int Radix()
      { return FLT_RADIX; }      // exponent radix

   static int Rounds()
      { return FLT_ROUNDS; }     // addition rounding (1 = does round)

};

#endif                           // USING_FLOAT


#ifdef USING_DOUBLE

class FloatingPointPrecision
{
public:

   static int Dig()
      { return DBL_DIG; }        // number of decimal digits or precision

   static Real Epsilon()
      { return DBL_EPSILON; }    // smallest number such that 1+Eps!=Eps

   static int Mantissa()
      { return DBL_MANT_DIG; }   // bits in mantisa

   static Real Maximum()
      { return DBL_MAX; }        // maximum value

   static int MaximumDecimalExponent()
      { return DBL_MAX_10_EXP; } // maximum decimal exponent

   static int MaximumExponent()
      { return DBL_MAX_EXP; }    // maximum binary exponent

   static Real LnMaximum()
      { return (Real)log(Maximum()); } // natural log of maximum

   static Real Minimum()
   {
//#ifdef __BCPLUSPLUS__
//       return 2.225074e-308;     // minimum positive value
//#else
       return DBL_MIN;
//#endif
   }

   static int MinimumDecimalExponent()
      { return DBL_MIN_10_EXP; } // minimum decimal exponent

   static int MinimumExponent()
      { return DBL_MIN_EXP; }    // minimum binary exponent

   static Real LnMinimum()
      { return (Real)log(Minimum()); } // natural log of minimum


   static int Radix()
      { return FLT_RADIX; }      // exponent radix

   static int Rounds()
      { return FLT_ROUNDS; }     // addition rounding (1 = does round)

};

#endif                             // USING_DOUBLE

#else                              // if there is no float.h

#ifdef OPT_COMPATIBLE
#define FLT_MAX MAXFLOAT
#endif


#ifdef USING_FLOAT

class FloatingPointPrecision
{
public:

   static Real Epsilon()
      { return pow(2.0,(int)(1-FSIGNIF)); }
                                   // smallest number such that 1+Eps!=Eps

   static Real Maximum()
      { return MAXFLOAT; }            // maximum value

   static Real LnMaximum()
      { return (Real)log(Maximum()); }  // natural log of maximum

   static Real Minimum()
      { return MINFLOAT; }             // minimum positive value

   static Real LnMinimum()
      { return (Real)log(Minimum()); }  // natural log of minimum

};

#endif                                  // USING_FLOAT


#ifdef USING_DOUBLE

class FloatingPointPrecision
{
public:

   static Real Epsilon()
      { return pow(2.0,(int)(1-DSIGNIF)); }
                                      // smallest number such that 1+Eps!=Eps

   static Real Maximum()
      { return MAXDOUBLE; }           // maximum value

   static Real LnMaximum()
      { return LN_MAXDOUBLE; }        // natural log of maximum

   static Real Minimum()
      { return MINDOUBLE; }

   static Real LnMinimum()
      { return LN_MINDOUBLE; }        // natural log of minimum
};

#endif                                // USING_DOUBLE

#endif                                // SystemV

#endif                                // _STANDARD_




#ifdef use_namespace
}
#endif                                // use_namespace



#endif                                // PRECISION_LIB
