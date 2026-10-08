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

==============================================================================*/

// SegmentationCore includes
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkTopologicalHierarchy.h"

// VTK includes
#include <vtkCubeSource.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataCollection.h>

// STD includes
#include <iostream>

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
int CheckExtent(int line, vtkIntArray* labelValues, vtkIntArray* labelExtents, int label, const int expectedExtent[6])
{
  vtkIdType index = labelValues->LookupValue(label);
  if (index < 0)
  {
    std::cerr << "\nLine " << line << " - label " << label << " not found" << std::endl;
    return EXIT_FAILURE;
  }
  int extent[6] = { 0, -1, 0, -1, 0, -1 };
  labelExtents->GetTypedTuple(index, extent);
  for (int i = 0; i < 6; ++i)
  {
    if (extent[i] != expectedExtent[i])
    {
      std::cerr << "\nLine " << line << " - extent of label " << label << " differs at index " << i //
                << "\n\tcurrent :" << extent[i] << "\n\texpected:" << expectedExtent[i] << std::endl;
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

#define CHECK_EXTENT(labelValues, labelExtents, label, expectedExtent)                                   \
  {                                                                                                      \
    if (CheckExtent(__LINE__, (labelValues), (labelExtents), (label), (expectedExtent)) != EXIT_SUCCESS) \
    {                                                                                                    \
      return EXIT_FAILURE;                                                                               \
    }                                                                                                    \
  }

//----------------------------------------------------------------------------
int TestEffectiveExtentPerLabel()
{
  // Labelmap with a large box (label 1), a smaller box inside it (label 2) that overwrites part of label 1,
  // and a separate box (label 3). Label 4 is not present.
  vtkNew<vtkOrientedImageData> labelmap;
  labelmap->SetExtent(0, 29, 0, 19, 0, 9);
  labelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  vtkOrientedImageDataResample::FillImage(labelmap, 0);
  const int box1[6] = { 2, 15, 3, 16, 1, 8 };
  const int box2[6] = { 5, 10, 6, 12, 3, 6 };
  const int box3[6] = { 20, 28, 0, 4, 0, 0 };
  FillBox(labelmap, box1, 1);
  FillBox(labelmap, box2, 2);
  FillBox(labelmap, box3, 3);

  vtkNew<vtkIntArray> labelValues;
  vtkNew<vtkIntArray> labelExtents;
  CHECK_INT(vtkOrientedImageDataResample::CalculateEffectiveExtentPerLabel(labelmap, labelValues, labelExtents), true);
  CHECK_INT(static_cast<int>(labelValues->GetNumberOfValues()), 3);
  CHECK_INT(labelExtents->GetNumberOfComponents(), 6);
  CHECK_INT(static_cast<int>(labelExtents->GetNumberOfTuples()), 3);
  // Labels are in increasing order
  CHECK_INT(labelValues->GetValue(0), 1);
  CHECK_INT(labelValues->GetValue(1), 2);
  CHECK_INT(labelValues->GetValue(2), 3);
  CHECK_EXTENT(labelValues, labelExtents, 1, box1);
  CHECK_EXTENT(labelValues, labelExtents, 2, box2);
  CHECK_EXTENT(labelValues, labelExtents, 3, box3);
  CHECK_INT(static_cast<int>(labelValues->LookupValue(4)), -1);

  // The whole-image effective extent is the union of all labels
  int effectiveExtent[6] = { 0, -1, 0, -1, 0, -1 };
  CHECK_INT(vtkOrientedImageDataResample::CalculateEffectiveExtent(labelmap, effectiveExtent), true);
  const int unionExtent[6] = { 2, 28, 0, 16, 0, 8 };
  for (int i = 0; i < 6; ++i)
  {
    CHECK_INT(effectiveExtent[i], unionExtent[i]);
  }

  // Empty and invalid inputs
  vtkNew<vtkOrientedImageData> emptyLabelmap;
  emptyLabelmap->SetExtent(0, 4, 0, 4, 0, 4);
  emptyLabelmap->AllocateScalars(VTK_SHORT, 1);
  vtkOrientedImageDataResample::FillImage(emptyLabelmap, 0);
  CHECK_INT(vtkOrientedImageDataResample::CalculateEffectiveExtentPerLabel(emptyLabelmap, labelValues, labelExtents), true);
  CHECK_INT(static_cast<int>(labelValues->GetNumberOfValues()), 0);
  CHECK_INT(static_cast<int>(labelExtents->GetNumberOfTuples()), 0);
  CHECK_INT(vtkOrientedImageDataResample::CalculateEffectiveExtentPerLabel(nullptr, labelValues, labelExtents), false);
  CHECK_INT(static_cast<int>(labelValues->GetNumberOfValues()), 0);

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestHierarchyFromBounds()
{
  // Bounding boxes: 0 contains 1 and 2, 1 contains 2, 3 is separate, 4 is empty (invalid bounds)
  vtkNew<vtkDoubleArray> bounds;
  bounds->SetNumberOfComponents(6);
  bounds->InsertNextTuple6(0.0, 100.0, 0.0, 100.0, 0.0, 100.0);
  bounds->InsertNextTuple6(10.0, 90.0, 10.0, 90.0, 10.0, 90.0);
  bounds->InsertNextTuple6(40.0, 60.0, 40.0, 60.0, 40.0, 60.0);
  bounds->InsertNextTuple6(200.0, 300.0, 0.0, 100.0, 0.0, 100.0);
  bounds->InsertNextTuple6(1.0, -1.0, 1.0, -1.0, 1.0, -1.0);

  vtkNew<vtkTopologicalHierarchy> hierarchy;
  hierarchy->SetInputBounds(bounds);
  hierarchy->Update();
  vtkIntArray* levels = hierarchy->GetOutputLevels();
  CHECK_INT(static_cast<int>(levels->GetNumberOfTuples()), 5);
  CHECK_INT(levels->GetValue(0), 2);
  CHECK_INT(levels->GetValue(1), 1);
  CHECK_INT(levels->GetValue(2), 0);
  CHECK_INT(levels->GetValue(3), 0);
  CHECK_INT(levels->GetValue(4), 0);

  // Same hierarchy from poly data gives the same levels
  vtkNew<vtkPolyDataCollection> polyDataCollection;
  for (vtkIdType index = 0; index < 4; ++index)
  {
    double* itemBounds = bounds->GetTuple6(index);
    vtkNew<vtkCubeSource> cube;
    cube->SetBounds(itemBounds);
    cube->Update();
    polyDataCollection->AddItem(cube->GetOutput());
  }
  vtkNew<vtkTopologicalHierarchy> polyDataHierarchy;
  polyDataHierarchy->SetInputPolyDataCollection(polyDataCollection);
  polyDataHierarchy->Update();
  vtkIntArray* polyDataLevels = polyDataHierarchy->GetOutputLevels();
  CHECK_INT(static_cast<int>(polyDataLevels->GetNumberOfTuples()), 4);
  for (vtkIdType index = 0; index < 4; ++index)
  {
    CHECK_INT(polyDataLevels->GetValue(index), levels->GetValue(index));
  }

  // Touching bounds do not contain each other
  vtkNew<vtkDoubleArray> touchingBounds;
  touchingBounds->SetNumberOfComponents(6);
  touchingBounds->InsertNextTuple6(0.0, 100.0, 0.0, 100.0, 0.0, 100.0);
  touchingBounds->InsertNextTuple6(10.0, 100.0, 10.0, 90.0, 10.0, 90.0);
  hierarchy->SetInputBounds(touchingBounds);
  hierarchy->Update();
  CHECK_INT(hierarchy->GetOutputLevels()->GetValue(0), 0);
  CHECK_INT(hierarchy->GetOutputLevels()->GetValue(1), 0);

  return EXIT_SUCCESS;
}

} // namespace

//----------------------------------------------------------------------------
int vtkTopologicalHierarchyTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  if (TestEffectiveExtentPerLabel() != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }
  if (TestHierarchyFromBounds() != EXIT_SUCCESS)
  {
    return EXIT_FAILURE;
  }
  std::cout << "Test passed" << std::endl;
  return EXIT_SUCCESS;
}
