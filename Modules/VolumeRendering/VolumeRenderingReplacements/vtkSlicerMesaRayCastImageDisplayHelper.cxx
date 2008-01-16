/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerMesaRayCastImageDisplayHelper.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// Make sure this is first, so any includes of gl.h can be stoped if needed
#define VTK_IMPLEMENT_MESA_CXX

#include "MangleMesaInclude/gl_mangle.h"
#include "MangleMesaInclude/gl.h"

#include <math.h>
#include "vtkSlicerMesaRayCastImageDisplayHelper.h"

// make sure this file is included before the #define takes place
// so we don't get two vtkSlicerMesaRayCastImageDisplayHelper classes defined.
#include "vtkSlicerOpenGLRayCastImageDisplayHelper.h"
#include "vtkSlicerMesaRayCastImageDisplayHelper.h"

// Make sure vtkSlicerMesaRayCastImageDisplayHelper is a copy of 
// vtkSlicerOpenGLRayCastImageDisplayHelper with vtkSlicerOpenGLRayCastImageDisplayHelper 
// replaced with vtkSlicerMesaRayCastImageDisplayHelper
#define vtkSlicerOpenGLRayCastImageDisplayHelper vtkSlicerMesaRayCastImageDisplayHelper
#include "vtkSlicerOpenGLRayCastImageDisplayHelper.cxx"
#undef vtkSlicerOpenGLRayCastImageDisplayHelper

vtkCxxRevisionMacro(vtkSlicerMesaRayCastImageDisplayHelper, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkSlicerMesaRayCastImageDisplayHelper);
