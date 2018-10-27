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

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStreamingVolumeNode.h"

int vtkMRMLStreamingVolumeNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLStreamingVolumeNode> node1;
  vtkNew<vtkMRMLScene> scene;
  scene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

  int width = 10;
  int height = 10;
  vtkSmartPointer<vtkImageData> imageData1 = vtkSmartPointer<vtkImageData>::New();
  imageData1->SetDimensions(width, height, 1);
  imageData1->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
  unsigned char* originImagePointer = (unsigned char*)imageData1->GetScalarPointer();
  for (int y = 0; y < height; y++)
    {
    for (int x = 0; x < width; x++)
      {
      for (int c = 0; c < 3; c++)
        {
        unsigned char value = x + y + c;
        *originImagePointer = value;
        ++originImagePointer;
        }
      }
    }

  vtkSmartPointer<vtkMRMLStreamingVolumeNode> streamingVolumeNode1 = vtkSmartPointer<vtkMRMLStreamingVolumeNode>::New();
  streamingVolumeNode1->SetCodecFourCC("RV24");
  streamingVolumeNode1->SetAndObserveImageData(imageData1);
  CHECK_NULL(streamingVolumeNode1->GetFrame());

  streamingVolumeNode1->EncodeImageData();

  vtkSmartPointer<vtkStreamingVolumeFrame> frameData = streamingVolumeNode1->GetFrame();
  CHECK_NOT_NULL(frameData);

  vtkSmartPointer<vtkMRMLStreamingVolumeNode> streamingVolumeNode2 = vtkSmartPointer<vtkMRMLStreamingVolumeNode>::New();
  streamingVolumeNode2->SetAndObserveFrame(frameData);

  // Calling GetImageData should decode the frame
  vtkSmartPointer<vtkImageData> imageData2 = streamingVolumeNode2->GetImageData();
  CHECK_NOT_NULL(imageData2);

  int image1Dimensions[3] = { 0,0,0 };
  imageData1->GetDimensions(image1Dimensions);

  int image2Dimensions[3] = { 0,0,0 };
  imageData2->GetDimensions(image2Dimensions);

  for (int i = 0; i < 3; ++i)
    {
    if (image1Dimensions[i] != image2Dimensions[i])
      {
      return EXIT_FAILURE;
      }
    }

  unsigned char* image1Pointer = (unsigned char*)imageData1->GetScalarPointer();
  unsigned char* image2Pointer = (unsigned char*)imageData2->GetScalarPointer();
  for (int z = 0; z < image1Dimensions[2]; z++)
    {
    for (int y = 0; y < image1Dimensions[1]; y++)
      {
      for (int x = 0; x < image1Dimensions[0]; x++)
        {
        for (int c = 0; c < 3; c++)
          {
          CHECK_INT(*image2Pointer, *image1Pointer);
          ++image1Pointer;
          ++image2Pointer;
          }
        }
      }
    }

  return EXIT_SUCCESS;
}
