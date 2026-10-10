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

==============================================================================*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMessageCollection.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkNew.h>

namespace
{
std::vector<int> ReportedProgress;

//---------------------------------------------------------------------------
void onImportProgress(vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eid), void* vtkNotUsed(clientData), void* callData)
{
  ReportedProgress.push_back(static_cast<int>(reinterpret_cast<long long>(callData)));
}

//---------------------------------------------------------------------------
// A scene file may contain nodes of types that are not available in the application (removed node types,
// nodes of extensions that are not installed). These nodes are skipped with a warning, the rest of the scene is loaded.
int testImportUnknownNodeType()
{
  vtkNew<vtkMRMLScene> scene;
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString("<MRML version=\"Slicer4.4.0\">"
                           " <Model id=\"vtkMRMLModelNode1\" name=\"Model1\"></Model>"
                           " <VolumeRenderingScenario id=\"vtkMRMLVolumeRenderingScenarioNode1\" name=\"Scenario\""
                           "  parametersNodeID=\"vtkMRMLVolumeRenderingDisplayNode1\"></VolumeRenderingScenario>"
                           " <SomeUnknownNodeType id=\"vtkMRMLSomeUnknownNodeTypeNode1\" name=\"Unknown\"></SomeUnknownNodeType>"
                           " <Model id=\"vtkMRMLModelNode2\" name=\"Model2\"></Model>"
                           "</MRML>");
  // Messages logged during import are collected in the message collection (not printed to the output window)
  vtkNew<vtkMRMLMessageCollection> userMessages;
  CHECK_BOOL(scene->Import(userMessages) != 0, true);
  // Unknown node types are reported as warnings (not errors, as the rest of the scene is loaded)
  CHECK_INT(userMessages->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent), 0);
  CHECK_INT(userMessages->GetNumberOfMessagesOfType(vtkCommand::WarningEvent), 2);

  // Known nodes before and after the unknown nodes are loaded, unknown nodes are not added
  // (the scene contains the two models and the subject hierarchy node that is created at the end of the import)
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelNode"), 2);
  CHECK_INT(scene->GetNumberOfNodes(), 3);
  CHECK_NOT_NULL(scene->GetFirstNodeByName("Model1"));
  CHECK_NOT_NULL(scene->GetFirstNodeByName("Model2"));
  CHECK_NULL(scene->GetFirstNodeByName("Scenario"));
  CHECK_NULL(scene->GetFirstNodeByName("Unknown"));
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
// Scene import reports its progress (0-100) using ProgressImportEvent
int testImportProgress()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkCallbackCommand> progressCommand;
  progressCommand->SetCallback(onImportProgress);
  scene->AddObserver(vtkMRMLScene::ProgressImportEvent, progressCommand);

  std::string sceneXML = "<MRML version=\"Slicer4.4.0\">";
  for (int i = 0; i < 50; i++)
  {
    sceneXML += " <Model id=\"vtkMRMLModelNode" + std::to_string(i + 1) + "\" name=\"Model" + std::to_string(i + 1) + "\"></Model>";
  }
  sceneXML += "</MRML>";
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(sceneXML);
  ReportedProgress.clear();
  CHECK_BOOL(scene->Import() != 0, true);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelNode"), 50);

  // Progress is reported several times, it never decreases, starts at the parsing stage and ends at 100
  CHECK_BOOL(ReportedProgress.size() >= 10, true);
  for (size_t i = 1; i < ReportedProgress.size(); i++)
  {
    CHECK_BOOL(ReportedProgress[i] >= ReportedProgress[i - 1], true);
  }
  CHECK_BOOL(ReportedProgress.front() < vtkMRMLScene::ImportProgressParsingComplete, true);
  CHECK_INT(ReportedProgress.back(), 100);
  return EXIT_SUCCESS;
}
} // namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneImportLegacyNodeTest(int, char*[])
{
  CHECK_EXIT_SUCCESS(testImportUnknownNodeType());
  CHECK_EXIT_SUCCESS(testImportProgress());
  return EXIT_SUCCESS;
}
