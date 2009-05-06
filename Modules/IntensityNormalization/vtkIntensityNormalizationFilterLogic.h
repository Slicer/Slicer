/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkIntensityNormalizationFilterLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkIntensityNormalizationFilterLogic_h
#define __vtkIntensityNormalizationFilterLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkIntensityNormalizationFilter.h"
#include "vtkMRMLIntensityNormalizationFilterNode.h"


class vtkITKGradientAnisotropicDiffusionImageFilter;

class vtkImageMeanIntensityNormalization;

class VTK_IntensityNormalizationFilter_EXPORT vtkIntensityNormalizationFilterLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkIntensityNormalizationFilterLogic *New();
  vtkTypeMacro(vtkIntensityNormalizationFilterLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (IntensityNormalizationFilterNode, vtkMRMLIntensityNormalizationFilterNode);
  void SetAndObserveIntensityNormalizationFilterNode(vtkMRMLIntensityNormalizationFilterNode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->IntensityNormalizationFilterNode, n);
    }

  // The method that creates and runs VTK or ITK pipeline
  void Apply();
  
protected:
  vtkIntensityNormalizationFilterLogic();
  virtual ~vtkIntensityNormalizationFilterLogic();
  vtkIntensityNormalizationFilterLogic(const vtkIntensityNormalizationFilterLogic&);
  void operator=(const vtkIntensityNormalizationFilterLogic&);

  vtkMRMLIntensityNormalizationFilterNode* IntensityNormalizationFilterNode;
  vtkITKGradientAnisotropicDiffusionImageFilter* GradientAnisotropicDiffusionImageFilter;
  vtkImageMeanIntensityNormalization* ImageMeanIntensityNormalization;

};

#endif

