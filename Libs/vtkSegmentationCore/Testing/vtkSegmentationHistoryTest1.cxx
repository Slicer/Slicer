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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// VTK includes
#include <vtkImageAccumulate.h>
#include <vtkImageThreshold.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkVersion.h>

// Get CHECK_INT from vtkAddonTestingMacros.h to avoid dependency on vtkAddon
namespace
{

//----------------------------------------------------------------------------
bool CheckInt(int line, const std::string& description, int current, int expected)
{
  if (current == expected)
  {
    return EXIT_SUCCESS;
  }
  std::cerr << "\nLine " << line << " - " << description.c_str() << " : test failed"
            << "\n\tcurrent :" << current << "\n\texpected:" << expected << std::endl;
  return EXIT_FAILURE;
}

// Use a macro to be able to print the evaluated expression and the line number
#define CHECK_INT(actual, expected)                                                         \
  {                                                                                         \
    if (CheckInt(__LINE__, #actual " != " #expected, (actual), (expected)) != EXIT_SUCCESS) \
    {                                                                                       \
      return EXIT_FAILURE;                                                                  \
    }                                                                                       \
  }

} // namespace

// SegmentationCore includes
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkSegment.h"
#include "vtkSegmentation.h"
#include "vtkSegmentationConverterFactory.h"
#include "vtkSegmentationHistory.h"

int CreateCubeLabelmap(vtkOrientedImageData* imageData, int extent[6]);
void SetReferenceGeometry(vtkSegmentation*);

//----------------------------------------------------------------------------
int GetVoxelCount(vtkImageData* labelmap, int labelValue)
{
  vtkNew<vtkImageThreshold> threshold;
  threshold->SetInputData(labelmap);
  threshold->SetInValue(1);
  threshold->SetOutValue(0);
  threshold->ThresholdBetween(labelValue, labelValue);
  vtkNew<vtkImageAccumulate> accumulate;
  accumulate->SetInputConnection(threshold->GetOutputPort());
  accumulate->IgnoreZeroOn();
  accumulate->Update();
  return accumulate->GetVoxelCount();
}

//----------------------------------------------------------------------------
int vtkSegmentationHistoryTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Register converter rules
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New());
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New());

  // Create some segmentation content

  int segment1LabelValue = 1;
  int segment2LabelValue = 2;

  vtkNew<vtkSegment> segment1;
  segment1->SetName("Segment_1");
  segment1->SetLabelValue(segment1LabelValue);
  vtkNew<vtkSegment> segment2;
  segment2->SetName("Segment_2");
  segment2->SetLabelValue(segment2LabelValue);

  vtkNew<vtkOrientedImageData> labelmap;
  segment1->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), labelmap);
  segment2->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), labelmap);

  vtkNew<vtkSegmentation> segmentation;
  segmentation->AddSegment(segment1);
  segmentation->AddSegment(segment2);

  // Initialize segmentation history

  vtkNew<vtkSegmentationHistory> history;
  history->SetSegmentation(segmentation);

  CHECK_INT(history->GetNumberOfStates(), 0);

  // Make a change in the segmentation and save the state.
  // Check that a state is indeed saved.

  int segmentExtent[6] = { 0, 25, 0, 25, 0, 25 };
  CreateCubeLabelmap(labelmap, segmentExtent);
  history->SaveState();
  CHECK_INT(history->GetNumberOfStates(), 1);

  int originalSegment1VoxelCount = GetVoxelCount(labelmap, segment1LabelValue);
  int originalSegment2VoxelCount = GetVoxelCount(labelmap, segment2LabelValue);

  // Segmentation state is already saved, check that it does not create a new state
  // (it would be the duplicate of the previous state)
  history->SaveState();
  CHECK_INT(history->GetNumberOfStates(), 1);

  // Make a change in the segmentation.

  int modifierExtent[6] = { 5, 10, 5, 15, 15, 20 };
  vtkNew<vtkOrientedImageData> modifierLabelmap;
  CreateCubeLabelmap(modifierLabelmap, modifierExtent);
  vtkOrientedImageDataResample::ModifyImage(
    labelmap, modifierLabelmap, vtkOrientedImageDataResample::OPERATION_MASKING, nullptr, 0.0, 2.0);
  CHECK_INT(history->GetNumberOfStates(), 1);

  int modfiedSegment1VoxelCount = GetVoxelCount(labelmap, segment1LabelValue);
  int modfiedSegment2VoxelCount = GetVoxelCount(labelmap, segment2LabelValue);
  if (modfiedSegment1VoxelCount == originalSegment1VoxelCount)
  {
    std::cerr << "Segment 1 original voxel count (" << originalSegment2VoxelCount << ") and modified voxel count ("
              << modfiedSegment1VoxelCount << ") should not match!" << std::endl;
    return EXIT_FAILURE;
  }

  if (modfiedSegment2VoxelCount == originalSegment2VoxelCount)
  {
    std::cerr << "Segment 2 original voxel count (" << originalSegment2VoxelCount << ") and modified voxel count ("
              << modfiedSegment2VoxelCount << ") should not match!" << std::endl;
    return EXIT_FAILURE;
  }

  /////////////////////////////////////////////////
  // Test undo
  // Voxel count should be the same as the original
  /////////////////////////////////////////////////
  history->RestorePreviousState();

  // Restoring the previous state saves the current state (to allow restoring next state)
  CHECK_INT(history->GetNumberOfStates(), 2);

  vtkOrientedImageData* undoLabelmap = vtkOrientedImageData::SafeDownCast(
    segment1->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()));

  int undoSegment1VoxelCount = GetVoxelCount(undoLabelmap, segment1LabelValue);
  if (undoSegment1VoxelCount != originalSegment1VoxelCount)
  {
    std::cerr << "Segment 1 original voxel count (" << originalSegment1VoxelCount << ") and undo voxel count ("
              << undoSegment1VoxelCount << ") does not match!" << std::endl;
    return EXIT_FAILURE;
  }

  int undoSegment2VoxelCount = GetVoxelCount(undoLabelmap, segment2LabelValue);
  if (undoSegment2VoxelCount != originalSegment2VoxelCount)
  {
    std::cerr << "Segment 2 original voxel count (" << originalSegment2VoxelCount << ") and undo voxel count ("
              << undoSegment2VoxelCount << ") does not match!" << std::endl;
    return EXIT_FAILURE;
  }

  // Segmentation state is already saved, check that it does not create a new state
  // (it would be the duplicate of the previous state) and does not remove future states.
  history->SaveState();
  CHECK_INT(history->GetNumberOfStates(), 2);

  /////////////////////////////////////////////////
  // Test redo
  // Voxel count should be the same as the modified
  /////////////////////////////////////////////////

  history->RestoreNextState();
  CHECK_INT(history->GetNumberOfStates(), 2);

  vtkOrientedImageData* redoLabelmap = vtkOrientedImageData::SafeDownCast(
    segment1->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()));
  int redoSegment1VoxelCount = GetVoxelCount(redoLabelmap, segment1LabelValue);
  if (redoSegment1VoxelCount != modfiedSegment1VoxelCount)
  {
    std::cerr << "Segment 1 modified voxel count (" << modfiedSegment1VoxelCount << ") and redo voxel count ("
              << redoSegment1VoxelCount << ") does not match!" << std::endl;
    return EXIT_FAILURE;
  }

  int redoSegment2VoxelCount = GetVoxelCount(redoLabelmap, segment2LabelValue);
  if (redoSegment2VoxelCount != modfiedSegment2VoxelCount)
  {
    std::cerr << "Segment 2 modified voxel count (" << modfiedSegment2VoxelCount << ") and redo voxel count ("
              << redoSegment2VoxelCount << ") does not match!" << std::endl;
    return EXIT_FAILURE;
  }

  // Add two more states to have some more items in the history
  history->SaveState();
  vtkOrientedImageData::SafeDownCast(
    segment1->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()))
    ->Modified();
  CHECK_INT(history->GetNumberOfStates(), 2);

  history->SaveState();
  vtkOrientedImageData::SafeDownCast(
    segment1->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()))
    ->Modified();
  CHECK_INT(history->GetNumberOfStates(), 3);

  history->RestorePreviousState();
  // current state was automatically saved, and last restored state is 2
  CHECK_INT(history->GetNumberOfStates(), 4);

  history->RestorePreviousState();
  // restoring previous state does not erase next states (so still 4 states), but last restored state is 1
  CHECK_INT(history->GetNumberOfStates(), 4);

  history->SaveState();
  vtkOrientedImageData::SafeDownCast(
    segment1->GetRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()))
    ->Modified();

  // when segmentation is modified, future states are removed, so we have only the two saved states
  CHECK_INT(history->GetNumberOfStates(), 2);

  history->RestorePreviousState();
  // restoring previous state saves the current modified state
  CHECK_INT(history->GetNumberOfStates(), 3);

  std::cout << "Segmentation history test 1 passed." << std::endl;
  return EXIT_SUCCESS;
}
