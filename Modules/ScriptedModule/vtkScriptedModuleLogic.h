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
  void SetAndObserveScriptedModuleNode (vtkMRMLScriptedModuleNode *n)
    {
    vtkSetAndObserveMRMLNodeMacro( this->ScriptedModuleNode, n);
    }

  // Description:
  // Get the path to the module's resources directory.
  // Override the parent since the ModuleLocation is usually inside
  // an extra Tcl/ or Python/ subdir (strip it).
  virtual const char* GetModuleShareDirectory();

  // Description:
  // Get the path to the module's library directory.
  // Override the parent since the ModuleLocation is usually inside
  // an extra Tcl/ or Python/ subdir (strip it).
  virtual const char* GetModuleLibDirectory();

protected:
  
private:
  vtkScriptedModuleLogic();
  ~vtkScriptedModuleLogic();
  vtkScriptedModuleLogic(const vtkScriptedModuleLogic&);
  void operator=(const vtkScriptedModuleLogic&);

  vtkMRMLScriptedModuleNode* ScriptedModuleNode;
};

#endif

