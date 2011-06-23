/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkIA_FEMeshLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkIA_FEMeshLogic_h
#define __vtkIA_FEMeshLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"
#include "vtkIA_FEMesh.h"
#include "vtkMRMLIA_FEMeshNode.h"


class VTK_IA_FEMesh_EXPORT vtkIA_FEMeshLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkIA_FEMeshLogic *New();
  vtkTypeMacro(vtkIA_FEMeshLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents(vtkObject *vtkNotUsed(caller),
                                 unsigned long vtkNotUsed(event),
                                 void *vtkNotUsed(callData)){};

  // Description: Get/Set MRML node
  vtkGetObjectMacro (IA_FEMeshNode, vtkMRMLIA_FEMeshNode);
  vtkSetObjectMacro (IA_FEMeshNode, vtkMRMLIA_FEMeshNode);
  
  void Apply();
  
  // called during instantiation; so declare custom MRML nodes here
  virtual void SetMRMLScene(vtkMRMLScene *mrml);

  // here is where the declaration is performed
  void RegisterNodes();

protected:
  vtkIA_FEMeshLogic();
  ~vtkIA_FEMeshLogic();
  vtkIA_FEMeshLogic(const vtkIA_FEMeshLogic&);
  void operator=(const vtkIA_FEMeshLogic&);

  vtkMRMLIA_FEMeshNode* IA_FEMeshNode;
  bool First;   // set during first time through mrml loading


};

#endif

