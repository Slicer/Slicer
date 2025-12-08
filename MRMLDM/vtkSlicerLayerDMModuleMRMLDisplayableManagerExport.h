///  vtkSlicerLayerDMModuleMRMLDisplayableManagerExport
///
/// The vtkSlicerLayerDMModuleMRMLDisplayableManagerExport captures some system differences between Unix
/// and Windows operating systems, and enables auto init for the VTK module.

#ifndef __vtkSlicerLayerDMModuleMRMLDisplayableManagerExport_h
#define __vtkSlicerLayerDMModuleMRMLDisplayableManagerExport_h

#if defined(_WIN32) && !defined(MRMLLayerDM_STATIC)
# if defined(vtkSlicerLayerDMModuleMRMLDisplayableManager_EXPORTS)
#  define VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT __declspec(dllexport)
# else
#  define VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT __declspec(dllimport)
# endif
#else
# define VTK_SLICER_LAYERDM_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
#endif

#if defined(vtkSlicerLayerDMModuleMRMLDisplayableManager_AUTOINIT)
# include <vtkAutoInit.h>
VTK_AUTOINIT(vtkSlicerLayerDMModuleMRMLDisplayableManager)
#endif

#endif
