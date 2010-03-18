
#ifndef __vtkLiverAblationWin32Header_h
#define __vtkLiverAblationWin32Header_h

#include <vtkLiverAblationConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(LiverAblation_EXPORTS)
#define VTK_LIVERABLATION_EXPORT __declspec( dllexport ) 
#else
#define VTK_LIVERABLATION_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_LIVERABLATION_EXPORT 
#endif
#endif
