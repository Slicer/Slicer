
#ifndef __vtkChangeTrackerWin32Header_h
#define __vtkChangeTrackerWin32Header_h

#include <vtkChangeTrackerConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(ChangeTracker_EXPORTS)
#define VTK_CHANGETRACKER_EXPORT __declspec( dllexport ) 
#else
#define VTK_CHANGETRACKER_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_CHANGETRACKER_EXPORT 
#endif
#endif
