/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerCropVolumeLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// CLI invocation
#include <qSlicerCLIModule.h>
#include <vtkSlicerCLIModuleLogic.h>

// CropLogic includes
#include "vtkSlicerCLIModuleLogic.h"
#include "vtkSlicerCropVolumeLogic.h"
#include "vtkSlicerVolumesLogic.h"

// CropMRML includes

// MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionWeightedVolumeNode.h>
#include <vtkMRMLDiffusionWeightedVolumeDisplayNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLAnnotationROINode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkImageClip.h>
#include <vtkNew.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkVersion.h>

// STD includes
#include <cassert>
#include <iostream>

//----------------------------------------------------------------------------
class vtkSlicerCropVolumeLogic::vtkInternal
{
public:
  vtkInternal();

  vtkSlicerVolumesLogic* VolumesLogic;
  vtkSlicerCLIModuleLogic* ResampleLogic;
};

//----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic::vtkInternal::vtkInternal()
{
  this->VolumesLogic = 0;
  this->ResampleLogic = 0;
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCropVolumeLogic);

//----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic::vtkSlicerCropVolumeLogic()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic::~vtkSlicerCropVolumeLogic()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::SetVolumesLogic(vtkSlicerVolumesLogic* logic)
{
  this->Internal->VolumesLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic* vtkSlicerCropVolumeLogic::GetVolumesLogic()
{
  return this->Internal->VolumesLogic;
}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::SetResampleLogic(vtkSlicerCLIModuleLogic* logic)
{
  this->Internal->ResampleLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerCLIModuleLogic* vtkSlicerCropVolumeLogic::GetResampleLogic()
{
  return this->Internal->ResampleLogic;
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
  vtkMRMLScene *scene = this->GetMRMLScene();

  vtkMRMLVolumeNode *inputVolume =
    vtkMRMLVolumeNode::SafeDownCast(scene->GetNodeByID(pnode->GetInputVolumeNodeID()));
  vtkMRMLAnnotationROINode *inputROI =
    vtkMRMLAnnotationROINode::SafeDownCast(scene->GetNodeByID(pnode->GetROINodeID()));

  if(!inputVolume || !inputROI)
    {
    std::cerr << "Failed to look up input volume and/or ROI!" << std::endl;
    return -1;
    }

  vtkMRMLVolumeNode *outputVolume = NULL;

  // make sure inputs are initialized
  if(!inputVolume || !inputROI )
    {
    std::cerr << "CropVolume: Inputs are not initialized" << std::endl;
    return -1;
    }

  // check the output volume type
  vtkMRMLDiffusionTensorVolumeNode *dtvnode= vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(inputVolume);
  vtkMRMLDiffusionWeightedVolumeNode *dwvnode= vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(inputVolume);
  vtkMRMLVectorVolumeNode *vvnode= vtkMRMLVectorVolumeNode::SafeDownCast(inputVolume);
  vtkMRMLScalarVolumeNode *svnode = vtkMRMLScalarVolumeNode::SafeDownCast(inputVolume);

  if(!this->Internal->VolumesLogic)
    {
      std::cerr << "CropVolume: ERROR: failed to get hold of Volumes logic" << std::endl;
      return -2;
    }

  std::ostringstream outSS;
  double outputSpacing[3], spacingScaleConst = pnode->GetSpacingScalingConst();
  outSS << inputVolume->GetName() << "-subvolume-scale_" << spacingScaleConst;

  if(dtvnode)
    {
    std::cerr << "CropVolume: ERROR: Diffusion tensor volumes are not supported by this module!" << std::endl;
    return -2;
    }
  // need to create clones and display nodes here, since
  // VolumesLogic::CloneVolume() handles only ScalarVolumeNode's
  else if(dwvnode)
    {
    vtkNew<vtkMRMLDiffusionWeightedVolumeNode> outputDWVNode;
    outputDWVNode->CopyWithScene(dwvnode);
    vtkNew<vtkMRMLDiffusionWeightedVolumeDisplayNode> dwiDisplayNode;
    dwiDisplayNode->CopyWithScene(dwvnode->GetDisplayNode());
    scene->AddNode(dwiDisplayNode.GetPointer());

    vtkNew<vtkImageData> outputImageData;
    outputImageData->DeepCopy(dwvnode->GetImageData());
    outputDWVNode->SetAndObserveImageData(outputImageData.GetPointer());

    outputDWVNode->SetAndObserveDisplayNodeID(dwiDisplayNode->GetID());
    outputDWVNode->SetAndObserveStorageNodeID(NULL);
    scene->AddNode(outputDWVNode.GetPointer());

    outputVolume = outputDWVNode.GetPointer();
    }
  else if(vvnode)
    {
    vtkNew<vtkMRMLVectorVolumeNode> outputVVNode;
    outputVVNode->CopyWithScene(dwvnode);
    vtkNew<vtkMRMLVectorVolumeDisplayNode> vvDisplayNode;
    vvDisplayNode->CopyWithScene(vvnode->GetDisplayNode());
    scene->AddNode(vvDisplayNode.GetPointer());

    vtkNew<vtkImageData> outputImageData;
    outputImageData->DeepCopy(vvnode->GetImageData());
    outputVVNode->SetAndObserveImageData(outputImageData.GetPointer());

    outputVVNode->SetAndObserveDisplayNodeID(vvDisplayNode->GetID());
    outputVVNode->SetAndObserveStorageNodeID(NULL);
    scene->AddNode(outputVVNode.GetPointer());

    outputVolume = outputVVNode.GetPointer();
    }
  else if(svnode)
    {
    outputVolume = this->Internal->VolumesLogic->CloneVolume(this->GetMRMLScene(), inputVolume, outSS.str().c_str());
    }
  else
    {
    std::cerr << "Input volume not recognized!" << std::endl;
    return -1;
    }

  outputVolume->SetName(outSS.str().c_str());

  if(pnode->GetVoxelBased()) // voxel based cropping selected
    {
      this->CropVoxelBased(inputROI,inputVolume,outputVolume);
    }
  else  // interpolated cropping selected
    {
      vtkMRMLScalarVolumeNode *refVolume;
      vtkMatrix4x4 *inputRASToIJK = vtkMatrix4x4::New();
      vtkMatrix4x4 *inputIJKToRAS = vtkMatrix4x4::New();
      vtkMatrix4x4 *outputRASToIJK = vtkMatrix4x4::New();
      vtkMatrix4x4 *outputIJKToRAS = vtkMatrix4x4::New();

      refVolume = this->Internal->VolumesLogic->CreateAndAddLabelVolume(
          this->GetMRMLScene(), inputVolume, "CropVolume_ref_volume");
      refVolume->HideFromEditorsOn();

      refVolume->GetRASToIJKMatrix(inputRASToIJK);
      refVolume->GetIJKToRASMatrix(inputIJKToRAS);
      outputRASToIJK->Identity();
      outputIJKToRAS->Identity();

      // prepare the resampling reference volume
      double roiRadius[3], roiXYZ[3];
      inputROI->GetRadiusXYZ(roiRadius);
      inputROI->GetXYZ(roiXYZ);

      double* inputSpacing = inputVolume->GetSpacing();
      double minSpacing = inputSpacing[0];
      if (minSpacing > inputSpacing[1])
        {
          minSpacing = inputSpacing[1];
        }
      if (minSpacing > inputSpacing[2])
        {
          minSpacing = inputSpacing[2];
        }

      if (pnode->GetIsotropicResampling())
        {
          outputSpacing[0] = minSpacing * spacingScaleConst;
          outputSpacing[1] = minSpacing * spacingScaleConst;
          outputSpacing[2] = minSpacing * spacingScaleConst;
        }
      else
        {
          outputSpacing[0] = inputSpacing[0] * spacingScaleConst;
          outputSpacing[1] = inputSpacing[1] * spacingScaleConst;
          outputSpacing[2] = inputSpacing[2] * spacingScaleConst;
        }

      int outputExtent[3];

      outputExtent[0] = roiRadius[0] / outputSpacing[0] * 2.;
      outputExtent[1] = roiRadius[1] / outputSpacing[1] * 2.;
      outputExtent[2] = roiRadius[2] / outputSpacing[2] * 2.;

      outputIJKToRAS->SetElement(0, 0, outputSpacing[0]);
      outputIJKToRAS->SetElement(1, 1, outputSpacing[1]);
      outputIJKToRAS->SetElement(2, 2, outputSpacing[2]);

      outputIJKToRAS->SetElement(0, 3,
          roiXYZ[0] - roiRadius[0] + outputSpacing[0] * .5);
      outputIJKToRAS->SetElement(1, 3,
          roiXYZ[1] - roiRadius[1] + outputSpacing[1] * .5);
      outputIJKToRAS->SetElement(2, 3,
          roiXYZ[2] - roiRadius[2] + outputSpacing[2] * .5);

      // account for the ROI parent transform, if present
      vtkMRMLTransformNode *roiTransform = inputROI->GetParentTransformNode();
      if (roiTransform && roiTransform->IsTransformToWorldLinear())
        {
          vtkMatrix4x4 *roiMatrix = vtkMatrix4x4::New();
          roiTransform->GetMatrixTransformToWorld(roiMatrix);
          outputIJKToRAS->Multiply4x4(roiMatrix, outputIJKToRAS,
              outputIJKToRAS);
        }

      outputRASToIJK->DeepCopy(outputIJKToRAS);
      outputRASToIJK->Invert();

      vtkImageData* outputImageData = vtkImageData::New();
      outputImageData->SetDimensions(outputExtent[0], outputExtent[1],
          outputExtent[2]);
      outputImageData->AllocateScalars(VTK_DOUBLE, 1);

      refVolume->SetAndObserveImageData(outputImageData);
      outputImageData->Delete();

      refVolume->SetIJKToRASMatrix(outputIJKToRAS);
      refVolume->SetRASToIJKMatrix(outputRASToIJK);

      inputRASToIJK->Delete();
      inputIJKToRAS->Delete();
      outputRASToIJK->Delete();
      outputIJKToRAS->Delete();

      if (this->Internal->ResampleLogic == 0)
        {
          std::cerr << "CropVolume: ERROR: resample logic is not set!";
          return -3;
        }

      vtkSmartPointer<vtkMRMLCommandLineModuleNode> cmdNode =
          this->Internal->ResampleLogic->CreateNodeInScene();
      assert(cmdNode.GetPointer() != 0);

      cmdNode->SetParameterAsString("inputVolume", inputVolume->GetID());
      cmdNode->SetParameterAsString("referenceVolume", refVolume->GetID());
      cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());

      vtkMRMLTransformNode *movingVolumeTransform = inputVolume->GetParentTransformNode();

      if (movingVolumeTransform != NULL && movingVolumeTransform->IsLinear())
        {
        cmdNode->SetParameterAsString("transformationFile",
            movingVolumeTransform->GetID());
        }

      std::string interp = "linear";
      switch (pnode->GetInterpolationMode())
        {
      case 1:
        interp = "nn";
        break;
      case 2:
        interp = "linear";
        break;
      case 3:
        interp = "ws";
        break;
      case 4:
        interp = "bs";
        break;
        }

      cmdNode->SetParameterAsString("interpolationType", interp.c_str());
      this->Internal->ResampleLogic->ApplyAndWait(cmdNode);

      this->GetMRMLScene()->RemoveNode(refVolume);
      this->GetMRMLScene()->RemoveNode(cmdNode);

    }

  outputVolume->SetAndObserveTransformNodeID(NULL);
  pnode->SetOutputVolumeNodeID(outputVolume->GetID());

  return 0;
}


//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::CropVoxelBased(vtkMRMLAnnotationROINode* roi, vtkMRMLVolumeNode* inputVolume, vtkMRMLVolumeNode* outputVolume)
{
  if(!roi || !inputVolume || !outputVolume)
    return;

  vtkNew<vtkImageData> imageDataWorkingCopy;
  imageDataWorkingCopy->DeepCopy(inputVolume->GetImageData());

  vtkNew<vtkMatrix4x4> inputRASToIJK;
  inputVolume->GetRASToIJKMatrix(inputRASToIJK.GetPointer());

  vtkNew<vtkMatrix4x4> inputIJKToRAS;
  inputVolume->GetIJKToRASMatrix(inputIJKToRAS.GetPointer());

  double roiXYZ[3];
  double roiRadius[3];

  roi->GetXYZ(roiXYZ);
  roi->GetRadiusXYZ(roiRadius);

  double minXYZRAS[] = {roiXYZ[0]-roiRadius[0], roiXYZ[1]-roiRadius[1],roiXYZ[2]-roiRadius[2],1.};
  double maxXYZRAS[] = {roiXYZ[0]+roiRadius[0], roiXYZ[1]+roiRadius[1],roiXYZ[2]+roiRadius[2],1.};

  vtkMRMLTransformNode* roiTransform  = roi->GetParentTransformNode();
  // account for the ROI parent transform, if present
  if (roiTransform && roiTransform->IsTransformToWorldLinear())
    {
      vtkNew<vtkMatrix4x4> roiTransformMatrix;
      roiTransform->GetMatrixTransformToWorld(roiTransformMatrix.GetPointer());
      if (roiTransformMatrix.GetPointer())
        {
          // multiply ROI's min and max corners with parent's transform to world to get real RAS position
          roiTransformMatrix->MultiplyPoint(minXYZRAS, minXYZRAS);
          roiTransformMatrix->MultiplyPoint(maxXYZRAS, maxXYZRAS);
        }
    }

  double minXYZIJK[4], maxXYZIJK[4];

  //transform to ijk
  inputRASToIJK->MultiplyPoint(minXYZRAS, minXYZIJK);
  inputRASToIJK->MultiplyPoint(maxXYZRAS, maxXYZIJK);

  double minX = std::min(minXYZIJK[0],maxXYZIJK[0]);
  double maxX = std::max(minXYZIJK[0],maxXYZIJK[0]) + 0.5; // 0.5 for rounding purposes to make sure everything selected by roi is cropped
  double minY = std::min(minXYZIJK[1],maxXYZIJK[1]);
  double maxY = std::max(minXYZIJK[1],maxXYZIJK[1]) + 0.5;
  double minZ = std::min(minXYZIJK[2],maxXYZIJK[2]);
  double maxZ = std::max(minXYZIJK[2],maxXYZIJK[2]) + 0.5;

  int originalImageExtents[6];
  imageDataWorkingCopy->GetExtent(originalImageExtents);

  minX = std::max(minX,0.);
  maxX = std::min(maxX,static_cast<double>(originalImageExtents[1]));
  minY = std::max(minY,0.);
  maxY = std::min(maxY,static_cast<double>(originalImageExtents[3]));
  minZ = std::max(minZ,0.);
  maxZ = std::min(maxZ,static_cast<double>(originalImageExtents[5]));

  int outputWholeExtent[6] = {
    static_cast<int>(minX),
    static_cast<int>(maxX),
    static_cast<int>(minY),
    static_cast<int>(maxY),
    static_cast<int>(minZ),
    static_cast<int>(maxZ)};

  const double ijkNewOrigin[] = {
    static_cast<double>(outputWholeExtent[0]),
    static_cast<double>(outputWholeExtent[2]),
    static_cast<double>(outputWholeExtent[4]),
    static_cast<double>(1.0)};

  double  rasNewOrigin[4];
  inputIJKToRAS->MultiplyPoint(ijkNewOrigin,rasNewOrigin);


  vtkNew<vtkImageClip> imageClip;
  imageClip->SetInputData(imageDataWorkingCopy.GetPointer());
  imageClip->SetOutputWholeExtent(outputWholeExtent);
  imageClip->SetClipData(true);

  imageClip->Update();


  vtkNew<vtkMatrix4x4> outputIJKToRAS;
  outputIJKToRAS->DeepCopy(inputIJKToRAS.GetPointer());

  outputIJKToRAS->SetElement(0,3,rasNewOrigin[0]);
  outputIJKToRAS->SetElement(1,3,rasNewOrigin[1]);
  outputIJKToRAS->SetElement(2,3,rasNewOrigin[2]);

  outputVolume->SetOrigin(rasNewOrigin[0],rasNewOrigin[1],rasNewOrigin[2]);

  vtkNew<vtkMatrix4x4> outputRASToIJK;
  outputRASToIJK->DeepCopy(outputIJKToRAS.GetPointer());
  outputRASToIJK->Invert();

  vtkNew<vtkImageData> outputImageData;
  outputImageData->DeepCopy(imageClip->GetOutput());

  int extent[6];
  imageClip->GetOutput()->GetExtent(extent);

  outputImageData->SetExtent(0, extent[1]-extent[0], 0, extent[3]-extent[2], 0, extent[5]-extent[4]);

  outputVolume->SetAndObserveImageData(outputImageData.GetPointer());
  outputVolume->SetIJKToRASMatrix(outputIJKToRAS.GetPointer());
  outputVolume->SetRASToIJKMatrix(outputRASToIJK.GetPointer());

  outputVolume->Modified();

}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::RegisterNodes()
{
  if(!this->GetMRMLScene())
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode* pNode = vtkMRMLCropVolumeParametersNode::New();
  this->GetMRMLScene()->RegisterNodeClass(pNode);
  pNode->Delete();
}


void vtkSlicerCropVolumeLogic::SnapROIToVoxelGrid(vtkMRMLAnnotationROINode* inputROI, vtkMRMLVolumeNode* inputVolume)
{
  if (!inputROI || !inputVolume)
    {
      vtkDebugMacro(
          "vtkSlicerCropVolumeLogic: Snapping ROI to voxel grid failed (input ROI and/or input Volume are invalid)");
      return;
    }

  vtkSmartPointer<vtkMRMLScene> scene = this->GetMRMLScene();

  if (!scene)
    {
      vtkDebugMacro(
          "vtkSlicerCropVolumeLogic: Snapping ROI to voxel grid failed (MRML Scene is not available)");
      return;
    }

  vtkNew<vtkMatrix4x4> rotationMat;

  bool ok = vtkSlicerCropVolumeLogic::ComputeIJKToRASRotationOnlyMatrix(
      inputVolume, rotationMat.GetPointer());


  if (!ok)
    {
      vtkDebugMacro(
          "vtkSlicerCropVolumeLogic: Snapping ROI to voxel grid failed (IJK to RAS rotation only matrix computation failed)");
      return;
    }

  vtkNew<vtkMRMLLinearTransformNode> roiTransformNode;
  roiTransformNode->ApplyTransformMatrix(rotationMat.GetPointer());
  roiTransformNode->SetScene(this->GetMRMLScene());

  this->GetMRMLScene()->AddNode(roiTransformNode.GetPointer());

  inputROI->SetAndObserveTransformNodeID(roiTransformNode->GetID());
}



//----------------------------------------------------------------------------
bool vtkSlicerCropVolumeLogic::ComputeIJKToRASRotationOnlyMatrix(vtkMRMLVolumeNode* inputVolume, vtkMatrix4x4* outputMatrix)
{
  if(inputVolume == NULL || outputMatrix == NULL)
    return false;

  vtkNew<vtkMatrix4x4> rotationMat;
  rotationMat->Identity();

  vtkNew<vtkMatrix4x4> inputIJKToRASMat;
  inputIJKToRASMat->Identity();

  inputVolume->GetIJKToRASMatrix(inputIJKToRASMat.GetPointer());
  const char* scanOrder = vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(inputIJKToRASMat.GetPointer());

  vtkNew<vtkMatrix4x4> orientMat;
  orientMat->Identity();

  bool orientation = vtkSlicerCropVolumeLogic::ComputeOrientationMatrixFromScanOrder(scanOrder,orientMat.GetPointer());

  if(!orientation)
    return false;

  orientMat->Invert();

  vtkNew<vtkMatrix4x4> directionsMat;
  directionsMat->Identity();

  inputVolume->GetIJKToRASDirectionMatrix(directionsMat.GetPointer());

  vtkMatrix4x4::Multiply4x4(directionsMat.GetPointer(),orientMat.GetPointer(),rotationMat.GetPointer());

  outputMatrix->DeepCopy(rotationMat.GetPointer());

  return true;
}


//----------------------------------------------------------------------------
bool vtkSlicerCropVolumeLogic::IsVolumeTiltedInRAS( vtkMRMLVolumeNode* inputVolume, vtkMatrix4x4* rotationMatrix)
{
  assert(inputVolume);

  vtkNew<vtkMatrix4x4> iJKToRASMat;
  vtkNew<vtkMatrix4x4> directionMat;

  inputVolume->GetIJKToRASMatrix(iJKToRASMat.GetPointer());
  inputVolume->GetIJKToRASDirectionMatrix(directionMat.GetPointer());

  const char* scanOrder = vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(iJKToRASMat.GetPointer());

  vtkNew<vtkMatrix4x4> orientMat;
  vtkSlicerCropVolumeLogic::ComputeOrientationMatrixFromScanOrder(scanOrder,orientMat.GetPointer());

  bool same = true;

  for(int i=0; i < 4; ++i)
    {
      for(int j=0; j < 4; ++j)
        {
          if(directionMat->GetElement(i,j) != orientMat->GetElement(i,j))
            {
              same = false;
              break;
            }
        }
      if(same == false)
        break;
    }

  if(!rotationMatrix)
    rotationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  if(same)
    {
      rotationMatrix->Identity();
    }
  else
    {
      vtkSlicerCropVolumeLogic::ComputeIJKToRASRotationOnlyMatrix(inputVolume,rotationMatrix);
      return true;
    }



  return false;
}

//----------------------------------------------------------------------------
bool
vtkSlicerCropVolumeLogic::ComputeOrientationMatrixFromScanOrder(
    const char *order, vtkMatrix4x4 *outputMatrix)
{
  vtkNew<vtkMatrix4x4> orientMat;
  orientMat->Identity();

  if (!strcmp(order, "IS") || !strcmp(order, "Axial IS")
      || !strcmp(order, "Axial"))
    {
      double elems[] =
        { -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
      orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order, "SI") || !strcmp(order, "Axial SI"))
    {
      double elems[] =
        { -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1 };
      orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order, "RL") || !strcmp(order, "Sagittal RL")
      || !strcmp(order, "Sagittal"))
    {
      double elems[] =
        { 0, 0, -1, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 1 };
      orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order, "LR") || !strcmp(order, "Sagittal LR"))
    {
      double elems[] =
        { 0, 0, 1, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 1 };
      orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order, "PA") || !strcmp(order, "Coronal PA")
      || !strcmp(order, "Coronal"))
    {
      double elems[] =
        { -1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1 };
      orientMat->DeepCopy(elems);
    }
  else if (!strcmp(order, "AP") || !strcmp(order, "Coronal AP"))
    {
      double elems[] =
        { -1, 0, 0, 0, 0, 0, -1, 0, 0, -1, 0, 0, 0, 0, 0, 1 };
      orientMat->DeepCopy(elems);
    }
  else
    {
      return false;
    }


  outputMatrix->DeepCopy(orientMat.GetPointer());
  return true;
}
