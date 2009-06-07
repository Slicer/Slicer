/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkThresholdingFilterLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkThresholdingFilterLogic_h
#define __vtkThresholdingFilterLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkMRMLScene.h"

#include "vtkThresholdingFilter.h"
#include "vtkMRMLThresholdingFilterNode.h"


class vtkITKGradientAnisotropicDiffusionImageFilter;

class vtkExtractVOI;

class vtkImageThreshold;

class vtkImageClip;

class vtkImageData;

class vtkImageReslice;

class VTK_ThresholdingFilter_EXPORT vtkThresholdingFilterLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkThresholdingFilterLogic *New();
  vtkTypeMacro(vtkThresholdingFilterLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                   void *callData ){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (ThresholdingFilterNode, vtkMRMLThresholdingFilterNode);
  void SetAndObserveThresholdingFilterNode(vtkMRMLThresholdingFilterNode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->ThresholdingFilterNode, n);
    }

  // The method that creates and runs VTK or ITK pipeline
  void ApplyPreview();
  void Apply();
  
  void Preview();
  
  void Cut();
  
  void ChangeSlide();
  
  void ChangeSlide2();
  
  int InitMaxThreshold();
  int InitMinThreshold();
  int AxialMax();
  int AxialMin();
  int SagittalMax();
  int CoronalMax();
  
  double originOutvolume[3];
    
protected:
  vtkThresholdingFilterLogic();
  virtual ~vtkThresholdingFilterLogic();
  vtkThresholdingFilterLogic(const vtkThresholdingFilterLogic&);
  void operator=(const vtkThresholdingFilterLogic&);

  vtkMRMLThresholdingFilterNode* ThresholdingFilterNode;
  vtkITKGradientAnisotropicDiffusionImageFilter* GradientAnisotropicDiffusionImageFilter;
  
  vtkExtractVOI* ExtractVOI;
  vtkExtractVOI* ExtractVOI2;
  vtkImageThreshold* ImageThreshold;
  vtkImageClip* ImageClip;
  vtkImageData* CurrentSlide;
  
  vtkImageData* Image1;
  vtkImageData* Image2;
  vtkImageData* Image3;
  vtkImageData* Image4;
  vtkImageData* Image5;
  
  vtkImageReslice* ImageReslice1;
  vtkImageReslice* ImageReslice2;
  vtkImageReslice* ImageReslice3;
  vtkImageReslice* ImageReslice4;
  vtkImageReslice* ImageReslice5;
  

};

#endif

