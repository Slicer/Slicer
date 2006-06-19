/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   MRML
  Module:    $RCSfile: vtkMRML.h,v $
  Date:      $Date$
  Version:   $Rev$

=========================================================================auto=*/

/*
 * This is needed for loading mrml code as module.
 */


//
// use an ifdef on MRML_VTK5 to flag code that won't
// compile on vtk4.4 and before
//
#if ( (VTK_MAJOR_VERSION >= 5) || ( VTK_MAJOR_VERSION == 4 && VTK_MINOR_VERSION >= 5 ) )
#define MRML_VTK5
#endif

#include "vtkMRMLWin32Header.h"
