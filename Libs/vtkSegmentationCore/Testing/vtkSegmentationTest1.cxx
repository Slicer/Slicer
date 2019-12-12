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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// VTK includes
#include <vtkNew.h>
#include <vtkVersion.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkMatrix4x4.h>
#include <vtkImageAccumulate.h>

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkSegment.h"
#include "vtkSegmentationConverter.h"
#include "vtkOrientedImageData.h"
#include "vtkSegmentationConverterFactory.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"
#include "vtkClosedSurfaceToBinaryLabelmapConversionRule.h"

void CreateSpherePolyData(vtkPolyData* polyData);
void CreateCubeLabelmap(vtkOrientedImageData* imageData);

//----------------------------------------------------------------------------
int vtkSegmentationTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Register converter rules
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkBinaryLabelmapToClosedSurfaceConversionRule>::New() );
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkClosedSurfaceToBinaryLabelmapConversionRule>::New() );

  //////////////////////////////////////////////////////////////////////////
  // Create segmentation with one segment from model and test segment
  // operations and conversion to labelmap with and without reference geometry

  // Generate sphere model
  vtkNew<vtkPolyData> spherePolyData;
  CreateSpherePolyData(spherePolyData.GetPointer());

  // Create segment
  vtkNew<vtkSegment> sphereSegment;
  sphereSegment->SetName("sphere1");
  sphereSegment->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), spherePolyData.GetPointer());
  if (!sphereSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()))
    {
    std::cerr << __LINE__ << ": Failed to add closed surface representation to segment!" << std::endl;
    return EXIT_FAILURE;
    }

  // Create segmentation with segment
  vtkNew<vtkSegmentation> sphereSegmentation;
  sphereSegmentation->SetMasterRepresentationName(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName() );
  sphereSegmentation->AddSegment(sphereSegment.GetPointer());
  if (sphereSegmentation->GetNumberOfSegments() != 1)
    {
    std::cerr << __LINE__ << ": Failed to add segment to segmentation!" << std::endl;
    return EXIT_FAILURE;
    }

  // Convert to binary labelmap without reference geometry
  sphereSegmentation->CreateRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  vtkOrientedImageData* defaultImageData = vtkOrientedImageData::SafeDownCast(
    sphereSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
  if (!defaultImageData)
    {
    std::cerr << __LINE__ << ": Failed to convert closed surface representation to binary labelmap without reference geometry!" << std::endl;
    return EXIT_FAILURE;
    }
  std::string defaultGeometryString = vtkSegmentationConverter::SerializeImageGeometry(defaultImageData);
  std::string expectedDefaultGeometryString =
    "0.23597152210841135;0;0;20.75216293334961;"
    "0;0.23597152210841135;0;20.75216293334961;"
    "0;0;0.23597152210841135;20;"
    "0;0;0;1;"
    "0;248;0;248;-1;255;";
  if (defaultGeometryString != expectedDefaultGeometryString)
    {
    std::cerr << __LINE__ << ": Default reference geometry mismatch. Expected: "
      << expectedDefaultGeometryString << ". Actual: " << defaultGeometryString << "." << std::endl;
    return EXIT_FAILURE;
    }
  vtkNew<vtkImageAccumulate> imageAccumulate;
  imageAccumulate->SetInputData(defaultImageData);
  imageAccumulate->Update();
  if (imageAccumulate->GetMax()[0] != 1)
    {
    std::cerr << __LINE__ << ": Binary labelmap converted without reference geometry has no foreground voxels!" << std::endl;
    return EXIT_FAILURE;
    }
  int expectedVoxelCount = 15934257;
  if (imageAccumulate->GetVoxelCount() != expectedVoxelCount)
    {
    std::cerr << __LINE__ << ": Binary labelmap voxel count mismatch after converting without reference geometry."
      << " Expected: " << expectedVoxelCount << ". Actual: << " << imageAccumulate->GetVoxelCount() << "." << std::endl;
    return EXIT_FAILURE;
    }

  // Remove binary labelmap representation from segment
  sphereSegment->RemoveRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  if (sphereSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
    {
    std::cerr << __LINE__ << ": Failed to remove binary labelmap representation from segment!" << std::endl;
    return EXIT_FAILURE;
    }

  // Create non-default reference geometry for conversion
  vtkNew<vtkMatrix4x4> referenceGeometryMatrix;
  referenceGeometryMatrix->Identity();
  referenceGeometryMatrix->SetElement(0,0,2.0);
  referenceGeometryMatrix->SetElement(1,1,2.0);
  referenceGeometryMatrix->SetElement(2,2,2.0);
  int referenceGeometryExtent[6] = {0,99,0,99,0,99};
  std::string referenceGeometryString = vtkSegmentationConverter::SerializeImageGeometry(referenceGeometryMatrix.GetPointer(), referenceGeometryExtent);
  std::string expectedReferenceGeometryString = "2;0;0;0;0;2;0;0;0;0;2;0;0;0;0;1;0;99;0;99;0;99;";
  if (referenceGeometryString != expectedReferenceGeometryString)
    {
    std::cerr << __LINE__ << ": Failed to serialize reference geometry. Expected: "
      << expectedReferenceGeometryString << ". Actual: " << referenceGeometryString << "." << std::endl;
    return EXIT_FAILURE;
    }
  sphereSegmentation->SetConversionParameter(
    vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), referenceGeometryString );

  // Convert to binary labelmap with reference geometry
  sphereSegmentation->CreateRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  vtkOrientedImageData* customImageData = vtkOrientedImageData::SafeDownCast(
    sphereSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()) );
  if (!customImageData)
    {
    std::cerr << __LINE__ << ": Failed to convert closed surface representation to binary labelmap with custom reference geometry!" << std::endl;
    return EXIT_FAILURE;
    }
  imageAccumulate->SetInputData(customImageData);
  imageAccumulate->Update();
  if (imageAccumulate->GetMax()[0] != 1)
    {
    std::cerr << __LINE__ << ": Binary labelmap converted with custom reference geometry has no foreground voxels!" << std::endl;
    return EXIT_FAILURE;
    }
  if (imageAccumulate->GetVoxelCount() != 29791)
    {
    std::cerr << __LINE__ << ": Unexpected binary labelmap extent after converting with custom reference geometry!" << std::endl;
    return EXIT_FAILURE;
    }

  // Add second segment
  vtkNew<vtkPolyData> spherePolyData2;
  CreateSpherePolyData(spherePolyData2.GetPointer());
  vtkNew<vtkSegment> sphereSegment2;
  sphereSegment2->SetName("sphere2");
  sphereSegment2->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), spherePolyData2.GetPointer());
  if (!sphereSegment2->GetRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()))
    {
    std::cerr << __LINE__ << ": Failed to add closed surface representation to second segment!" << std::endl;
    return EXIT_FAILURE;
    }
  sphereSegmentation->AddSegment(sphereSegment2.GetPointer());
  if (sphereSegmentation->GetNumberOfSegments() != 2)
    {
    std::cerr << __LINE__ << ": Failed to add second segment to segmentation!" << std::endl;
    return EXIT_FAILURE;
    }
  if (!sphereSegment2->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
    {
    std::cerr << __LINE__ << ": Failed to auto-convert second segment to binary labelmap on adding it to segmentation!" << std::endl;
    return EXIT_FAILURE;
    }

  // Remove segment
  sphereSegmentation->RemoveSegment(sphereSegment2.GetPointer());
  if (sphereSegmentation->GetNumberOfSegments() != 1)
    {
    std::cerr << __LINE__ << ": Failed to remove second segment from segmentation!" << std::endl;
    return EXIT_FAILURE;
    }

  // Re-add segment
  sphereSegmentation->AddSegment(sphereSegment2.GetPointer());
  if (sphereSegmentation->GetNumberOfSegments() != 2)
    {
    std::cerr << __LINE__ << ": Failed to re-add second segment to segmentation!" << std::endl;
    return EXIT_FAILURE;
    }

  // Try to add segment with unsupported representation
  vtkNew<vtkPolyData> unsupportedPolyData;
  CreateSpherePolyData(unsupportedPolyData.GetPointer());
  vtkNew<vtkSegment> unsupportedSegment;
  unsupportedSegment->SetName("unsupported");
  unsupportedSegment->AddRepresentation("Unsupported", unsupportedPolyData.GetPointer());
  sphereSegmentation->AddSegment(unsupportedSegment.GetPointer());
  if (sphereSegmentation->GetNumberOfSegments() != 2)
    {
    std::cerr << __LINE__ << ": Unexpected outcome when adding segment containing unsupported representation to segmentation!" << std::endl;
    return EXIT_FAILURE;
    }

  //////////////////////////////////////////////////////////////////////////
  // Create segmentation with one segment from labelmap and test conversion
  // to closed surface model

  // Generate cube image data
  vtkNew<vtkOrientedImageData> cubeImageData;
  CreateCubeLabelmap(cubeImageData.GetPointer());

  // Create segment
  vtkNew<vtkSegment> cubeSegment;
  cubeSegment->SetName("cube");
  cubeSegment->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName(), cubeImageData.GetPointer());
  if (!cubeSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
    {
    std::cerr << __LINE__ << ": Failed to add binary labelmap representation to segment!" << std::endl;
    return EXIT_FAILURE;
    }

  // Create segmentation with segment
  vtkNew<vtkSegmentation> cubeSegmentation;
  cubeSegmentation->SetMasterRepresentationName(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName() );
  cubeSegmentation->AddSegment(cubeSegment.GetPointer());
  if (cubeSegmentation->GetNumberOfSegments() != 1)
    {
    std::cerr << __LINE__ << ": Failed to add segment to second segmentation!" << std::endl;
    return EXIT_FAILURE;
    }

  // Convert to closed surface model
  cubeSegmentation->CreateRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  vtkPolyData* closedSurfaceModel = vtkPolyData::SafeDownCast(
    cubeSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName()) );
  if (!closedSurfaceModel)
    {
    std::cerr << __LINE__ << ": Failed to convert binary labelmap representation to closed surface model!" << std::endl;
    return EXIT_FAILURE;
    }

  // Add segment with closed surface representation, see if it is converted to master
  vtkNew<vtkPolyData> nonMasterPolyData;
  CreateSpherePolyData(nonMasterPolyData.GetPointer());
  vtkNew<vtkSegment> nonMasterSegment;
  nonMasterSegment->SetName("non master");
  nonMasterSegment->AddRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName(), nonMasterPolyData.GetPointer() );
  cubeSegmentation->AddSegment(nonMasterSegment.GetPointer());
  if (cubeSegmentation->GetNumberOfSegments() != 2)
    {
    std::cerr << __LINE__ << ": Failed to add segment with non-master representation to segmentation!" << std::endl;
    return EXIT_FAILURE;
    }
  if (!nonMasterSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
    {
    std::cerr << __LINE__ << ": Master representation was not created when adding non-master segment to segmentation!" << std::endl;
    return EXIT_FAILURE;
    }

  //////////////////////////////////////////////////////////////////////////
  // Copy and move segments between segmentations

  // Copy
  cubeSegmentation->CopySegmentFromSegmentation(sphereSegmentation.GetPointer(), "sphere1");
  if (sphereSegmentation->GetNumberOfSegments() != 2 || cubeSegmentation->GetNumberOfSegments() != 3)
    {
    std::cerr << __LINE__ << ": Error when copying segment from one segmentation to another!" << std::endl;
    return EXIT_FAILURE;
    }

  // Move
  sphereSegmentation->CopySegmentFromSegmentation(cubeSegmentation.GetPointer(), "cube", true);
  if (sphereSegmentation->GetNumberOfSegments() != 3 || cubeSegmentation->GetNumberOfSegments() != 2)
    {
    std::cerr << __LINE__ << ": Error when moving segment from one segmentation to another!" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Segmentation test passed." << std::endl;
  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
void CreateSpherePolyData(vtkPolyData* polyData)
{
  if (!polyData)
    {
    return;
    }

  vtkNew<vtkSphereSource> sphere;
  sphere->SetCenter(50,50,50);
  sphere->SetRadius(30);
  sphere->Update();
  polyData->DeepCopy(sphere->GetOutput());
}

//----------------------------------------------------------------------------
void CreateCubeLabelmap(vtkOrientedImageData* imageData)
{
  if (!imageData)
    {
    return;
    }

  unsigned int size = 100;

  // Create new one because by default the direction is identity, origin is zeros and spacing is ones
  vtkNew<vtkOrientedImageData> identityImageData;
  identityImageData->SetExtent(0,size-1,0,size,0,size-1);
  identityImageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  unsigned char* imagePtr = (unsigned char*)identityImageData->GetScalarPointer();
  for (unsigned int x=0; x<size; ++x)
    {
    for (unsigned int y=0; y<size; ++y)
      {
      for (unsigned int z=0; z<size; ++z)
        {
        unsigned char* currentPtr = imagePtr + z*size*size + y*size + x;
        if (x>100/4 && x<size*3/4 && y>100/4 && y<size*3/4 && z>100/4 && z<size*3/4)
          {
          (*currentPtr) = 1;
          }
        else
          {
          (*currentPtr) = 0;
          }
        }
      }
    }

  imageData->DeepCopy(identityImageData.GetPointer());
}
