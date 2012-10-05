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

// MRMLLogic includes
#include "vtkMRMLColorLogic.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLdGEMRICProceduralColorNode.h>
#include <vtkMRMLPETProceduralColorNode.h>

// VTK includes
#include <vtkNew.h>
#include <vtkTimerLog.h>

// STD includes

#include "vtkMRMLCoreTestingMacros.h"

//----------------------------------------------------------------------------
namespace
{
bool TestPerformance();
bool TestNodeIDs();
bool TestDefaults();
}

//----------------------------------------------------------------------------
int vtkMRMLColorLogicTest1(int vtkNotUsed(argc), char * vtkNotUsed(argv)[] )
{
  bool res = true;
  res = TestPerformance() && res;
  res = TestNodeIDs() && res;
  res = TestDefaults() && res;
  return res ? EXIT_SUCCESS : EXIT_FAILURE;
}
namespace
{

//----------------------------------------------------------------------------
bool TestPerformance()
{
  // To load freesurfer files, SLICER_HOME is requested
  //vtksys::SystemTools::PutEnv("SLICER_HOME=..." );
  vtkSmartPointer<vtkMRMLScene> scene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkMRMLColorLogic* colorLogic = vtkMRMLColorLogic::New();

  vtkNew<vtkTimerLog> overallTimer;
  overallTimer->StartTimer();

  colorLogic->SetMRMLScene(scene);

  overallTimer->StopTimer();
  std::cout << "<DartMeasurement name=\"AddDefaultColorNodes\" "
            << "type=\"numeric/double\">"
            << overallTimer->GetElapsedTime() << "</DartMeasurement>" << std::endl;
  overallTimer->StartTimer();

  colorLogic->Delete();

  overallTimer->StopTimer();
  std::cout << "<DartMeasurement name=\"RemoveDefaultColorNodes\" "
            << "type=\"numeric/double\">"
            << overallTimer->GetElapsedTime()
            << "</DartMeasurement>" << std::endl;

  return true;
}

//----------------------------------------------------------------------------
bool TestNodeIDs()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLColorLogic> colorLogic;
  colorLogic->SetMRMLScene(scene.GetPointer());

  vtkMRMLNode* node= 0;
  scene->InitTraversal();
  while ( (node = scene->GetNextNodeByClass("vtkMRMLColorTableNode")) )
    {
    vtkMRMLColorTableNode* colorNode =
      vtkMRMLColorTableNode::SafeDownCast(node);
    const char* nodeID =
      vtkMRMLColorLogic::GetColorTableNodeID(colorNode->GetType());
    if (strcmp(colorNode->GetID(), nodeID) != 0)
      {
      std::cout << "Failed to generate color table node ID for "
                << colorNode->GetType() << std::endl;
      return false;
      }
    }
  scene->InitTraversal();
  while ( (node = scene->GetNextNodeByClass("vtkMRMLPETProceduralColorNode")) )
    {
    vtkMRMLPETProceduralColorNode* colorNode =
      vtkMRMLPETProceduralColorNode::SafeDownCast(node);
    const char* nodeID =
      vtkMRMLColorLogic::GetPETColorNodeID(colorNode->GetType());
    if (strcmp(colorNode->GetID(), nodeID) != 0)
      {
      std::cout << "Failed to generate color table node ID for "
                << colorNode->GetType() << std::endl;
      return false;
      }
    }
  scene->InitTraversal();
  while ( (node = scene->GetNextNodeByClass("vtkMRMLdGEMRICProceduralColorNode")) )
    {
    vtkMRMLdGEMRICProceduralColorNode* colorNode =
      vtkMRMLdGEMRICProceduralColorNode::SafeDownCast(node);
    const char* nodeID =
      vtkMRMLColorLogic::GetdGEMRICColorNodeID(colorNode->GetType());
    if (strcmp(colorNode->GetID(), nodeID) != 0)
      {
      std::cout << "Failed to generate color table node ID for "
                << colorNode->GetType() << std::endl;
      return false;
      }
    }
  // To test free surfers, SLICER_HOME env variable needs to be set.
  return true;
}

//----------------------------------------------------------------------------
bool TestDefaults()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkMRMLColorLogic> colorLogic;
  colorLogic->SetMRMLScene(scene.GetPointer());
  if (scene->GetNodeByID(colorLogic->GetDefaultVolumeColorNodeID()) == 0)
    {
    std::cout << "Can't find default volume color node with ID: "
              << colorLogic->GetDefaultVolumeColorNodeID() << std::endl;
    return false;
    }
  if (scene->GetNodeByID(colorLogic->GetDefaultLabelMapColorNodeID()) == 0)
    {
    std::cout << "Can't find default labelmap color node with ID: "
              << colorLogic->GetDefaultLabelMapColorNodeID() << std::endl;
    return false;
    }
  if (scene->GetNodeByID(colorLogic->GetDefaultEditorColorNodeID()) == 0)
    {
    std::cout << "Can't find default editor color node with ID: "
              << colorLogic->GetDefaultEditorColorNodeID() << std::endl;
    return false;
    }
  if (scene->GetNodeByID(colorLogic->GetDefaultModelColorNodeID()) == 0)
    {
    std::cout << "Can't find default model color node with ID: "
              << colorLogic->GetDefaultModelColorNodeID() << std::endl;
    return false;
    }
  if (scene->GetNodeByID(colorLogic->GetDefaultChartColorNodeID()) == 0)
    {
    std::cout << "Can't find default chart color node with ID: "
              << colorLogic->GetDefaultChartColorNodeID() << std::endl;
    return false;
    }
  //if (scene->GetNodeByID(colorLogic->GetDefaultFreeSurferLabelMapColorNodeID()) == 0)
  //  {
  //  std::cout << "Can't find default free surfer color node with ID: "
  //            << colorLogic->GetDefaultFreeSurferLabelMapColorNodeID() << std::endl;
  //  return false;
  //  }
  return true;
}

}
