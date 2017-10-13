#ifndef __vtkRemoteIOExport_h
#define __vtkRemoteIOExport_h

#include <vtkRemoteIOConfigure.h>

#if defined(WIN32) && !defined(RemoteIO_STATIC)
#if defined(RemoteIO_EXPORTS)
#define VTK_RemoteIO_EXPORT __declspec( dllexport )
#else
#define VTK_RemoteIO_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_RemoteIO_EXPORT
#endif

#endif
