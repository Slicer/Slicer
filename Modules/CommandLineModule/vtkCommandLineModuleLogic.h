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

#include <string>

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
  
  // The method that creates and runs the command line module
  void Apply();

  // Set/Get the directory to use for temporary files
  void SetTemporaryDirectory(const char *tempdir)
    { this->TemporaryDirectory = tempdir; }
  const char *GetTemporaryDirectory() const
    { return this->TemporaryDirectory.c_str(); }

protected:
  //BTX
  std::string ConstructTemporaryFileName(const std::string& name, bool isCommandLineModule) const;
  //ETX
  
private:
  vtkCommandLineModuleLogic();
  virtual ~vtkCommandLineModuleLogic();
  vtkCommandLineModuleLogic(const vtkCommandLineModuleLogic&);
  void operator=(const vtkCommandLineModuleLogic&);

  vtkMRMLCommandLineModuleNode* CommandLineModuleNode;
//BTX
  std::string TemporaryDirectory;
//ETX
};

#endif

