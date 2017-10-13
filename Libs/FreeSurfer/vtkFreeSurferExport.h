/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// vtkFreeSurferExport
///
/// The vtkFreeSurferExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkFreeSurferExport_h
#define __vtkFreeSurferExport_h

#include <FreeSurferConfigure.h>

#if defined(WIN32) && !defined(FreeSurfer_STATIC)
#if defined(FreeSurfer_EXPORTS)
#define VTK_FreeSurfer_EXPORT __declspec( dllexport )
#else
#define VTK_FreeSurfer_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_FreeSurfer_EXPORT
#endif

#endif
