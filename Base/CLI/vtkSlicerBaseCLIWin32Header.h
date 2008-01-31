/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerBaseCLIWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkSlicerBaseCLIWin32Header - manage Windows system differences
// .SECTION Description
// The vtkSlicerBaseCLIWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkSlicerBaseCLIWin32Header_h
#define __vtkSlicerBaseCLIWin32Header_h

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
