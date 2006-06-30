/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRAblationLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRAblationLogic_h
#define __vtkMRAblationLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerVolumesGUI.h"

#include "vtkMRAblation.h"
#include "vtkMRMLMRAblationNode.h"


class vtkITKGradientAnisotropicDiffusionImageFilter;

class VTK_MRABLATION_EXPORT vtkMRAblationLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkMRAblationLogic *New();
  vtkTypeMacro(vtkMRAblationLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (MRAblationNode, vtkMRMLMRAblationNode);
  vtkSetObjectMacro (MRAblationNode, vtkMRMLMRAblationNode);
  
  // The method that creates and runs VTK or ITK pipeline
  void Apply(vtkSlicerVolumesGUI *volsGUI);
  
protected:
  vtkMRAblationLogic();
  ~vtkMRAblationLogic();
  vtkMRAblationLogic(const vtkMRAblationLogic&);
  void operator=(const vtkMRAblationLogic&);

  vtkMRMLMRAblationNode* MRAblationNode;
  vtkITKGradientAnisotropicDiffusionImageFilter* GradientAnisotropicDiffusionImageFilter;


};

#endif

