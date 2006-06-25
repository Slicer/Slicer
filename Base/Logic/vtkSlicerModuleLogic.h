/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerModuleLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerModuleLogic - superclass for slicer module logic classes
// .SECTION Description
// Superclass for all slicer m,odule logic classes
// There must be a corresponding vtkSlicerGUI subclass corresponding 
// to each logic class that handles all GUI interaction (no GUI code
// goes in the logic class).

#ifndef __vtkSlicerModuleLogic_h
#define __vtkSlicerModuleLogic_h

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerLogic.h"
#include "vtkSlicerApplicationLogic.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerModuleLogic : public vtkSlicerLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkSlicerModuleLogic *New();
  vtkTypeRevisionMacro(vtkSlicerModuleLogic,vtkSlicerLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get access to overall application state
  vtkGetObjectMacro(ApplicationLogic, vtkSlicerApplicationLogic);
  vtkSetObjectMacro(ApplicationLogic, vtkSlicerApplicationLogic);

protected:
  vtkSlicerModuleLogic();
  ~vtkSlicerModuleLogic();
  vtkSlicerModuleLogic(const vtkSlicerModuleLogic&);
  void operator=(const vtkSlicerModuleLogic&);

  vtkSlicerApplicationLogic *ApplicationLogic;

};

#endif

