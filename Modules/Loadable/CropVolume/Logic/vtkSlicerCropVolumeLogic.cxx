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
#include <vtkSlicerCLIModuleLogic.h>

// CropLogic includes
#include "vtkSlicerCLIModuleLogic.h"
#include "vtkSlicerCropVolumeLogic.h"
#include "vtkSlicerVolumesLogic.h"

// CropMRML includes

// MRML includes
#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionWeightedVolumeNode.h>
#include <vtkMRMLDiffusionWeightedVolumeDisplayNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkGeneralTransform.h>
#include <vtkImageConstantPad.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkMatrix4x4.h>
#include <vtkMatrix3x3.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkVersion.h>

#include <vtkAddonMathUtilities.h>

// STD includes
#include <cassert>
#include <iostream>

//----------------------------------------------------------------------------
class vtkSlicerCropVolumeLogic::vtkInternal
{
public:
  vtkInternal();
  static void GetROIXYZ(vtkMRMLDisplayableNode* roi, double* xyz)
    {
    vtkMRMLMarkupsROINode* markupsROI = vtkMRMLMarkupsROINode::SafeDownCast(roi);
    if (markupsROI)
      {
      // We don't call markupsROI->GetXYZ(xyz) here because where
      // GetROIXYZ method is called, the ROI (not the local) coordinate
      // system is used. ROI center coordinates in ROI coordinate system
      // are always (0,0,0).
      xyz[0] = 0.0;
      xyz[1] = 0.0;
      xyz[2] = 0.0;
      }
    else
      {
      vtkGenericWarningMacro("vtkSlicerCropVolumeLogic::vtkInternal::GetROIXYZ failed: invalid ROI");
      }
    }

  static void SetROIXYZ(vtkMRMLDisplayableNode* roi, double* xyz)
    {
    vtkMRMLMarkupsROINode* markupsROI = vtkMRMLMarkupsROINode::SafeDownCast(roi);
    if (markupsROI)
      {
      markupsROI->SetXYZ(xyz);
      }
    else
      {
      vtkGenericWarningMacro("vtkSlicerCropVolumeLogic::vtkInternal::SetXYZ failed: invalid ROI");
      }
    }

  static void GetROIRadius(vtkMRMLDisplayableNode* roi, double* radius)
    {
    vtkMRMLMarkupsROINode* markupsROI = vtkMRMLMarkupsROINode::SafeDownCast(roi);
    if (markupsROI)
      {
      markupsROI->GetRadiusXYZ(radius);
      }
    else
      {
      vtkGenericWarningMacro("vtkSlicerCropVolumeLogic::vtkInternal::GetROIRadius failed: invalid ROI");
      }
    }

  static void SetROIRadius(vtkMRMLDisplayableNode* roi, double* radius)
    {
    vtkMRMLMarkupsROINode* markupsROI = vtkMRMLMarkupsROINode::SafeDownCast(roi);
    if (markupsROI)
      {
      markupsROI->SetRadiusXYZ(radius);
      }
    else
      {
      vtkGenericWarningMacro("vtkSlicerCropVolumeLogic::vtkInternal::SetROIRadius failed: invalid ROI");
      }
    }


  static void GetMatrixTransformFromObjectToNode(vtkMRMLDisplayableNode* roi, vtkMRMLTransformableNode* toNode, vtkMatrix4x4* objectToNode)
    {
    vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(roi->GetParentTransformNode(), toNode->GetParentTransformNode(), objectToNode);
    vtkMRMLMarkupsROINode* markupsROI = vtkMRMLMarkupsROINode::SafeDownCast(roi);
    if (markupsROI)
      {
      vtkMatrix4x4::Multiply4x4(objectToNode, markupsROI->GetObjectToNodeMatrix(), objectToNode);
      }
    }

  static void GetObjectToVolumeIJK(vtkMRMLDisplayableNode* roi, vtkMRMLVolumeNode* volumeNode, vtkMatrix4x4* objectToVolumeIJK)
    {
    vtkNew<vtkMatrix4x4> objectToVolumeRAS;
    GetMatrixTransformFromObjectToNode(roi, volumeNode, objectToVolumeRAS);
    vtkNew<vtkMatrix4x4> rasToIJK;
    volumeNode->GetRASToIJKMatrix(rasToIJK);
    vtkMatrix4x4::Multiply4x4(rasToIJK, objectToVolumeRAS, objectToVolumeIJK);
    }

};

//----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic::vtkInternal::vtkInternal()=default;

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
void vtkSlicerCropVolumeLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSlicerCropVolumeLogic:             " << this->GetClassName() << "\n";
}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::RegisterNodes()
{
  if (!this->GetMRMLScene())
    {
    return;
    }
  this->GetMRMLScene()->RegisterNodeClass(vtkSmartPointer<vtkMRMLCropVolumeParametersNode>::New());
}

