// /  BRAINSCommonLibWin32Header - manage Windows system differences
// /
// / The BRAINSCommonLibWin32Header captures some system differences between
// Unix
// / and Windows operating systems.

#ifndef __BRAINSCommonLibWin32Header_h
#define __BRAINSCommonLibWin32Header_h

#include <BRAINSCommonLib.h>

#if defined( WIN32 ) && !defined( BRAINSCommonLib_STATIC )
#  if defined( BRAINSCommonLib_EXPORTS )
#    define BRAINSCommonLib_EXPORT __declspec(dllexport)
#  else
#    define BRAINSCommonLib_EXPORT __declspec(dllimport)
#  endif
#else
#  define BRAINSCommonLib_EXPORT
#endif

#endif
