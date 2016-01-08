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
bool isImageDataValid(int line, vtkAlgorithmOutput* imageDataConnection);
vtkMRMLScalarVolumeNode * TestScalarVolumeLoading( const char* volumeName,
                                                   vtkSlicerVolumesLogic* logic );
vtkMRMLLabelMapVolumeNode * TestLabelMapVolumeLoading( const char* volumeName,
                                                       vtkSlicerVolumesLogic* logic );
bool TestCheckForLabelVolumeValidity( vtkMRMLScalarVolumeNode* scalarVolume,
                                      vtkMRMLLabelMapVolumeNode* labelMapVolume,
                                      vtkSlicerVolumesLogic* logic );

//-----------------------------------------------------------------------------
int vtkSlicerVolumesLogicTest1( int argc, char * argv[] )
{
  itk::itkFactoryRegistration();

  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkSlicerVolumesLogic> logic;

  if (argc < 2)
    {
    std::cerr << "Line " << __LINE__
              << " - Missing parameters !\n"
              << "Usage: vtkSlicerVolumesLogicTest1 volumeName [-I]"
              << std::endl;
    return EXIT_FAILURE;
    }

  logic->SetMRMLScene(scene.GetPointer());
  const char* volumeName = argv[1];
  bool res = true;

  vtkMRMLScalarVolumeNode * scalarVolume = TestScalarVolumeLoading(volumeName, logic.GetPointer());
  res = res && scalarVolume;
  vtkMRMLLabelMapVolumeNode * labelMapVolume = TestLabelMapVolumeLoading(volumeName, logic.GetPointer());
  res = res && labelMapVolume;

  if (scalarVolume && labelMapVolume)
  {
    res = res && TestCheckForLabelVolumeValidity(scalarVolume, labelMapVolume, logic.GetPointer());
  }

  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}

//-----------------------------------------------------------------------------
bool isImageDataValid(int line, vtkAlgorithmOutput* imageDataConnection)
{
  if (!imageDataConnection ||
      !imageDataConnection->GetProducer())
    {
    std::cerr << "Line " << line
              << " - No image data port !" << std::endl;
    return false;
    }

  imageDataConnection->GetProducer()->Update();
  vtkInformation* info =
    imageDataConnection->GetProducer()->GetOutputInformation(0);
  if (!info)
    {
    std::cerr << "Line " << line
              << " - No output information !" << std::endl;
    return false;
    }

  vtkInformation *scalarInfo = vtkDataObject::GetActiveFieldInformation(info,
    vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  if (!scalarInfo)
    {
    std::cerr << "Line " << line
              << " - No scalar information !" << std::endl;
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode * TestScalarVolumeLoading( const char* volumeName,
                                                   vtkSlicerVolumesLogic* logic )
{
  vtkMRMLVolumeNode* volume =
    logic->AddArchetypeVolume(volumeName, "volume", 0);
  if(!volume)
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to load as scalar volume !" << std::endl;
    return NULL;
    }

  if(!isImageDataValid(__LINE__, volume->GetImageDataConnection()))
    {
    return NULL;
    }

  vtkMRMLScalarVolumeNode *scalarVolume =
    vtkMRMLScalarVolumeNode::SafeDownCast(volume);
  if(!scalarVolume)
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to read as scalar volume !" << std::endl;
    return NULL;
    }

  return scalarVolume;
}

//-----------------------------------------------------------------------------
vtkMRMLLabelMapVolumeNode * TestLabelMapVolumeLoading( const char* volumeName,
                                                       vtkSlicerVolumesLogic* logic )
{
  vtkMRMLVolumeNode* volume =
    logic->AddArchetypeVolume(volumeName, "volume", 1 /* bit 0: label map */);
  if(!volume)
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to load as scalar volume !" << std::endl;
    return NULL;
    }

  if(!isImageDataValid(__LINE__, volume->GetImageDataConnection()))
    {
    return NULL;
    }

  vtkMRMLLabelMapVolumeNode *labelMapVolume =
    vtkMRMLLabelMapVolumeNode::SafeDownCast(volume);
  if(!labelMapVolume)
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to read as label map volume !" << std::endl;
    return NULL;
    }

  return labelMapVolume;
}

//-----------------------------------------------------------------------------
bool TestCheckForLabelVolumeValidity( vtkMRMLScalarVolumeNode* scalarVolume,
                                      vtkMRMLLabelMapVolumeNode* labelMapVolume,
                                      vtkSlicerVolumesLogic* logic )
{
  std::string warnings;

  warnings = logic->CheckForLabelVolumeValidity(NULL, NULL);
  if (warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - did not detect two null volumes in CheckForLabelVolumeValidity" << std::endl;
    return false;
    }

  warnings = logic->CheckForLabelVolumeValidity(scalarVolume, NULL);
  if (warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - did not detect null label volume in CheckForLabelVolumeValidity" << std::endl;
    return false;
    }

  warnings = logic->CheckForLabelVolumeValidity(NULL, labelMapVolume);
  if (warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - did not detect null scalar volume map in CheckForLabelVolumeValidity" << std::endl;
    return false;
    }

  warnings = logic->CheckForLabelVolumeValidity(scalarVolume, labelMapVolume);
  if (!warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - got a warning when comparing identical volumes in CheckForLabelVolumeValidity: " << warnings.c_str() << std::endl;
    return false;
    }

  return true;
}