//----------------------------------------------------------------------------
int vtkSlicerCropVolumeLogic::Apply(vtkMRMLCropVolumeParametersNode* pnode)
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("CropVolume: Invalid scene");
    return -1;
    }

  // Check inputs
  if (!pnode)
    {
    vtkErrorMacro("CropVolume: Invalid parameter node");
    return -1;
    }
  vtkMRMLVolumeNode* inputVolume = vtkMRMLVolumeNode::SafeDownCast(scene->GetNodeByID(pnode->GetInputVolumeNodeID()));
  vtkMRMLDisplayableNode* inputROI = vtkMRMLDisplayableNode::SafeDownCast(scene->GetNodeByID(pnode->GetROINodeID()));
  vtkMRMLMarkupsROINode* inputMarkupsROI = vtkMRMLMarkupsROINode::SafeDownCast(inputROI);
  if (!inputVolume || !inputMarkupsROI)
    {
    vtkErrorMacro("CropVolume: Invalid input volume or ROI");
    return -1;
    }
  if (vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(inputVolume))
    {
    vtkErrorMacro("CropVolume: Diffusion tensor volumes are not supported by this module");
    return -2;
    }

  // Check/create output volume
  vtkMRMLVolumeNode *outputVolume =
    vtkMRMLVolumeNode::SafeDownCast(scene->GetNodeByID(pnode->GetOutputVolumeNodeID()));
  if (outputVolume)
    {
    // Output volume is provided, use that (if compatible)
    if (outputVolume->GetClassName() != inputVolume->GetClassName())
      {
      vtkErrorMacro("CropVolume: output volume (" << outputVolume->GetClassName() <<
        ") is not compatible with input volume (" << inputVolume->GetClassName() << ")");
      return -1;
      }
    }
  else
    {

    vtkSlicerVolumesLogic* volumesLogic =
      vtkSlicerVolumesLogic::SafeDownCast(this->GetModuleLogic("Volumes"));

    // Create compatible output volume
    if (!volumesLogic)
      {
      vtkErrorMacro("CropVolume: invalid Volumes logic");
      return -2;
      }
    std::ostringstream outSS;
    outSS << (inputVolume->GetName() ? inputVolume->GetName() : "Volume") << " cropped";
    outputVolume = volumesLogic->CloneVolume(this->GetMRMLScene(), inputVolume, outSS.str().c_str());
    if (!outputVolume)
      {
      vtkErrorMacro("CropVolume: failed to create output volume");
      return -2;
      }
    vtkMRMLTransformNode *outputTransform = outputVolume->GetParentTransformNode();
    if (outputTransform && !outputTransform->IsTransformToWorldLinear())
      {
      // Output node must not be under non-linear transform
      outputVolume->SetAndObserveTransformNodeID(nullptr);
      }
    }

  int errorCode = 0;
  if (pnode->GetVoxelBased()) // voxel based cropping selected
    {
    errorCode = this->CropVoxelBased(inputROI, inputVolume, outputVolume, false, pnode->GetFillValue());
    }
  else  // interpolated cropping selected
    {
    errorCode = this->CropInterpolated(inputROI, inputVolume, outputVolume,
      pnode->GetIsotropicResampling(), pnode->GetSpacingScalingConst(), pnode->GetInterpolationMode(), pnode->GetFillValue());
    }
  pnode->SetOutputVolumeNodeID(outputVolume->GetID());
  return errorCode;
}

