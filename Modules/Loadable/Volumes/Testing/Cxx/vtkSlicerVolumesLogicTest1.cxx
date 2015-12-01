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

// Volumes logic
#include "vtkSlicerVolumesLogic.h"

// MRML includes
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataSetAttributes.h>
#include <vtkImageData.h>
#include <vtkImageAlgorithm.h>
#include <vtkInformation.h>
#include <vtkNew.h>
#include <vtkTrivialProducer.h>

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>

//-----------------------------------------------------------------------------
bool isImageDataValid(vtkAlgorithmOutput* imageDataConnection)
{
  if (!imageDataConnection ||
      !imageDataConnection->GetProducer())
    {
    std::cout << "No image data port" << std::endl;
    return false;
    }
  imageDataConnection->GetProducer()->Update();
  vtkInformation* info =
    imageDataConnection->GetProducer()->GetOutputInformation(0);
  if (!info)
    {
    std::cout << "No output information" << std::endl;
    return false;
    }
  vtkInformation *scalarInfo = vtkDataObject::GetActiveFieldInformation(info,
    vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  if (!scalarInfo)
    {
    std::cout << "No scalar information" << std::endl;
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
int vtkSlicerVolumesLogicTest1( int argc, char * argv[] )
{
  itk::itkFactoryRegistration();

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVolumesLogic> logic;

  if (argc < 2)
    {
    std::cerr << "Usage: vtkSlicerVolumesLogicTest1 volumeName [-I]" << std::endl;
    return EXIT_FAILURE;
    }

  logic->SetMRMLScene(scene.GetPointer());

  // Load file as scalar volume

  vtkMRMLVolumeNode* volume =
    logic->AddArchetypeVolume(argv[1], "volume", 0);

  if (!volume ||
      !isImageDataValid(volume->GetImageDataConnection()))
    {
    std::cerr << "Failed to load scalar image." << std::endl;
    return EXIT_FAILURE;
    }

  if (volume && volume->GetImageData())
    {
    volume->GetImageData()->Print(std::cerr);
    }

  // Load file as label map volume

  vtkMRMLVolumeNode* volume2 =
    logic->AddArchetypeVolume(argv[1], "volume", 1 /* bit 0: label map */);

  if (!volume2 ||
      !isImageDataValid(volume2->GetImageDataConnection()))
    {
    std::cerr << "Failed to load label map image." << std::endl;
    return EXIT_FAILURE;
    }

  if (volume2 && volume2->GetImageData())
    {
    volume2->GetImageData()->Print(std::cerr);
    }

  // basic checks on volume geometry
  vtkMRMLScalarVolumeNode *scalarVolume =
    vtkMRMLScalarVolumeNode::SafeDownCast(volume);
  vtkMRMLLabelMapVolumeNode *labelMapVolume =
    vtkMRMLLabelMapVolumeNode::SafeDownCast(volume2);

  if (scalarVolume == NULL)
    {
    std::cerr << "ERROR: failed to read as scalar volume" << std::endl;
    return EXIT_FAILURE;
    }
  if (labelMapVolume == NULL)
    {
    std::cerr << "ERROR: failed to read as label map volume" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Testing volume geometry" << std::endl;
  std::string warnings;
  warnings = logic->CheckForLabelVolumeValidity(NULL, NULL);
  if (warnings.empty())
    {
    std::cerr << "ERROR: did not detect two null volumes in check for label volume validity" << std::endl;
    return EXIT_FAILURE;
    }
  warnings = logic->CompareVolumeGeometry(NULL, NULL);
  if (warnings.empty())
    {
    std::cerr << "ERROR: did not detect two null volumes in Compare Volume Geometry" << std::endl;
    return EXIT_FAILURE;
    }
  warnings = logic->CheckForLabelVolumeValidity(scalarVolume, NULL);
  if (warnings.empty())
    {
    std::cerr << "ERROR: did not detect null labe volumes in CheckForLabelVolumeValidity" << std::endl;
    return EXIT_FAILURE;
    }
  warnings = logic->CheckForLabelVolumeValidity(NULL, labelMapVolume);
  if (warnings.empty())
    {
    std::cerr << "ERROR: did not detect null volumes and incorrect label map in CheckForLabelVolumeValidity" << std::endl;
    return EXIT_FAILURE;
    }
  warnings = logic->CompareVolumeGeometry(scalarVolume, labelMapVolume);
  if (!warnings.empty())
    {
    std::cerr << "ERROR: got a warning when comparing identical volumes in CompareVolumeGeometry: " << warnings.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
