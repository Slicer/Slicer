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
#include "vtkSegmentationConverterFactory.h"

// Segmentation core includes
#include <vtkBinaryLabelmapToClosedSurfaceConversionRule.h>
#include <vtkClosedSurfaceToBinaryLabelmapConversionRule.h>
#include <vtkOrientedImageData.h>
#include <vtkOrientedImageDataResample.h>
#include <vtkSegment.h>
#include <vtkSegmentation.h>
#include <vtkSegmentationConverter.h>

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

//----------------------------------------------------------------------------
int TestSegmentDisplayPropertiesInAddedDisplayNode()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSegmentationNode> segmentationNode;
  scene->AddNode(segmentationNode);

  // A segment that is added without a display node keeps the invalid color
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  const std::string segmentId = "segment";
  segmentation->AddEmptySegment(segmentId);
  double color[3] = { 0.0, 0.0, 0.0 };
  segmentation->GetSegment(segmentId)->GetColor(color);
  CHECK_DOUBLE(color[0], vtkSegment::SEGMENT_COLOR_INVALID[0]);
  CHECK_DOUBLE(color[1], vtkSegment::SEGMENT_COLOR_INVALID[1]);
  CHECK_DOUBLE(color[2], vtkSegment::SEGMENT_COLOR_INVALID[2]);

  // The display node has display properties for a segment that is not in the segmentation yet
  // (as when display properties are read from a scene file before the segments are read)
  vtkNew<vtkMRMLSegmentationDisplayNode> displayNode;
  scene->AddNode(displayNode);
  const std::string notYetAddedSegmentId = "notYetAddedSegment";
  vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties hiddenSegmentProperties;
  hiddenSegmentProperties.Visible = false;
  displayNode->SetSegmentDisplayProperties(notYetAddedSegmentId, hiddenSegmentProperties);

  // The segment list of the display node is updated as soon as it is added:
  // the segment gets a generated color without any display property being requested
  segmentationNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  segmentation->GetSegment(segmentId)->GetColor(color);
  CHECK_BOOL(color[0] != vtkSegment::SEGMENT_COLOR_INVALID[0]      //
               || color[1] != vtkSegment::SEGMENT_COLOR_INVALID[1] //
               || color[2] != vtkSegment::SEGMENT_COLOR_INVALID[2],
             true);

  // Display properties of the segment that was not in the segmentation are kept
  segmentation->AddEmptySegment(notYetAddedSegmentId);
  CHECK_BOOL(displayNode->GetSegmentVisibility(notYetAddedSegmentId), false);
  CHECK_BOOL(displayNode->GetSegmentVisibility(segmentId), true);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
// Fill a box of the labelmap (IJK extent, inclusive) with a label value
void FillBox(vtkOrientedImageData* labelmap, const int extent[6], unsigned char label)
{
  for (int k = extent[4]; k <= extent[5]; ++k)
  {
    for (int j = extent[2]; j <= extent[3]; ++j)
    {
      for (int i = extent[0]; i <= extent[1]; ++i)
      {
        *static_cast<unsigned char*>(labelmap->GetScalarPointer(i, j, k)) = label;
      }
    }
  }
}

