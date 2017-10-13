/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// vtkSlicerBaseCLIExport
//
// The vtkSlicerBaseCLIExport captures some system differences between Unix
// and Windows operating systems.

#ifndef __vtkSlicerBaseCLIExport_h
#define __vtkSlicerBaseCLIExport_h

#include <vtkSlicerBaseCLIConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(SlicerBaseCLI_EXPORTS)
#define VTK_SLICER_BASE_CLI_EXPORT __declspec( dllexport )
#else
#define VTK_SLICER_BASE_CLI_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_SLICER_BASE_CLI_EXPORT
#endif

#endif
