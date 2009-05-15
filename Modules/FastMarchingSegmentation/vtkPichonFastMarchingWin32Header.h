
#ifndef __vtkPichonFastMarchingWin32Header_h
#define __vtkPichonFastMarchingWin32Header_h

#include <vtkPichonFastMarchingConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(FastMarching_EXPORTS)
#define VTK_PICHONFASTMARCHING_EXPORT __declspec( dllexport ) 
#else
#define VTK_PICHONFASTMARCHING_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_PICHONFASTMARCHING_EXPORT 
#endif
#endif
