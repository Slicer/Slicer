// /  BRAINSROIAutoCommonLibWin32Header - manage Windows system differences
// /
// / The BRAINSROIAutoCommonLibWin32Header captures some system differences
// between Unix
// / and Windows operating systems.

#ifndef __BRAINSROIAutoCommonLibWin32Header_h
#define __BRAINSROIAutoCommonLibWin32Header_h

#include <BRAINSROIAutoCommonLib.h>

#if defined( WIN32 ) && !defined( BRAINSROIAutoCommonLib_STATIC )
#  if defined( BRAINSROIAutoCommonLib_EXPORTS )
#    define BRAINSROIAutoCommonLib_EXPORT __declspec(dllexport)
#  else
#    define BRAINSROIAutoCommonLib_EXPORT __declspec(dllimport)
#  endif
#else
#  define BRAINSROIAutoCommonLib_EXPORT
#endif

#endif
