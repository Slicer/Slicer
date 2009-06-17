#ifndef __vtkModelMirrorWin32Header_h
#define __vtkModelMirrorWin32Header_h

#include <vtkModelMirrorConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(ModelMirror_EXPORTS)
#define VTK_MODELMIRROR_EXPORT __declspec( dllexport ) 
#else
#define VTK_MODELMIRROR_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_MODELMIRROR_EXPORT 
#endif

#endif
