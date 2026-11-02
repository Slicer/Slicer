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

// MRML includes
#include "vtkMRMLVectorFieldMask.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLVolumeNode.h"

// Segmentation includes
#include <vtkOrientedImageData.h>
#include <vtkSegment.h>
#include <vtkSegmentation.h>
#include <vtkSegmentationConverter.h>

// VTK includes
#include <vtkAbstractTransform.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkGeneralTransform.h>
#include <vtkGeometryFilter.h>
#include <vtkImageData.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkTransformPolyDataFilter.h>

// STD includes
#include <algorithm>
#include <cmath>
#include <memory>

//----------------------------------------------------------------------------
/// Tells whether a position in world coordinates is inside a region.
class vtkMRMLVectorFieldRegionTester
{
public:
  virtual ~vtkMRMLVectorFieldRegionTester() = default;
  virtual bool IsInside(const double position_World[3]) = 0;
};

//----------------------------------------------------------------------------
namespace
{
const char InsideArrayName[] = "InsideRegion";

using RegionTester = vtkMRMLVectorFieldRegionTester;

/// The transform from the coordinates of a node to world, through its parent transforms.
/// Identity for a node that is not under a transform.
void GetNodeToWorldTransform(vtkMRMLNode* node, vtkGeneralTransform* nodeToWorld)
{
  nodeToWorld->Identity();
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(node);
  vtkMRMLTransformNode* parentTransformNode = (transformableNode ? transformableNode->GetParentTransformNode() : nullptr);
  if (parentTransformNode)
  {
    vtkMRMLTransformNode::GetTransformBetweenNodes(parentTransformNode, nullptr, nodeToWorld);
  }
}

//----------------------------------------------------------------------------
/// A box, given by the transform from world to the coordinates that its bounds are in.
class BoxTester : public RegionTester
{
public:
  BoxTester(vtkAbstractTransform* worldToBox, const double bounds[6])
    : WorldToBox(worldToBox)
  {
    std::copy(bounds, bounds + 6, this->Bounds);
  }
  bool IsInside(const double position_World[3]) override
  {
    double position_Box[3] = { position_World[0], position_World[1], position_World[2] };
    if (this->WorldToBox)
    {
      this->WorldToBox->TransformPoint(position_World, position_Box);
    }
    for (int axis = 0; axis < 3; ++axis)
    {
      if (position_Box[axis] < this->Bounds[2 * axis] || position_Box[axis] > this->Bounds[2 * axis + 1])
      {
        return false;
      }
    }
    return true;
  }

private:
  vtkSmartPointer<vtkAbstractTransform> WorldToBox;
  double Bounds[6];
};

//----------------------------------------------------------------------------
/// The inside of a closed surface, in world coordinates.
class SurfaceTester : public RegionTester
{
public:
  explicit SurfaceTester(vtkPolyData* surface_World) { this->Distance->SetInput(surface_World); }
  bool IsInside(const double position_World[3]) override
  {
    double position[3] = { position_World[0], position_World[1], position_World[2] };
    return this->Distance->EvaluateFunction(position) < 0.0;
  }

private:
  vtkNew<vtkImplicitPolyDataDistance> Distance;
};

//----------------------------------------------------------------------------
/// The voxels of a labelmap that have a given value.
class LabelmapTester : public RegionTester
{
public:
  LabelmapTester(vtkOrientedImageData* labelmap, int labelValue, vtkAbstractTransform* worldToNode)
    : Labelmap(labelmap)
    , LabelValue(labelValue)
  {
    vtkNew<vtkMatrix4x4> nodeToIJK;
    labelmap->GetWorldToImageMatrix(nodeToIJK);
    this->WorldToIJK->Concatenate(nodeToIJK);
    if (worldToNode)
    {
      this->WorldToIJK->Concatenate(worldToNode);
    }
    labelmap->GetExtent(this->Extent);
  }
  bool IsInside(const double position_World[3]) override
  {
    double position_IJK[3] = { 0.0, 0.0, 0.0 };
    this->WorldToIJK->TransformPoint(position_World, position_IJK);
    int ijk[3] = { 0, 0, 0 };
    for (int axis = 0; axis < 3; ++axis)
    {
      ijk[axis] = static_cast<int>(std::floor(position_IJK[axis] + 0.5));
      if (ijk[axis] < this->Extent[2 * axis] || ijk[axis] > this->Extent[2 * axis + 1])
      {
        return false;
      }
    }
    return static_cast<int>(this->Labelmap->GetScalarComponentAsDouble(ijk[0], ijk[1], ijk[2], 0)) == this->LabelValue;
  }

private:
  vtkSmartPointer<vtkOrientedImageData> Labelmap;
  int LabelValue;
  vtkNew<vtkGeneralTransform> WorldToIJK;
  int Extent[6];
};

//----------------------------------------------------------------------------
/// Inside if inside any of several regions (the segments of a segmentation).
class UnionTester : public RegionTester
{
public:
  bool IsInside(const double position_World[3]) override
  {
    for (const std::unique_ptr<RegionTester>& tester : this->Testers)
    {
      if (tester->IsInside(position_World))
      {
        return true;
      }
    }
    return false;
  }
  std::vector<std::unique_ptr<RegionTester>> Testers;
};

//----------------------------------------------------------------------------
/// A surface mesh in the coordinates of a node, brought into world coordinates.
vtkSmartPointer<vtkPolyData> GetSurfaceInWorld(vtkPointSet* mesh, vtkAbstractTransform* nodeToWorld)
{
  if (!mesh || mesh->GetNumberOfPoints() == 0)
  {
    return nullptr;
  }
  vtkSmartPointer<vtkPolyData> surface = vtkPolyData::SafeDownCast(mesh);
  if (!surface)
  {
    // A volumetric mesh: its boundary
    vtkNew<vtkGeometryFilter> boundary;
    boundary->SetInputData(mesh);
    boundary->Update();
    surface = boundary->GetOutput();
  }
  vtkNew<vtkTransformPolyDataFilter> toWorld;
  toWorld->SetInputData(surface);
  toWorld->SetTransform(nodeToWorld);
  toWorld->Update();
  return toWorld->GetOutput();
}

//----------------------------------------------------------------------------
std::unique_ptr<RegionTester> CreateTester(vtkMRMLNode* node, const std::vector<std::string>& segmentIDs)
{
  if (!node)
  {
    return nullptr;
  }
  vtkNew<vtkGeneralTransform> nodeToWorld;
  GetNodeToWorldTransform(node, nodeToWorld);

  // A region of interest: its box, which is defined in world coordinates already
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(node);
  if (roiNode)
  {
    double size[3] = { 0.0, 0.0, 0.0 };
    roiNode->GetSize(size);
    double bounds[6] = { -0.5 * size[0], 0.5 * size[0], -0.5 * size[1], 0.5 * size[1], -0.5 * size[2], 0.5 * size[2] };
    vtkNew<vtkMatrix4x4> worldToROI;
    vtkMatrix4x4::Invert(roiNode->GetObjectToWorldMatrix(), worldToROI);
    vtkNew<vtkGeneralTransform> worldToBox;
    worldToBox->Concatenate(worldToROI);
    return std::make_unique<BoxTester>(worldToBox, bounds);
  }

  // A segmentation: the inside of its segments, in the source representation (labelmap or closed surface)
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  if (segmentationNode)
  {
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    if (!segmentation)
    {
      return nullptr;
    }
    std::vector<std::string> ids = segmentIDs;
    if (ids.empty())
    {
      segmentation->GetSegmentIDs(ids);
    }
    // The source representation is the accurate one; the other is a fallback if the source is neither
    bool surfaceIsSource = (segmentation->GetSourceRepresentationName() == vtkSegmentationConverter::GetClosedSurfaceRepresentationName());
    auto tester = std::make_unique<UnionTester>();
    for (const std::string& segmentID : ids)
    {
      vtkSegment* segment = segmentation->GetSegment(segmentID);
      if (!segment)
      {
        continue;
      }
      auto addLabelmapTester = [&]()
      {
        vtkOrientedImageData* labelmap = vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()));
        if (!labelmap || !labelmap->GetPointData()->GetScalars())
        {
          return false;
        }
        tester->Testers.push_back(std::make_unique<LabelmapTester>(labelmap, segment->GetLabelValue(), nodeToWorld->GetInverse()));
        return true;
      };
      auto addSurfaceTester = [&]()
      {
        vtkPolyData* surface = vtkPolyData::SafeDownCast(segment->GetRepresentation(vtkSegmentationConverter::GetClosedSurfaceRepresentationName()));
        vtkSmartPointer<vtkPolyData> surface_World = GetSurfaceInWorld(surface, nodeToWorld);
        if (!surface_World)
        {
          return false;
        }
        tester->Testers.push_back(std::make_unique<SurfaceTester>(surface_World));
        return true;
      };
      if (surfaceIsSource)
      {
        addSurfaceTester() || addLabelmapTester();
      }
      else
      {
        addLabelmapTester() || addSurfaceTester();
      }
    }
    return tester;
  }

  // A model: the inside of its surface
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  if (modelNode)
  {
    vtkSmartPointer<vtkPolyData> surface_World = GetSurfaceInWorld(modelNode->GetMesh(), nodeToWorld);
    if (!surface_World)
    {
      return nullptr;
    }
    return std::make_unique<SurfaceTester>(surface_World);
  }

  // A volume: its extent, along its own axes
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  if (volumeNode && volumeNode->GetImageData())
  {
    int extent[6] = { 0, -1, 0, -1, 0, -1 };
    volumeNode->GetImageData()->GetExtent(extent);
    double bounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    for (int axis = 0; axis < 3; ++axis)
    {
      // The voxels are samples, each one covering half a voxel on either side
      bounds[2 * axis] = extent[2 * axis] - 0.5;
      bounds[2 * axis + 1] = extent[2 * axis + 1] + 0.5;
    }
    vtkNew<vtkMatrix4x4> rasToIJK;
    volumeNode->GetRASToIJKMatrix(rasToIJK);
    vtkNew<vtkGeneralTransform> worldToIJK;
    worldToIJK->Concatenate(rasToIJK);
    worldToIJK->Concatenate(nodeToWorld->GetInverse());
    return std::make_unique<BoxTester>(worldToIJK, bounds);
  }

  // Any other displayable node: its bounding box in world coordinates
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (displayableNode)
  {
    double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    displayableNode->GetRASBounds(bounds);
    if (bounds[0] > bounds[1])
    {
      return nullptr;
    }
    return std::make_unique<BoxTester>(nullptr, bounds);
  }
  return nullptr;
}
} // namespace

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLVectorFieldMask);