//----------------------------------------------------------------------------
bool vtkSlicerCropVolumeLogic::GetVoxelBasedCropOutputExtent(vtkMRMLDisplayableNode* roi, vtkMRMLVolumeNode* inputVolume,
  int outputExtent[6], bool limitToInputExtent/*=true*/)
{
  outputExtent[0] = outputExtent[2] = outputExtent[4] = 0;
  outputExtent[1] = outputExtent[3] = outputExtent[5] = -1;
  if ((!roi) || !inputVolume || !inputVolume->GetImageData())
    {
    return false;
    }

  int originalImageExtents[6] = { 0 };
  inputVolume->GetImageData()->GetExtent(originalImageExtents);

  vtkMRMLTransformNode* roiTransform = roi->GetParentTransformNode();
  if (roiTransform && !roiTransform->IsTransformToWorldLinear())
    {
    vtkGenericWarningMacro("CropVolume: ROI is transformed using a non-linear transform. The transformation will be ignored");
    roiTransform = nullptr;
    }

  if (inputVolume->GetParentTransformNode() && !inputVolume->GetParentTransformNode()->IsTransformToWorldLinear())
    {
    vtkGenericWarningMacro("vtkSlicerCropVolumeLogic::CropVoxelBased: voxel-based cropping of non-linearly transformed input volume is not supported");
    return -1;
    }

  vtkNew<vtkMatrix4x4> objectToVolumeIJKTransformMatrix;
  vtkSlicerCropVolumeLogic::vtkInternal::GetObjectToVolumeIJK(roi, inputVolume, objectToVolumeIJKTransformMatrix);

  double roiXYZ[3] = { 0.0, 0.0, 0.0 };
  double roiRadius[3] = { 0.0, 0.0, 0.0 };
  vtkSlicerCropVolumeLogic::vtkInternal::GetROIXYZ(roi, roiXYZ);
  vtkSlicerCropVolumeLogic::vtkInternal::GetROIRadius(roi, roiRadius);

  const int numberOfCorners = 8;
  double volumeCorners_ROI[numberOfCorners][4] =
    {
    { roiXYZ[0] - roiRadius[0], roiXYZ[1] - roiRadius[1], roiXYZ[2] - roiRadius[2], 1. },
    { roiXYZ[0] + roiRadius[0], roiXYZ[1] - roiRadius[1], roiXYZ[2] - roiRadius[2], 1. },
    { roiXYZ[0] - roiRadius[0], roiXYZ[1] + roiRadius[1], roiXYZ[2] - roiRadius[2], 1. },
    { roiXYZ[0] + roiRadius[0], roiXYZ[1] + roiRadius[1], roiXYZ[2] - roiRadius[2], 1. },
    { roiXYZ[0] - roiRadius[0], roiXYZ[1] - roiRadius[1], roiXYZ[2] + roiRadius[2], 1. },
    { roiXYZ[0] + roiRadius[0], roiXYZ[1] - roiRadius[1], roiXYZ[2] + roiRadius[2], 1. },
    { roiXYZ[0] - roiRadius[0], roiXYZ[1] + roiRadius[1], roiXYZ[2] + roiRadius[2], 1. },
    { roiXYZ[0] + roiRadius[0], roiXYZ[1] + roiRadius[1], roiXYZ[2] + roiRadius[2], 1. },
    };

  // Get ROI extent in IJK coordinate system
  double outputExtentDouble[6] = { 0 };
  for (int cornerPointIndex = 0; cornerPointIndex < numberOfCorners; cornerPointIndex++)
    {
    double volumeCorner_IJK[4] = { 0, 0, 0, 1 };
    objectToVolumeIJKTransformMatrix->MultiplyPoint(volumeCorners_ROI[cornerPointIndex], volumeCorner_IJK);
    for (int axisIndex = 0; axisIndex < 3; ++axisIndex)
      {
      if (cornerPointIndex == 0 || volumeCorner_IJK[axisIndex] < outputExtentDouble[axisIndex * 2])
        {
        outputExtentDouble[axisIndex * 2] = volumeCorner_IJK[axisIndex];
        }
      if (cornerPointIndex == 0 || volumeCorner_IJK[axisIndex] > outputExtentDouble[axisIndex * 2 + 1])
        {
        outputExtentDouble[axisIndex * 2 + 1] = volumeCorner_IJK[axisIndex];
        }
      }
    }

  double tolerance = 0.001;
  for (int axisIndex = 0; axisIndex < 3; ++axisIndex)
    {
    outputExtent[axisIndex * 2] = int(ceil(outputExtentDouble[axisIndex * 2] + 0.5 - tolerance));
    outputExtent[axisIndex * 2 + 1] = int(floor(outputExtentDouble[axisIndex * 2 + 1] - 0.5 + tolerance));
    }
  if (limitToInputExtent)
    {
    int* inputExtent = inputVolume->GetImageData()->GetExtent();
    for (int axisIndex = 0; axisIndex < 3; ++axisIndex)
      {
      outputExtent[axisIndex * 2] = std::max(inputExtent[axisIndex * 2], outputExtent[axisIndex * 2]);
      outputExtent[axisIndex * 2 + 1] = std::min(inputExtent[axisIndex * 2 + 1], outputExtent[axisIndex * 2 + 1]);
      }
    }

  return true;
}

//----------------------------------------------------------------------------
int vtkSlicerCropVolumeLogic::CropVoxelBased(vtkMRMLDisplayableNode* roi,
  vtkMRMLVolumeNode* inputVolume, vtkMRMLVolumeNode* outputVolume, bool limitToInputExtent/*=true*/, double fillValue/*=0.0*/)
{
  if(!roi || !inputVolume || !outputVolume)
    {
    return -1;
    }
  if (!inputVolume->GetImageData())
    {
    vtkGenericWarningMacro("vtkSlicerCropVolumeLogic::CropVoxelBased: input image is empty");
    outputVolume->SetAndObserveImageData(nullptr);
    return 0;
    }

  int outputExtent[6] = { 0, -1, 0, -1, 0, -1 };
  if (!vtkSlicerCropVolumeLogic::GetVoxelBasedCropOutputExtent(roi, inputVolume, outputExtent, limitToInputExtent))
    {
    vtkGenericWarningMacro("vtkSlicerCropVolumeLogic::CropVoxelBased: failed to get output geometry");
    return -1;
    }

  vtkNew<vtkMatrix4x4> inputIJKToRAS;
  inputVolume->GetIJKToRASMatrix(inputIJKToRAS.GetPointer());

  vtkNew<vtkImageConstantPad> imageClip;
  imageClip->SetInputData(inputVolume->GetImageData());
  imageClip->SetOutputWholeExtent(outputExtent);
  imageClip->SetConstant(fillValue);
  imageClip->Update();

  int wasModified = outputVolume->StartModify();
  outputVolume->SetAndObserveImageData(imageClip->GetOutput());
  outputVolume->SetIJKToRASMatrix(inputIJKToRAS.GetPointer());
  outputVolume->ShiftImageDataExtentToZeroStart();
  outputVolume->SetAndObserveTransformNodeID(inputVolume->GetTransformNodeID());
  outputVolume->EndModify(wasModified);

  return 0;
}

