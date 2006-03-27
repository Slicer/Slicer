/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkModulesBaseClassesWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkModulesBaseClassesWin32Header - manage Windows system differences
// .SECTION Description
// The vtkModulesBaseClassesWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkModulesBaseClassesWin32Header_h
#define __vtkModulesBaseClassesWin32Header_h

#include <vtkModulesBaseClassesConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(ModulesBaseClasses_EXPORTS)
#define VTK_MODULES_BASE_CLASSES_EXPORT __declspec( dllexport ) 
#else
#define VTK_MODULES_BASE_CLASSES_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_MODULES_BASE_CLASSES_EXPORT
#endif

#endif
