
#ifndef __vtkTumorGrowthWin32Header_h
#define __vtkTumorGrowthWin32Header_h

#include <vtkTumorGrowthConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(TumorGrowth_EXPORTS)
#define VTK_TUMORGROWTH_EXPORT __declspec( dllexport ) 
#else
#define VTK_TUMORGROWTH_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_TUMORGROWTH_EXPORT 
#endif
#endif
