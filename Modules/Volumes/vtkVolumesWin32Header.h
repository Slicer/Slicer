#ifndef __vtkVolumesWin32Header_h
#define __vtkVolumesWin32Header_h

#include <vtkVolumesConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(Volumes_EXPORTS)
#define VTK_VOLUMES_EXPORT __declspec( dllexport ) 
#else
#define VTK_VOLUMES_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_VOLUMES_EXPORT 
#endif

#endif
