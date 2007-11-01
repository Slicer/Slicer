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
#include "vtkSlicerApplication.h"

#include <string>

typedef enum { CommandLineModule, SharedObjectModule, PythonModule } CommandLineModuleType;

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

  // Description: For debugging, control deletion of temp files
  vtkBooleanMacro (DeleteTemporaryFiles, int);
  vtkSetMacro (DeleteTemporaryFiles, int);
  vtkGetMacro (DeleteTemporaryFiles, int);
  
  // The method that schedules the command line module to run
  void Apply();

  // Set/Get the directory to use for temporary files
  void SetTemporaryDirectory(const char *tempdir)
    { this->TemporaryDirectory = tempdir; }
  const char *GetTemporaryDirectory() const
    { return this->TemporaryDirectory.c_str(); }

protected:
  //BTX
  std::string ConstructTemporaryFileName(const std::string& tag,
                                         const std::string& type,
                                         const std::string& name,
                                     const std::vector<std::string>& extensions,
                                     CommandLineModuleType commandType) const;
  std::string ConstructTemporarySceneFileName(vtkMRMLScene *scene);
  //ETX

  // The method that runs the command line module
  void ApplyTask(void *clientdata);

  // Communicate progress back to the node
  static void ProgressCallback(void *);
  
private:
  vtkCommandLineModuleLogic();
  virtual ~vtkCommandLineModuleLogic();
  vtkCommandLineModuleLogic(const vtkCommandLineModuleLogic&);
  void operator=(const vtkCommandLineModuleLogic&);

  int DeleteTemporaryFiles;

  vtkMRMLCommandLineModuleNode* CommandLineModuleNode;
//BTX
  std::string TemporaryDirectory;
//ETX
};

#endif

