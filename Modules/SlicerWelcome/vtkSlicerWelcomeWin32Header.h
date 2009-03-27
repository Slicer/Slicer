#ifndef __vtkSlicerWelcomeWin32Header_h
#define __vtkSlicerWelcomeWin32Header_h

#include <vtkSlicerWelcomeConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(SlicerWelcome_EXPORTS)
#define VTK_SLICERWELCOME_EXPORT __declspec( dllexport ) 
#else
#define VTK_SLICERWELCOME_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_SLICERWELCOME_EXPORT 
#endif

#endif
