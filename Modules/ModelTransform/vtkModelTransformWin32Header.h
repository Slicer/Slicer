#ifndef __vtkModelTransformWin32Header_h
#define __vtkModelTransformWin32Header_h

#include <vtkModelTransformConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(ModelTransform_EXPORTS)
#define VTK_MODELTRANSFORM_EXPORT __declspec( dllexport ) 
#else
#define VTK_MODELTRANSFORM_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_MODELTRANSFORM_EXPORT 
#endif

#endif
