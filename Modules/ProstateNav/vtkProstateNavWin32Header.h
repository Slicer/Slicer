#ifndef __vtkProstateNavWin32Header_h
#define __vtkProstateNavWin32Header_h

#include <vtkProstateNavConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(ProstateNav_EXPORTS)
#define VTK_PROSTATENAV_EXPORT __declspec( dllexport ) 
#else
#define VTK_PROSTATENAV_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_PROSTATENAV_EXPORT 
#endif
#endif
