/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerCropVolumeLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// 
#include "vtkSlicerCropVolumeLogic.h"
#include "vtkSlicerColorLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkImageChangeInformation.h>

#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLVolumeNode.h>

// CLI invocation
#include <vtkMRMLCommandLineModuleNode.h>
#include <vtkSlicerCLIModuleLogic.h>

#include <math.h>

#include <vtkMRMLCropVolumeParametersNode.h>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerCropVolumeLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerCropVolumeLogic);

//----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic::vtkSlicerCropVolumeLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic::~vtkSlicerCropVolumeLogic()
{

}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::ProcessMRMLEvents(vtkObject *vtkNotUsed(caller),
                                              unsigned long vtkNotUsed(event),
                                              void *vtkNotUsed(callData))
{
}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::ProcessLogicEvents(vtkObject *vtkNotUsed(caller), 
                                            unsigned long vtkNotUsed(event),
                                            void *vtkNotUsed(callData))
{

}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSlicerCropVolumeLogic:             " << this->GetClassName() << "\n";
}

//----------------------------------------------------------------------------
int vtkSlicerCropVolumeLogic::Apply(vtkMRMLCropVolumeParametersNode* pnode)
{
  //vtkIndent i;
  //pnode->WriteXML(std::cerr, 1);
  //     qSlicerCoreApplication::application()->coreCommandOptions()->tempDirectory());
  // create an empty volume that covers the ROI defined in the pnode
  vtkMRMLVolumeNode *inputVolume = pnode->GetInputVolumeNode();
  vtkMRMLAnnotationROINode *inputROI = pnode->GetROINode();
  vtkMRMLVolumeNode *outputVolume = pnode->GetOutputVolumeNode();
  vtkMatrix4x4 *inputRASToIJK = vtkMatrix4x4::New();
  vtkMatrix4x4 *inputIJKToRAS = vtkMatrix4x4::New();
  vtkMatrix4x4 *outputRASToIJK = vtkMatrix4x4::New();
  vtkMatrix4x4 *outputIJKToRAS = vtkMatrix4x4::New();
  vtkMatrix4x4 *volumeXform = vtkMatrix4x4::New();
  vtkMatrix4x4 *roiXform = vtkMatrix4x4::New();
  vtkMatrix4x4 *T = vtkMatrix4x4::New();

  inputVolume->GetRASToIJKMatrix(inputRASToIJK);
  inputVolume->GetIJKToRASMatrix(inputIJKToRAS);
  outputRASToIJK->Identity();
  outputIJKToRAS->Identity();

  T->Identity();
  roiXform->Identity();
  volumeXform->Identity();

  double roiRadius[3], roiXYZ[3];
  // BUG: it looks like at this time ROI annotation node returns the pointer to the same array
  // for getcenter and getradius, so I need to store the values
  roiRadius[0] = inputROI->GetRadiusXYZ()[0];
  roiRadius[1] = inputROI->GetRadiusXYZ()[1];
  roiRadius[2] = inputROI->GetRadiusXYZ()[2];
  roiXYZ[0] = inputROI->GetXYZ()[0];
  roiXYZ[1] = inputROI->GetXYZ()[1];
  roiXYZ[2] = inputROI->GetXYZ()[2];

  double* inputSpacing = inputVolume->GetSpacing();
  double minSpacing = fmin(inputSpacing[0],fmin(inputSpacing[1],inputSpacing[2]));

  int outputExtent[3];

  outputExtent[0] = roiRadius[0]*minSpacing*2.;
  outputExtent[1] = roiRadius[1]*minSpacing*2.;
  outputExtent[2] = roiRadius[2]*minSpacing*2.;

  std::cout << "Roi radius is: " << roiRadius[0] << ", " << roiRadius[1] << ", "
      << roiRadius[2] << std::endl;
  std::cout << "Roi center is: " << roiXYZ[0] << ", " << roiXYZ[1] << ", "
      << roiXYZ[2] << std::endl;
  std::cout << "Spacing: " << inputSpacing[0] << ", " << inputSpacing[1] << ", "
      << inputSpacing[2] << std::endl;

  outputIJKToRAS->SetElement(0,0,minSpacing);
  outputIJKToRAS->SetElement(1,1,minSpacing);
  outputIJKToRAS->SetElement(2,2,minSpacing);

  outputIJKToRAS->SetElement(0,3,roiXYZ[0]-roiRadius[0]+minSpacing*.5);
  outputIJKToRAS->SetElement(1,3,roiXYZ[1]-roiRadius[1]+minSpacing*.5);
  outputIJKToRAS->SetElement(2,3,roiXYZ[2]-roiRadius[2]+minSpacing*.5);

  outputRASToIJK->DeepCopy(outputIJKToRAS);
  outputRASToIJK->Invert();

  vtkImageData* outputImageData = vtkImageData::New();
  outputImageData->SetDimensions(outputExtent[0], outputExtent[1], outputExtent[2]);
  outputImageData->AllocateScalars();

  //std::cerr << "Calculated extent: " << outputExtent[0] << ", " << outputExtent[1] << ", " << outputExtent[2] << std::endl;

  outputVolume->SetAndObserveImageData(outputImageData);
  outputImageData->Delete();

  // FIXME: does not seem to work -- test this
  outputVolume->SetIJKToRASMatrix(outputIJKToRAS);
  outputVolume->SetRASToIJKMatrix(outputRASToIJK);
//  outputVolume->SetOrigin(roiXYZ[0]-roiRadius[0]+minSpacing*.5,
//      roiXYZ[1]-roiRadius[1]+minSpacing*.5,
//      roiXYZ[2]-roiRadius[2]+minSpacing*.5);
  outputVolume->ModifiedSinceReadOn();

  vtkMRMLDisplayNode* inputDisplay = inputVolume->GetDisplayNode();
  if(inputDisplay){
    vtkMRMLDisplayNode* outputDisplay = inputDisplay->NewInstance();
    outputDisplay->Copy(inputDisplay);
    this->GetMRMLScene()->AddNodeNoNotify(outputDisplay);
    outputVolume->SetAndObserveDisplayNodeID(outputDisplay->GetID());
  }

//  FIXME: some of these causes segfault ...
//  inputRASToIJK->Delete();
//  inputIJKToRAS->Delete();
//  outputRASToIJK->Delete();
//  outputIJKToRAS->Delete();
//  volumeXform->Delete();
//  roiXform->Delete();
//  T->Delete();

  return 0;
}

void vtkSlicerCropVolumeLogic::RegisterNodes()
{
  if(!this->GetMRMLScene())
    return;
  //std::cout << "Registering nodes" << std::endl;
  //qDebug() << "Registering crop volume mrml node";

  vtkMRMLCropVolumeParametersNode* pNode = vtkMRMLCropVolumeParametersNode::New();
  this->GetMRMLScene()->RegisterNodeClass(pNode);
  pNode->Delete();

}
