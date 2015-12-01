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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>

int vtkMRMLScalarVolumeNodeTest2(int , char * [] )
{
  vtkNew<vtkImageData> imageData;
  imageData->SetDimensions(256, 256, 1);
  imageData->AllocateScalars(VTK_UNSIGNED_SHORT, 1); // allocate storage for image data

  // Fill every entry of the image data with "0"
  int* dims = imageData->GetDimensions();
  for (int z = 0; z < dims[2]; z++)
    {
    for (int y = 0; y < dims[1]; y++)
      {
      for (int x = 0; x < dims[0]; x++)
        {
        vtkTypeUInt16* pixel = static_cast<vtkTypeUInt16*>(imageData->GetScalarPointer(x,y,z));
        pixel[0] = 0;
        }
      }
    }

  vtkNew<vtkMRMLScene> scene;

  vtkNew<vtkMRMLScalarVolumeNode> volumeNode;
  volumeNode->SetAndObserveImageData(imageData.GetPointer());
  scene->AddNode(volumeNode.GetPointer());

  vtkNew<vtkMRMLScalarVolumeDisplayNode> displayNode;
  scene->AddNode(displayNode.GetPointer());

  vtkNew<vtkMRMLColorTableNode> colorNode;
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode.GetPointer());
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());

  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  return EXIT_SUCCESS;
}
