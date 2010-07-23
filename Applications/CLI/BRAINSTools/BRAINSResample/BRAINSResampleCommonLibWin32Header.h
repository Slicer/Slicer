// /  BRAINSResampleCommonLibWin32Header - manage Windows system differences
// /
// / The BRAINSResampleCommonLibWin32Header captures some system differences
// between Unix
// / and Windows operating systems.

#ifndef __BRAINSResampleCommonLibWin32Header_h
#define __BRAINSResampleCommonLibWin32Header_h

#include <BRAINSResampleCommonLib.h>

#if defined( WIN32 ) && !defined( BRAINSResampleCommonLib_STATIC )
#  if defined( BRAINSResampleCommonLib_EXPORTS )
#    define BRAINSResampleCommonLib_EXPORT __declspec(dllexport)
#  else
#    define BRAINSResampleCommonLib_EXPORT __declspec(dllimport)
#  endif
#else
#  define BRAINSResampleCommonLib_EXPORT
#endif

#endif
