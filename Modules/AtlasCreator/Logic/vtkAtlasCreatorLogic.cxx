/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkAtlasCreatorLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// Slicer includes
#include "vtkAtlasCreatorLogic.h"

// VTKITK includes
#include "vtkITKGradientAnisotropicDiffusionImageFilter.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

// VTK includes
#include "vtkObjectFactory.h"

// STD includes
#include <string>
#include <iostream>
#include <sstream>

vtkAtlasCreatorLogic* vtkAtlasCreatorLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkAtlasCreatorLogic");
  if(ret)
    {
      return (vtkAtlasCreatorLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkAtlasCreatorLogic;
}


//----------------------------------------------------------------------------
vtkAtlasCreatorLogic::vtkAtlasCreatorLogic()
{
  this->AtlasCreatorNode = NULL;
}

//----------------------------------------------------------------------------
vtkAtlasCreatorLogic::~vtkAtlasCreatorLogic()
{
  vtkSetMRMLNodeMacro(this->AtlasCreatorNode, NULL);
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

void vtkAtlasCreatorLogic::Apply()
{

  // check if MRML node is present 
  if (this->AtlasCreatorNode == NULL)
    {
    vtkErrorMacro("No input AtlasCreatorNode found");
    return;
    }
  
  // find input volume
    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->AtlasCreatorNode->GetInputVolumeRef()));
  if (inVolume == NULL)
    {
    vtkErrorMacro("No input volume found");
    return;
    }
  
  // find output volume
  vtkMRMLScalarVolumeNode *outVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->AtlasCreatorNode->GetOutputVolumeRef()));
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found with id= " << this->AtlasCreatorNode->GetOutputVolumeRef());
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

  this->GradientAnisotropicDiffusionImageFilter->SetConductanceParameter(this->AtlasCreatorNode->GetConductance());
  this->GradientAnisotropicDiffusionImageFilter->SetNumberOfIterations(this->AtlasCreatorNode->GetNumberOfIterations());
  this->GradientAnisotropicDiffusionImageFilter->SetTimeStep(this->AtlasCreatorNode->GetTimeStep()); 

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
