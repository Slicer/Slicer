#ifndef __vtkCustomLayoutWin32Header_h
#define __vtkCustomLayoutWin32Header_h

#include <vtkCustomLayoutConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(CustomLayout_EXPORTS)
#define VTK_CUSTOMLAYOUT_EXPORT __declspec( dllexport ) 
#else
#define VTK_CUSTOMLAYOUT_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_CUSTOMLAYOUT_EXPORT 
#endif

#endif
