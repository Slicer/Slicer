// /  BRAINSFitIGTCommonLibWin32Header - manage Windows system differences
// /
// / The BRAINSFitIGTCommonLibWin32Header captures some system differences
// between
// Unix
// / and Windows operating systems.

#ifndef __BRAINSFitIGTCommonLibWin32Header_h
#define __BRAINSFitIGTCommonLibWin32Header_h

#include <BRAINSFitIGTCommonLib.h>

#if defined( WIN32 ) && !defined( BRAINSFitIGTCommonLib_STATIC )
#  if defined( BRAINSFitIGTCommonLib_EXPORTS )
#    define BRAINSFitIGTCommonLib_EXPORT __declspec(dllexport)
#  else
#    define BRAINSFitIGTCommonLib_EXPORT __declspec(dllimport)
#  endif
#else
#  define BRAINSFitIGTCommonLib_EXPORT
#endif

#endif
