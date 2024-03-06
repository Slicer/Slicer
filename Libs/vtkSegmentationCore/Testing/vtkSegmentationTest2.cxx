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
#include <vtkNew.h>
#include <vtkVersion.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkMatrix4x4.h>
#include <vtkImageAccumulate.h>

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkSegment.h"
#include "vtkSegmentationConverter.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkSegmentationConverterFactory.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"

void CreateSpherePolyData(vtkPolyData* polyData, double center[3], double radius);
int CreateCubeLabelmap(vtkOrientedImageData* imageData, int extent[6]);

void SetReferenceGeometry(vtkSegmentation*);

bool TestSharedLabelmapConversion()
{
  // Generate sphere models
  vtkNew<vtkPolyData> spherePolyData1;
  double sphereCenter1[3] = { 0,0,0 };
  double sphereRadius1 = 1;
  CreateSpherePolyData(spherePolyData1.GetPointer(), sphereCenter1, sphereRadius1);

  vtkNew<vtkPolyData> spherePolyData2;
  double sphereCenter2[3] = { -1,-1,-1 };
  double sphereRadius2 = 2;
  CreateSpherePolyData(spherePolyData2.GetPointer(), sphereCenter2, sphereRadius2);

  vtkNew<vtkPolyData> spherePolyData3;
  double sphereCenter3[3] = { 5, 5,5 };
  double sphereRadius3 = 2;
  CreateSpherePolyData(spherePolyData3.GetPointer(), sphereCenter3, sphereRadius3);

  // Create segments
  vtkNew<vtkSegment> sphereSegment1;
  sphereSegment1->SetName("sphere1");
  sphereSegment1->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), spherePolyData1.GetPointer());

  vtkNew<vtkSegment> sphereSegment2;
  sphereSegment2->SetName("sphere2");
  sphereSegment2->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), spherePolyData2.GetPointer());

  vtkNew<vtkSegment> sphereSegment3;
  sphereSegment3->SetName("sphere3");
  sphereSegment3->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), spherePolyData3.GetPointer());

  vtkNew<vtkSegmentation> segmentation;
  segmentation->SetSourceRepresentationName(vtkSegmentationConverter::GetClosedSurfaceRepresentationName());
  segmentation->AddSegment(sphereSegment1);
  segmentation->AddSegment(sphereSegment2);
  segmentation->AddSegment(sphereSegment3);
  SetReferenceGeometry(segmentation);

  int numClosedSurfaceLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetClosedSurfaceRepresentationName());
  if (numClosedSurfaceLayers != 3)
  {
    std::cerr << __LINE__ << ": Invalid number of closed surface layers " << numClosedSurfaceLayers
      << " should be 3" << std::endl;
    return false;
  }

  segmentation->CreateRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());

  int numBinaryLabelmapLayers = segmentation->GetNumberOfLayers(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
  if (numBinaryLabelmapLayers != 2)
  {
    std::cerr << __LINE__ << ": Invalid number of binary labelmap layers " << numBinaryLabelmapLayers
      << " should be 2" << std::endl;
    return false;
  }

  std::vector<std::string> sharedSegmentIDs;
  segmentation->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentation->GetSegmentIdBySegment(sphereSegment1), sharedSegmentIDs, true);
  if (sharedSegmentIDs.size() != 2)
  {
    std::cerr << __LINE__ << ": Invalid number of shared labelmaps for segment " << sphereSegment1->GetName()
      << ": " << sharedSegmentIDs.size() << " should be 2" << std::endl;
    return false;
  }

  segmentation->GetSegmentIDsSharingBinaryLabelmapRepresentation(segmentation->GetSegmentIdBySegment(sphereSegment2), sharedSegmentIDs, true);
  if (sharedSegmentIDs.size() != 1)
  {
    std::cerr << __LINE__ << ": Invalid number of shared labelmaps for segment " << sphereSegment2->GetName()
      << ": " << sharedSegmentIDs.size() << " should be 1" << std::endl;
    return false;
  }

  return true;
}

