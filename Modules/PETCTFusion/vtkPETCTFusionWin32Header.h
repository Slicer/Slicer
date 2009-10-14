#ifndef __vtkPETCTFusionWin32Header_h
#define __vtkPETCTFusionWin32Header_h

#include <vtkPETCTFusionConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(PETCTFusion_EXPORTS)
#define VTK_PETCTFUSION_EXPORT __declspec( dllexport ) 
#else
#define VTK_PETCTFUSION_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_PETCTFUSION_EXPORT 
#endif

#endif
