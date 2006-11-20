/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkGradientAnisotropicDiffusionFilterLogic_h
#define __vtkGradientAnisotropicDiffusionFilterLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkGradientAnisotropicDiffusionFilter.h"
#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"


class vtkITKGradientAnisotropicDiffusionImageFilter;

class VTK_GRADIENTANISOTROPICDIFFUSIONFILTER_EXPORT vtkGradientAnisotropicDiffusionFilterLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkGradientAnisotropicDiffusionFilterLogic *New();
  vtkTypeMacro(vtkGradientAnisotropicDiffusionFilterLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (GradientAnisotropicDiffusionFilterNode, vtkMRMLGradientAnisotropicDiffusionFilterNode);
  void SetAndObserveGradientAnisotropicDiffusionFilterNode(vtkMRMLGradientAnisotropicDiffusionFilterNode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->GradientAnisotropicDiffusionFilterNode, n);
    }

  // The method that creates and runs VTK or ITK pipeline
  void Apply();
  
protected:
  vtkGradientAnisotropicDiffusionFilterLogic();
  virtual ~vtkGradientAnisotropicDiffusionFilterLogic();
  vtkGradientAnisotropicDiffusionFilterLogic(const vtkGradientAnisotropicDiffusionFilterLogic&);
  void operator=(const vtkGradientAnisotropicDiffusionFilterLogic&);

  vtkMRMLGradientAnisotropicDiffusionFilterNode* GradientAnisotropicDiffusionFilterNode;
  vtkITKGradientAnisotropicDiffusionImageFilter* GradientAnisotropicDiffusionImageFilter;

};

#endif

