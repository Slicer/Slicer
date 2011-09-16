// needs to be here for the commandline tool and its emsegmenter core dependies

#ifndef __vtkSlicerEMSegmentModuleLogicExport_h
#define __vtkSlicerEMSegmentModuleLogicExport_h

#if defined(WIN32) && !defined(vtkSlicerEMSegmentModuleLogic_STATIC)
 #if defined(vtkSlicerEMSegmentModuleLogic_EXPORTS)
  #define VTK_EMSEGMENT_EXPORT __declspec( dllexport )
 #else
  #define VTK_EMSEGMENT_EXPORT __declspec( dllimport )
 #endif
#else
 #define VTK_EMSEGMENT_EXPORT
#endif

#endif
