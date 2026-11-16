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

// Checks how vtkMRMLVectorFieldSamplePointSource turns sampling nodes into lattices and positions: the parts of each node
// type, lattices over an ROI, a slice and a bounding box, node points, steps along a curve, and random positions.

#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLVectorFieldDisplayNode.h"
#include "vtkMRMLVectorFieldSamplePointSource.h"
#include "vtkMRMLVectorFieldTestingHelpers.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkVector.h>

// STD includes
#include <set>
#include <utility>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

//----------------------------------------------------------------------------
vtkMRMLSliceNode* AddAxialSlice(vtkMRMLScene* scene, double fieldOfViewX, double fieldOfViewY)
{
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSliceNode"));
  sliceNode->SetDimensions(static_cast<int>(fieldOfViewX), static_cast<int>(fieldOfViewY), 1);
  sliceNode->SetFieldOfView(fieldOfViewX, fieldOfViewY, 1.0);
  sliceNode->UpdateMatrices();
  return sliceNode;
}

//----------------------------------------------------------------------------
int TestParts()
{
  std::cout << "Parts" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  const int points = vtkMRMLVectorFieldDisplayNode::SamplingRegionPartPoints;
  const int surface = vtkMRMLVectorFieldDisplayNode::SamplingRegionPartSurface;
  const int volume = vtkMRMLVectorFieldDisplayNode::SamplingRegionPartVolume;
  vtkMRMLNode* roiNode = AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(5.0, 5.0, 5.0));
  vtkMRMLNode* sliceNode = AddAxialSlice(scene, 100.0, 100.0);
  vtkMRMLNode* pointListNode = scene->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode");
  vtkMRMLNode* modelNode = scene->AddNewNodeByClass("vtkMRMLModelNode");
  vtkMRMLNode* segmentationNode = scene->AddNewNodeByClass("vtkMRMLSegmentationNode");
  vtkMRMLNode* volumeNode = scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode");

  // The inside of a box, the plane of a slice, the points of a point list, all of a model,
  // the surface and the inside of segments, the voxels and the inside of a volume
  const std::pair<vtkMRMLNode*, std::set<int>> supported[] = {
    { roiNode, { volume } },
    { sliceNode, { surface } },
    { pointListNode, { points } },
    { modelNode, { points, surface, volume } },
    { segmentationNode, { surface, volume } },
    { volumeNode, { points, volume } },
  };
  for (const auto& nodeParts : supported)
  {
    for (int part : { points, surface, volume })
    {
      bool expected = (nodeParts.second.count(part) > 0);
      if (vtkMRMLVectorFieldSamplePointSource::IsPartSupported(nodeParts.first, part) != expected)
      {
        std::cerr << nodeParts.first->GetClassName() << ": part " << part << " is expected to be " << (expected ? "" : "not ") << "supported" << std::endl;
        return EXIT_FAILURE;
      }
    }
    // The default part is always there
    CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::IsPartSupported(nodeParts.first, vtkMRMLVectorFieldDisplayNode::SamplingRegionPartDefault), true);
  }

  CHECK_INT(vtkMRMLVectorFieldSamplePointSource::GetPreferredPart(roiNode), volume);
  CHECK_INT(vtkMRMLVectorFieldSamplePointSource::GetPreferredPart(sliceNode), surface);
  CHECK_INT(vtkMRMLVectorFieldSamplePointSource::GetPreferredPart(pointListNode), points);
  CHECK_INT(vtkMRMLVectorFieldSamplePointSource::GetPreferredPart(modelNode), points);
  CHECK_INT(vtkMRMLVectorFieldSamplePointSource::GetPreferredPart(segmentationNode), volume);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestLattices()
{
  std::cout << "Lattices" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMatrix4x4> latticeToRAS;
  int latticeSize[3] = { 0, 0, 0 };

  // Inside a box, 10 mm apart, centered in it
  vtkMRMLMarkupsROINode* roiNode = AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(20.0, 10.0, 5.0));
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetLattice(roiNode, 10.0, 1, latticeToRAS, latticeSize), true);
  CHECK_INT(latticeSize[0], 5);
  CHECK_INT(latticeSize[1], 3);
  CHECK_INT(latticeSize[2], 2);
  CHECK_DOUBLE_TOLERANCE(latticeToRAS->GetElement(0, 0), 10.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(latticeToRAS->GetElement(0, 3), -20.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(latticeToRAS->GetElement(1, 3), -10.0, 1e-9);
  CHECK_DOUBLE_TOLERANCE(latticeToRAS->GetElement(2, 3), -5.0, 1e-9);
  // A whole number of groups of two steps along each axis: at least one group, centered
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetLattice(roiNode, 10.0, 2, latticeToRAS, latticeSize), true);
  CHECK_INT(latticeSize[0], 5);
  CHECK_INT(latticeSize[1], 3);
  CHECK_INT(latticeSize[2], 3);
  CHECK_DOUBLE_TOLERANCE(latticeToRAS->GetElement(2, 3), -10.0, 1e-9);
  // No spacing: about 20 steps along the longest axis
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetLattice(roiNode, 0.0, 1, latticeToRAS, latticeSize), true);
  CHECK_INT(latticeSize[0], 21);

  // In the plane of a slice: flat, over the field of view
  vtkMRMLSliceNode* sliceNode = AddAxialSlice(scene, 100.0, 50.0);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetLattice(sliceNode, 10.0, 1, latticeToRAS, latticeSize), true);
  CHECK_INT(latticeSize[0], 11);
  CHECK_INT(latticeSize[1], 6);
  CHECK_INT(latticeSize[2], 1);
  // A slice needs a spacing
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetLattice(sliceNode, 0.0, 1, latticeToRAS, latticeSize), false);

  // Over the bounding box of any other node
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  vtkNew<vtkPolyData> mesh;
  vtkNew<vtkPoints> meshPoints;
  meshPoints->InsertNextPoint(0.0, 0.0, 0.0);
  meshPoints->InsertNextPoint(30.0, 10.0, 0.0);
  mesh->SetPoints(meshPoints);
  modelNode->SetAndObserveMesh(mesh);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetLattice(modelNode, 10.0, 1, latticeToRAS, latticeSize), true);
  CHECK_INT(latticeSize[0], 4);
  CHECK_INT(latticeSize[1], 2);
  CHECK_INT(latticeSize[2], 1);
  CHECK_DOUBLE_TOLERANCE(latticeToRAS->GetElement(0, 3), 0.0, 1e-9);

  // Nothing to sample in
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetLattice(nullptr, 10.0, 1, latticeToRAS, latticeSize), false);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestPositions()
{
  std::cout << "Positions" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkPoints> positions;

  // The control points of a point list
  vtkMRMLMarkupsFiducialNode* pointListNode = AddPointList(scene, { vtkVector3d(1.0, 2.0, 3.0), vtkVector3d(4.0, 5.0, 6.0) });
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetNodePointPositions(pointListNode, positions), true);
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), 2);
  CHECK_DOUBLE(positions->GetPoint(1)[2], 6.0);

  // The points of a model, in the world
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  vtkNew<vtkPolyData> mesh;
  vtkNew<vtkPoints> meshPoints;
  meshPoints->InsertNextPoint(0.0, 0.0, 0.0);
  mesh->SetPoints(meshPoints);
  modelNode->SetAndObserveMesh(mesh);
  modelNode->SetAndObserveTransformNodeID(AddTranslation(scene, 100.0, 0.0, 0.0)->GetID());
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetNodePointPositions(modelNode, positions), true);
  CHECK_DOUBLE(positions->GetPoint(0)[0], 100.0);

  // The voxel centers of a volume: all of them, or every few along each axis
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode"));
  vtkNew<vtkImageData> image;
  image->SetDimensions(11, 2, 1);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  volumeNode->SetAndObserveImageData(image);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetNodePointPositions(volumeNode, positions), true);
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), 22);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetVoxelLatticePositions(volumeNode, 5.0, positions), true);
  // Every fifth voxel along each axis: 3 along the first, 1 along the second
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), 3);

  // Steps along a curve, 10 mm apart, from end to end
  vtkMRMLMarkupsCurveNode* curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsCurveNode"));
  curveNode->AddControlPoint(vtkVector3d(0.0, 0.0, 0.0));
  curveNode->AddControlPoint(vtkVector3d(100.0, 0.0, 0.0));
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetCurvePositions(curveNode, 10.0, positions), true);
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), 11);
  CHECK_DOUBLE_TOLERANCE(positions->GetPoint(0)[0], 0.0, 1e-6);
  CHECK_DOUBLE_TOLERANCE(positions->GetPoint(10)[0], 100.0, 1e-6);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestRandomPositions()
{
  std::cout << "Random positions" << std::endl;
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkPoints> positions;
  vtkNew<vtkPoints> samePositions;

  // In a box, always the same ones
  vtkMRMLMarkupsROINode* roiNode = AddROI(scene, vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(20.0, 10.0, 5.0));
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsInBox(roiNode, 100, positions), true);
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), 100);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsInBox(roiNode, 100, samePositions), true);
  for (vtkIdType pointIndex = 0; pointIndex < 100; ++pointIndex)
  {
    double* position = positions->GetPoint(pointIndex);
    CHECK_BOOL(std::abs(position[0]) <= 20.0 && std::abs(position[1]) <= 10.0 && std::abs(position[2]) <= 5.0, true);
    CHECK_DOUBLE(samePositions->GetPoint(pointIndex)[0], positions->GetPoint(pointIndex)[0]);
  }

  // On the rectangle of a slice, which has no inside
  vtkMRMLSliceNode* sliceNode = AddAxialSlice(scene, 100.0, 50.0);
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsInRectangle(sliceNode, 50, positions), true);
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), 50);
  for (vtkIdType pointIndex = 0; pointIndex < 50; ++pointIndex)
  {
    double* position = positions->GetPoint(pointIndex);
    CHECK_BOOL(std::abs(position[0]) <= 50.0 && std::abs(position[1]) <= 25.0 && std::abs(position[2]) < 1e-9, true);
  }
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsInRectangle(roiNode, 50, positions), false);

  // On a surface: at most as many as asked for
  vtkNew<vtkSphereSource> sphere;
  sphere->SetThetaResolution(30);
  sphere->SetPhiResolution(30);
  sphere->Update();
  CHECK_BOOL(vtkMRMLVectorFieldSamplePointSource::GetRandomPositionsOnSurface(sphere->GetOutput(), 20, positions), true);
  CHECK_BOOL(positions->GetNumberOfPoints() > 0 && positions->GetNumberOfPoints() <= 20, true);

  // A subset of positions: each at most once, always the same
  vtkNew<vtkPoints> allPositions;
  for (int index = 0; index < 100; ++index)
  {
    allPositions->InsertNextPoint(index, 0.0, 0.0);
  }
  vtkMRMLVectorFieldSamplePointSource::SelectRandomSubset(allPositions, 10, positions);
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), 10);
  std::set<double> distinct;
  for (vtkIdType pointIndex = 0; pointIndex < 10; ++pointIndex)
  {
    distinct.insert(positions->GetPoint(pointIndex)[0]);
  }
  CHECK_INT(static_cast<int>(distinct.size()), 10);
  vtkMRMLVectorFieldSamplePointSource::SelectRandomSubset(allPositions, 10, samePositions);
  CHECK_DOUBLE(samePositions->GetPoint(3)[0], positions->GetPoint(3)[0]);
  // All of them if there are not more than asked for
  vtkMRMLVectorFieldSamplePointSource::SelectRandomSubset(allPositions, 200, positions);
  CHECK_INT(static_cast<int>(positions->GetNumberOfPoints()), 100);
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
/// The filter computes the positions again only when what defines them changes
int TestSource()
{
  vtkNew<vtkMRMLScene> scene;
  vtkMRMLVectorFieldDisplayNode* displayNode = AddVolumeWithFieldDisplay(scene, CreateVectorImage(LinearField));
  vtkMRMLMarkupsFiducialNode* pointList = AddPointList(scene, { vtkVector3d(0.0, 0.0, 0.0), vtkVector3d(10.0, 5.0, -5.0) });
  displayNode->SetAndObserveSamplingRegionNode(pointList);

  vtkNew<vtkMRMLVectorFieldSamplePointSource> source;
  source->Update();
  CHECK_INT(static_cast<int>(source->GetOutput()->GetNumberOfPoints()), 0); // no display node
  source->SetDisplayNode(displayNode);
  source->Update();
  CHECK_INT(static_cast<int>(source->GetOutput()->GetNumberOfPoints()), 2);
  vtkMTimeType computedTime = source->GetOutput()->GetMTime();
  source->Update();
  CHECK_BOOL(source->GetOutput()->GetMTime() == computedTime, true);

  // A moved point, a transform of the region, or a setting of the display node computes them again
  pointList->SetNthControlPointPosition(1, 20.0, 5.0, -5.0);
  source->Update();
  CHECK_BOOL(source->GetOutput()->GetMTime() > computedTime, true);
  CHECK_DOUBLE_TOLERANCE(source->GetOutput()->GetPoint(1)[0], 20.0, 1e-6);
  computedTime = source->GetOutput()->GetMTime();
  vtkMRMLLinearTransformNode* transformNode = AddTranslation(scene, 0.0, 0.0, 0.0);
  pointList->SetAndObserveTransformNodeID(transformNode->GetID());
  vtkNew<vtkMatrix4x4> translation;
  translation->SetElement(0, 3, 100.0);
  transformNode->SetMatrixTransformToParent(translation);
  source->Update();
  CHECK_BOOL(source->GetOutput()->GetMTime() > computedTime, true);
  CHECK_DOUBLE_TOLERANCE(source->GetOutput()->GetPoint(0)[0], 100.0, 1e-6);

  // The streamline seeds of the same region
  vtkNew<vtkMRMLVectorFieldSamplePointSource> seedSource;
  seedSource->SetDisplayNode(displayNode);
  seedSource->SetOutputPoints(vtkMRMLVectorFieldSamplePointSource::OutputStreamlineSeeds);
  displayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  seedSource->Update();
  CHECK_INT(static_cast<int>(seedSource->GetOutput()->GetNumberOfPoints()), 2);
  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkMRMLVectorFieldSamplePointSourceTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestParts());
  CHECK_EXIT_SUCCESS(TestLattices());
  CHECK_EXIT_SUCCESS(TestPositions());
  CHECK_EXIT_SUCCESS(TestRandomPositions());
  CHECK_EXIT_SUCCESS(TestSource());
  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
