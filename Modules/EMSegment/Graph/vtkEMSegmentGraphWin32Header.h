
#ifndef __vtkEMSegmentGraphWin32Header_h
#define __vtkEMSegmentGraphWin32Header_h

#include <vtkEMSegmentGraphConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(EMSegmentGraph_EXPORTS)
#define VTK_EMSEGMENT_GRAPH_EXPORT __declspec( dllexport ) 
#else
#define VTK_EMSEGMENT_GRAPH_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_EMSEGMENT_GRAPH_EXPORT 
#endif
#endif
