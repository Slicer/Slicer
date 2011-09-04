/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerCropVolumeLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// Qt includes

// CLI invocation
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerModuleFactoryManager.h>
#include <qSlicerCLIModule.h>
#include <vtkSlicerCLIModuleLogic.h>

// CropLogic includes
#include "vtkSlicerCropVolumeLogic.h"
#include "vtkSlicerVolumesLogic.h"

// CropMRML includes

// MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionWeightedVolumeNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkImageData.h>

// STD includes

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
  vtkMRMLScalarVolumeNode *refVolume;
  vtkMRMLAnnotationROINode *inputROI = pnode->GetROINode();
  vtkMRMLVolumeNode *outputVolume = NULL;
  vtkMatrix4x4 *inputRASToIJK = vtkMatrix4x4::New();
  vtkMatrix4x4 *inputIJKToRAS = vtkMatrix4x4::New();
  vtkMatrix4x4 *outputRASToIJK = vtkMatrix4x4::New();
  vtkMatrix4x4 *outputIJKToRAS = vtkMatrix4x4::New();
  vtkMRMLLinearTransformNode *movingVolumeTransform = NULL;

  // make sure inputs are initialized
  if(!inputVolume || !inputROI ){
    std::cerr << "CropVolume: Inputs are not initialized" << std::endl;
    return -1;
  }

  // check the output volume type
  vtkMRMLDiffusionTensorVolumeNode *dtvnode= vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(inputVolume);
  vtkMRMLDiffusionWeightedVolumeNode *dwvnode= vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(inputVolume);
  vtkMRMLVectorVolumeNode *vvnode= vtkMRMLVectorVolumeNode::SafeDownCast(inputVolume);
  vtkMRMLScalarVolumeNode *svnode = vtkMRMLScalarVolumeNode::SafeDownCast(inputVolume);

  qSlicerModuleManager * moduleManager =
          qSlicerCoreApplication::application()->moduleManager();
  qSlicerModuleFactoryManager* moduleFactoryManager = moduleManager->factoryManager();
  QStringList moduleNames = moduleFactoryManager->moduleNames();

  qSlicerAbstractCoreModule * volumesModule = moduleManager->module("Volumes");
  if(!volumesModule){
    qWarning() << "CropVolume: ERROR: volumes module reference was not found!";
    return -3;
  }

  vtkSlicerVolumesLogic *volumesLogic = vtkSlicerVolumesLogic::SafeDownCast(volumesModule->logic());
  if(!volumesLogic){
      qWarning() << "CropVolume: ERROR: failed to get hold of Volumes logic";
      return -2;
  }
  if(dtvnode){
    qWarning() << "CropVolume: ERROR: Diffusion tensor volumes are not supported by this module!";
    return -2;
  }
  if(dwvnode){
    outputVolume = (vtkMRMLVolumeNode*) volumesLogic->CloneVolume(this->GetMRMLScene(), inputVolume, "output_dwv");
  }
  if(vvnode){
    outputVolume = (vtkMRMLVolumeNode*) volumesLogic->CloneVolume(this->GetMRMLScene(), inputVolume, "output_vv");
  }
  if(svnode){
    outputVolume = (vtkMRMLVolumeNode*) volumesLogic->CloneVolume(this->GetMRMLScene(), inputVolume, "output_sv");
  }
  refVolume = volumesLogic->CreateLabelVolume(this->GetMRMLScene(), inputVolume, "CropVolume_ref_volume");
  refVolume->HideFromEditorsOn();

  //vtkMatrix4x4 *volumeXform = vtkMatrix4x4::New();
  //vtkMatrix4x4 *roiXform = vtkMatrix4x4::New();
  //vtkMatrix4x4 *T = vtkMatrix4x4::New();

  refVolume->GetRASToIJKMatrix(inputRASToIJK);
  refVolume->GetIJKToRASMatrix(inputIJKToRAS);
  outputRASToIJK->Identity();
  outputIJKToRAS->Identity();

  //T->Identity();
  //roiXform->Identity();
  //volumeXform->Identity();

  // prepare the resampling reference volume
  double roiRadius[3], roiXYZ[3];
  inputROI->GetRadiusXYZ(roiRadius);
  inputROI->GetXYZ(roiXYZ);

  double* inputSpacing = inputVolume->GetSpacing();
  double outputSpacing[3], spacingScaleConst = pnode->GetSpacingScalingConst();
  double minSpacing = inputSpacing[0];
  if (minSpacing > inputSpacing[1])
    {
    minSpacing = inputSpacing[1];
    }
  if (minSpacing > inputSpacing[2])
    {
    minSpacing = inputSpacing[2];
    }

  if(pnode->GetIsotropicResampling()){
      outputSpacing[0] = minSpacing*spacingScaleConst;
      outputSpacing[1] = minSpacing*spacingScaleConst;
      outputSpacing[2] = minSpacing*spacingScaleConst;
  } else {
      outputSpacing[0] = inputSpacing[0]*spacingScaleConst;
      outputSpacing[1] = inputSpacing[1]*spacingScaleConst;
      outputSpacing[2] = inputSpacing[2]*spacingScaleConst;
  }

  int outputExtent[3];

  outputExtent[0] = roiRadius[0]/outputSpacing[0]*2.;
  outputExtent[1] = roiRadius[1]/outputSpacing[1]*2.;
  outputExtent[2] = roiRadius[2]/outputSpacing[2]*2.;

  outputIJKToRAS->SetElement(0,0,outputSpacing[0]);
  outputIJKToRAS->SetElement(1,1,outputSpacing[1]);
  outputIJKToRAS->SetElement(2,2,outputSpacing[2]);

  outputIJKToRAS->SetElement(0,3,roiXYZ[0]-roiRadius[0]+outputSpacing[0]*.5);
  outputIJKToRAS->SetElement(1,3,roiXYZ[1]-roiRadius[1]+outputSpacing[1]*.5);
  outputIJKToRAS->SetElement(2,3,roiXYZ[2]-roiRadius[2]+outputSpacing[2]*.5);

  outputRASToIJK->DeepCopy(outputIJKToRAS);
  outputRASToIJK->Invert();

  vtkImageData* outputImageData = vtkImageData::New();
  outputImageData->SetDimensions(outputExtent[0], outputExtent[1], outputExtent[2]);
  outputImageData->AllocateScalars();

  refVolume->SetAndObserveImageData(outputImageData);
  outputImageData->Delete();

  refVolume->SetIJKToRASMatrix(outputIJKToRAS);
  refVolume->SetRASToIJKMatrix(outputRASToIJK);

  /*
  vtkMRMLDisplayNode* inputDisplay = inputVolume->GetDisplayNode();
  if(inputDisplay){
    vtkMRMLDisplayNode* outputDisplay = inputDisplay->NewInstance();
    outputDisplay->Copy(inputDisplay);
    this->GetMRMLScene()->AddNodeNoNotify(outputDisplay);
    outputVolume->SetAndObserveDisplayNodeID(outputDisplay->GetID());
    outputDisplay->Delete();
  }
  */

  inputRASToIJK->Delete();
  inputIJKToRAS->Delete();
  outputRASToIJK->Delete();
  outputIJKToRAS->Delete();

  // use the prepared volume as the reference for resampling

  if(!moduleNames.contains("resamplevolume2")){
      qWarning() << "CropVolume: ERROR: resamplevolume2 module name was not found in the list of registered modules!";
      return -3;
  }

  qSlicerAbstractCoreModule * module = moduleManager->module("ResampleVolume2");
  if(!module){
      qWarning() << "CropVolume: ERROR: resamplevolume2 module reference was not found!";
      return -3;
  }

  qSlicerCLIModule * cliModule = qobject_cast<qSlicerCLIModule*>(module);
  vtkMRMLCommandLineModuleNode* cmdNode = cliModule->createNode();

  cmdNode->SetParameterAsString("inputVolume", inputVolume->GetID());
  cmdNode->SetParameterAsString("referenceVolume",refVolume->GetID());
  cmdNode->SetParameterAsString("outputVolume",outputVolume->GetID());
  movingVolumeTransform = static_cast<vtkMRMLLinearTransformNode*>(inputVolume->GetParentTransformNode());

  if(movingVolumeTransform != NULL)
    cmdNode->SetParameterAsString("transformationFile",movingVolumeTransform->GetID());

  std::string interp = "linear";
  switch(pnode->GetInterpolationMode()){
    case 1: interp = "nn"; break;
    case 2: interp = "linear"; break;
    case 3: interp = "ws"; break;
    case 4: interp = "bs"; break;
  }

  cmdNode->SetParameterAsString("interpolationType", interp.c_str());
  cliModule->run(cmdNode, true);

  this->GetMRMLScene()->RemoveNode(refVolume);

  std::ostringstream outSS;
  outSS << inputVolume->GetName() << "-subvolume-scale_" << spacingScaleConst;
  outputVolume->SetName(outSS.str().c_str());

  outputVolume->ModifiedSinceReadOn();
  pnode->SetAndObserveOutputVolumeNodeID(outputVolume->GetID());
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
