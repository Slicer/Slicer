/*==============================================================================

  Program: 3D Slicer

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
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkNew.h>

// STD includes
#include <cmath>
#include <string>
#include <vector>

namespace
{
std::vector<int> ReportedProgress;

//---------------------------------------------------------------------------
void onImportProgress(vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eid), void* vtkNotUsed(clientData), void* callData)
{
  // Call data is a pointer to the progress value (0.0-1.0), store it in percent
  ReportedProgress.push_back(static_cast<int>(std::lround(*static_cast<double*>(callData) * 100.0)));
}

//---------------------------------------------------------------------------
// Scene import reports its progress (0.0-1.0) using ProgressImportEvent
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
  // Import the scene twice to verify that progress reporting starts over for each import
  for (int importIndex = 0; importIndex < 2; importIndex++)
  {
    ReportedProgress.clear();
    CHECK_BOOL(scene->Import() != 0, true);
    CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelNode"), 50 * (importIndex + 1));

    // Progress is reported many times, it is always increasing (the same value is not reported twice),
    // starts at 0 and ends at 100.
    CHECK_BOOL(ReportedProgress.size() >= 20, true);
    for (size_t i = 0; i < ReportedProgress.size(); i++)
    {
      CHECK_BOOL(ReportedProgress[i] >= 0 && ReportedProgress[i] <= 100, true);
      if (i > 0)
      {
        CHECK_BOOL(ReportedProgress[i] > ReportedProgress[i - 1], true);
      }
    }
    CHECK_INT(ReportedProgress.front(), 0);
    CHECK_INT(ReportedProgress.back(), 100);

    // Progress is reported while nodes are added (5-20%), while node references are updated (20-30%, only if node IDs
    // changed, which is the case in the second import, as the node IDs are already used in the scene),
    // and while nodes are updated (30-100%).
    int numberOfAddingNodesValues = 0;
    int numberOfUpdatingReferencesValues = 0;
    int numberOfUpdatingNodesValues = 0;
    for (int progress : ReportedProgress)
    {
      if (progress > 5 && progress < 20)
      {
        numberOfAddingNodesValues++;
      }
      else if (progress > 20 && progress < 30)
      {
        numberOfUpdatingReferencesValues++;
      }
      else if (progress > 30 && progress < 100)
      {
        numberOfUpdatingNodesValues++;
      }
    }
    CHECK_BOOL(numberOfAddingNodesValues >= 5, true);
    CHECK_BOOL(numberOfUpdatingReferencesValues >= 3, importIndex > 0);
    CHECK_BOOL(numberOfUpdatingNodesValues >= 20, true);
  }
  return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------
bool StartEventCallDataIsNull = false;
void onStartImport(vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(eid), void* vtkNotUsed(clientData), void* callData)
{
  StartEventCallDataIsNull = (callData == nullptr);
}

//---------------------------------------------------------------------------
// Scene state events follow VTK conventions: start events have no call data,
// progress events pass a pointer to the progress value (0.0-1.0, values outside this range are clamped).
int testStateEventConventions()
{
  vtkNew<vtkMRMLScene> scene;
  vtkNew<vtkCallbackCommand> startCommand;
  startCommand->SetCallback(onStartImport);
  scene->AddObserver(vtkMRMLScene::StartImportEvent, startCommand);
  vtkNew<vtkCallbackCommand> progressCommand;
  progressCommand->SetCallback(onImportProgress);
  scene->AddObserver(vtkMRMLScene::ProgressImportEvent, progressCommand);

  StartEventCallDataIsNull = false;
  ReportedProgress.clear();
  scene->StartState(vtkMRMLScene::ImportState, 123);
  CHECK_BOOL(StartEventCallDataIsNull, true);
  scene->ProgressState(vtkMRMLScene::ImportState, 0.5);
  scene->ProgressState(vtkMRMLScene::ImportState, 2.0);
  scene->ProgressState(vtkMRMLScene::ImportState, -1.0);
  scene->EndState(vtkMRMLScene::ImportState);
  CHECK_INT(static_cast<int>(ReportedProgress.size()), 3);
  CHECK_INT(ReportedProgress[0], 50);
  CHECK_INT(ReportedProgress[1], 100);
  CHECK_INT(ReportedProgress[2], 0);
  return EXIT_SUCCESS;
}
} // namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneImportProgressTest(int, char*[])
{
  CHECK_EXIT_SUCCESS(testImportProgress());
  CHECK_EXIT_SUCCESS(testStateEventConventions());
  return EXIT_SUCCESS;
}
