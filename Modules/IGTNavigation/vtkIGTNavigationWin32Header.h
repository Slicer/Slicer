
#ifndef __vtkIGTNavigationWin32Header_h
#define __vtkIGTNavigationWin32Header_h

#include <vtkIGTNavigationConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(IGT_EXPORTS)
#define VTK_IGT_EXPORT __declspec( dllexport ) 
#else
#define VTK_IGT_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_IGT_EXPORT 
#endif
#endif
