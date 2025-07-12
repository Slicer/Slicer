/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   MRML
  Module:    $RCSfile: vtkMRML.h,v $
  Date:      $Date$
  Version:   $Rev$

=========================================================================auto=*/

/*
 * This is needed for loading mrml code as module.
 */

#ifndef vtkMRML_h
#define vtkMRML_h

#include "vtkMRMLExport.h"

// Macro for MRML application version comparison in preprocessor macros.
// Example:
// #if MRML_APPLICATION_SUPPORT_VERSION < MRML_VERSION_CHECK(4, 4, 0)
// ...
// #endif
#define MRML_VERSION_CHECK(major, minor, patch) ((major << 16) | (minor << 8) | (patch))

#endif