//----------------------------------------------------------------------------
bool vtkSlicerCropVolumeLogic::GetInterpolatedCropOutputGeometry(vtkMRMLDisplayableNode* roi, vtkMRMLVolumeNode* inputVolume,
  bool isotropicResampling, double spacingScale, int outputExtent[6], double outputSpacing[3])
{
  if (!roi || !inputVolume)
    {
    return false;
    }

  double* inputSpacing = inputVolume->GetSpacing();
  if (isotropicResampling)
    {
    double minSpacing = std::min(std::min(inputSpacing[0], inputSpacing[1]), inputSpacing[2]);
    outputSpacing[0] = minSpacing * spacingScale;
    outputSpacing[1] = minSpacing * spacingScale;
    outputSpacing[2] = minSpacing * spacingScale;
    }
  else
    {
    int inputAxisIndexForROIAxis[3] = { 0, 1, 2 };
    // Find which image axis corresponds to each ROI axis, to get the correct spacing value for each ROI axis
    vtkNew<vtkGeneralTransform> volumeToObjectTransform;
    vtkNew<vtkTransform> volumeToObjectTransformLinear;
    vtkMRMLTransformNode::GetTransformBetweenNodes(inputVolume->GetParentTransformNode(),
      roi->GetParentTransformNode(), volumeToObjectTransform.GetPointer());
    if (vtkMRMLTransformNode::IsGeneralTransformLinear(volumeToObjectTransform.GetPointer(), volumeToObjectTransformLinear.GetPointer()))
      {
      // Transformation between input volume and ROI is linear, therefore we can find matching axes
      vtkNew<vtkMatrix4x4> objectToVolumeIJKTransformMatrix;
      vtkSlicerCropVolumeLogic::vtkInternal::GetObjectToVolumeIJK(roi, inputVolume, objectToVolumeIJKTransformMatrix);
      vtkNew<vtkMatrix4x4> volumeIJKToObject;
      vtkMatrix4x4::Invert(objectToVolumeIJKTransformMatrix, volumeIJKToObject);

      double scale[3] = { 1.0 };
      vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(volumeIJKToObject.GetPointer(), scale);
      // Find the volumeIJK axis that is best aligned with each ROI azis
      for (int roiAxisIndex = 0; roiAxisIndex < 3; roiAxisIndex++)
        {
        double largestComponentValue = 0;
        for (int volumeIJKAxisIndex = 0; volumeIJKAxisIndex < 3; volumeIJKAxisIndex++)
          {
          double currentComponentValue = fabs(volumeIJKToObject->GetElement(roiAxisIndex, volumeIJKAxisIndex));
          if (currentComponentValue > largestComponentValue)
            {
            largestComponentValue = currentComponentValue;
            inputAxisIndexForROIAxis[roiAxisIndex] = volumeIJKAxisIndex;
            }
          }
        }
      }
    outputSpacing[0] = inputSpacing[inputAxisIndexForROIAxis[0]] * spacingScale;
    outputSpacing[1] = inputSpacing[inputAxisIndexForROIAxis[1]] * spacingScale;
    outputSpacing[2] = inputSpacing[inputAxisIndexForROIAxis[2]] * spacingScale;
    }

  // prepare the resampling reference volume
  double roiRadius[3] = { 0.0, 0.0, 0.0 };
  vtkSlicerCropVolumeLogic::vtkInternal::GetROIRadius(roi, roiRadius);

  outputExtent[0] = outputExtent[2] = outputExtent[4] = 0;
  // add a bit of tolerance in deciding how many voxels the output should contain
  // to make sure that if the ROI size is set to match the image size exactly then we
  // output extent contains the whole image
  double tolerance = 0.001;
  outputExtent[1] = int(roiRadius[0] / outputSpacing[0] * 2. + tolerance) - 1;
  outputExtent[3] = int(roiRadius[1] / outputSpacing[1] * 2. + tolerance) - 1;
  outputExtent[5] = int(roiRadius[2] / outputSpacing[2] * 2. + tolerance) - 1;

  return true;
}

