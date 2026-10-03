/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationNode.h"

// Segmentation core includes
#include <vtkSegment.h>
#include <vtkSegmentation.h>

// VTK includes
#include <vtkNew.h>

// STD includes
#include <iostream>

namespace
{
//----------------------------------------------------------------------------
int TestSegmentDisplayPropertiesInAllDisplayNodes()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSegmentationNode> segmentationNode;
  scene->AddNode(segmentationNode);
  vtkNew<vtkMRMLSegmentationDisplayNode> displayNode1;
  scene->AddNode(displayNode1);
  segmentationNode->AddAndObserveDisplayNodeID(displayNode1->GetID());
  vtkNew<vtkMRMLSegmentationDisplayNode> displayNode2;
  scene->AddNode(displayNode2);
  segmentationNode->AddAndObserveDisplayNodeID(displayNode2->GetID());

  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  const std::string segmentId = "segment";
  segmentation->AddEmptySegment(segmentId);

  // The added segment gets a generated color
  double color[3] = { 0.0, 0.0, 0.0 };
  segmentation->GetSegment(segmentId)->GetColor(color);
  CHECK_BOOL(color[0] != vtkSegment::SEGMENT_COLOR_INVALID[0]      //
               || color[1] != vtkSegment::SEGMENT_COLOR_INVALID[1] //
               || color[2] != vtkSegment::SEGMENT_COLOR_INVALID[2],
             true);

  // Hide the segment in both display nodes
  displayNode1->SetSegmentVisibility(segmentId, false);
  displayNode2->SetSegmentVisibility(segmentId, false);
  CHECK_BOOL(displayNode1->GetSegmentVisibility(segmentId), false);
  CHECK_BOOL(displayNode2->GetSegmentVisibility(segmentId), false);

  // Display properties of a removed segment are removed from all display nodes,
  // so a new segment that is added with the same ID gets default display properties in all display nodes
  segmentation->RemoveSegment(segmentId);
  segmentation->AddEmptySegment(segmentId);
  CHECK_BOOL(displayNode1->GetSegmentVisibility(segmentId), true);
  CHECK_BOOL(displayNode2->GetSegmentVisibility(segmentId), true);

  return EXIT_SUCCESS;
}
} // namespace

//----------------------------------------------------------------------------
int vtkMRMLSegmentationNodeTest1(int, char*[])
{
  CHECK_EXIT_SUCCESS(TestSegmentDisplayPropertiesInAllDisplayNodes());
  return EXIT_SUCCESS;
}
