//$$ include.h           include files required by various versions of C++

#ifndef INCLUDE_LIB
#define INCLUDE_LIB

//#define use_namespace                   // define name spaces

//#define SETUP_C_SUBSCRIPTS              // allow element access via A[i][j]

//#define OPT_COMPATIBLE                  // for use with opt++

// Activate just one of the following 3 statements

//#define SimulateExceptions              // use simulated exceptions
#define UseExceptions                   // use C++ exceptions
//#define DisableExceptions               // do not use exceptions


//#define TEMPS_DESTROYED_QUICKLY         // for compilers that delete
                                        // temporaries too quickly

//#define TEMPS_DESTROYED_QUICKLY_R       // the same thing but applied
                                        // to return from functions only

//#define DO_FREE_CHECK                   // check news and deletes balance

#define USING_DOUBLE                    // elements of type double
//#define USING_FLOAT                   // elements of type float

#define bool_LIB 0                      // for compatibility with my older libraries

//#define ios_format_flags ios::fmtflags  // for Gnu 3 and Intel for Linux


//#define _STANDARD_                    // using standard library

//#define use_float_h                   // use float.h for precision data


//#define HAS_INT64                     // if unsigned _int64 is recognised
                                        // used by newran03
                                        
// comment out next line if Exception causes a problem
#define TypeDefException

//*********************** end of options set by user ********************


// for Gnu C++ version 3
#if defined __GNUG__ && __GNUG__ >= 3
   #define _STANDARD_                   // use standard library
   #define ios_format_flags ios::fmtflags
#endif

// for Intel C++ for Linux
#if defined __ICC
   #define _STANDARD_                   // use standard library
   #define ios_format_flags ios::fmtflags
#endif

// for Microsoft Visual C++ 7 and above (and Intel simulating these)
#if defined _MSC_VER && _MSC_VER >= 1300
   #define _STANDARD_                   // use standard library
#endif


#ifdef _STANDARD_                       // using standard library
   #include <cstdlib>
/*   #if defined _MSC_VER && _MSC_VER == 1200
      #include <limits>              // for VC++6
      #endif*/
   #ifdef WANT_STREAM
      #include <iostream>
      #include <iomanip>
   #endif
   #ifdef WANT_MATH
      #include <cmath>
   #endif
   #ifdef WANT_STRING
      #include <cstring>
   #endif
   #ifdef WANT_TIME
      #include <ctime>
   #endif
   #ifdef WANT_FSTREAM
      #include <fstream>
   #endif
   using namespace std;
#else

#define DEFAULT_HEADER                  // use AT&T style header
                                        // if no other compiler is recognised

#ifdef _MSC_VER                         // Microsoft
   #include <stdlib.h>

//   reactivate these statements to run under MSC version 7.0
//   typedef int jmp_buf[9];
//   extern "C"
//   {
//      int __cdecl setjmp(jmp_buf);
//      void __cdecl longjmp(jmp_buf, int);
//   }

   #ifdef WANT_STREAM
      #include <iostream.h>
      #include <iomanip.h>
   #endif
   #ifdef WANT_MATH
      #include <math.h>
      #include <float.h>
   #endif
   #ifdef WANT_STRING
      #include <string.h>
   #endif
   #ifdef WANT_TIME
      #include <time.h>
   #endif
   #ifdef WANT_FSTREAM
      #include <fstream.h>
   #endif
   #undef DEFAULT_HEADER
#endif

#ifdef __ZTC__                          // Zortech
   #include <stdlib.h>
   #ifdef WANT_STREAM
      #include <iostream.hpp>
      #include <iomanip.hpp>
      #define flush ""                  // not defined in iomanip?
   #endif
   #ifdef WANT_MATH
      #include <math.h>
      #include <float.h>
   #endif
   #ifdef WANT_STRING
      #include <string.h>
   #endif
   #ifdef WANT_TIME
      #include <time.h>
   #endif
   #ifdef WANT_FSTREAM
      #include <fstream.h>
   #endif
   #undef DEFAULT_HEADER
#endif

