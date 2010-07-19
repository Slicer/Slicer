
#ifndef __vtkEMSegmentAlgorithmWin32Header_h
#define __vtkEMSegmentAlgorithmWin32Header_h

#include <vtkEMSegmentAlgorithmConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(EMSegmentAlgorithm_EXPORTS)
#define VTK_EMSEGMENT_ALGORITHM_EXPORT __declspec( dllexport ) 
#else
#define VTK_EMSEGMENT_ALGORITHM_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_EMSEGMENT_ALGORITHM_EXPORT
#endif
#endif
