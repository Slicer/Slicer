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
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerModuleFactoryManager.h>
#include <qSlicerAbstractCoreModule.h>
#include <qSlicerCLIModule.h>
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
  vtkMRMLVolumeNode *inputVolume = pnode->GetInputVolumeNode();
  vtkMRMLAnnotationROINode *inputROI = pnode->GetROINode();
  vtkMRMLVolumeNode *outputVolume = pnode->GetOutputVolumeNode();
  vtkMatrix4x4 *inputRASToIJK = vtkMatrix4x4::New();
  vtkMatrix4x4 *inputIJKToRAS = vtkMatrix4x4::New();
  vtkMatrix4x4 *outputRASToIJK = vtkMatrix4x4::New();
  vtkMatrix4x4 *outputIJKToRAS = vtkMatrix4x4::New();
  //vtkMatrix4x4 *volumeXform = vtkMatrix4x4::New();
  //vtkMatrix4x4 *roiXform = vtkMatrix4x4::New();
  //vtkMatrix4x4 *T = vtkMatrix4x4::New();

  inputVolume->GetRASToIJKMatrix(inputRASToIJK);
  inputVolume->GetIJKToRASMatrix(inputIJKToRAS);
  outputRASToIJK->Identity();
  outputIJKToRAS->Identity();

  //T->Identity();
  //roiXform->Identity();
  //volumeXform->Identity();

  double roiRadius[3], roiXYZ[3];
  inputROI->GetRadiusXYZ(roiRadius);
  inputROI->GetXYZ(roiXYZ);

  double* inputSpacing = inputVolume->GetSpacing();
  double minSpacing = fmin(inputSpacing[0],fmin(inputSpacing[1],inputSpacing[2]));

  int outputExtent[3];

  outputExtent[0] = roiRadius[0]/minSpacing*2.;
  outputExtent[1] = roiRadius[1]/minSpacing*2.;
  outputExtent[2] = roiRadius[2]/minSpacing*2.;

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

  outputVolume->SetAndObserveImageData(outputImageData);
  outputImageData->Delete();

  outputVolume->SetIJKToRASMatrix(outputIJKToRAS);
  outputVolume->SetRASToIJKMatrix(outputRASToIJK);

  outputVolume->ModifiedSinceReadOn();

  vtkMRMLDisplayNode* inputDisplay = inputVolume->GetDisplayNode();
  if(inputDisplay){
    vtkMRMLDisplayNode* outputDisplay = inputDisplay->NewInstance();
    outputDisplay->Copy(inputDisplay);
    this->GetMRMLScene()->AddNodeNoNotify(outputDisplay);
    outputVolume->SetAndObserveDisplayNodeID(outputDisplay->GetID());
  }

  inputRASToIJK->Delete();
  inputIJKToRAS->Delete();
  outputRASToIJK->Delete();
  outputIJKToRAS->Delete();

  // use the prepared volume as the reference for resampling
  qSlicerModuleManager * moduleManager =
          qSlicerCoreApplication::application()->moduleManager();
  qSlicerModuleFactoryManager* moduleFactoryManager = moduleManager->factoryManager();
  QStringList moduleNames = moduleFactoryManager->moduleNames();

  if(!moduleNames.contains("resamplevolume2")){
      std::cerr << "resamplevolume2 module name was not found in the list of registered modules!" << std::endl;
      return -1;
  }

  qSlicerAbstractCoreModule * module = moduleManager->module("resamplevolume2");
  if(!module){
      std::cerr << "resamplevolume2 module reference was not found!" << std::endl;
      return -1;
  }

  qSlicerCLIModule * cliModule = qobject_cast<qSlicerCLIModule*>(module);
  vtkMRMLCommandLineModuleNode* cmdNode = cliModule->createNode();

  cmdNode->SetParameterAsString("inputVolume", inputVolume->GetID());
  cmdNode->SetParameterAsString("referenceVolume",outputVolume->GetID());
  cmdNode->SetParameterAsString("outputVolume",outputVolume->GetID());
  cliModule->run(cmdNode, true);

  return 0;
}

void vtkSlicerCropVolumeLogic::RegisterNodes()
{
  if(!this->GetMRMLScene())
    return;
  vtkMRMLCropVolumeParametersNode* pNode = vtkMRMLCropVolumeParametersNode::New();
  this->GetMRMLScene()->RegisterNodeClass(pNode);
  pNode->Delete();
}
