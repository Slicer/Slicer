/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

/// vtkITKExport
///
/// The vtkITKExport captures some system differences between Unix
/// and Windows operating systems.

#ifndef __vtkITKExport_h
#define __vtkITKExport_h

#include <vtkITKConfigure.h>

#if defined(WIN32) && !defined(VTKITK_STATIC)
#if defined(vtkITK_EXPORTS)
#define VTK_ITK_EXPORT __declspec( dllexport )
#else
#define VTK_ITK_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_ITK_EXPORT
#endif

#endif