//----------------------------------------------------------------------------
int vtkSlicerCropVolumeLogic::CropInterpolated(vtkMRMLDisplayableNode* roi, vtkMRMLVolumeNode* inputVolume, vtkMRMLVolumeNode* outputVolume,
  bool isotropicResampling, double spacingScale, int interpolationMode, double fillValue)
{
  if (!roi || !inputVolume || !outputVolume)
    {
    return -1;
    }

  vtkSlicerCLIModuleLogic* resampleLogic =
    vtkSlicerCLIModuleLogic::SafeDownCast(this->GetModuleLogic("ResampleScalarVectorDWIVolume"));
  if (!resampleLogic)
    {
    vtkErrorMacro("CropVolume: resample logic is not set");
    return -3;
    }

  int outputExtent[6] = { 0, -1, 0, -1, 0, -1 };
  double outputSpacing[3] = { 0 };
  this->GetInterpolatedCropOutputGeometry(roi, inputVolume, isotropicResampling, spacingScale, outputExtent, outputSpacing);

  double roiXYZ[3] = { 0.0, 0.0, 0.0 };
  double roiRadius[3] = { 0.0, 0.0, 0.0 };
  vtkSlicerCropVolumeLogic::vtkInternal::GetROIXYZ(roi, roiXYZ);
  vtkSlicerCropVolumeLogic::vtkInternal::GetROIRadius(roi, roiRadius);

  vtkNew<vtkMatrix4x4> outputIJKToRAS;
  outputIJKToRAS->SetElement(0, 0, outputSpacing[0]);
  outputIJKToRAS->SetElement(1, 1, outputSpacing[1]);
  outputIJKToRAS->SetElement(2, 2, outputSpacing[2]);
  outputIJKToRAS->SetElement(0, 3, roiXYZ[0] - roiRadius[0]);
  outputIJKToRAS->SetElement(1, 3, roiXYZ[1] - roiRadius[1]);
  outputIJKToRAS->SetElement(2, 3, roiXYZ[2] - roiRadius[2]);

  // account for the ROI parent transform, if present
  vtkMRMLTransformNode *roiTransform = roi->GetParentTransformNode();
  vtkMRMLTransformNode *outputTransform = outputVolume->GetParentTransformNode();
  if (roiTransform && !roiTransform->IsTransformToWorldLinear())
    {
    // We can only display a If ROI is transformed with a warping transform then we ignore the transformation because non-linear
    // transform of ROI node is not supported.
    vtkErrorMacro("vtkSlicerCropVolumeLogic::CropInterpolated: ROI is under a non-linear transform");
    return -5;
    }
  if (outputTransform && !outputTransform->IsTransformToWorldLinear())
    {
    // The resample module can only create a rectangular output.
    vtkErrorMacro("vtkSlicerCropVolumeLogic::CropInterpolated: output volume is under a non-linear transform");
    return -6;
    }

  vtkNew<vtkMatrix4x4> roiMatrix;
  vtkSlicerCropVolumeLogic::vtkInternal::GetMatrixTransformFromObjectToNode(roi, outputVolume, roiMatrix);
  outputIJKToRAS->Multiply4x4(roiMatrix.GetPointer(), outputIJKToRAS.GetPointer(),
    outputIJKToRAS.GetPointer());

  vtkNew<vtkMatrix4x4> rasToLPS;
  rasToLPS->SetElement(0, 0, -1);
  rasToLPS->SetElement(1, 1, -1);
  vtkNew<vtkMatrix4x4> outputIJKToLPS;
  vtkMatrix4x4::Multiply4x4(rasToLPS.GetPointer(), outputIJKToRAS.GetPointer(), outputIJKToLPS.GetPointer());

  // contains axis directions, in unconventional indexing (column, row)
  // so that it can be conveniently normalized
  double outputDirectionColRow[3][3] = {{ 0 }};
  for (int column = 0; column < 3; column++)
    {
    for (int row = 0; row < 3; row++)
      {
      outputDirectionColRow[column][row] = outputIJKToLPS->GetElement(row, column);
      }
    outputSpacing[column] = vtkMath::Normalize(outputDirectionColRow[column]);
    }

  vtkMRMLCommandLineModuleNode* cmdNode = resampleLogic->CreateNodeInScene();
  if (cmdNode == nullptr)
    {
    vtkErrorMacro("CropVolume: failed to create resample node");
    return -4;
    }

  cmdNode->SetParameterAsString("inputVolume", inputVolume->GetID());
  cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());

  std::stringstream sizeStream;
  sizeStream << (outputExtent[1] - outputExtent[0] + 1)  << ","
    << (outputExtent[3] - outputExtent[2] + 1) << ","
    << (outputExtent[5] - outputExtent[4] + 1);
  cmdNode->SetParameterAsString("outputImageSize", sizeStream.str());

  // Center the output image in the ROI. For that, compute the size difference between
  // the ROI and the output image.
  double sizeDifference_IJK[3] =
    {
    roiRadius[0] * 2 / outputSpacing[0] - (outputExtent[1] - outputExtent[0] + 1),
    roiRadius[1] * 2 / outputSpacing[1] - (outputExtent[3] - outputExtent[2] + 1),
    roiRadius[2] * 2 / outputSpacing[2] - (outputExtent[5] - outputExtent[4] + 1)
    };
  // Origin is in the voxel's center. Shift the origin by half voxel
  // to have the ROI edge at the output image voxel edge.
  double outputOrigin_IJK[4] =
    {
    0.5 + sizeDifference_IJK[0] / 2,
    0.5 + sizeDifference_IJK[1] / 2,
    0.5 + sizeDifference_IJK[2] / 2,
    1.0
    };
  double outputOrigin_RAS[4] = { 0.0, 0.0, 0.0, 1.0 };
  outputIJKToRAS->MultiplyPoint(outputOrigin_IJK, outputOrigin_RAS);

  vtkNew<vtkMRMLMarkupsFiducialNode> originMarkupNode;
  // Markups are transformed from RAS to LPS by the CLI infrastructure, so we pass them in RAS
  originMarkupNode->AddControlPoint(outputOrigin_RAS);
  this->GetMRMLScene()->AddNode(originMarkupNode.GetPointer());
  cmdNode->SetParameterAsString("outputImageOrigin", originMarkupNode->GetID());

  std::stringstream spacingStream;
  spacingStream << std::setprecision(15) << outputSpacing[0] << "," << outputSpacing[1] << "," << outputSpacing[2];
  cmdNode->SetParameterAsString("outputImageSpacing", spacingStream.str());

  std::stringstream directionStream;
  directionStream << std::setprecision(15);
  for (int row = 0; row < 3; row++)
    {
    for (int column = 0; column < 3; column++)
      {
      if (row > 0 || column > 0)
        {
        directionStream << ",";
        }
      directionStream << outputDirectionColRow[column][row];
      }
    }

  cmdNode->SetParameterAsString("directionMatrix", directionStream.str());

  vtkMRMLTransformNode* inputToRASTransformNodeToRemove = nullptr;
  if (inputVolume->GetParentTransformNode() != nullptr)
    {
    vtkNew<vtkGeneralTransform> inputToRASTransform;
    inputVolume->GetParentTransformNode()->GetTransformToNode(outputVolume->GetParentTransformNode(), inputToRASTransform.GetPointer());
    vtkNew<vtkMRMLTransformNode> inputToRASTransformNode;
    inputToRASTransformNode->SetAndObserveTransformToParent(inputToRASTransform.GetPointer());
    this->GetMRMLScene()->AddNode(inputToRASTransformNode.GetPointer());
    inputToRASTransformNodeToRemove = inputToRASTransformNode.GetPointer();
    cmdNode->SetParameterAsString("transformationFile", inputToRASTransformNode->GetID());
    }

  std::string interp = "linear";
  switch (interpolationMode)
    {
    case vtkMRMLCropVolumeParametersNode::InterpolationNearestNeighbor:
      interp = "nn";
      break;
    case vtkMRMLCropVolumeParametersNode::InterpolationLinear:
      interp = "linear";
      break;
    case vtkMRMLCropVolumeParametersNode::InterpolationWindowedSinc:
      interp = "ws";
      break;
    case vtkMRMLCropVolumeParametersNode::InterpolationBSpline:
      interp = "bs";
      break;
    }

  cmdNode->SetParameterAsString("interpolationType", interp.c_str());

  cmdNode->SetParameterAsDouble("defaultPixelValue", fillValue);

  resampleLogic->ApplyAndWait(cmdNode, false);

  this->GetMRMLScene()->RemoveNode(cmdNode);
  this->GetMRMLScene()->RemoveNode(originMarkupNode.GetPointer());
  if (inputToRASTransformNodeToRemove != nullptr)
    {
    this->GetMRMLScene()->RemoveNode(inputToRASTransformNodeToRemove);
    }

  // success
  return 0;
}

