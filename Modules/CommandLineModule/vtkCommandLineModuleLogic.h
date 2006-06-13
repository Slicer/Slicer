/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkCommandLineModuleLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkCommandLineModuleLogic_h
#define __vtkCommandLineModuleLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkCommandLineModule.h"
#include "vtkMRMLCommandLineModuleNode.h"

class VTK_COMMANDLINEMODULE_EXPORT vtkCommandLineModuleLogic : public vtkSlicerModuleLogic
{
public:
  static vtkCommandLineModuleLogic *New();
  vtkTypeMacro(vtkCommandLineModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (CommandLineModuleNode, vtkMRMLCommandLineModuleNode);
  vtkSetObjectMacro (CommandLineModuleNode, vtkMRMLCommandLineModuleNode);
  
  // The method that creates and runs VTK or ITK pipeline
  void Apply();
  
private:
  vtkCommandLineModuleLogic();
  ~vtkCommandLineModuleLogic();
  vtkCommandLineModuleLogic(const vtkCommandLineModuleLogic&);
  void operator=(const vtkCommandLineModuleLogic&);

  vtkMRMLCommandLineModuleNode* CommandLineModuleNode;
};

#endif

