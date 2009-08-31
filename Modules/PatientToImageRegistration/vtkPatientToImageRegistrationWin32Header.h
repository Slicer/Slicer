
#ifndef __vtkPatientToImageRegistrationWin32Header_h
#define __vtkPatientToImageRegistrationWin32Header_h

#include <vtkPatientToImageRegistrationConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(PatientToImageRegistration_EXPORTS)
#define VTK_PatientToImageRegistration_EXPORT __declspec( dllexport ) 
#else
#define VTK_PatientToImageRegistration_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_PatientToImageRegistration_EXPORT 
#endif
#endif