//-----------------------------------------------------------------------------
bool vtkSlicerCropVolumeLogic::FitROIToInputVolume(vtkMRMLCropVolumeParametersNode* parametersNode)
{
  if (!parametersNode)
    {
    return false;
    }
  vtkMRMLDisplayableNode* roiNode = parametersNode->GetROINode();
  vtkMRMLVolumeNode* volumeNode = parametersNode->GetInputVolumeNode();
  if (!roiNode || !volumeNode)
    {
    return false;
    }

  vtkMRMLTransformNode* roiTransform = roiNode->GetParentTransformNode();
  if (roiTransform && !roiTransform->IsTransformToWorldLinear())
    {
    roiTransform = nullptr;
    roiNode->SetAndObserveTransformNodeID(nullptr);
    parametersNode->DeleteROIAlignmentTransformNode();
    }

  vtkNew<vtkMatrix4x4> worldToObject;
  vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(nullptr, roiTransform, worldToObject.GetPointer());

  vtkMRMLMarkupsROINode* markupsROI = vtkMRMLMarkupsROINode::SafeDownCast(roiNode);
  if (markupsROI)
    {
    vtkNew<vtkMatrix4x4> localToObjectMatrix;
    vtkMatrix4x4::Invert(markupsROI->GetObjectToNodeMatrix(), localToObjectMatrix);
    vtkMatrix4x4::Multiply4x4(localToObjectMatrix, worldToObject, worldToObject);
    }

  double volumeBounds_ROI[6] = { 0 }; // volume bounds in ROI's coordinate system
  volumeNode->GetSliceBounds(volumeBounds_ROI, worldToObject.GetPointer());

  double roiCenter[3] = { 0 };
  double roiRadius[3] = { 0 };
  for (int i = 0; i < 3; i++)
    {
    roiCenter[i] = (volumeBounds_ROI[i * 2 + 1] + volumeBounds_ROI[i * 2]) / 2;
    roiRadius[i] = (volumeBounds_ROI[i * 2 + 1] - volumeBounds_ROI[i * 2]) / 2;
    }

  if (markupsROI)
    {
    // ROI center is specified in the local coordinate system
    double roiCenter_ROI[4] = { roiCenter[0], roiCenter[1], roiCenter[2], 1.0 };
    double roiCenter_Local[4] = { 0.0, 0.0, 0.0, 1.0 };
    markupsROI->GetObjectToNodeMatrix()->MultiplyPoint(roiCenter_ROI, roiCenter_Local);
    roiCenter[0] = roiCenter_Local[0];
    roiCenter[1] = roiCenter_Local[1];
    roiCenter[2] = roiCenter_Local[2];
    }

  vtkSlicerCropVolumeLogic::vtkInternal::SetROIXYZ(roiNode, roiCenter);
  vtkSlicerCropVolumeLogic::vtkInternal::SetROIRadius(roiNode, roiRadius);

  return true;
}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::SnapROIToVoxelGrid(vtkMRMLCropVolumeParametersNode* parametersNode)
{
  if (!parametersNode || !parametersNode->GetInputVolumeNode() || !parametersNode->GetROINode())
    {
    // no misalignment (not enough nodes to be misaligned)
    return;
    }

  // Is it already aligned?
  if (IsROIAlignedWithInputVolume(parametersNode))
    {
    // already aligned, nothing to do
    return;
    }

  vtkMRMLMarkupsROINode* markupsROI = vtkMRMLMarkupsROINode::SafeDownCast(parametersNode->GetROINode());

  double originalBounds_World[6] = { 0, -1, 0, -1, 0, -1 };
  parametersNode->GetROINode()->GetRASBounds(originalBounds_World);

  // If we don't transform it, is it aligned? (only checked for non-markups ROI, because we don't need parent transform for markups ROI)
  if (!markupsROI && parametersNode->GetROINode()->GetParentTransformNode() != nullptr)
    {
    parametersNode->GetROINode()->SetAndObserveTransformNodeID(nullptr);
    if (IsROIAlignedWithInputVolume(parametersNode))
      {
      // ROI is aligned if it's not transformed, no need for ROI alignment transform
      parametersNode->DeleteROIAlignmentTransformNode();
      // Update ROI to approximately match original region
      double xyz[3] =
        {
        (originalBounds_World[1] + originalBounds_World[0]) / 2.0,
        (originalBounds_World[3] + originalBounds_World[2]) / 2.0,
        (originalBounds_World[5] + originalBounds_World[4]) / 2.0
        };
      double radius[3] =
        {
        (originalBounds_World[1] - originalBounds_World[0]) / 2.0,
        (originalBounds_World[3] - originalBounds_World[2]) / 2.0,
        (originalBounds_World[5] - originalBounds_World[4]) / 2.0
        };
      vtkSlicerCropVolumeLogic::vtkInternal::SetROIXYZ(parametersNode->GetROINode(), xyz);
      vtkSlicerCropVolumeLogic::vtkInternal::SetROIRadius(parametersNode->GetROINode(), radius);
      return;
      }
    }

  // It's a non-trivial rotation, align the ROI axes
  vtkNew<vtkMatrix4x4> worldToObjectTransformMatrix;
  if (markupsROI)
    {
    // Keep current transforms, use ObjectToNode transform to align
    vtkNew<vtkMatrix4x4> volumeRasToObjectLocal;
    vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(parametersNode->GetInputVolumeNode()->GetParentTransformNode(),
      markupsROI->GetParentTransformNode(), volumeRasToObjectLocal);
    vtkNew<vtkMatrix4x4> volumeIJKToRAS;
    parametersNode->GetInputVolumeNode()->GetIJKToRASMatrix(volumeIJKToRAS.GetPointer());
    vtkNew<vtkMatrix4x4> volumeIJKToObjectLocal;
    vtkMatrix4x4::Multiply4x4(volumeRasToObjectLocal, volumeIJKToRAS.GetPointer(), volumeIJKToObjectLocal);
    double scale[3] = { 1.0 };
    vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(volumeIJKToObjectLocal, scale);

    MRMLNodeModifyBlocker blocker(markupsROI);
    markupsROI->GetObjectToNodeMatrix()->DeepCopy(volumeIJKToObjectLocal);
    markupsROI->UpdateControlPointsFromROI();
    markupsROI->Modified();

    vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(nullptr, markupsROI->GetParentTransformNode(), worldToObjectTransformMatrix);
    vtkNew<vtkMatrix4x4> localToObjectMatrix;
    vtkMatrix4x4::Invert(markupsROI->GetObjectToNodeMatrix(), localToObjectMatrix);
    vtkMatrix4x4::Multiply4x4(localToObjectMatrix, worldToObjectTransformMatrix, worldToObjectTransformMatrix);
    }
  else
    {
    // Use the ROI alignment transform node to align
    vtkNew<vtkMatrix4x4> volumeRasToWorld;
    vtkMRMLTransformNode::GetMatrixTransformBetweenNodes(parametersNode->GetInputVolumeNode()->GetParentTransformNode(),
      nullptr, volumeRasToWorld.GetPointer());
    vtkNew<vtkMatrix4x4> volumeIJKToRAS;
    parametersNode->GetInputVolumeNode()->GetIJKToRASMatrix(volumeIJKToRAS.GetPointer());
    vtkNew<vtkMatrix4x4> volumeIJKToWorld;
    vtkMatrix4x4::Multiply4x4(volumeRasToWorld.GetPointer(), volumeIJKToRAS.GetPointer(), volumeIJKToWorld.GetPointer());
    double scale[3] = { 1.0 };
    vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(volumeIJKToWorld.GetPointer(), scale);

    // Apply transform to ROI alignment transform
    if (!parametersNode->GetROIAlignmentTransformNode())
      {
      vtkNew<vtkMRMLTransformNode> roiTransformNode;
      roiTransformNode->SetName("Crop volume ROI alignment");
      parametersNode->GetScene()->AddNode(roiTransformNode.GetPointer());
      parametersNode->SetROIAlignmentTransformNodeID(roiTransformNode->GetID());
      }
    parametersNode->GetROIAlignmentTransformNode()->SetAndObserveTransformNodeID(nullptr);
    parametersNode->GetROIAlignmentTransformNode()->SetMatrixTransformToParent(volumeIJKToWorld.GetPointer());
    parametersNode->GetROINode()->SetAndObserveTransformNodeID(parametersNode->GetROIAlignmentTransformNode()->GetID());

    parametersNode->GetROIAlignmentTransformNode()->GetMatrixTransformFromWorld(worldToObjectTransformMatrix.GetPointer());
    }

  // Update ROI to approximately match original region
  const int numberOfCornerPoints = 8;
  double cornerPoints_World[numberOfCornerPoints][4] =
    {
    { originalBounds_World[0], originalBounds_World[2], originalBounds_World[4], 1 },
    { originalBounds_World[0], originalBounds_World[2], originalBounds_World[5], 1 },
    { originalBounds_World[0], originalBounds_World[3], originalBounds_World[4], 1 },
    { originalBounds_World[0], originalBounds_World[3], originalBounds_World[5], 1 },
    { originalBounds_World[1], originalBounds_World[2], originalBounds_World[4], 1 },
    { originalBounds_World[1], originalBounds_World[2], originalBounds_World[5], 1 },
    { originalBounds_World[1], originalBounds_World[3], originalBounds_World[4], 1 },
    { originalBounds_World[1], originalBounds_World[3], originalBounds_World[5], 1 }
    };
  vtkBoundingBox boundingBox_ROI;
  for (int i = 0; i < numberOfCornerPoints; i++)
    {
    double* cornerPoint_ROI = worldToObjectTransformMatrix->MultiplyDoublePoint(cornerPoints_World[i]);
    boundingBox_ROI.AddPoint(cornerPoint_ROI);
    }
  double roiCenter[3] = { 0 };
  boundingBox_ROI.GetCenter(roiCenter);

  if (markupsROI)
    {
    // ROI center is specified in the local coordinate system
    double roiCenter_ROI[4] = { roiCenter[0], roiCenter[1], roiCenter[2], 1.0 };
    double roiCenter_Local[4] = { 0.0, 0.0, 0.0, 1.0 };
    markupsROI->GetObjectToNodeMatrix()->MultiplyPoint(roiCenter_ROI, roiCenter_Local);
    roiCenter[0] = roiCenter_Local[0];
    roiCenter[1] = roiCenter_Local[1];
    roiCenter[2] = roiCenter_Local[2];
    }


  vtkSlicerCropVolumeLogic::vtkInternal::SetROIXYZ(parametersNode->GetROINode(), roiCenter);
  double diameters_ROI[3] = { 0 };
  boundingBox_ROI.GetLengths(diameters_ROI);
  double radius_ROI[3] = { diameters_ROI[0] / 2, diameters_ROI[1] / 2, diameters_ROI[2] / 2 };
  vtkSlicerCropVolumeLogic::vtkInternal::SetROIRadius(parametersNode->GetROINode(), radius_ROI);
}