#if defined __BCPLUSPLUS__ || defined __TURBOC__  // Borland or Turbo
   #include <stdlib.h>
   #ifdef WANT_STREAM
      #include <iostream.h>
      #include <iomanip.h>
   #endif
   #ifdef WANT_MATH
      #include <math.h>
      #include <float.h>            // Borland has both float and values
                                    // but values.h returns +INF for
                                    // MAXDOUBLE in BC5
   #endif
   #ifdef WANT_STRING
      #include <string.h>
   #endif
   #ifdef WANT_TIME
      #include <time.h>
   #endif
   #ifdef WANT_FSTREAM
      #include <fstream.h>
   #endif
   #undef DEFAULT_HEADER
#endif

#ifdef __GNUG__                         // Gnu C++
   #include <stdlib.h>
   #ifdef WANT_STREAM
      #include <iostream.h>
      #include <iomanip.h>
   #endif
   #ifdef WANT_MATH
      #include <math.h>
      #include <float.h>
   #endif
   #ifdef WANT_STRING
      #include <string.h>
   #endif
   #ifdef WANT_TIME
      #include <time.h>
   #endif
   #ifdef WANT_FSTREAM
      #include <fstream.h>
   #endif
   #undef DEFAULT_HEADER
#endif

#ifdef __WATCOMC__                      // Watcom C/C++
   #include <stdlib.h>
   #ifdef WANT_STREAM
      #include <iostream.h>
      #include <iomanip.h>
   #endif
   #ifdef WANT_MATH
      #include <math.h>
      #include <float.h>
   #endif
   #ifdef WANT_STRING
      #include <string.h>
   #endif
   #ifdef WANT_TIME
      #include <time.h>
   #endif
   #ifdef WANT_FSTREAM
      #include <fstream.h>
   #endif
   #undef DEFAULT_HEADER
#endif


#ifdef macintosh                        // MPW C++ on the Mac
#include <stdlib.h>
#ifdef WANT_STREAM
#include <iostream.h>
#include <iomanip.h>
#endif
#ifdef WANT_MATH
#include <float.h>
#include <math.h>
#endif
#ifdef WANT_STRING
#include <string.h>
#endif
#ifdef WANT_TIME
#include <time.h>
#endif
#ifdef WANT_FSTREAM
#include <fstream.h>
#endif
#undef DEFAULT_HEADER
#endif

#ifdef use_float_h                      // use float.h for precision values
#include <stdlib.h>
#ifdef WANT_STREAM
#include <iostream.h>
#include <iomanip.h>
#endif
#ifdef WANT_MATH
#include <float.h>
#include <math.h>
#endif
#ifdef WANT_STRING
#include <string.h>
#endif
#ifdef WANT_TIME
#include <time.h>
#endif
#ifdef WANT_FSTREAM
#include <fstream.h>
#endif
#undef DEFAULT_HEADER
#endif


#ifdef DEFAULT_HEADER                   // for example AT&T
#define ATandT
#include <stdlib.h>
#ifdef WANT_STREAM
#include <iostream.h>
#include <iomanip.h>
#endif
#ifdef WANT_MATH
#include <math.h>
#define SystemV                         // use System V
#include <values.h>
#endif
#ifdef WANT_STRING
#include <string.h>
#endif
#ifdef WANT_TIME
#include <time.h>
#endif
#ifdef WANT_FSTREAM
#include <fstream.h>
#endif
#endif                                  // DEFAULT_HEADER

#endif                                  // _STANDARD_

#ifdef use_namespace
namespace RBD_COMMON {
#endif


#ifdef USING_FLOAT                      // set precision type to float
typedef float Real;
typedef double long_Real;
#endif

#ifdef USING_DOUBLE                     // set precision type to double
typedef double Real;
typedef long double long_Real;
#endif


// This is for (very old) compilers that do not have bool automatically defined

#ifndef bool_LIB
#define bool_LIB 0

class bool
{
        int value;
public:
        bool(const int b) { value = b ? 1 : 0; }
        bool(const void* b) { value = b ? 1 : 0; }
        bool() {}
        operator int() const { return value; }
        int operator!() const { return !value; }
};


const bool true = 1;
const bool false = 0;

#endif


#ifdef use_namespace
}
#endif


#ifdef use_namespace
namespace RBD_COMMON {}
namespace RBD_LIBRARIES                 // access all my libraries
{
   using namespace RBD_COMMON;
}
#endif


#endif
