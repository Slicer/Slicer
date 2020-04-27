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

// .NAME vtkSlicerSegmentationGeometryLogic
// .SECTION Description
// This class manages the logic associated with converting and handling
// segmentation node objects.

#ifndef __vtkSlicerSegmentationGeometryLogic_h
#define __vtkSlicerSegmentationGeometryLogic_h

// Slicer includes
#include "vtkSlicerSegmentationsModuleLogicExport.h"

// Segmentations includes
#include "vtkMRMLSegmentationNode.h"

// vtkSegmentationCore includes
#include "vtkOrientedImageData.h"

/// \ingroup Slicer_QtModules_Segmentations
class VTK_SLICER_SEGMENTATIONS_LOGIC_EXPORT vtkSlicerSegmentationGeometryLogic : public vtkObject
{
public:
  static vtkSlicerSegmentationGeometryLogic* New();
  vtkTypeMacro(vtkSlicerSegmentationGeometryLogic, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Calculate output geometry into \sa GeometryImageData with current options
  /// \return Error message. Empty when successful
  std::string CalculateOutputGeometry();

  /// Determine if source node is a non-empty segmentation with a labelmap master
  bool IsSourceSegmentationWithBinaryLabelmapMaster();

  /// Determine if input segmentation is non-empty, and with binary labelmap master
  bool InputSegmentationCanBeResampled();

  /// Match the axes of the input labelmap to the axes of the transformed source node.
  /// Use this calculated permutation for updating spacing widget from geometry and interpreting spacing input
  void ComputeSourceAxisIndexForInputAxis();

  vtkGetObjectMacro(OutputGeometryImageData, vtkOrientedImageData);

  /// Reset geometry image data \sa GeometryImageData
  void ResetGeometryImageData();

  vtkGetObjectMacro(InputSegmentationNode, vtkMRMLSegmentationNode);
  vtkSetObjectMacro(InputSegmentationNode, vtkMRMLSegmentationNode);

  vtkGetObjectMacro(SourceGeometryNode, vtkMRMLDisplayableNode);
  virtual void SetSourceGeometryNode(vtkMRMLDisplayableNode* node);

  vtkGetMacro(IsotropicSpacing, bool);
  vtkSetMacro(IsotropicSpacing, bool);
  vtkBooleanMacro(IsotropicSpacing, bool);

  vtkGetMacro(OversamplingFactor, double);
  vtkSetMacro(OversamplingFactor, double);

  vtkGetVectorMacro(UserSpacing, double, 3);
  vtkSetVectorMacro(UserSpacing, double, 3);

  vtkGetVectorMacro(InputAxisIndexForSourceAxis, int, 3);

  vtkGetVectorMacro(SourceAxisIndexForInputAxis, int, 3);

protected:
  vtkSlicerSegmentationGeometryLogic();
  ~vtkSlicerSegmentationGeometryLogic() override;

protected:

  /// Calculate output geometry into \sa GeometryImageData with current options from image
  /// or segmentation that uses labelmap as master representation.
  /// \return Error message. Empty when successful
  std::string CalculateOutputGeometryFromImage();

  /// Calculate output geometry into \sa GeometryImageData with current options from node bounds.
  /// If keepCurrentAxisDirections is true then the current axes directions will be preserved,
  /// otherwise the source axes directions are used.
  /// \return Error message. Empty when successful
  std::string CalculateOutputGeometryFromBounds(bool keepCurrentAxisDirections);

  /// Input segmentation MRML node to modify the labelmap geometry of
  vtkMRMLSegmentationNode* InputSegmentationNode;

  /// MRML node specifying the source geometry
  vtkMRMLDisplayableNode* SourceGeometryNode;

  /// Flag indicating whether isotropic spacing is requested. Off by default
  /// Resample the volume to have isotropic spacing, which means the voxels will be cubes.
  /// Use smallest spacing. Useful if the volume has elongated voxels.
  bool IsotropicSpacing;

  /// Oversampling factor:
  /// Split each voxel of the volume to this many voxels along each direction. Useful when increasing the resolution is needed
  /// 1 by default.
  double OversamplingFactor;

  /// Explicitly specified spacing. Only applied if \sa SourceGeometryNode does not contain volume data
  /// (i.e. not a volume node nor a segmentation node with labelmap master, but an ROI, model, or segmentation with poly data master)
  double UserSpacing[3]{ 1.0, 1.0, 1.0 };

  /// Oriented image data containing output geometry. This is what the class calculates from the inputs
  vtkOrientedImageData* OutputGeometryImageData;

  /// Source to input axes mapping \sa matchInputAndSourceAxes
  int InputAxisIndexForSourceAxis[3]{ 0, 1, 2 };
  /// Inverse permutation of \sa InputAxisIndexForSourceAxis
  int SourceAxisIndexForInputAxis[3]{ 0, 1, 2 };

private:
  vtkSlicerSegmentationGeometryLogic(const vtkSlicerSegmentationGeometryLogic&) = delete;
  void operator=(const vtkSlicerSegmentationGeometryLogic&) = delete;
};

#endif
