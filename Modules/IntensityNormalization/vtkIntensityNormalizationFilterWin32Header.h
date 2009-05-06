/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkIntensityNormalizationFilterWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkIntensityNormalizationFilterWin32Header - manage Windows system differences
// .SECTION Description
// The vtkIntensityNormalizationFilterWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkIntensityNormalizationFilterWin32Header_h
#define __vtkIntensityNormalizationFilterWin32Header_h

#include <vtkIntensityNormalizationFilterConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(IntensityNormalizationFilter_EXPORTS)
#define VTK_IntensityNormalizationFilter_EXPORT __declspec( dllexport ) 
#else
#define VTK_IntensityNormalizationFilter_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_IntensityNormalizationFilter_EXPORT 
#endif

#endif
