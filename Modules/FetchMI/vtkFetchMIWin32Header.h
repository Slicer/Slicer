#ifndef __vtkFetchMIWin32Header_h
#define __vtkFetchMIWin32Header_h

#include <vtkFetchMIConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(FetchMI_EXPORTS)
#define VTK_FETCHMI_EXPORT __declspec( dllexport ) 
#else
#define VTK_FETCHMI_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_FETCHMI_EXPORT 
#endif

#endif
