
#ifndef __vtkMRMLEMSWin32Header_h
#define __vtkMRMLEMSWin32Header_h

#include <vtkMRMLEMSConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(EMSegmentMRML_EXPORTS)
#define VTK_MRMLEMS_EXPORT __declspec( dllexport ) 
#else
#define VTK_MRMLEMS_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_MRMLEMS_EXPORT 
#endif
#endif
