/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerLogic - superclass for slicer logic classes
// .SECTION Description
// Superclass for all slicer logic classes (application, views, slices).
// There must be a corresponding vtkSlicerGUI subclass corresponding 
// to each logic class that handles all GUI interaction (no GUI code
// goes in the logic class).

#ifndef __vtkSlicerLogic_h
#define __vtkSlicerLogic_h

#include "vtkSlicerBaseLogic.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerLogic : public vtkObject 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerLogic *New();
  vtkTypeRevisionMacro(vtkSlicerLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Additional functionality:
  //  -- an undo stack (also need helper classes)
  //  -- a tracing/macro infrastructure
    
protected:
  vtkSlicerLogic();
  ~vtkSlicerLogic();
  vtkSlicerLogic(const vtkSlicerLogic&);
  void operator=(const vtkSlicerLogic&);
};

#endif

