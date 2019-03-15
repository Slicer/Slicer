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
#include "vtkMRMLCoreTestingMacros.h"

// MRML includes
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>

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
int TestCheckForLabelVolumeValidity( vtkMRMLScalarVolumeNode* scalarVolume,
                                     vtkMRMLLabelMapVolumeNode* labelMapVolume,
                                     vtkSlicerVolumesLogic* logic );
int TestCloneVolume( vtkMRMLScalarVolumeNode* scalarVolume,
                     vtkMRMLScene* scene,
                     vtkSlicerVolumesLogic *logic);

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

  vtkMRMLScalarVolumeNode * scalarVolume = TestScalarVolumeLoading(volumeName, logic.GetPointer());
  CHECK_NOT_NULL(scalarVolume);

  vtkMRMLLabelMapVolumeNode * labelMapVolume = TestLabelMapVolumeLoading(volumeName, logic.GetPointer());
  CHECK_NOT_NULL(labelMapVolume);
  CHECK_INT(labelMapVolume->GetDisplayNode()->GetSliceIntersectionThickness(), 3);

  // Add default node
  vtkNew<vtkMRMLLabelMapVolumeDisplayNode> defaultDisplayNode;
  defaultDisplayNode->SetSliceIntersectionThickness(1);
  scene->AddDefaultNode(defaultDisplayNode.GetPointer());

  // Check that node attribute is initialized considering the default node
  labelMapVolume = TestLabelMapVolumeLoading(volumeName, logic.GetPointer());
    CHECK_NOT_NULL(labelMapVolume);
    CHECK_INT(labelMapVolume->GetDisplayNode()->GetSliceIntersectionThickness(), 1);


  CHECK_EXIT_SUCCESS(TestCheckForLabelVolumeValidity(scalarVolume, labelMapVolume, logic.GetPointer()));

  CHECK_EXIT_SUCCESS(TestCloneVolume(scalarVolume, scene.GetPointer(), logic.GetPointer()));

  return EXIT_SUCCESS;
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
    return nullptr;
    }

  if(!isImageDataValid(__LINE__, volume->GetImageDataConnection()))
    {
    return nullptr;
    }

  vtkMRMLScalarVolumeNode *scalarVolume =
    vtkMRMLScalarVolumeNode::SafeDownCast(volume);
  if(!scalarVolume)
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to read as scalar volume !" << std::endl;
    return nullptr;
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
    return nullptr;
    }

  if(!isImageDataValid(__LINE__, volume->GetImageDataConnection()))
    {
    return nullptr;
    }

  vtkMRMLLabelMapVolumeNode *labelMapVolume =
    vtkMRMLLabelMapVolumeNode::SafeDownCast(volume);
  if(!labelMapVolume)
    {
    std::cerr << "Line " << __LINE__
              << " - Failed to read as label map volume !" << std::endl;
    return nullptr;
    }

  return labelMapVolume;
}

//-----------------------------------------------------------------------------
int TestCheckForLabelVolumeValidity( vtkMRMLScalarVolumeNode* scalarVolume,
                                     vtkMRMLLabelMapVolumeNode* labelMapVolume,
                                     vtkSlicerVolumesLogic* logic )
{
  std::string warnings;

  warnings = logic->CheckForLabelVolumeValidity(nullptr, nullptr);
  if (warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - did not detect two null volumes in CheckForLabelVolumeValidity" << std::endl;
    return EXIT_FAILURE;
    }

  warnings = logic->CheckForLabelVolumeValidity(scalarVolume, nullptr);
  if (warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - did not detect null label volume in CheckForLabelVolumeValidity" << std::endl;
    return EXIT_FAILURE;
    }

  warnings = logic->CheckForLabelVolumeValidity(nullptr, labelMapVolume);
  if (warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - did not detect null scalar volume map in CheckForLabelVolumeValidity" << std::endl;
    return EXIT_FAILURE;
    }

  warnings = logic->CheckForLabelVolumeValidity(scalarVolume, labelMapVolume);
  if (!warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - got a warning when comparing identical volumes in CheckForLabelVolumeValidity: " << warnings.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
int TestCloneVolume( vtkMRMLScalarVolumeNode* scalarVolume,
                     vtkMRMLScene* scene,
                     vtkSlicerVolumesLogic* logic )
{
  vtkNew<vtkMRMLVolumeArchetypeStorageNode> volumeStorageNode;
  scene->AddNode(volumeStorageNode.GetPointer());
  vtkNew<vtkMRMLScalarVolumeDisplayNode> scalarVolumeDisplayNode;
  scene->AddNode(scalarVolumeDisplayNode.GetPointer());
  vtkNew<vtkMRMLLabelMapVolumeDisplayNode> labelMapVolumeDisplayNode;
  scene->AddNode(labelMapVolumeDisplayNode.GetPointer());

  scalarVolume->SetAndObserveStorageNodeID(volumeStorageNode->GetID());
  scalarVolume->SetAndObserveNthDisplayNodeID(0,scalarVolumeDisplayNode->GetID());
  scalarVolume->SetAndObserveNthDisplayNodeID(1,labelMapVolumeDisplayNode->GetID());

  vtkMRMLScalarVolumeNode* clonedVolume = logic->CloneVolume(scene, scalarVolume, "clonedVolume");

  if (!clonedVolume)
    {
    std::cerr << "Line " << __LINE__
              << " - CloneVolume failed" << std::endl;
    return EXIT_FAILURE;
    }

  if (clonedVolume->GetNumberOfStorageNodes())
    {
    std::cerr << "Line " << __LINE__
              << " - a cloned volume should not have any storage nodes" << std::endl;
    return EXIT_FAILURE;
    }

  for (int i = 0; i < clonedVolume->GetNumberOfDisplayNodes(); ++i)
    {
    if ((std::string)clonedVolume->GetNthDisplayNodeID(i) == (std::string)scalarVolume->GetNthDisplayNodeID(i))
      {
      std::cerr << "Line " << __LINE__
                << " - the display node #" <<i<< " is already referenced to the original volume" << std::endl;
      return EXIT_FAILURE;
      }
    }

  std::string warnings = logic->CompareVolumeGeometry(scalarVolume, clonedVolume);
  if (!warnings.empty())
    {
    std::cerr << "Line " << __LINE__
              << " - CompareVolumeGeometry returned a warning when comparing the original and the cloned volumes: "
              << warnings.c_str() << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
