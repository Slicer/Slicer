#ifndef __BRAINSFITPRIMARY_h
#define __BRAINSFITPRIMARY_h

#include "BRAINSFitCommonLibWin32Header.h"

#if defined(WIN32) && !defined(BRAINSFit_STATIC)
#if defined(BRAINSFit)
#define MODULE_EXPORT __declspec( dllexport ) 
#else
#define MODULE_EXPORT __declspec( dllimport ) 
#endif
#else
#define MODULE_EXPORT
#endif

extern "C" MODULE_EXPORT int BRAINSFitPrimary(int argc, char *argv[]);

#endif // __BRAINSFITPRIMARY_h
