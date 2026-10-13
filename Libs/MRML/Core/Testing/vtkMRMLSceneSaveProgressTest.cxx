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
#include "vtkMRMLTextNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <cmath>
#include <string>
#include <vector>

namespace
{

const int NUMBER_OF_TEXT_NODES = 10;

struct SaveEventRecorder
{
  int NumberOfStartEvents{ 0 };
  int NumberOfEndEvents{ 0 };
  std::vector<int> Progress;
  void Reset() { *this = SaveEventRecorder(); }
};
SaveEventRecorder Recorder;

//---------------------------------------------------------------------------
void onSaveEvent(vtkObject* vtkNotUsed(caller), unsigned long eid, void* vtkNotUsed(clientData), void* callData)
{
  switch (eid)
  {
    case vtkMRMLScene::StartSaveEvent: Recorder.NumberOfStartEvents++; break;
    case vtkMRMLScene::ProgressSaveEvent:
      // Call data is a pointer to the progress value (0.0-1.0), store it in percent
      Recorder.Progress.push_back(static_cast<int>(std::lround(*static_cast<double*>(callData) * 100.0)));
      break;
    case vtkMRMLScene::EndSaveEvent: Recorder.NumberOfEndEvents++; break;
    default: break;
  }
}

//---------------------------------------------------------------------------
// Saving is reported once, progress is increasing (the same value is not reported twice) from 0 to 100
int checkSaveProgress()
{
  CHECK_INT(Recorder.NumberOfStartEvents, 1);
  CHECK_INT(Recorder.NumberOfEndEvents, 1);
  // Progress is reported after saving each storable node
  CHECK_BOOL(static_cast<int>(Recorder.Progress.size()) >= NUMBER_OF_TEXT_NODES, true);
  for (size_t i = 0; i < Recorder.Progress.size(); i++)
  {
    CHECK_BOOL(Recorder.Progress[i] >= 0 && Recorder.Progress[i] <= 100, true);
    if (i > 0)
    {
      CHECK_BOOL(Recorder.Progress[i] > Recorder.Progress[i - 1], true);
    }
  }
  CHECK_INT(Recorder.Progress.front(), 0);
  CHECK_INT(Recorder.Progress.back(), 100);
  return EXIT_SUCCESS;
}

} // namespace

//---------------------------------------------------------------------------
int vtkMRMLSceneSaveProgressTest(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " /path/to/temp" << std::endl;
    return EXIT_FAILURE;
  }
  const std::string directory = vtksys::SystemTools::CollapseFullPath(std::string(argv[1]) + "/vtkMRMLSceneSaveProgressTest");
  vtksys::SystemTools::RemoveADirectory(directory);
  vtksys::SystemTools::MakeDirectory(directory);

  vtkNew<vtkMRMLScene> scene;
  for (int i = 0; i < NUMBER_OF_TEXT_NODES; i++)
  {
    vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode", "Text" + std::to_string(i)));
    textNode->SetText(("Text " + std::to_string(i)).c_str());
  }

  vtkNew<vtkCallbackCommand> saveCommand;
  saveCommand->SetCallback(onSaveEvent);
  scene->AddObserver(vtkMRMLScene::StartSaveEvent, saveCommand);
  scene->AddObserver(vtkMRMLScene::ProgressSaveEvent, saveCommand);
  scene->AddObserver(vtkMRMLScene::EndSaveEvent, saveCommand);

  // Saving to a data bundle directory
  Recorder.Reset();
  const std::string bundleDirectory = directory + "/Bundle";
  vtksys::SystemTools::MakeDirectory(bundleDirectory);
  CHECK_BOOL(scene->SaveSceneToSlicerDataBundleDirectory(bundleDirectory.c_str()), true);
  CHECK_EXIT_SUCCESS(checkSaveProgress());

  // Saving to a data bundle file: saving the data is 0-80%, compressing the bundle is 80-100%
  Recorder.Reset();
  const std::string bundleFile = directory + "/Scene.mrb";
  CHECK_BOOL(scene->WriteToMRB(bundleFile.c_str()), true);
  CHECK_BOOL(vtksys::SystemTools::FileExists(bundleFile, true), true);
  CHECK_EXIT_SUCCESS(checkSaveProgress());
  CHECK_INT(Recorder.Progress[Recorder.Progress.size() - 2], 80);

  // Progress is not reported after saving is completed
  Recorder.Reset();
  scene->AddNewNodeByClass("vtkMRMLTextNode");
  CHECK_INT(static_cast<int>(Recorder.Progress.size()), 0);

  vtksys::SystemTools::RemoveADirectory(directory);
  return EXIT_SUCCESS;
}
