/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkIntensityNormalizationFilterLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkIntensityNormalizationFilterLogic.h"
#include "vtkIntensityNormalizationFilter.h"
#include "vtkImageMeanIntensityNormalization.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

vtkIntensityNormalizationFilterLogic* vtkIntensityNormalizationFilterLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkIntensityNormalizationFilterLogic");
  if(ret)
    {
      return (vtkIntensityNormalizationFilterLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIntensityNormalizationFilterLogic;
}


//----------------------------------------------------------------------------
vtkIntensityNormalizationFilterLogic::vtkIntensityNormalizationFilterLogic()
{
  this->IntensityNormalizationFilterNode = NULL;
}

//----------------------------------------------------------------------------
vtkIntensityNormalizationFilterLogic::~vtkIntensityNormalizationFilterLogic()
{
  vtkSetMRMLNodeMacro(this->IntensityNormalizationFilterNode, NULL);
}

//----------------------------------------------------------------------------
void vtkIntensityNormalizationFilterLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

void vtkIntensityNormalizationFilterLogic::Apply()
{

  // check if MRML node is present 
  if (this->IntensityNormalizationFilterNode == NULL)
    {
    vtkErrorMacro("No input IntensityNormalizationFilterNode found");
    return;
    }
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->IntensityNormalizationFilterNode->GetInputVolumeRef()));
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }
  
  // find output volume
  vtkMRMLScalarVolumeNode *outVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->IntensityNormalizationFilterNode->GetOutputVolumeRef()));
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= " << this->IntensityNormalizationFilterNode->GetOutputVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());

  
 /* 
    // setup vtk filter
   this->ImageMeanIntensityNormalization =
      vtkImageMeanIntensityNormalization::New();*/
   /* this->ImageMeanIntensityNormalization->SetNormType
      (INTENSITY_NORM_MEAN_MRI);
    this->ImageMeanIntensityNormalization->SetInitialHistogramSmoothingWidth
      (this->IntensityNormalizationFilterNode->GetNumberOfIterations());
    this->ImageMeanIntensityNormalization->SetMaxHistogramSmoothingWidth
      (this->IntensityNormalizationFilterNode->GetTimeStep());
    this->ImageMeanIntensityNormalization->SetRelativeMaxVoxelNum
      (0.99);
    this->ImageMeanIntensityNormalization->SetPrintInfo
      (0);
    this->ImageMeanIntensityNormalization->SetInput(inVolume->GetImageData());
*/
    this->ImageMeanIntensityNormalization =
      vtkImageMeanIntensityNormalization::New();
    this->ImageMeanIntensityNormalization->SetNormType
      (INTENSITY_NORM_MEAN_MRI);
   //this->ImageMeanIntensityNormalization->SetNormValue
   //   (80);
    this->ImageMeanIntensityNormalization->SetNormValue
      (this->IntensityNormalizationFilterNode->GetConductance());   
    this->ImageMeanIntensityNormalization->SetNormType
      (1);
    this->ImageMeanIntensityNormalization->SetInitialHistogramSmoothingWidth
      (this->IntensityNormalizationFilterNode->GetNumberOfIterations());
    this->ImageMeanIntensityNormalization->SetMaxHistogramSmoothingWidth
      (this->IntensityNormalizationFilterNode->GetTimeStep());
    this->ImageMeanIntensityNormalization->SetRelativeMaxVoxelNum
      (0.99);
    this->ImageMeanIntensityNormalization->SetPrintInfo
      (1);
    this->ImageMeanIntensityNormalization->SetInput(inVolume->GetImageData());

    // execute filter

    this->ImageMeanIntensityNormalization->Update();
  

  // set ouput of the filter to VolumeNode's ImageData
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  vtkImageData* image = vtkImageData::New(); 
  image->DeepCopy( this->ImageMeanIntensityNormalization->GetOutput() );
  outVolume->SetAndObserveImageData(image);
  image->Delete();
  outVolume->SetModifiedSinceRead(1);

  //outVolume->SetImageData(this->GradientAnisotropicDiffusionImageFilter->GetOutput());

  // delete the filter
  this->ImageMeanIntensityNormalization->Delete();
}
