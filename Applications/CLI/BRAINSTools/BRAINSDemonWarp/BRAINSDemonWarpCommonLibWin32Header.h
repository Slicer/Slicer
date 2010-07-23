// /  BRAINSDemonWarpCommonLibWin32Header - manage Windows system differences
// /
// / The BRAINSDemonWarpCommonLibWin32Header captures some system differences
// between Unix
// / and Windows operating systems.

#ifndef __BRAINSDemonWarpCommonLibWin32Header_h
#define __BRAINSDemonWarpCommonLibWin32Header_h

#include <BRAINSDemonWarpCommonLib.h>

#if defined( WIN32 ) && !defined( BRAINSDemonWarpCommonLib_STATIC )
#  if defined( BRAINSDemonWarpCommonLib_EXPORTS )
#    define BRAINSDemonWarpCommonLib_EXPORT __declspec(dllexport)
#  else
#    define BRAINSDemonWarpCommonLib_EXPORT __declspec(dllimport)
#  endif
#else
#  define BRAINSDemonWarpCommonLib_EXPORT
#endif

#endif
