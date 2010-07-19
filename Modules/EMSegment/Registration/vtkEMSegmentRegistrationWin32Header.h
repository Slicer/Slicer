
#ifndef __vtkEMSegmentRegistrationWin32Header_h
#define __vtkEMSegmentRegistrationWin32Header_h

#include <vtkEMSegmentRegistrationConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(EMSegmentRegistration_EXPORTS)
#define VTK_EMSEGMENT_REGISTRATION_EXPORT __declspec( dllexport ) 
#else
#define VTK_EMSEGMENT_REGISTRATION_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_EMSEGMENT_REGISTRATION_EXPORT 
#endif
#endif
