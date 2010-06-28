#ifndef __vtkMeasurementsWin32Header_h
#define __vtkMeasurementsWin32Header_h

//#include <vtkMeasurementsConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(Measurements_EXPORTS)
#define VTK_MEASUREMENTS_EXPORT __declspec( dllexport ) 
#else
#define VTK_MEASUREMENTS_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_MEASUREMENTS_EXPORT 
#endif

#endif
