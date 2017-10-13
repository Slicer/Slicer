/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// vtkSlicerBaseLogicExport
///
/// The vtkSlicerBaseLogicExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkSlicerBaseLogicExport_h
#define __vtkSlicerBaseLogicExport_h

#include <vtkSlicerBaseLogicConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(SlicerBaseLogic_EXPORTS)
#define VTK_SLICER_BASE_LOGIC_EXPORT __declspec( dllexport )
#else
#define VTK_SLICER_BASE_LOGIC_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_SLICER_BASE_LOGIC_EXPORT
#endif

#endif
