/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D MRML
  Module:    $RCSfile: vtkMRMLDisplayableManagerWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
///  vtkMRMLDisplayableManagerWin32Header - manage Windows system differences
///
/// The vtkMRMLDisplayableManagerWin32Header captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkMRMLDisplayableManagerWin32Header_h
#define __vtkMRMLDisplayableManagerWin32Header_h

#include <vtkMRMLDisplayableManagerConfigure.h>

#if defined(WIN32) && !defined(VTKMRMLDisplayableManager_STATIC)
#if defined(MRMLDisplayableManager_EXPORTS)
#define VTK_MRML_DISPLAYABLEMANAGER_EXPORT __declspec( dllexport )
#else
#define VTK_MRML_DISPLAYABLEMANAGER_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_MRML_DISPLAYABLEMANAGER_EXPORT
#endif

#endif
