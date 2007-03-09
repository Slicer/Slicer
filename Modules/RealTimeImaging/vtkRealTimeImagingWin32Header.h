
#ifndef __vtkRealTimeImagingWin32Header_h
#define __vtkRealTimeImagingWin32Header_h

#include <vtkRealTimeImagingConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(RealTimeImaging_EXPORTS)
#define VTK_REALTIMEIMAGING_EXPORT __declspec( dllexport ) 
#else
#define VTK_REALTIMEIMAGING_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_REALTIMEIMAGING_EXPORT 
#endif
#endif
