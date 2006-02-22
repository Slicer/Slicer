/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D MRML
  Module:    $RCSfile: vtkMRMLWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkMRMLWin32Header - manage Windows system differences
// .SECTION Description
// The vtkMRMLWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkMRMLWin32Header_h
#define __vtkMRMLWin32Header_h

#include <vtkMRMLConfigure.h>

#if defined(WIN32) && !defined(VTKMRML_STATIC)
#if defined(MRML_EXPORTS)
#define VTK_MRML_EXPORT __declspec( dllexport ) 
#else
#define VTK_MRML_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_MRML_EXPORT
#endif

#endif
