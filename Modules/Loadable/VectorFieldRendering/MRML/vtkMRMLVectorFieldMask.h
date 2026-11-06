/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __vtkMRMLVectorFieldMask_h
#define __vtkMRMLVectorFieldMask_h

// MRML includes
#include "vtkSlicerVectorFieldRenderingModuleMRMLExport.h"

// VTK includes
#include <vtkPassInputTypeAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <memory>
#include <string>
#include <vector>

class vtkAbstractTransform;
class vtkMRMLNode;
class vtkMRMLVectorFieldRegionTester;

/// \brief Marks the points of a data set where a vector field is to be shown.
///
/// Adds the point data array GetInsideArrayName(): 1 at points inside every keep-inside region and outside every
/// keep-outside region, 0 elsewhere.
///
/// It is useful for field visualization, for example glyphs are placed only where the inside array value is 1;
/// grid lines, isosurfaces and streamlines are clipped.
///
/// Region insides: markups ROI box, closed model surface, segmentation segments, volume extent, other displayable
/// node bounding box. Nodes without an inside (plane, point list) are ignored. Input points are transformed to world
/// by GetPointsToWorldTransform().
///
/// \sa vtkMRMLVectorFieldDisplayNode
class VTK_SLICER_VECTORFIELDRENDERING_MODULE_MRML_EXPORT vtkMRMLVectorFieldMask : public vtkPassInputTypeAlgorithm
{
public:
  static vtkMRMLVectorFieldMask* New();
  vtkTypeMacro(vtkMRMLVectorFieldMask, vtkPassInputTypeAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Name of the point data array that is 1 where the field is shown and 0 elsewhere.
  static const char* GetInsideArrayName();

  /// True if the node has an inside that points can be tested against.
  static bool CanBeRegion(vtkMRMLNode* node);

  ///@{
  /// Regions the points are tested against; setting the same regions does not modify the filter. segmentIDs selects
  /// segments of a segmentation (empty: all); keepInside keeps the points inside (true) or outside (false) the region.
  void SetNumberOfRegions(int numberOfRegions);
  int GetNumberOfRegions();
  void SetRegion(int index, vtkMRMLNode* node, const std::vector<std::string>& segmentIDs, bool keepInside);
  ///@}

  ///@{
  /// Transform from the coordinates of the input points to world. nullptr means that the
  /// points are in world coordinates already.
  void SetPointsToWorldTransform(vtkAbstractTransform* transform);
  vtkAbstractTransform* GetPointsToWorldTransform();
  ///@}

  /// Includes the modified time of the regions and what defines their inside (model mesh, segments, parent
  /// transforms), so editing a region retests the points on the next update.
  vtkMTimeType GetMTime() override;

  /// Modified time of a region node and what defines where it is: model mesh, volume image, segments, and every
  /// parent transform (editing a transform does not modify the transform node).
  static vtkMTimeType GetRegionMTime(vtkMRMLNode* node);

protected:
  vtkMRMLVectorFieldMask();
  ~vtkMRMLVectorFieldMask() override;
  vtkMRMLVectorFieldMask(const vtkMRMLVectorFieldMask&) = delete;
  void operator=(const vtkMRMLVectorFieldMask&) = delete;

  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

  /// Build the region tests again where a region has changed
  void UpdateTesters();
  /// True if the field is shown at a position in input point coordinates: inside every keep-inside region and
  /// outside every keep-outside region. The region tests must be up to date.
  bool IsShownByTesters(const double position[3]);

  struct Region
  {
    vtkWeakPointer<vtkMRMLNode> Node;
    std::vector<std::string> SegmentIDs;
    bool KeepInside{ true };
    /// Tests the inside of the region; built again when the region or its transforms change
    std::shared_ptr<vtkMRMLVectorFieldRegionTester> Tester;
    vtkMTimeType TesterMTime{ 0 };
    bool TesterValid{ false };
  };
  std::vector<Region> Regions;
  vtkSmartPointer<vtkAbstractTransform> PointsToWorldTransform;
};

#endif
