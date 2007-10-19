/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumeMathWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkSlicerVolumeMathWin32Header - manage Windows system differences
// .SECTION Description
// The vtkSlicerVolumeMathWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkSlicerVolumeMathWin32Header_h
#define __vtkSlicerVolumeMathWin32Header_h

#include <vtkSlicerVolumeMathConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(SlicerVolumeMath_EXPORTS)
#define VTK_SLICERVOLUMEMATH_EXPORT __declspec( dllexport ) 
#else
#define VTK_SLICERVOLUMEMATH_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_SLICERVOLUMEMATH_EXPORT 
#endif

#endif
