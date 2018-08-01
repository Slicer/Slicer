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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// VTK includes
#include <vtkImageAccumulate.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>

// vtkSegmentationCore includes
#include "vtkClosedSurfaceToFractionalLabelmapConversionRule.h"
#include <vtkOrientedImageData.h>
#include <vtkPolyDataToFractionalLabelmapFilter.h>
#include <vtkSegment.h>
#include <vtkSegmentation.h>
#include <vtkSegmentationConverter.h>
#include <vtkSegmentationConverterFactory.h>

void CreateSpherePolyData(vtkPolyData* polyData);

//----------------------------------------------------------------------------
int vtkClosedSurfaceToFractionalLabelMapConversionTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Register converter rules
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkClosedSurfaceToFractionalLabelmapConversionRule>::New() );

  // Generate sphere model
  vtkNew<vtkPolyData> spherePolyData;
  CreateSpherePolyData(spherePolyData.GetPointer());

  // Create segment
  vtkNew<vtkSegment> sphereSegment;
  sphereSegment->SetName("sphere1");
  sphereSegment->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), spherePolyData.GetPointer());

  // Image geometry used for conversion
  std::string serializedImageGeometry = "1; 0; 0; 20.7521629333496;"
                                        "0; 1; 0; 20.7521629333496;"
                                        "0; 0; 1; 20;"
                                        "0; 0; 0;  1;"
                                        "0; 59; 0; 59; 0; 60;";

  // Create segmentation with segment
  vtkNew<vtkSegmentation> sphereSegmentation;
  sphereSegmentation->SetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), serializedImageGeometry);
  sphereSegmentation->SetMasterRepresentationName(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  sphereSegmentation->AddSegment(sphereSegment.GetPointer());

  sphereSegmentation->CreateRepresentation(vtkSegmentationConverter::GetSegmentationFractionalLabelmapRepresentationName());
  if (!sphereSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationFractionalLabelmapRepresentationName()))
  {
    std::cerr << __LINE__ << ": Failed to add fractional labelmap representation to segment!" << std::endl;
    return EXIT_FAILURE;
  }

  vtkOrientedImageData* fractionalLabelmap = vtkOrientedImageData::SafeDownCast(
    sphereSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationFractionalLabelmapRepresentationName()) );

  vtkNew<vtkImageAccumulate> imageAccumulate;
  imageAccumulate->SetInputData(fractionalLabelmap);
  imageAccumulate->Update();

  int expectedVoxelCount = 234484;
  int voxelCount = imageAccumulate->GetVoxelCount();
  if (voxelCount != expectedVoxelCount)
  {
    std::cerr << __LINE__ << ": Fractional voxel count: " << +voxelCount <<
      " does not match expected value: " << std::fixed << +expectedVoxelCount <<  "!" << std::endl;
    return EXIT_FAILURE;
  }

  FRACTIONAL_DATA_TYPE maxValue = imageAccumulate->GetMax()[0];
  int expectedMaxValue = FRACTIONAL_MAX;
  if (maxValue != expectedMaxValue)
  {
    std::cerr << __LINE__ << ": Fractional max: " << +maxValue <<
      " does not match expected value: " << std::fixed << +expectedMaxValue <<  "!" << std::endl;
    return EXIT_FAILURE;
  }

  FRACTIONAL_DATA_TYPE  minValue = imageAccumulate->GetMin()[0];
  int expectedMinValue = FRACTIONAL_MIN;
  if (minValue != expectedMinValue)
  {
    std::cerr << __LINE__ << ": Fractional min: " << +minValue <<
      " does not match expected value: " << std::fixed << +expectedMinValue <<  "!" << std::endl;
    return EXIT_FAILURE;
  }

  double expectedMeanValue = 0;
  if (VTK_FRACTIONAL_DATA_TYPE == VTK_CHAR)
  {
    // Average signed value
    expectedMeanValue = -18.846224;
  }
  else if (VTK_FRACTIONAL_DATA_TYPE == VTK_UNSIGNED_CHAR)
  {
    // Average unsigned value
    expectedMeanValue = 89.153776;
  }
  else
  {
    std::cerr << __LINE__ << ": Fractional datatype: " << std::fixed << VTK_FRACTIONAL_DATA_TYPE <<
      " is not a supported datatype:!" << std::endl;
    return EXIT_FAILURE;
  }
  double meanValue = imageAccumulate->GetMean()[0];
  if ( std::abs(meanValue - expectedMeanValue) > 0.00001)
  {
    std::cerr << __LINE__ << ": Fractional mean: " << std::fixed << meanValue <<
      " does not match expected value: " << std::fixed << expectedMeanValue <<  "!" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Closed surface to fractional labelmap conversion test passed." << std::endl;
  return EXIT_SUCCESS;
}