//----------------------------------------------------------------------------
bool vtkSlicerCropVolumeLogic::IsROIAlignedWithInputVolume(vtkMRMLCropVolumeParametersNode* parametersNode)
{
  if (!parametersNode || !parametersNode->GetInputVolumeNode() || !parametersNode->GetROINode())
    {
    // no misalignment (not enough nodes to be misaligned)
    return true;
    }

  if (parametersNode->GetInputVolumeNode()->GetParentTransformNode()
    && !parametersNode->GetInputVolumeNode()->GetParentTransformNode()->IsTransformToWorldLinear())
    {
    // no misalignment, as if input volume is under a non-linear transform then we cannot align a ROI
    return true;
    }

  if (parametersNode->GetROINode()->GetParentTransformNode()
    && !parametersNode->GetROINode()->GetParentTransformNode()->IsTransformToWorldLinear())
    {
    // misaligned, as ROI node is under non-linear transform
    return false;
    }

  vtkNew<vtkMatrix4x4> objectToVolumeIJKTransformMatrix;
  vtkSlicerCropVolumeLogic::vtkInternal::GetObjectToVolumeIJK(parametersNode->GetROINode(), parametersNode->GetInputVolumeNode(),
    objectToVolumeIJKTransformMatrix);
  vtkNew<vtkMatrix4x4> volumeIJKToObject;
  vtkMatrix4x4::Invert(objectToVolumeIJKTransformMatrix, volumeIJKToObject);

  double scale[3] = { 1.0 };
  vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(volumeIJKToObject.GetPointer(), scale);

  double tolerance = 0.001;
  for (int row = 0; row < 3; row++)
    {
    for (int column = 0; column < 3; column++)
      {
        double elemAbs = fabs(volumeIJKToObject->GetElement(row, column));
      if (elemAbs > tolerance && elemAbs < 1 - tolerance)
        {
        // axes are neither orthogonal nor parallel
        return false;
        }
      }
    }
  return true;
}
