
#ifndef __vtkIGTWin32Header_h
#define __vtkIGTWin32Header_h

#include <IGTConfigure.h>

#if defined(WIN32) && !defined(IGT_STATIC)
#if defined(IGT_EXPORTS)
#define VTK_IGT_EXPORT __declspec( dllexport ) 
#else
#define VTK_IGT_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_IGT_EXPORT
#endif

#endif


