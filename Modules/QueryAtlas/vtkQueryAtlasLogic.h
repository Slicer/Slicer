/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkQueryAtlasLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkQueryAtlasLogic - slicer logic class for query atlas
// .SECTION Description
// This class manages the logic associated with the Query Atlas

#ifndef __vtkQueryAtlasLogic_h
#define __vtkQueryAtlasLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkQueryAtlasLogic.h"
#include "vtkMRMLQueryAtlasNode.h"


class VTK_QUERYATLAS_EXPORT vtkQueryAtlasLogic : public vtkSlicerModuleLogic 
{
  public:
  
  // The Usual vtk class functions
  static vtkQueryAtlasLogic *New();
  vtkTypeRevisionMacro(vtkQueryAtlasLogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  vtkGetObjectMacro (QueryAtlasNode, vtkMRMLQueryAtlasNode);
  vtkSetObjectMacro (QueryAtlasNode, vtkMRMLQueryAtlasNode);
  
  // The method that creates and runs VTK or ITK pipeline
  void Apply() { };

protected:
  vtkQueryAtlasLogic();
  ~vtkQueryAtlasLogic();
  vtkQueryAtlasLogic(const vtkQueryAtlasLogic&);
  void operator=(const vtkQueryAtlasLogic&);
  vtkMRMLQueryAtlasNode* QueryAtlasNode;
};

#endif

