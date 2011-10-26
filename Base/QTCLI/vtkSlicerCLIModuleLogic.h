/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#ifndef __vtkSlicerCLIModuleLogic_h
#define __vtkSlicerCLIModuleLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRMLCLI includes
#include "vtkMRMLCommandLineModuleNode.h"

// MRML include
#include "vtkMRMLScene.h"

// STL includes
#include <string>

#include "qSlicerBaseQTCLIExport.h"

typedef enum { CommandLineModule, SharedObjectModule, PythonModule } CommandLineModuleType;

/// Historically was vtkCommandLineModuleLogic
class Q_SLICER_BASE_QTCLI_EXPORT vtkSlicerCLIModuleLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerCLIModuleLogic *New();
  vtkTypeMacro(vtkSlicerCLIModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents(vtkObject * vtkNotUsed(caller),
                                 unsigned long vtkNotUsed(event),
                                 void * vtkNotUsed(callData)){}

  // Description: For debugging, control deletion of temp files
  vtkBooleanMacro (DeleteTemporaryFiles, int);
  vtkSetMacro (DeleteTemporaryFiles, int);
  vtkGetMacro (DeleteTemporaryFiles, int);

  // Description: For debugging, control redirection of cout and cerr
  vtkBooleanMacro (RedirectModuleStreams, int);
  vtkSetMacro (RedirectModuleStreams, int);
  vtkGetMacro (RedirectModuleStreams, int);
  
  // The method that schedules the command line module to run
  void Apply( vtkMRMLCommandLineModuleNode* node );
  void ApplyAndWait ( vtkMRMLCommandLineModuleNode* node );

  // Set/Get the directory to use for temporary files
  void SetTemporaryDirectory(const char *tempdir)
    { this->TemporaryDirectory = tempdir; }

//   void LazyEvaluateModuleTarget(ModuleDescription& moduleDescriptionObject);
//   void LazyEvaluateModuleTarget(vtkMRMLCommandLineModuleNode* node) 
//     { this->LazyEvaluateModuleTarget(node->GetModuleDescription()); }

protected:
  std::string ConstructTemporaryFileName(const std::string& tag,
                                         const std::string& type,
                                         const std::string& name,
                                     const std::vector<std::string>& extensions,
                                     CommandLineModuleType commandType);
  std::string ConstructTemporarySceneFileName(vtkMRMLScene *scene);
  std::string FindHiddenNodeID(const ModuleDescription& d,
                               const ModuleParameter& p);

  // The method that runs the command line module
  void ApplyTask(void *clientdata);

  // Communicate progress back to the node
  static void ProgressCallback(void *);
  
private:
  vtkSlicerCLIModuleLogic();
  virtual ~vtkSlicerCLIModuleLogic();
  vtkSlicerCLIModuleLogic(const vtkSlicerCLIModuleLogic&);
  void operator=(const vtkSlicerCLIModuleLogic&);

  int DeleteTemporaryFiles;

  int RedirectModuleStreams;

  vtkMRMLCommandLineModuleNode* CommandLineModuleNode;
  std::string TemporaryDirectory;
};

#endif

