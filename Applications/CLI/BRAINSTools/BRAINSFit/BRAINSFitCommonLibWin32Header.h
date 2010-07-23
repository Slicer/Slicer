// /  BRAINSFitCommonLibWin32Header - manage Windows system differences
// /
// / The BRAINSFitCommonLibWin32Header captures some system differences between
// Unix
// / and Windows operating systems.

#ifndef __BRAINSFitCommonLibWin32Header_h
#define __BRAINSFitCommonLibWin32Header_h

#include <BRAINSFitCommonLib.h>

#if defined( WIN32 ) && !defined( BRAINSFitCommonLib_STATIC )
#  if defined( BRAINSFitCommonLib_EXPORTS )
#    define BRAINSFitCommonLib_EXPORT __declspec(dllexport)
#  else
#    define BRAINSFitCommonLib_EXPORT __declspec(dllimport)
#  endif
#else
#  define BRAINSFitCommonLib_EXPORT
#endif

#endif
