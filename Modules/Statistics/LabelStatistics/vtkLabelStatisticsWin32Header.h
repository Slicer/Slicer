/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkLabelStatisticsWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkLabelStatisticsWin32Header - manage Windows system differences
// .SECTION Description
// The vtkLabelStatisticsWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkLabelStatisticsWin32Header_h
#define __vtkLabelStatisticsWin32Header_h

#include <vtkLabelStatisticsConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(LabelStatistics_EXPORTS)
#define VTK_LABELSTATISTICS_EXPORT __declspec( dllexport ) 
#else
#define VTK_LABELSTATISTICS_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_LABELSTATISTICS_EXPORT 
#endif

#endif
