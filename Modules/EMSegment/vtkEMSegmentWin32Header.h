
#ifndef __vtkEMSegmentWin32Header_h
#define __vtkEMSegmentWin32Header_h

#include <vtkEMSegmentConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(EMSegment_EXPORTS)
#define VTK_EMSEGMENT_EXPORT __declspec( dllexport ) 
#else
#define VTK_EMSEGMENT_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_EMSEGMENT_EXPORT 
#endif
#endif