//----------------------------------------------------------------------------
int TestCalculateAutoOpacitiesForSegments()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLSegmentationNode> segmentationNode;
  scene->AddNode(segmentationNode);

  // Shared labelmap with three nested segments (outer: 1, middle: 2, inner: 3) and a separate segment (4).
  // The spacing is anisotropic and the image is shifted, to check that bounds are computed in world coordinates.
  vtkNew<vtkOrientedImageData> labelmap;
  labelmap->SetExtent(0, 59, 0, 39, 0, 29);
  labelmap->SetSpacing(0.5, 1.0, 2.0);
  labelmap->SetOrigin(-10.0, 20.0, 30.0);
  labelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  vtkOrientedImageDataResample::FillImage(labelmap, 0);
  const int outerBox[6] = { 2, 40, 2, 36, 1, 28 };
  const int middleBox[6] = { 6, 30, 6, 30, 4, 24 };
  const int innerBox[6] = { 10, 20, 10, 20, 8, 16 };
  const int separateBox[6] = { 45, 58, 0, 10, 0, 5 };
  FillBox(labelmap, outerBox, 1);
  FillBox(labelmap, middleBox, 2);
  FillBox(labelmap, innerBox, 3);
  FillBox(labelmap, separateBox, 4);

  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  const char* binaryLabelmapName = vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName();
  segmentation->SetSourceRepresentationName(binaryLabelmapName);
  const std::string segmentIds[4] = { "outer", "middle", "inner", "separate" };
  for (int index = 0; index < 4; ++index)
  {
    vtkNew<vtkSegment> segment;
    segment->SetName(segmentIds[index].c_str());
    segment->SetLabelValue(index + 1);
    segment->AddRepresentation(binaryLabelmapName, labelmap);
    segmentation->AddSegment(segment, segmentIds[index]);
  }

  vtkNew<vtkMRMLSegmentationDisplayNode> displayNode;
  scene->AddNode(displayNode);
  segmentationNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  // Binary labelmap shown in 3D views: opacities are computed from the voxels of the segments,
  // and translucent opacities are the levels that the labelmap surface rendering uses
  displayNode->SetPreferredDisplayRepresentationName3D(binaryLabelmapName);
  CHECK_BOOL(displayNode->CalculateAutoOpacitiesForSegments(), true);
  CHECK_BOOL(segmentation->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()), false);
  vtkMRMLSegmentationDisplayNode::SegmentDisplayProperties properties;
  // Two hierarchy levels (outer and middle contain other segments) are distributed among the translucent levels
  displayNode->GetSegmentDisplayProperties("outer", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, displayNode->GetLabelmapSurfaceTranslucentOpacity(1), 1e-6);
  displayNode->GetSegmentDisplayProperties("middle", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, displayNode->GetLabelmapSurfaceTranslucentOpacity(3), 1e-6);
  displayNode->GetSegmentDisplayProperties("inner", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 1.0, 1e-6);
  displayNode->GetSegmentDisplayProperties("separate", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 1.0, 1e-6);

  // Closed surface shown in 3D views: opacities are computed from the poly data of the segments
  displayNode->SetPreferredDisplayRepresentationName3D(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  CHECK_BOOL(displayNode->CalculateAutoOpacitiesForSegments(), true);
  CHECK_BOOL(segmentation->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()), true);
  displayNode->GetSegmentDisplayProperties("outer", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 1.0 / 3.0, 1e-6);
  displayNode->GetSegmentDisplayProperties("middle", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 2.0 / 3.0, 1e-6);
  displayNode->GetSegmentDisplayProperties("inner", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 1.0, 1e-6);
  displayNode->GetSegmentDisplayProperties("separate", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 1.0, 1e-6);

  // A segment without voxels takes no part in the hierarchy: it stays opaque and does not change the others
  displayNode->SetPreferredDisplayRepresentationName3D(binaryLabelmapName);
  segmentation->AddEmptySegment("empty");
  CHECK_BOOL(displayNode->CalculateAutoOpacitiesForSegments(), true);
  displayNode->GetSegmentDisplayProperties("empty", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 1.0, 1e-6);
  displayNode->GetSegmentDisplayProperties("outer", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, displayNode->GetLabelmapSurfaceTranslucentOpacity(1), 1e-6);

  // Fewer opacity levels: with a single translucent level (0.5) both containing segments get that opacity
  displayNode->SetNumberOfLabelmapSurfaceOpacityLevels(3);
  CHECK_INT(displayNode->GetNumberOfLabelmapSurfaceOpacityLevels(), 3);
  CHECK_INT(displayNode->GetLabelmapSurfaceTranslucentOpacityLevel(0.9), 1);
  CHECK_DOUBLE_TOLERANCE(displayNode->GetLabelmapSurfaceTranslucentOpacity(1), 0.5, 1e-6);
  CHECK_BOOL(displayNode->CalculateAutoOpacitiesForSegments(), true);
  displayNode->GetSegmentDisplayProperties("outer", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 0.5, 1e-6);
  displayNode->GetSegmentDisplayProperties("middle", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 0.5, 1e-6);
  displayNode->GetSegmentDisplayProperties("inner", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 1.0, 1e-6);

  // More opacity levels: the hierarchy levels are rendered as distinct opacities again (0.3 and 0.7 of 11 levels)
  displayNode->SetNumberOfLabelmapSurfaceOpacityLevels(11);
  CHECK_BOOL(displayNode->CalculateAutoOpacitiesForSegments(), true);
  displayNode->GetSegmentDisplayProperties("outer", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 0.3, 1e-6);
  displayNode->GetSegmentDisplayProperties("middle", properties);
  CHECK_DOUBLE_TOLERANCE(properties.Opacity3D, 0.7, 1e-6);

  // Invalid values are clamped to the minimum
  displayNode->SetNumberOfLabelmapSurfaceOpacityLevels(1);
  CHECK_INT(displayNode->GetNumberOfLabelmapSurfaceOpacityLevels(), 3);

  return EXIT_SUCCESS;
}
} // namespace

//----------------------------------------------------------------------------
int vtkMRMLSegmentationNodeTest1(int, char*[])
{
  vtkSegmentationConverterFactory* converterFactory = vtkSegmentationConverterFactory::GetInstance();
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New());
  converterFactory->RegisterConverterRule(vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New());

  CHECK_EXIT_SUCCESS(TestSegmentDisplayPropertiesInAllDisplayNodes());
  CHECK_EXIT_SUCCESS(TestSegmentDisplayPropertiesInAddedDisplayNode());
  CHECK_EXIT_SUCCESS(TestCalculateAutoOpacitiesForSegments());
  return EXIT_SUCCESS;
}
