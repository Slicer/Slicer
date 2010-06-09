
#ifndef __vtkBrainlabModuleWin32Header_h
#define __vtkBrainlabModuleWin32Header_h

#include <vtkBrainlabModuleConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(BrainlabModule_EXPORTS)
#define VTK_IGT_EXPORT __declspec( dllexport ) 
#else
#define VTK_IGT_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_IGT_EXPORT 
#endif
#endif
