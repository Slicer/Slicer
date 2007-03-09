
#ifndef __IGTWin32Header_h
#define __IGTWin32Header_h

#include <IGTConfigure.h>

#if defined(WIN32) && !defined(IGT_STATIC)
#if defined(IGT_EXPORTS)
#define IGT_EXPORT __declspec( dllexport ) 
#else
#define IGT_EXPORT __declspec( dllimport ) 
#endif
#else
#define IGT_EXPORT
#endif

#endif


