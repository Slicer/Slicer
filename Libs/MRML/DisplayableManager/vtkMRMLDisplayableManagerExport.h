/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkMRMLDisplayableManagerExport
///
/// The vtkMRMLDisplayableManagerExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkMRMLDisplayableManagerExport_h
#define __vtkMRMLDisplayableManagerExport_h

#include <vtkMRMLDisplayableManagerConfigure.h>

#if defined(WIN32) && !defined(MRMLDisplayableManager_STATIC)
#if defined(MRMLDisplayableManager_EXPORTS)
#define VTK_MRML_DISPLAYABLEMANAGER_EXPORT __declspec( dllexport )
#else
#define VTK_MRML_DISPLAYABLEMANAGER_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_MRML_DISPLAYABLEMANAGER_EXPORT
#endif

#if defined(MRMLDisplayableManager_AUTOINIT)
#include <vtkAutoInit.h>
VTK_AUTOINIT(MRMLDisplayableManager)
#endif

#endif
