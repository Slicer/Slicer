/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer 
  Module:    $RCSfile: vtkIGTWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkIGTWin32Header - manage Windows system differences
// .SECTION Description
// The vtkIGTWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkIGTWin32Header_h
#define __vtkIGTWin32Header_h

#include <vtkIGTConfigure.h>

#if defined(WIN32) && !defined(VTKIGT_STATIC)
#if defined(IGT_EXPORTS)
#define VTK_IGT_EXPORT __declspec( dllexport ) 
#else
#define VTK_IGT_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_IGT_EXPORT
#endif

#endif
