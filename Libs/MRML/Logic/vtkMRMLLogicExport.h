/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkMRMLLogicExport
///
/// The vtkMRMLLogicExport captures some system differences between Unix
/// and Windows operating systems.


#ifndef __vtkMRMLLogicExport_h
#define __vtkMRMLLogicExport_h

#include <vtkMRMLLogicConfigure.h>

#if defined(WIN32) && !defined(VTKMRMLLogic_STATIC)
#if defined(MRMLLogic_EXPORTS)
#define VTK_MRML_LOGIC_EXPORT __declspec( dllexport )
#else
#define VTK_MRML_LOGIC_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_MRML_LOGIC_EXPORT
#endif

#endif
