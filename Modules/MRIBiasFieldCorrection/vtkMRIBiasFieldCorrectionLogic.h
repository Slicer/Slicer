/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRIBiasFieldCorrectionLogic.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#ifndef __vtkMRIBiasFieldCorrectionLogic_h
#define __vtkMRIBiasFieldCorrectionLogic_h

#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkMRMLScene.h"

#include "vtkMRIBiasFieldCorrection.h"
#include "vtkMRMLMRIBiasFieldCorrectionNode.h"


class vtkITKGradientAnisotropicDiffusionImageFilter;

//class vtkImageMeanIntensityNormalization;

class vtkExtractVOI;

class vtkImageThreshold;

class vtkImageClip;

class vtkImageData;

class vtkImageReslice;

class VTK_MRIBiasFieldCorrection_EXPORT vtkMRIBiasFieldCorrectionLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkMRIBiasFieldCorrectionLogic *New();
  vtkTypeMacro(vtkMRIBiasFieldCorrectionLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // TODO: do we need to observe MRML here?
  virtual void ProcessMrmlEvents(vtkObject *vtkNotUsed(caller),
                                 unsigned long vtkNotUsed(event),
                                 void *vtkNotUsed(callData)){};

  // Description: Get/Set MRML node storing parameter values
  vtkGetObjectMacro (MRIBiasFieldCorrectionNode, vtkMRMLMRIBiasFieldCorrectionNode);
  void SetAndObserveMRIBiasFieldCorrectionNode(vtkMRMLMRIBiasFieldCorrectionNode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->MRIBiasFieldCorrectionNode, n);
    }

  // The method that creates and runs VTK or ITK pipeline
  void ApplyPreview(double red,double yellow,double green,vtkImageData* image);
  
  void SliceProcess(vtkTransform* xyToijk,double dim0,double dim1);
  
  void Apply();
  
  void Preview();
  
  void Cut();
  
  int InitMaxThreshold();
  int InitMinThreshold();
  int AxialMax();
  int AxialMin();
  int SagittalMax();
  int CoronalMax();
  
  double originOutvolume[3];
    
protected:
  vtkMRIBiasFieldCorrectionLogic();
  virtual ~vtkMRIBiasFieldCorrectionLogic();
  vtkMRIBiasFieldCorrectionLogic(const vtkMRIBiasFieldCorrectionLogic&);
  void operator=(const vtkMRIBiasFieldCorrectionLogic&);

  vtkMRMLMRIBiasFieldCorrectionNode* MRIBiasFieldCorrectionNode;
  vtkITKGradientAnisotropicDiffusionImageFilter* GradientAnisotropicDiffusionImageFilter;
 // vtkImageMeanIntensityNormalization* ImageMeanIntensityNormalization;
  
  vtkExtractVOI* ExtractVOI;
  vtkExtractVOI* ExtractVOI2;
  vtkImageThreshold* ImageThreshold;
  vtkImageClip* ImageClip;
  vtkImageData* CurrentSlide;
  
  vtkImageData* SAGITTAL;
  vtkImageData* TRANSVERSAL;
  vtkImageData* CORONAL;
  
  vtkImageData* STORAGE;
  vtkImageData* PREVIEW;
  vtkImageData* MASK;
  
  //double oldPosition[3];
    
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

