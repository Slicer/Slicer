/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkModule.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.18 $

=========================================================================auto=*/
// .NAME vtkModule - Abstract Superclass for all specific types of MRML nodes.
// .SECTION Description
// This node encapsulates the functionality common to all types of MRML nodes.
// This includes member variables for ID, Description, and Options,
// as well as member functions to Copy() and Write().

#ifndef __vtkModule_h
#define __vtkModule_h

#include "vtkObject.h"
#include "vtkCallbackCommand.h"

#include "vtkKWFrame.h"
#include "vtkMRMLScene.h"

#include "vtkModulesBaseClasses.h"


class VTK_MODULES_BASE_CLASSES_EXPORT vtkModule : public vtkObject
{
public:
  static vtkModule *New();
  vtkTypeMacro(vtkModule,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  
  // Description:
  // Text description of this module, to be set by the user
  vtkSetStringMacro(Description);
  vtkGetStringMacro(Description);

  // Description:
  // Root directory of MRML scene
  vtkSetStringMacro(SceneRootDir);
  vtkGetStringMacro(SceneRootDir);
  
  // Description:
  // Name of this module, to be set by the user
  vtkSetStringMacro(Name);
  vtkGetStringMacro(Name);
  
  // Description:
  // MRML Scene
  vtkGetObjectMacro(Scene, vtkMRMLScene);

  virtual void SetScene(vtkMRMLScene* scene);

  virtual void BuildGUI(vtkKWFrame* frame){};

  virtual void Enter(){};

  virtual void Exit(){};

  virtual void ProcessMRMLSceneModified(vtkMRMLScene *scene){};

protected:
  
  vtkModule();
  // critical to have a virtual destructor!
  virtual ~vtkModule();
  vtkModule(const vtkModule&);
  void operator=(const vtkModule&);
  
  vtkSetMacro(Indent, int);
  
  char *Description;
  char *Name;
  char *SceneRootDir;
  vtkMRMLScene *Scene;
  vtkCallbackCommand *MRMLCallbackCommand;
};

#endif