bool TestSharedLabelmapCollapse()
{
  vtkNew<vtkOrientedImageData> cubeImage1;
  int extent1[6] = { 0, 2, 0, 2, 0, 2 };
  int imageCount1 = CreateCubeLabelmap(cubeImage1, extent1);

  vtkNew<vtkOrientedImageData> cubeImage2;
  int extent2[6] = { -2, 2, -2, 2, -2, 2 };
  int imageCount2 = CreateCubeLabelmap(cubeImage2, extent2);

  vtkNew<vtkOrientedImageData> cubeImage3;
  int extent3[6] = { 0, 2, 0, 2, 0, 2 };
  int imageCount3 = CreateCubeLabelmap(cubeImage3, extent3);

  vtkNew<vtkOrientedImageData> cubeImage4;
  int extent4[6] = { 3, 5, 3, 5, 3, 5 };
  int imageCount4 = CreateCubeLabelmap(cubeImage4, extent4);

  vtkNew<vtkSegment> segment1;
  segment1->SetName("cube1");
  segment1->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), cubeImage1);

  vtkNew<vtkSegment> segment2;
  segment2->SetName("cube2");
  segment2->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), cubeImage2);

  vtkNew<vtkSegment> segment3;
  segment3->SetName("cube3");
  segment3->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), cubeImage3);

  vtkNew<vtkSegment> segment4;
  segment4->SetName("cube4");
  segment4->AddRepresentation(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName(), cubeImage4);

  std::vector<vtkSegment*> segments =
    {
    segment1,
    segment2,
    segment3,
    segment4,
    };

  vtkNew<vtkSegmentation> segmentation;
  segmentation->SetSourceRepresentationName(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
  segmentation->AddSegment(segment1);
  segmentation->AddSegment(segment2);
  segmentation->AddSegment(segment3);
  segmentation->AddSegment(segment4);

  int numberOfLayers = 0;

  numberOfLayers = segmentation->GetNumberOfLayers();
  if (numberOfLayers != 4)
  {
    std::cerr << "Invalid number of layers " << numberOfLayers << " should be 4" << std::endl;
    return false;
  }

  /////////////////////////////////////
  segmentation->CollapseBinaryLabelmaps(false); // Merge to multiple layers

  numberOfLayers = segmentation->GetNumberOfLayers();
  if (numberOfLayers != 3)
  {
    std::cerr << "Safe merge failed: Invalid number of layers " << numberOfLayers << " should be 3" << std::endl;
    return false;
  }

  vtkNew<vtkImageAccumulate> imageAccumulate;
  double frequency = 0.0;

  std::vector<int> expectedResults =
    {
    imageCount1,
    imageCount2,
    imageCount3,
    imageCount4,
    };
  for (size_t i = 0; i < segments.size(); ++i)
  {
    vtkSegment* segment = segments[i];
    int expectedFrequency = expectedResults[i];

    vtkOrientedImageData* segmentLabelmap = vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(
      vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()));
    double labelValue = segment->GetLabelValue();
    imageAccumulate->SetInputData(segmentLabelmap);
    imageAccumulate->Update();
    frequency = imageAccumulate->GetOutput()->GetPointData()->GetScalars()->GetTuple1((vtkIdType)labelValue);
    if (frequency != expectedFrequency)
    {
      std::cerr << "Invalid number of voxels in " << segment->GetName() << " " << frequency << " should be " << expectedFrequency << std::endl;
      return false;
    }
  }

  /////////////////////////////////////
  segmentation->CollapseBinaryLabelmaps(true); // Overwrite merge

  numberOfLayers = segmentation->GetNumberOfLayers();
  if (numberOfLayers != 1)
  {
    std::cerr << "Overwrite merge failed: Invalid number of layers " << numberOfLayers << " should be 1" << std::endl;
    return false;
  }


  expectedResults =
  {
    0,
    imageCount2 - imageCount3,
    imageCount3,
    imageCount4,
  };
  for (size_t i = 0; i < segments.size(); ++i)
  {
    vtkSegment* segment = segments[i];
    int expectedFrequency = expectedResults[i];

    vtkOrientedImageData* segmentLabelmap = vtkOrientedImageData::SafeDownCast(segment->GetRepresentation(
      vtkSegmentationConverter::GetBinaryLabelmapRepresentationName()));
    double labelValue = segment->GetLabelValue();
    imageAccumulate->SetInputData(segmentLabelmap);
    imageAccumulate->Update();
    frequency = imageAccumulate->GetOutput()->GetPointData()->GetScalars()->GetTuple1((vtkIdType)labelValue);
    if (frequency != expectedFrequency)
    {
      std::cerr << "Invalid number of voxels in " << segment->GetName() << " " << frequency << " should be " << expectedFrequency << std::endl;
      return false;
    }
  }

  /////////////////////////////////////
  segmentation->SeparateSegmentLabelmap(segmentation->GetSegmentIdBySegment(segment1));

  numberOfLayers = segmentation->GetNumberOfLayers();
  if (numberOfLayers != 2)
  {
    std::cerr << "Separate segment labelmap failed: Invalid number of layers " << numberOfLayers << " should be 2" << std::endl;
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool TestSharedLabelmapCasting()
{
  vtkNew<vtkOrientedImageData> labelmap;
  labelmap->SetDimensions(1, 1, 1);
  labelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  vtkNew<vtkSegment> segment;
  segment->SetName("Segment");
  segment->AddRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(), labelmap);

  vtkNew<vtkSegmentation> segmentation;
  segmentation->AddSegment(segment);

  if (labelmap->GetScalarType() != VTK_UNSIGNED_CHAR)
  {
    std::cerr << "Invalid scalar type " << labelmap->GetScalarType() << " should be " << VTK_UNSIGNED_CHAR << std::endl;
    return false;
  }

  for (int i = 0; i < 254; ++i)
  {
    segmentation->AddEmptySegment();
  }

  if (labelmap->GetScalarType() != VTK_UNSIGNED_CHAR)
  {
    std::cerr << "Invalid scalar type " << labelmap->GetScalarType() << " should be " << VTK_UNSIGNED_CHAR << std::endl;
    return false;
  }

  segmentation->AddEmptySegment();
  if (labelmap->GetScalarType() != VTK_UNSIGNED_SHORT)
  {
    std::cerr << "Invalid scalar type " << labelmap->GetScalarType() << " should be " << VTK_UNSIGNED_SHORT << std::endl;
    return false;
  }

  vtkOrientedImageDataResample::CastImageForValue(labelmap, VTK_UNSIGNED_SHORT_MAX + 1);
  if (labelmap->GetScalarType() != VTK_UNSIGNED_INT)
  {
    std::cerr << "Invalid scalar type " << labelmap->GetScalarType() << " should be " << VTK_UNSIGNED_INT << std::endl;
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool TestLabelmapFloatToIntegerConversion(int scalarType)
{

  int numberOfValues = 5;
  double values[5] = { -2.0, -1.0, 0.0, 1.0, 2.0 };
  int numberOfOffsets = 7;
  double offsets[7] = { -0.3, -0.1, -0.01, 0.0, 0.01, 0.1, 0.3 };

  vtkNew<vtkOrientedImageData> labelmap;
  labelmap->SetDimensions(numberOfValues, numberOfOffsets, 1);
  labelmap->AllocateScalars(scalarType, 1);
  for (int i = 0; i < numberOfValues; ++i)
  {
    for (int j = 0; j < numberOfOffsets; ++j)
    {
      labelmap->SetScalarComponentFromDouble(i, j, 0, 0, values[i] + offsets[j]);
    }
  }

  if (!vtkOrientedImageDataResample::CastSegmentationToSmallestIntegerType(labelmap))
  {
    std::cerr << "Unable to convert labelmap of type: " << vtkImageScalarTypeNameMacro(labelmap->GetScalarType()) << " to integer type" << std::endl;
    return false;
  }

  for (int i = 0; i < numberOfValues; ++i)
  {
    for (int j = 0; j < numberOfOffsets; ++j)
    {
      double value = labelmap->GetScalarComponentAsDouble(i, j, 0, 0);
      double expectedValue = std::trunc(values[i] + offsets[j]); // Casting from float to integer we expect truncation
      if (value != expectedValue)
      {
        std::cerr << "Invalid value at index " << i << ", " << j << std::endl;
        std::cerr << "\tOriginal value:" << values[i] << std::endl;
        std::cerr << "\tOffset:" << offsets[j] << std::endl;
        std::cerr << "\tScalar value: " << value << std::endl;
        std::cerr << "\Expected value: " << expectedValue << std::endl;
        std::cerr << "\tOriginal type: " << vtkImageScalarTypeNameMacro(scalarType) << std::endl;
        std::cerr << "\tInteger type: " << vtkImageScalarTypeNameMacro(labelmap->GetScalarType()) << std::endl;
        return false;
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------
bool TestLabelmapValidation()
{
  vtkNew<vtkOrientedImageData> labelmap;
  labelmap->SetDimensions(1, 1, 1);

  int ret = vtkOrientedImageDataResample::TYPE_ERROR;

  labelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  labelmap->SetScalarComponentFromDouble(0, 0, 0, 0, 0.0);
  ret = vtkOrientedImageDataResample::IsImageScalarTypeValid(labelmap);
  if (ret != vtkOrientedImageDataResample::TYPE_OK)
  {
    std::cerr << "Invalid return value " << ret << " should be " << vtkOrientedImageDataResample::TYPE_OK << std::endl;
    return false;
  }

  labelmap->AllocateScalars(VTK_LONG, 1);
  labelmap->SetScalarComponentFromDouble(0, 0, 0, 0, 0.0);
  ret = vtkOrientedImageDataResample::IsImageScalarTypeValid(labelmap);
    if (ret != vtkOrientedImageDataResample::TYPE_CONVERSION_NEEDED)
  {
    std::cerr << "Invalid return value " << ret << " should be " << vtkOrientedImageDataResample::TYPE_CONVERSION_NEEDED << std::endl;
    return false;
  }

  labelmap->AllocateScalars(VTK_DOUBLE, 1);
  labelmap->SetScalarComponentFromDouble(0, 0, 0, 0, VTK_DOUBLE_MAX);
  ret = vtkOrientedImageDataResample::IsImageScalarTypeValid(labelmap);
  if (ret != vtkOrientedImageDataResample::TYPE_CONVERSION_CLAMPING_NEEDED)
  {
    std::cerr << "Invalid return value " << ret << " should be " << vtkOrientedImageDataResample::TYPE_CONVERSION_CLAMPING_NEEDED << std::endl;
    return false;
  }

  labelmap->AllocateScalars(VTK_FLOAT, 1);
  labelmap->SetScalarComponentFromDouble(0, 0, 0, 0, 0.0);
  ret = vtkOrientedImageDataResample::IsImageScalarTypeValid(labelmap);
  if (ret != vtkOrientedImageDataResample::TYPE_CONVERSION_TRUNCATION_NEEDED)
  {
    std::cerr << "Invalid return value " << ret << " should be " << vtkOrientedImageDataResample::TYPE_CONVERSION_TRUNCATION_NEEDED << std::endl;
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
int vtkSegmentationTest2(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Register converter rules
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New() );
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New() );

  if (!TestSharedLabelmapConversion())
  {
    return EXIT_FAILURE;
  }

  if (!TestSharedLabelmapCollapse())
  {
    return EXIT_FAILURE;
  }

  if (!TestSharedLabelmapCasting())
  {
    return EXIT_FAILURE;
  }

  if (!TestLabelmapFloatToIntegerConversion(VTK_FLOAT))
  {
    return EXIT_FAILURE;
  }

  if (!TestLabelmapFloatToIntegerConversion(VTK_DOUBLE))
  {
    return EXIT_FAILURE;
  }

  if (!TestLabelmapValidation())
  {
    return EXIT_FAILURE;
  }

  std::cout << "Segmentation test 2 passed." << std::endl;
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
void SetReferenceGeometry(vtkSegmentation* segmentation)
{
  //// Create non-default reference geometry for conversion
  vtkNew<vtkMatrix4x4> referenceGeometryMatrix;
  referenceGeometryMatrix->Identity();
  referenceGeometryMatrix->SetElement(0, 0, 0.1);
  referenceGeometryMatrix->SetElement(1, 1, 0.1);
  referenceGeometryMatrix->SetElement(2, 2, 0.1);
  int referenceGeometryExtent[6] = { -50,50,-50,50,-50,50 };
  std::string referenceGeometryString = vtkSegmentationConverter::SerializeImageGeometry(referenceGeometryMatrix.GetPointer(), referenceGeometryExtent);
  segmentation->SetConversionParameter(
    vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), referenceGeometryString);
}

//----------------------------------------------------------------------------
void CreateSpherePolyData(vtkPolyData* polyData, double center[3], double radius)
{
  vtkNew<vtkSphereSource> sphere;
  sphere->SetCenter(center);
  sphere->SetRadius(radius);
  sphere->Update();
  polyData->ShallowCopy(sphere->GetOutput());
}

//----------------------------------------------------------------------------
int CreateCubeLabelmap(vtkOrientedImageData* imageData, int extent[6])
{
  // Create new one because by default the direction is identity, origin is zeros and spacing is ones
  vtkNew<vtkOrientedImageData> identityImageData;
  imageData->SetExtent(extent);
  imageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  unsigned char* imagePtr = (unsigned char*)imageData->GetScalarPointer();
  int size = (extent[1] - extent[0] + 1) * (extent[3] - extent[2] + 1) * (extent[5] - extent[4] + 1);
  memset(imagePtr, 1, ( size * imageData->GetScalarSize() * imageData->GetNumberOfScalarComponents()));
  return size;
}
