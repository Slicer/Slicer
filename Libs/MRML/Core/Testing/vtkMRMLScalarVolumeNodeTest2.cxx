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

#include "vtkMRMLCoreTestingMacros.h"

// MRML includes
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkImageData.h>

// STD includes

int vtkMRMLScalarVolumeNodeTest2(int , char * [] )
{
  vtkSmartPointer< vtkMRMLScalarVolumeNode > volumeNode = vtkSmartPointer< vtkMRMLScalarVolumeNode >::New();

  vtkSmartPointer< vtkImageData > imageData = vtkSmartPointer< vtkImageData >::New();
  imageData->SetDimensions(256, 256, 1);
  imageData->SetScalarTypeToUnsignedShort();
  imageData->SetNumberOfScalarComponents(1); // image holds one value intensities
  //imageData->SetSpacing(2., 2., 512.); not used by vtkMRMLVolumeNode
  //imageData->SetOrigin(0.0,0.0,0.0); not used by vtkMRMLVolumeNode
  imageData->AllocateScalars(); // allocate storage for image data  
  
  volumeNode->SetAndObserveImageData(imageData);

  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> displayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  scene->AddNode(volumeNode);
  scene->AddNode(displayNode);

  vtkSmartPointer<vtkMRMLColorTableNode> colorNode = vtkSmartPointer<vtkMRMLColorTableNode>::New();
  colorNode->SetTypeToGrey();
  scene->AddNode(colorNode);
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());

  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  return EXIT_SUCCESS;
}