//----------------------------------------------------------------------------
vtkMRMLVectorFieldMask::vtkMRMLVectorFieldMask() = default;

//----------------------------------------------------------------------------
vtkMRMLVectorFieldMask::~vtkMRMLVectorFieldMask() = default;

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldMask::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PointsToWorldTransform: " << this->PointsToWorldTransform.GetPointer() << "\n";
  os << indent << "NumberOfRegions: " << this->Regions.size() << "\n";
  for (const Region& region : this->Regions)
  {
    os << indent.GetNextIndent() << (region.Node ? region.Node->GetID() : "(none)") << (region.KeepInside ? " inside" : " outside") << "\n";
  }
}

//----------------------------------------------------------------------------
const char* vtkMRMLVectorFieldMask::GetInsideArrayName()
{
  return InsideArrayName;
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldMask::CanBeRegion(vtkMRMLNode* node)
{
  if (!node)
  {
    return false;
  }
  // A plane, a curve or a point list encloses nothing
  if (node->IsA("vtkMRMLMarkupsNode") && !node->IsA("vtkMRMLMarkupsROINode"))
  {
    return false;
  }
  return (vtkMRMLDisplayableNode::SafeDownCast(node) != nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldMask::SetNumberOfRegions(int numberOfRegions)
{
  numberOfRegions = std::max(0, numberOfRegions);
  if (static_cast<int>(this->Regions.size()) == numberOfRegions)
  {
    return;
  }
  this->Regions.resize(numberOfRegions);
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldMask::GetNumberOfRegions()
{
  return static_cast<int>(this->Regions.size());
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldMask::SetRegion(int index, vtkMRMLNode* node, const std::vector<std::string>& segmentIDs, bool keepInside)
{
  if (index < 0 || index >= static_cast<int>(this->Regions.size()))
  {
    vtkErrorMacro("SetRegion: index " << index << " is out of range");
    return;
  }
  Region& region = this->Regions[index];
  if (region.Node == node && region.SegmentIDs == segmentIDs && region.KeepInside == keepInside)
  {
    return;
  }
  region.Node = node;
  region.SegmentIDs = segmentIDs;
  region.KeepInside = keepInside;
  region.Tester.reset();
  region.TesterValid = false;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldMask::SetPointsToWorldTransform(vtkAbstractTransform* transform)
{
  if (this->PointsToWorldTransform == transform)
  {
    return;
  }
  this->PointsToWorldTransform = transform;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkAbstractTransform* vtkMRMLVectorFieldMask::GetPointsToWorldTransform()
{
  return this->PointsToWorldTransform;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMRMLVectorFieldMask::GetRegionMTime(vtkMRMLNode* node)
{
  if (!node)
  {
    return 0;
  }
  vtkMTimeType mTime = node->GetMTime();
  // Editing a transform modifies the transform object, not the transform node, and any
  // transform of the chain moves the region
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(node);
  for (vtkMRMLTransformNode* transformNode = (transformableNode ? transformableNode->GetParentTransformNode() : nullptr); transformNode;
       transformNode = transformNode->GetParentTransformNode())
  {
    mTime = std::max(mTime, transformNode->GetMTime());
    if (transformNode->GetTransformToParent())
    {
      mTime = std::max(mTime, transformNode->GetTransformToParent()->GetMTime());
    }
  }
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  if (modelNode && modelNode->GetMesh())
  {
    mTime = std::max(mTime, modelNode->GetMesh()->GetMTime());
  }
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);
  if (volumeNode && volumeNode->GetImageData())
  {
    mTime = std::max(mTime, volumeNode->GetImageData()->GetMTime());
  }
  // Editing a segment changes its representations, not the segmentation node
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  vtkSegmentation* segmentation = (segmentationNode ? segmentationNode->GetSegmentation() : nullptr);
  if (segmentation)
  {
    mTime = std::max(mTime, segmentation->GetMTime());
    std::vector<std::string> segmentIDs;
    segmentation->GetSegmentIDs(segmentIDs);
    for (const std::string& segmentID : segmentIDs)
    {
      vtkSegment* segment = segmentation->GetSegment(segmentID);
      for (const char* representationName : { vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), vtkSegmentationConverter::GetClosedSurfaceRepresentationName() })
      {
        vtkDataObject* representation = (segment ? segment->GetRepresentation(representationName) : nullptr);
        if (representation)
        {
          mTime = std::max(mTime, representation->GetMTime());
        }
      }
    }
  }
  return mTime;
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMRMLVectorFieldMask::GetMTime()
{
  vtkMTimeType mTime = this->Superclass::GetMTime();
  if (this->PointsToWorldTransform)
  {
    mTime = std::max(mTime, this->PointsToWorldTransform->GetMTime());
  }
  for (const Region& region : this->Regions)
  {
    mTime = std::max(mTime, vtkMRMLVectorFieldMask::GetRegionMTime(region.Node));
  }
  return mTime;
}

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldMask::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkDataSet* input = vtkDataSet::GetData(inputVector[0]);
  vtkDataSet* output = vtkDataSet::GetData(outputVector);
  if (!input || !output)
  {
    return 0;
  }
  output->ShallowCopy(input);

  this->UpdateTesters();
  vtkIdType numberOfPoints = input->GetNumberOfPoints();
  // Floating point, so that geometry clipped where it changes from 1 to 0 is cut between the
  // samples rather than at one of them
  vtkNew<vtkFloatArray> inside;
  inside->SetName(InsideArrayName);
  inside->SetNumberOfValues(numberOfPoints);
  for (vtkIdType pointIndex = 0; pointIndex < numberOfPoints; ++pointIndex)
  {
    double position[3] = { 0.0, 0.0, 0.0 };
    input->GetPoint(pointIndex, position);
    inside->SetValue(pointIndex, this->IsShownByTesters(position) ? 1 : 0);
  }
  output->GetPointData()->AddArray(inside);
  return 1;
}

//----------------------------------------------------------------------------
void vtkMRMLVectorFieldMask::UpdateTesters()
{
  // Building a tester copies and transforms the surface of the region and builds a locator for
  // it, so it is only built again when the region changes, not when the points do
  for (Region& region : this->Regions)
  {
    vtkMTimeType regionMTime = vtkMRMLVectorFieldMask::GetRegionMTime(region.Node);
    if (!region.TesterValid || region.TesterMTime != regionMTime)
    {
      region.Tester = CreateTester(region.Node, region.SegmentIDs);
      region.TesterMTime = regionMTime;
      region.TesterValid = true;
    }
  }
}

//----------------------------------------------------------------------------
bool vtkMRMLVectorFieldMask::IsShownByTesters(const double position[3])
{
  // The regions are tested in world coordinates
  double position_World[3] = { position[0], position[1], position[2] };
  if (this->PointsToWorldTransform)
  {
    this->PointsToWorldTransform->TransformPoint(position, position_World);
  }
  for (const Region& region : this->Regions)
  {
    if (region.Tester && region.Tester->IsInside(position_World) != region.KeepInside)
    {
      return false;
    }
  }
  return true;
}
