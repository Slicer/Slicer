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

#include "vtkCommandLineModuleLogic.h"
#include "vtkCommandLineModule.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"

vtkCommandLineModuleLogic* vtkCommandLineModuleLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCommandLineModuleLogic");
  if(ret)
    {
      return (vtkCommandLineModuleLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCommandLineModuleLogic;
}


//----------------------------------------------------------------------------
vtkCommandLineModuleLogic::vtkCommandLineModuleLogic()
{
  this->CommandLineModuleNode = NULL;
}

//----------------------------------------------------------------------------
vtkCommandLineModuleLogic::~vtkCommandLineModuleLogic()
{
  this->SetCommandLineModuleNode(NULL);
}

//----------------------------------------------------------------------------
void vtkCommandLineModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

void vtkCommandLineModuleLogic::Apply()
{
  // check if MRML node is present 
  if (this->CommandLineModuleNode == NULL)
    {
    vtkErrorMacro("No input CommandLineModuleNode found");
    return;
    }
  
//   // find input volume
//   vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->CommandLineModuleNode->GetInputVolumeRef()));
//   if (inVolume == NULL)
//     {
//     vtkErrorMacro("No input volume found");
//     return;
//     }

  
//   // find output volume
//   vtkMRMLScalarVolumeNode *outVolume =  vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->CommandLineModuleNode->GetOutputVolumeRef()));
//   if (outVolume == NULL)
//     {
//     vtkErrorMacro("No output volume found with id= " << this->CommandLineModuleNode->GetOutputVolumeRef());
//     return;
//     }

//   // copy RASToIJK matrix, and other attributes from input to output
//   std::string name (outVolume->GetName());
//   std::string id (outVolume->GetID());

//   outVolume->Copy(inVolume);

//   outVolume->SetName(name.c_str());
//   outVolume->SetID(id.c_str());


  
  // build the command line
  //
  //

  
  // run the filter
  //
  //

  
  // import the results
  //
  //


  
  
  // set ouput of the filter to VolumeNode's ImageData
  // set ouput of the filter to VolumeNode's ImageData
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
//   vtkImageData* image = vtkImageData::New(); 
//   image->DeepCopy( this->GradientAnisotropicDiffusionImageFilter->GetOutput() );
//   outVolume->SetImageData(image);
//   image->Delete();
  // or
  //
  // outVolume->SetImageData(this->GradientAnisotropicDiffusionImageFilter->GetOutput());

}
