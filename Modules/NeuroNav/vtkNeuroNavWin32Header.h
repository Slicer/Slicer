
#ifndef __vtkNeuroNavWin32Header_h
#define __vtkNeuroNavWin32Header_h

#include <vtkNeuroNavConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(NeuroNav_EXPORTS)
#define VTK_NEURONAV_EXPORT __declspec( dllexport ) 
#else
#define VTK_NEURONAV_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_NEURONAV_EXPORT 
#endif
#endif
