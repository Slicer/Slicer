/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE and the Applied Cancer Research Unit program
  of Cancer Care Ontario with funds provided by the Ontario Ministry of Health and
  Long-Term Care

==============================================================================*/

// Segmentations includes
#include "vtkSlicerSegmentationGeometryLogic.h"

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkSegmentationConverter.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkCalculateOversamplingFactor.h"

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLNodePropertyMacros.h"


// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkAddonMathUtilities.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSegmentationGeometryLogic);

//----------------------------------------------------------------------------
vtkSlicerSegmentationGeometryLogic::vtkSlicerSegmentationGeometryLogic()
: UserSpacing{ 1.0, 1.0, 1.0 }
, InputAxisIndexForSourceAxis{ 0, 1, 2 }
, SourceAxisIndexForInputAxis{ 0, 1, 2 }

{
  this->InputSegmentationNode = 0;
  this->SourceGeometryNode = 0;
  this->OversamplingFactor = 1.0;
  this->IsotropicSpacing = false;

  this->OutputGeometryImageData = vtkOrientedImageData::New();
}

//----------------------------------------------------------------------------
vtkSlicerSegmentationGeometryLogic::~vtkSlicerSegmentationGeometryLogic()
{
  this->SetInputSegmentationNode(NULL);

  if (this->OutputGeometryImageData)
    {
    this->OutputGeometryImageData->Delete();
    this->OutputGeometryImageData = 0;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerSegmentationGeometryLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(IsotropicSpacing);
  vtkMRMLPrintFloatMacro(OversamplingFactor);
  vtkMRMLPrintVectorMacro(UserSpacing, double, 3);
  vtkMRMLPrintVectorMacro(InputAxisIndexForSourceAxis, int, 3);
  vtkMRMLPrintVectorMacro(SourceAxisIndexForInputAxis, int, 3);
  vtkMRMLPrintEndMacro();

  if (this->InputSegmentationNode)
    {
    os << indent << "InputSegmentationNode: " ;
    this->InputSegmentationNode->PrintSelf(os, indent.GetNextIndent());
    }

  if (this->SourceGeometryNode)
    {
    os << indent << "SourceGeometryNode: " ;
    this->SourceGeometryNode->PrintSelf(os, indent.GetNextIndent());
    }

  os << indent << "OutputGeometryImageData: " ;
  this->OutputGeometryImageData->PrintSelf(os, indent.GetNextIndent());
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentationGeometryLogic::SetSourceGeometryNode(vtkMRMLDisplayableNode* node)
{
  vtkSetObjectBodyMacro(SourceGeometryNode, vtkMRMLDisplayableNode, node);

  // Calculate axis permutation if necessary
  if (this->SourceGeometryNode && this->InputSegmentationNode)
    {
    this->ComputeSourceAxisIndexForInputAxis();
    }
}

//-----------------------------------------------------------------------------
std::string vtkSlicerSegmentationGeometryLogic::CalculateOutputGeometry()
{
  // Reset geometry
  this->ResetGeometryImageData();

  if (!this->InputSegmentationNode)
    {
    return "No input segmentation specified";
    }

  // Determine source type
  vtkMRMLScalarVolumeNode* sourceVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->SourceGeometryNode);

  vtkMRMLSegmentationNode* sourceSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->SourceGeometryNode);
  vtkSmartPointer<vtkOrientedImageData> sourceBinaryLabelmap;
  if (this->IsSourceSegmentationWithBinaryLabelmapMaster())
    {
    //TODO: Fractional labelmaps cannot be used yet as source, as DetermineCommonLabelmapGeometry only supports binary labelmaps
    sourceBinaryLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
    std::string geometryString = sourceSegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry();
    vtkSegmentationConverter::DeserializeImageGeometry(geometryString, sourceBinaryLabelmap, false);
    }

  //
  // Source type is volume (volume node or segmentation with non-empty binary labelmap master)
  //
  if (sourceVolumeNode || sourceBinaryLabelmap.GetPointer())
    {
    // Set initial geometry from source volume
    vtkNew<vtkMatrix4x4> labelmapIJKToSegmentationMatrix;
    if (sourceVolumeNode && sourceVolumeNode->GetImageData())
      {
      sourceVolumeNode->GetIJKToRASMatrix(labelmapIJKToSegmentationMatrix.GetPointer());
      this->OutputGeometryImageData->SetExtent(sourceVolumeNode->GetImageData()->GetExtent());
      }
    else if (sourceBinaryLabelmap)
      {
      sourceBinaryLabelmap->GetImageToWorldMatrix(labelmapIJKToSegmentationMatrix.GetPointer());
      this->OutputGeometryImageData->SetExtent(sourceBinaryLabelmap->GetExtent());
      }
    else
      {
      return "Invalid source volume";
      }

    // Apply parent transforms
    vtkNew<vtkGeneralTransform> inputSegmentationToSourceTransform;
    vtkNew<vtkTransform> inputSegmentationToSourceTransformLinear;
    vtkMRMLTransformNode::GetTransformBetweenNodes(this->InputSegmentationNode->GetParentTransformNode(),
      this->SourceGeometryNode->GetParentTransformNode(), inputSegmentationToSourceTransform.GetPointer());
    if (vtkMRMLTransformNode::IsGeneralTransformLinear(inputSegmentationToSourceTransform.GetPointer(), inputSegmentationToSourceTransformLinear.GetPointer()))
      {
      // Transformation between segmentation and source is linear
      vtkNew<vtkMatrix4x4> labelmapIJKToSourceMatrix;
      vtkNew<vtkMatrix4x4> inputSegmentationToSourceMatrix;
      inputSegmentationToSourceTransformLinear->GetMatrix(inputSegmentationToSourceMatrix.GetPointer());
      vtkMatrix4x4::Multiply4x4(inputSegmentationToSourceMatrix.GetPointer(), labelmapIJKToSegmentationMatrix.GetPointer(), labelmapIJKToSourceMatrix.GetPointer());
      this->OutputGeometryImageData->SetImageToWorldMatrix(labelmapIJKToSourceMatrix.GetPointer());
      }
    else
      {
      vtkWarningMacro("CalculateOutputGeometry: Ignoring parent transforms because non-linear components have been found");
      this->OutputGeometryImageData->SetImageToWorldMatrix(labelmapIJKToSegmentationMatrix.GetPointer());
      }

    // Apply optional settings
    if (this->IsotropicSpacing)
      {
      double* spacing = this->OutputGeometryImageData->GetSpacing();
      double minSpacing = this->OutputGeometryImageData->GetMinSpacing();

      int newExtent[6] = {0,-1,0,-1,0,-1};
      int extent[6] = {0,-1,0,-1,0,-1};
      this->OutputGeometryImageData->GetExtent(extent);
      for (unsigned int axis=0; axis<3; ++axis)
        {
        double oversamplingForAxis = spacing[this->SourceAxisIndexForInputAxis[axis]] / minSpacing;
        int dimension = extent[axis*2+1] - extent[axis*2] + 1;
        int extentMin = static_cast<int>(ceil(oversamplingForAxis * extent[axis * 2]));
        int extentMax = extentMin + static_cast<int>(floor(oversamplingForAxis*dimension)) - 1;
        newExtent[axis*2] = extentMin;
        newExtent[axis*2+1] = extentMax;
        }
      this->OutputGeometryImageData->SetSpacing(minSpacing, minSpacing, minSpacing);
      this->OutputGeometryImageData->SetExtent(newExtent);
      }

    if (this->OversamplingFactor != 1.0)
      {
      vtkCalculateOversamplingFactor::ApplyOversamplingOnImageGeometry(this->OutputGeometryImageData, this->OversamplingFactor);
      }
    }
  //
  // Source is ROI, model, or segmentation with poly data master
  //
  else
    {
    // Get initial spacing
    double outputSpacing[3] = { 0 };
    outputSpacing[0] = this->UserSpacing[0];
    outputSpacing[1] = this->UserSpacing[1];
    outputSpacing[2] = this->UserSpacing[2];
    if (outputSpacing[0] <= 0 || outputSpacing[1] <= 0 || outputSpacing[2] <= 0)
      {
      return "All spacing values must be larger than 0";
      }

    // Get source bounds
    double sourceBounds[6] = {0, -1, 0, -1, 0, -1};
    this->SourceGeometryNode->GetBounds(sourceBounds);

    // Determine transform between source node and input segmentation
    vtkNew<vtkGeneralTransform> segmentationToSourceTransform;
    vtkNew<vtkTransform> segmentationToSourceTransformLinear;
    vtkMRMLTransformNode::GetTransformBetweenNodes(this->InputSegmentationNode->GetParentTransformNode(),
      this->SourceGeometryNode->GetParentTransformNode(), segmentationToSourceTransform.GetPointer());
    vtkNew<vtkMatrix4x4> segmentationToSourceMatrix;
    if (vtkMRMLTransformNode::IsGeneralTransformLinear(segmentationToSourceTransform.GetPointer(), segmentationToSourceTransformLinear.GetPointer()))
      {
      // Transformation between segmentation and source is linear
      segmentationToSourceTransformLinear->GetMatrix(segmentationToSourceMatrix.GetPointer());
      }
    else
      {
      vtkWarningMacro("CalculateOutputGeometry: Ignoring parent transforms because non-linear components have been found");
      segmentationToSourceMatrix->Identity();
      }

    // If input segmentation has non-empty binary labelmap master that need to be resampled,
    // then match the axes of the labelmap to the axes of the transformed source node, and
    // determine directions and spacing according to that.
    // In this case, origin is also given by the input segmentation's labelmap
    if (this->InputSegmentationCanBeResampled())
      {
      vtkNew<vtkOrientedImageData> inputBinaryLabelmap;
      std::string geometryString = this->InputSegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry();
      vtkSegmentationConverter::DeserializeImageGeometry(geometryString, inputBinaryLabelmap.GetPointer(), false);

      // Find which labelmap axis corresponds to each source axis, to get the correct spacing value for each source axis
      vtkNew<vtkMatrix4x4> inputLabelmapIJKToInputSegmentation;
      inputBinaryLabelmap->GetImageToWorldMatrix(inputLabelmapIJKToInputSegmentation.GetPointer());
      vtkNew<vtkMatrix4x4> inputLabelmapIJKToSource;
      vtkMatrix4x4::Multiply4x4(segmentationToSourceMatrix.GetPointer(), inputLabelmapIJKToInputSegmentation.GetPointer(), inputLabelmapIJKToSource.GetPointer());
      this->OutputGeometryImageData->SetImageToWorldMatrix(inputLabelmapIJKToSource.GetPointer());

      outputSpacing[0] = this->UserSpacing[this->InputAxisIndexForSourceAxis[0]];
      outputSpacing[1] = this->UserSpacing[this->InputAxisIndexForSourceAxis[1]];
      outputSpacing[2] = this->UserSpacing[this->InputAxisIndexForSourceAxis[2]];
      this->OutputGeometryImageData->SetSpacing(outputSpacing);
      }
    else
      {
      // Directions according to segmentation to source transformation
      this->OutputGeometryImageData->SetImageToWorldMatrix(segmentationToSourceMatrix.GetPointer());
      // Spacing as specified on the UI
      this->OutputGeometryImageData->SetSpacing(outputSpacing);
      }

    // Calculate origin according to the bounds
    const double origin_Source[4] = { sourceBounds[0], sourceBounds[2], sourceBounds[4], 1.0 };
    vtkNew<vtkMatrix4x4> sourceToSegmentationMatrix;
    sourceToSegmentationMatrix->DeepCopy(segmentationToSourceMatrix.GetPointer());
    sourceToSegmentationMatrix->Invert();
    double origin_Segmentation[4] = { 0.0, 0.0, 0.0, 0.0 };
    sourceToSegmentationMatrix->MultiplyPoint(origin_Source, origin_Segmentation);

    vtkNew<vtkMatrix4x4> labelmapIJKToSegmentationMatrix;
    this->OutputGeometryImageData->GetImageToWorldMatrix(labelmapIJKToSegmentationMatrix.GetPointer());
    const double voxelCenter_IJK[4] = { 0.5, 0.5, 0.5, 1.0 };
    vtkNew<vtkMatrix4x4> labelmapIJKToSegmentationDirectionMatrix;
    labelmapIJKToSegmentationDirectionMatrix->DeepCopy(labelmapIJKToSegmentationMatrix.GetPointer());
    labelmapIJKToSegmentationDirectionMatrix->SetElement(0,3,0.0);
    labelmapIJKToSegmentationDirectionMatrix->SetElement(1,3,0.0);
    labelmapIJKToSegmentationDirectionMatrix->SetElement(2,3,0.0);
    double voxelCenter_Segmentation[4] = { 0.0, 0.0, 0.0, 0.0 };
    labelmapIJKToSegmentationDirectionMatrix->MultiplyPoint(voxelCenter_IJK, voxelCenter_Segmentation);
    origin_Segmentation[0] = origin_Segmentation[0] + voxelCenter_Segmentation[0];
    origin_Segmentation[1] = origin_Segmentation[1] + voxelCenter_Segmentation[1];
    origin_Segmentation[2] = origin_Segmentation[2] + voxelCenter_Segmentation[2];
    this->OutputGeometryImageData->SetOrigin(origin_Segmentation);

    // Calculate extent
    int outputExtent[6] = { 0, -1, 0, -1, 0, -1 };
    const double farCorner_Source[4] = { sourceBounds[1], sourceBounds[3], sourceBounds[5], 1.0 };
    double farCorner_Segmentation[4] = { 0.0, 0.0, 0.0, 0.0 };
    sourceToSegmentationMatrix->MultiplyPoint(farCorner_Source, farCorner_Segmentation);
    // Add a bit of tolerance in deciding how many voxels the output should contain
    // to make sure that if the ROI size is set to match the image size exactly then we
    // output extent contains the whole image
    double tolerance = 0.001;
    outputExtent[1] = ceil((farCorner_Segmentation[0] - origin_Segmentation[0]) / outputSpacing[0] + tolerance) - 1;
    outputExtent[3] = ceil((farCorner_Segmentation[1] - origin_Segmentation[1]) / outputSpacing[1] + tolerance) - 1;
    outputExtent[5] = ceil((farCorner_Segmentation[2] - origin_Segmentation[2]) / outputSpacing[2] + tolerance) - 1;
    this->OutputGeometryImageData->SetExtent(outputExtent);
    }

    return "";
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentationGeometryLogic::ResetGeometryImageData()
{
  vtkNew<vtkMatrix4x4> identityMatrix;
  identityMatrix->Identity();
  this->OutputGeometryImageData->SetImageToWorldMatrix(identityMatrix.GetPointer());
  this->OutputGeometryImageData->SetDimensions(0, 0, 0);
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationGeometryLogic::IsSourceSegmentationWithBinaryLabelmapMaster()
{
  vtkMRMLSegmentationNode* sourceSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->SourceGeometryNode);
  std::string binaryLabelmapName = vtkSegmentationConverter::GetBinaryLabelmapRepresentationName();
  vtkSmartPointer<vtkOrientedImageData> sourceBinaryLabelmap;
  if ( sourceSegmentationNode
    && sourceSegmentationNode->GetSegmentation()
    && sourceSegmentationNode->GetSegmentation()->GetNumberOfSegments() > 0
    && sourceSegmentationNode->GetSegmentation()->ContainsRepresentation(binaryLabelmapName)
    && sourceSegmentationNode->GetSegmentation()->GetMasterRepresentationName() == binaryLabelmapName )
    {
    return true;
    }

  return false;
}

//-----------------------------------------------------------------------------
bool vtkSlicerSegmentationGeometryLogic::InputSegmentationCanBeResampled()
{
  if ( this->InputSegmentationNode != 0
    &&  this->InputSegmentationNode->GetSegmentation()->GetNumberOfSegments() > 0
    && this->InputSegmentationNode->GetSegmentation()->ContainsRepresentation(
        vtkSegmentationConverter::GetBinaryLabelmapRepresentationName())
    && this->InputSegmentationNode->GetSegmentation()->GetMasterRepresentationName()
        == vtkSegmentationConverter::GetBinaryLabelmapRepresentationName() )
    {
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkSlicerSegmentationGeometryLogic::ComputeSourceAxisIndexForInputAxis()
{
  // Initialize source axis permutation and inverse
  this->InputAxisIndexForSourceAxis[0] = 0;
  this->InputAxisIndexForSourceAxis[1] = 1;
  this->InputAxisIndexForSourceAxis[2] = 2;
  this->SourceAxisIndexForInputAxis[0] = 0;
  this->SourceAxisIndexForInputAxis[1] = 1;
  this->SourceAxisIndexForInputAxis[2] = 2;

  vtkMRMLTransformableNode* transformableSourceNode = vtkMRMLTransformableNode::SafeDownCast(this->SourceGeometryNode);
  if (!transformableSourceNode || !this->InputSegmentationNode)
    {
    vtkErrorMacro("ComputeSourceAxisIndexForInputAxis: Invalid input nodes");
    return;
    }
  if (transformableSourceNode->GetScene() != this->InputSegmentationNode->GetScene())
    {
    vtkErrorMacro("ComputeSourceAxisIndexForInputAxis: MRML scene of the given source node and the widget are different, cannot set node");
    return;
    }

  // If source is volume type and input segmentation has non-empty binary labelmap master that need to be resampled,
  // then match the axes of the input labelmap to the axes of the transformed source node.
  // Use this calculated permutation for updating spacing widget from geometry and interpreting spacing input
  if ( (transformableSourceNode->IsA("vtkMRMLScalarVolumeNode") || this->IsSourceSegmentationWithBinaryLabelmapMaster())
    && this->InputSegmentationCanBeResampled() )
    {
    // Determine transform between source node and input segmentation
    vtkNew<vtkGeneralTransform> segmentationToSourceTransform;
    vtkNew<vtkTransform> segmentationToSourceTransformLinear;
    vtkMRMLTransformNode::GetTransformBetweenNodes(this->InputSegmentationNode->GetParentTransformNode(),
      transformableSourceNode->GetParentTransformNode(), segmentationToSourceTransform.GetPointer());
    vtkNew<vtkMatrix4x4> segmentationToSourceMatrix;
    if (vtkMRMLTransformNode::IsGeneralTransformLinear(segmentationToSourceTransform.GetPointer(), segmentationToSourceTransformLinear.GetPointer()))
      {
      // Transformation between segmentation and source is linear
      segmentationToSourceTransformLinear->GetMatrix(segmentationToSourceMatrix.GetPointer());
      }
    else
      {
      vtkWarningMacro("ComputeSourceAxisIndexForInputAxis: Ignoring parent transforms because non-linear components have been found");
      segmentationToSourceMatrix->Identity();
      }

    vtkNew<vtkOrientedImageData> inputBinaryLabelmap;
    std::string geometryString = this->InputSegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry();
    vtkSegmentationConverter::DeserializeImageGeometry(geometryString, inputBinaryLabelmap.GetPointer(), false);

    // Find which labelmap axis corresponds to each source axis, to get the correct spacing value for each source axis
    vtkNew<vtkMatrix4x4> inputLabelmapIJKToInputSegmentation;
    inputBinaryLabelmap->GetImageToWorldMatrix(inputLabelmapIJKToInputSegmentation.GetPointer());
    vtkNew<vtkMatrix4x4> inputLabelmapIJKToSource;
    vtkMatrix4x4::Multiply4x4(segmentationToSourceMatrix.GetPointer(), inputLabelmapIJKToInputSegmentation.GetPointer(), inputLabelmapIJKToSource.GetPointer());

    // Find the axis that is best aligned with each source axis
    double scale[3] = { 1.0 };
    vtkAddonMathUtilities::NormalizeOrientationMatrixColumns(inputLabelmapIJKToSource.GetPointer(), scale);
    for (int sourceAxisIndex=0; sourceAxisIndex<3; sourceAxisIndex++)
      {
      double largestComponentValue = 0.0;
      for (int labelmapIJKAxisIndex=0; labelmapIJKAxisIndex<3; labelmapIJKAxisIndex++)
        {
        double currentComponentValue = fabs(inputLabelmapIJKToSource->GetElement(sourceAxisIndex, labelmapIJKAxisIndex));
        if (currentComponentValue > largestComponentValue)
          {
          largestComponentValue = currentComponentValue;
          this->InputAxisIndexForSourceAxis[sourceAxisIndex] = labelmapIJKAxisIndex;
          }
        }
      }
    }

  // Calculate inverse permutation
  for (int i=0; i<3; ++i)
    {
    this->SourceAxisIndexForInputAxis[this->InputAxisIndexForSourceAxis[i]] = i;
    }
}
