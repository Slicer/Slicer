/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// vtkMRMLExport
///
/// The vtkMRMLExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkMRMLExport_h
#define __vtkMRMLExport_h

#include <vtkMRMLConfigure.h>

#if defined(WIN32) && !defined(VTKMRML_STATIC)
#if defined(MRMLCore_EXPORTS)
#define VTK_MRML_EXPORT __declspec( dllexport )
#else
#define VTK_MRML_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_MRML_EXPORT
#endif

#endif
