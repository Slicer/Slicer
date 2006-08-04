/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkScriptedModuleLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkScriptedModuleLogic_h
#define __vtkScriptedModuleLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkScriptedModule.h"
#include "vtkMRMLScriptedModuleNode.h"

#include <string>

class VTK_SCRIPTED_EXPORT vtkScriptedModuleLogic : public vtkSlicerModuleLogic
{
public:
  static vtkScriptedModuleLogic *New();
  vtkTypeMacro(vtkScriptedModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (ScriptedModuleNode, vtkMRMLScriptedModuleNode);
  vtkSetObjectMacro (ScriptedModuleNode, vtkMRMLScriptedModuleNode);

  // Description: The name of the Module - this is used to 
  // construct the proc invocations
  vtkGetStringMacro (ModuleName);
  vtkSetStringMacro (ModuleName);
  
protected:
  
private:
  vtkScriptedModuleLogic();
  ~vtkScriptedModuleLogic();
  vtkScriptedModuleLogic(const vtkScriptedModuleLogic&);
  void operator=(const vtkScriptedModuleLogic&);

  vtkMRMLScriptedModuleNode* ScriptedModuleNode;
  char *ModuleName;
};

#endif

