/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkWFEngineModuleLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkWFEngineModuleLogic - slicer logic class for query atlas
// .SECTION Description
// This class manages the logic associated with the Query Atlas

#ifndef __vtkWFEngineModuleLogic_h
#define __vtkWFEngineModuleLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkWFEngineModuleLogic.h"
#include "vtkMRMLWFEngineModuleNode.h"

//VTK_WFEngineModule_EXPORT
class VTK_WFENGINEMODULE_EXPORT vtkWFEngineModuleLogic : public vtkSlicerModuleLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkWFEngineModuleLogic *New();
  vtkTypeRevisionMacro(vtkWFEngineModuleLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  vtkGetObjectMacro (WFEngineModuleNode, vtkMRMLWFEngineModuleNode);
  vtkSetObjectMacro (WFEngineModuleNode, vtkMRMLWFEngineModuleNode);
  
  // The method that creates and runs VTK or ITK pipeline
  void Apply() { };

protected:
  vtkWFEngineModuleLogic();
  ~vtkWFEngineModuleLogic();
  vtkWFEngineModuleLogic(const vtkWFEngineModuleLogic&);
  void operator=(const vtkWFEngineModuleLogic&);
  vtkMRMLWFEngineModuleNode* WFEngineModuleNode;
};

#endif

