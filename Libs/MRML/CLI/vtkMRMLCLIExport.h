/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// vtkMRMLCLIExport
///
/// The vtkMRMLCLIExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkMRMLCLIExport_h
#define __vtkMRMLCLIExport_h

#include <vtkMRMLCLIConfigure.h>

#if defined(WIN32) && !defined(VTKMRMLCLI_STATIC)
#if defined(MRMLCLI_EXPORTS)
#define VTK_MRML_CLI_EXPORT __declspec( dllexport )
#else
#define VTK_MRML_CLI_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_MRML_CLI_EXPORT
#endif

#endif
