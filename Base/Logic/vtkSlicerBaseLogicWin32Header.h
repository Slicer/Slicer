/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkSlicerBaseLogicWin32Header - manage Windows system differences
///
/// The vtkSlicerBaseLogicWin32Header captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkSlicerBaseLogicWin32Header_h
#define __vtkSlicerBaseLogicWin32Header_h

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
