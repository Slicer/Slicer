/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkGradientAnisotropicDiffusionFilterLogic.h"
#include "vtkITKGradientAnisotropicDiffusionImageFilter.h"
#include "vtkGradientAnisotropicDiffusionFilter.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

vtkGradientAnisotropicDiffusionFilterLogic* vtkGradientAnisotropicDiffusionFilterLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkGradientAnisotropicDiffusionFilterLogic");
  if(ret)
    {
      return (vtkGradientAnisotropicDiffusionFilterLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkGradientAnisotropicDiffusionFilterLogic;
}


//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterLogic::vtkGradientAnisotropicDiffusionFilterLogic()
{
  this->GradientAnisotropicDiffusionFilterNode = NULL;
}

//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterLogic::~vtkGradientAnisotropicDiffusionFilterLogic()
{
  vtkSetMRMLNodeMacro(this->GradientAnisotropicDiffusionFilterNode, NULL);
}

//----------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

void vtkGradientAnisotropicDiffusionFilterLogic::Apply()
{

  // check if MRML node is present 
  if (this->GradientAnisotropicDiffusionFilterNode == NULL)
    {
    vtkErrorMacro("No input GradientAnisotropicDiffusionFilterNode found");
    return;
    }
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GradientAnisotropicDiffusionFilterNode->GetInputVolumeRef()));
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }
  
  // find output volume
  vtkMRMLScalarVolumeNode *outVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->GradientAnisotropicDiffusionFilterNode->GetOutputVolumeRef()));
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= " << this->GradientAnisotropicDiffusionFilterNode->GetOutputVolumeRef());
    return;
    }

  // copy RASToIJK matrix, and other attributes from input to output
  std::string name (outVolume->GetName());
  std::string id (outVolume->GetID());

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  outVolume->SetName(name.c_str());
  //outVolume->SetID(id.c_str());

  // create filter
  //vtkITKGradientAnisotropicDiffusionImageFilter* filter = vtkITKGradientAnisotropicDiffusionImageFilter::New();
  this->GradientAnisotropicDiffusionImageFilter = vtkITKGradientAnisotropicDiffusionImageFilter::New();

  // set filter input and parameters
  this->GradientAnisotropicDiffusionImageFilter->SetInput(inVolume->GetImageData());

  this->GradientAnisotropicDiffusionImageFilter->SetConductanceParameter(this->GradientAnisotropicDiffusionFilterNode->GetConductance());
  this->GradientAnisotropicDiffusionImageFilter->SetNumberOfIterations(this->GradientAnisotropicDiffusionFilterNode->GetNumberOfIterations());
  this->GradientAnisotropicDiffusionImageFilter->SetTimeStep(this->GradientAnisotropicDiffusionFilterNode->GetTimeStep()); 

  // run the filter
  this->GradientAnisotropicDiffusionImageFilter->Update();

  // set ouput of the filter to VolumeNode's ImageData
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  vtkImageData* image = vtkImageData::New(); 
  image->DeepCopy( this->GradientAnisotropicDiffusionImageFilter->GetOutput() );
  outVolume->SetAndObserveImageData(image);
  image->Delete();
  outVolume->SetModifiedSinceRead(1);

  //outVolume->SetImageData(this->GradientAnisotropicDiffusionImageFilter->GetOutput());

  // delete the filter
  this->GradientAnisotropicDiffusionImageFilter->Delete();
}
