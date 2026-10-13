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
#include "vtkMRMLFileIOManager.h"
#include "vtkMRMLFileReader.h"
#include "vtkMRMLFileWriter.h"
#include "vtkMRMLIOProperties.h"
#include "vtkMRMLOperationProgress.h"
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTextNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTimerLog.h>
#include <vtkSmartPointer.h>
#include <vtksys/FStream.hxx>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <string>
#include <vector>

namespace
{

const int NUMBER_OF_NODES_IN_SCENE = 50;

//-----------------------------------------------------------------------------
std::string createSceneXML(int numberOfNodes)
{
  std::string sceneXML = "<MRML version=\"Slicer4.4.0\">";
  for (int i = 0; i < numberOfNodes; i++)
  {
    sceneXML += " <Model id=\"vtkMRMLModelNode" + std::to_string(i + 1) + "\" name=\"Model" + std::to_string(i + 1) + "\"></Model>";
  }
  sceneXML += "</MRML>";
  return sceneXML;
}

/// A reader that imports a scene into the scene of the manager (similarly to the scene file reader).
class vtkTestSceneReader : public vtkMRMLFileReader
{
public:
  static vtkTestSceneReader* New();
  vtkTypeMacro(vtkTestSceneReader, vtkMRMLFileReader);
  bool Load(vtkMRMLIOProperties* vtkNotUsed(properties)) override
  {
    vtkMRMLScene* scene = this->GetScene();
    scene->SetLoadFromXMLString(1);
    scene->SetSceneXMLString(createSceneXML(NUMBER_OF_NODES_IN_SCENE));
    bool success = scene->Import() != 0;
    scene->SetLoadFromXMLString(0);
    return success;
  }
};
vtkStandardNewMacro(vtkTestSceneReader);

/// A reader that loads a file using the manager (a scene file or list of scene files, specified in the "sceneFileName" property).
class vtkTestNestedLoadingReader : public vtkMRMLFileReader
{
public:
  static vtkTestNestedLoadingReader* New();
  vtkTypeMacro(vtkTestNestedLoadingReader, vtkMRMLFileReader);
  bool Load(vtkMRMLIOProperties* properties) override
  {
    vtkNew<vtkMRMLIOProperties> sceneProperties;
    if (properties->IsStringListProperty("sceneFileName"))
    {
      sceneProperties->SetStringListProperty("fileName", properties->GetStringListProperty("sceneFileName"));
    }
    else
    {
      sceneProperties->SetStringProperty("fileName", properties->GetStringProperty("sceneFileName"));
    }
    return this->GetFileIOManager()->LoadNodes("TestSceneFile", sceneProperties);
  }
};
vtkStandardNewMacro(vtkTestNestedLoadingReader);

/// A writer that reports save progress of the scene (similarly to saving the scene as a data bundle).
class vtkTestSceneBundleWriter : public vtkMRMLFileWriter
{
public:
  static vtkTestSceneBundleWriter* New();
  vtkTypeMacro(vtkTestSceneBundleWriter, vtkMRMLFileWriter);
  bool Write(vtkMRMLIOProperties* vtkNotUsed(properties)) override
  {
    vtkMRMLScene* scene = this->GetScene();
    scene->StartState(vtkMRMLScene::SaveState);
    for (int progress = 0; progress <= 100; progress += 4)
    {
      scene->ProgressState(vtkMRMLScene::SaveState, progress / 100.0);
    }
    scene->EndState(vtkMRMLScene::SaveState);
    return true;
  }
};
vtkStandardNewMacro(vtkTestSceneBundleWriter);

/// A writer that writes the text of text nodes.
class vtkTestTextWriter : public vtkMRMLFileWriter
{
public:
  static vtkTestTextWriter* New();
  vtkTypeMacro(vtkTestTextWriter, vtkMRMLFileWriter);
  bool Write(vtkMRMLIOProperties* properties) override
  {
    this->ClearWrittenNodeIDs();
    vtkMRMLTextNode* node = vtkMRMLTextNode::SafeDownCast(this->GetScene()->GetNodeByID(properties->GetStringProperty("nodeID")));
    if (!node)
    {
      return false;
    }
    vtksys::ofstream file(properties->GetStringProperty("fileName").c_str());
    file << node->GetText();
    this->AddWrittenNodeID(node->GetID());
    return true;
  }
};
vtkStandardNewMacro(vtkTestTextWriter);

/// Records the loading progress events of the manager
struct ProgressRecorder
{
  int NumberOfStartEvents{ 0 };
  int NumberOfEndEvents{ 0 };
  int NumberOfFilesAtStart{ 0 };
  std::string TitleAtStart;
  bool AbortableAtStart{ false };
  std::vector<int> FileIndices;
  std::vector<double> CurrentFileProgress;
  std::vector<double> Progress;
  /// Time of each progress event (universal time in seconds)
  std::vector<double> ProgressTimes;
  std::vector<std::string> Messages;
  /// If non-negative then loading is aborted when progress reaches this value
  double AbortAtProgress{ -1.0 };
  /// Call data of the events (the same object is expected in all events of an operation)
  vtkSmartPointer<vtkMRMLOperationProgress> ProgressObject;
  bool SameProgressObjectInAllEvents{ true };

  void Reset() { *this = ProgressRecorder(); }
};
ProgressRecorder Recorder;

//-----------------------------------------------------------------------------
void onLoadingEvent(vtkObject* vtkNotUsed(caller), unsigned long eid, void* vtkNotUsed(clientData), void* callData)
{
  vtkMRMLOperationProgress* progress = vtkMRMLOperationProgress::SafeDownCast(static_cast<vtkObject*>(callData));
  if (!progress)
  {
    Recorder.SameProgressObjectInAllEvents = false;
    return;
  }
  if (eid == vtkMRMLFileIOManager::OperationStartEvent)
  {
    Recorder.ProgressObject = progress;
  }
  else if (progress != Recorder.ProgressObject)
  {
    Recorder.SameProgressObjectInAllEvents = false;
  }
  switch (eid)
  {
    case vtkMRMLFileIOManager::OperationStartEvent:
      Recorder.NumberOfStartEvents++;
      Recorder.NumberOfFilesAtStart = progress->GetNumberOfFiles();
      Recorder.TitleAtStart = progress->GetTitle();
      Recorder.AbortableAtStart = progress->GetAbortable();
      break;
    case vtkMRMLFileIOManager::OperationProgressEvent:
      Recorder.Progress.push_back(progress->GetProgress());
      Recorder.ProgressTimes.push_back(vtkTimerLog::GetUniversalTime());
      Recorder.Messages.push_back(progress->GetProgressText());
      Recorder.FileIndices.push_back(progress->GetCurrentFileIndex());
      Recorder.CurrentFileProgress.push_back(progress->GetCurrentFileProgress());
      if (Recorder.AbortAtProgress >= 0.0 && progress->GetProgress() >= Recorder.AbortAtProgress)
      {
        progress->AbortExecuteOn();
      }
      break;
    case vtkMRMLFileIOManager::OperationEndEvent: Recorder.NumberOfEndEvents++; break;
    default: break;
  }
}

//-----------------------------------------------------------------------------
std::string createFile(const std::string& directory, const std::string& name)
{
  std::string path = directory + "/" + name;
  vtksys::ofstream file(path.c_str());
  file << "test";
  return path;
}

//-----------------------------------------------------------------------------
// One loading or saving operation is reported: progress starts at 0.0, never decreases, and ends at 1.0.
// The number of files is 0 (unknown amount of work) at the start when a single file is processed.
int checkOperation(int expectedNumberOfFiles, const std::string& expectedTitle = "Loading data", int minimumNumberOfProgressEvents = 20)
{
  CHECK_STD_STRING(Recorder.TitleAtStart, expectedTitle);
  CHECK_STD_STRING(Recorder.ProgressObject->GetTitle(), expectedTitle);
  CHECK_INT(Recorder.NumberOfStartEvents, 1);
  CHECK_INT(Recorder.NumberOfEndEvents, 1);
  CHECK_INT(Recorder.NumberOfFilesAtStart, expectedNumberOfFiles == 1 ? 0 : expectedNumberOfFiles);
  // Progress of loading scene files is reported, therefore the number of files is known at the end
  CHECK_INT(Recorder.ProgressObject->GetNumberOfFiles(), expectedNumberOfFiles);
  // Loading can only be aborted between files
  CHECK_BOOL(Recorder.AbortableAtStart, expectedNumberOfFiles > 1);
  // The index of the file that is being loaded
  for (size_t i = 0; i < Recorder.FileIndices.size(); i++)
  {
    CHECK_BOOL(Recorder.FileIndices[i] >= 0 && Recorder.FileIndices[i] < expectedNumberOfFiles, true);
    if (i > 0)
    {
      CHECK_BOOL(Recorder.FileIndices[i] >= Recorder.FileIndices[i - 1], true);
    }
  }
  CHECK_INT(Recorder.FileIndices.front(), 0);
  CHECK_INT(Recorder.FileIndices.back(), expectedNumberOfFiles - 1);
  // Progress of the current file: unknown (-1) when loading of a file starts,
  // then it is consistent with the overall progress (each file covers an equal part of the overall progress range)
  for (size_t i = 0; i < Recorder.CurrentFileProgress.size(); i++)
  {
    double fileProgress = Recorder.CurrentFileProgress[i];
    CHECK_BOOL(fileProgress == -1.0 || (fileProgress >= 0.0 && fileProgress <= 1.0), true);
    if (i == 0 || Recorder.FileIndices[i] != Recorder.FileIndices[i - 1])
    {
      CHECK_DOUBLE(fileProgress, -1.0);
    }
    if (fileProgress >= 0.0)
    {
      CHECK_DOUBLE_TOLERANCE(Recorder.Progress[i], (Recorder.FileIndices[i] + fileProgress) / expectedNumberOfFiles, 1e-9);
    }
  }
  CHECK_DOUBLE(Recorder.CurrentFileProgress.back(), 1.0);
  CHECK_BOOL(Recorder.SameProgressObjectInAllEvents, true);
  CHECK_BOOL(static_cast<int>(Recorder.Progress.size()) >= minimumNumberOfProgressEvents, true);
  for (size_t i = 0; i < Recorder.Progress.size(); i++)
  {
    CHECK_BOOL(Recorder.Progress[i] >= 0.0 && Recorder.Progress[i] <= 1.0, true);
    if (i > 0)
    {
      // Either the progress, the file progress, the file index, or the message changes
      bool progressChanged = Recorder.Progress[i] > Recorder.Progress[i - 1];
      bool fileProgressChanged = Recorder.CurrentFileProgress[i] != Recorder.CurrentFileProgress[i - 1];
      bool messageChanged = Recorder.Messages[i] != Recorder.Messages[i - 1];
      bool fileIndexChanged = Recorder.FileIndices[i] != Recorder.FileIndices[i - 1];
      CHECK_BOOL(progressChanged || fileProgressChanged || messageChanged || fileIndexChanged, true);
      CHECK_BOOL(Recorder.Progress[i] >= Recorder.Progress[i - 1], true);
    }
  }
  CHECK_DOUBLE(Recorder.Progress.front(), 0.0);
  CHECK_DOUBLE(Recorder.Progress.back(), 1.0);
  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
// Number of progress values reported while the specified file was loaded (in a list of numberOfFiles files)
int getNumberOfProgressValuesInFile(int fileIndex, int numberOfFiles)
{
  int count = 0;
  for (double progress : Recorder.Progress)
  {
    if (progress > static_cast<double>(fileIndex) / numberOfFiles && progress < static_cast<double>(fileIndex + 1) / numberOfFiles)
    {
      count++;
    }
  }
  return count;
}

} // namespace

//-----------------------------------------------------------------------------
int vtkMRMLFileIOManagerProgressTest(int argc, char* argv[])
{
  const std::string baseDirectory = (argc > 1) ? std::string(argv[1]) : vtksys::SystemTools::GetCurrentWorkingDirectory();
  const std::string directory = baseDirectory + "/vtkMRMLFileIOManagerProgressTest";
  vtksys::SystemTools::RemoveADirectory(directory);
  vtksys::SystemTools::MakeDirectory(directory);
  const std::string sceneFile = createFile(directory, "Scene.testscene");
  const std::string nestedFile = createFile(directory, "Nested.testnested");

  vtkNew<vtkMRMLFileIOManager> manager;
  vtkNew<vtkMRMLScene> scene;
  manager->SetScene(scene);
  // Report all progress changes (limiting of the update rate is tested at the end)
  CHECK_DOUBLE(manager->GetMaximumProgressUpdatesPerSecond(), 3.0);
  manager->SetMaximumProgressUpdatesPerSecond(0.0);

  vtkNew<vtkTestSceneReader> sceneReader;
  sceneReader->SetFileType("TestSceneFile");
  sceneReader->SetNameFilters(std::vector<std::string>{ "Test scene (*.testscene)" });
  manager->RegisterReader(sceneReader);
  vtkNew<vtkTestNestedLoadingReader> nestedReader;
  nestedReader->SetFileType("TestNestedFile");
  nestedReader->SetNameFilters(std::vector<std::string>{ "Test nested (*.testnested)" });
  manager->RegisterReader(nestedReader);

  vtkNew<vtkCallbackCommand> loadingCommand;
  loadingCommand->SetCallback(onLoadingEvent);
  manager->AddObserver(vtkMRMLFileIOManager::OperationStartEvent, loadingCommand);
  manager->AddObserver(vtkMRMLFileIOManager::OperationProgressEvent, loadingCommand);
  manager->AddObserver(vtkMRMLFileIOManager::OperationEndEvent, loadingCommand);
  // Standard VTK progress events are not invoked by the manager (the callback ignores them, but they would be
  // recorded as invalid events because their call data is not a vtkMRMLOperationProgress object)
  manager->AddObserver(vtkCommand::StartEvent, loadingCommand);
  manager->AddObserver(vtkCommand::ProgressEvent, loadingCommand);
  manager->AddObserver(vtkCommand::EndEvent, loadingCommand);

  // Loading a scene file reports the scene import progress
  Recorder.Reset();
  CHECK_BOOL(manager->LoadFile(sceneFile), true);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelNode"), NUMBER_OF_NODES_IN_SCENE);
  CHECK_EXIT_SUCCESS(checkOperation(1));
  CHECK_BOOL(Recorder.Messages.front().find("Scene.testscene") != std::string::npos, true);
  CHECK_BOOL(Recorder.ProgressObject->GetProgressText().find("Scene.testscene") != std::string::npos, true);
  CHECK_DOUBLE(Recorder.ProgressObject->GetProgress(), 1.0);
  CHECK_BOOL(Recorder.ProgressObject->GetAbortExecute(), false);

  // The scene is no longer observed after loading is completed
  Recorder.Reset();
  scene->SetLoadFromXMLString(1);
  scene->SetSceneXMLString(createSceneXML(10));
  CHECK_BOOL(scene->Import() != 0, true);
  scene->SetLoadFromXMLString(0);
  CHECK_INT(static_cast<int>(Recorder.Progress.size()), 0);
  CHECK_INT(Recorder.NumberOfStartEvents, 0);

  // A reader that loads a scene file using the manager: it is reported as a single loading operation,
  // and the scene import progress is reported once
  Recorder.Reset();
  vtkNew<vtkMRMLIOProperties> nestedProperties;
  nestedProperties->SetStringProperty("fileName", nestedFile);
  nestedProperties->SetStringProperty("sceneFileName", sceneFile);
  CHECK_BOOL(manager->LoadNodes("TestNestedFile", nestedProperties), true);
  CHECK_EXIT_SUCCESS(checkOperation(1));
  CHECK_BOOL(getNumberOfProgressValuesInFile(0, 1) >= 10, true);

  // Loading a list of files: single loading operation, each file covers an equal part of the progress range,
  // and progress of loading each file is reported
  Recorder.Reset();
  vtkNew<vtkMRMLIOProperties> listProperties;
  listProperties->SetStringListProperty("fileName", std::vector<std::string>{ sceneFile, sceneFile, sceneFile, sceneFile });
  CHECK_BOOL(manager->LoadNodes("TestSceneFile", listProperties), true);
  CHECK_EXIT_SUCCESS(checkOperation(4));
  for (int fileIndex = 0; fileIndex < 4; fileIndex++)
  {
    CHECK_BOOL(getNumberOfProgressValuesInFile(fileIndex, 4) >= 5, true);
  }

  // Loading a list of files that are specified by separate properties
  Recorder.Reset();
  vtkNew<vtkCollection> propertiesList;
  for (int i = 0; i < 2; i++)
  {
    vtkNew<vtkMRMLIOProperties> fileProperties;
    fileProperties->SetStringProperty("fileType", "TestSceneFile");
    fileProperties->SetStringProperty("fileName", sceneFile);
    propertiesList->AddItem(fileProperties);
  }
  CHECK_BOOL(manager->LoadNodesFromList(propertiesList), true);
  CHECK_EXIT_SUCCESS(checkOperation(2));
  CHECK_BOOL(getNumberOfProgressValuesInFile(0, 2) >= 5, true);
  CHECK_BOOL(getNumberOfProgressValuesInFile(1, 2) >= 5, true);

  // Abort requests are ignored if the operation is not abortable (single file): the file is loaded and progress is completed
  Recorder.Reset();
  Recorder.AbortAtProgress = 0.0;
  CHECK_BOOL(manager->LoadFile(sceneFile), true);
  CHECK_EXIT_SUCCESS(checkOperation(1));

  // Abort requests are ignored when a reader of a non-abortable operation loads a list of files using the manager
  Recorder.Reset();
  Recorder.AbortAtProgress = 0.0;
  int numberOfModelsBeforeNestedList = scene->GetNumberOfNodesByClass("vtkMRMLModelNode");
  vtkNew<vtkMRMLIOProperties> nestedListProperties;
  nestedListProperties->SetStringProperty("fileName", nestedFile);
  nestedListProperties->SetStringListProperty("sceneFileName", std::vector<std::string>{ sceneFile, sceneFile });
  CHECK_BOOL(manager->LoadNodes("TestNestedFile", nestedListProperties), true);
  CHECK_EXIT_SUCCESS(checkOperation(1));
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelNode") - numberOfModelsBeforeNestedList, 2 * NUMBER_OF_NODES_IN_SCENE);

  // Aborting: loading stops after the file that is being loaded
  Recorder.Reset();
  Recorder.AbortAtProgress = 0.3;
  int numberOfModelsBefore = scene->GetNumberOfNodesByClass("vtkMRMLModelNode");
  vtkNew<vtkMRMLMessageCollection> abortUserMessages;
  CHECK_BOOL(manager->LoadNodes("TestSceneFile", listProperties, nullptr, abortUserMessages), false);
  // The user is informed about how many files were loaded
  CHECK_INT(abortUserMessages->GetNumberOfMessagesOfType(vtkCommand::WarningEvent), 1);
  CHECK_BOOL(abortUserMessages->GetAllMessagesAsString().find("2 of 4") != std::string::npos, true);
  CHECK_BOOL(Recorder.ProgressObject->GetAbortExecute(), true);
  vtkSmartPointer<vtkMRMLOperationProgress> abortedProgressObject = Recorder.ProgressObject;
  CHECK_INT(Recorder.NumberOfStartEvents, 1);
  CHECK_INT(Recorder.NumberOfEndEvents, 1);
  // Aborted while loading the second file
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelNode") - numberOfModelsBefore, 2 * NUMBER_OF_NODES_IN_SCENE);
  CHECK_BOOL(Recorder.Progress.back() < 1.0, true);

  // Abort request while the last file is loaded: all files are loaded, therefore loading is not aborted
  Recorder.Reset();
  Recorder.AbortAtProgress = 0.8;
  abortUserMessages->ClearMessages();
  numberOfModelsBefore = scene->GetNumberOfNodesByClass("vtkMRMLModelNode");
  CHECK_BOOL(manager->LoadNodes("TestSceneFile", listProperties, nullptr, abortUserMessages), true);
  CHECK_INT(abortUserMessages->GetNumberOfMessagesOfType(vtkCommand::WarningEvent), 0);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLModelNode") - numberOfModelsBefore, 4 * NUMBER_OF_NODES_IN_SCENE);
  CHECK_DOUBLE(Recorder.Progress.back(), 1.0);
  CHECK_INT(Recorder.NumberOfEndEvents, 1);

  // The next loading operation uses a new progress object (the abort request does not affect it)
  Recorder.Reset();
  CHECK_BOOL(manager->LoadFile(sceneFile), true);
  CHECK_EXIT_SUCCESS(checkOperation(1));
  CHECK_BOOL(Recorder.ProgressObject != abortedProgressObject, true);
  CHECK_BOOL(Recorder.ProgressObject->GetAbortExecute(), false);

  //
  // Saving
  //

  vtkNew<vtkTestSceneBundleWriter> sceneBundleWriter;
  sceneBundleWriter->SetFileType("SceneFile");
  sceneBundleWriter->SetNameFilters(std::vector<std::string>{ "Test bundle (*.testbundle)" });
  manager->RegisterWriter(sceneBundleWriter);
  vtkNew<vtkTestTextWriter> textWriter;
  textWriter->SetFileType("TextFile");
  textWriter->SetNameFilters(std::vector<std::string>{ "Text (.txt)" });
  textWriter->SetNodeClassName("vtkMRMLTextNode");
  manager->RegisterWriter(textWriter);

  // Saving a scene reports the save progress of the scene
  Recorder.Reset();
  const std::string bundleFile = directory + "/Scene.testbundle";
  CHECK_BOOL(manager->SaveScene(bundleFile), true);
  CHECK_EXIT_SUCCESS(checkOperation(1, "Saving data"));
  CHECK_BOOL(Recorder.Messages.front().find("Scene.testbundle") != std::string::npos, true);

  // Exporting nodes: each exported node counts as one file, exporting is abortable
  std::vector<std::string> textNodeIDs;
  std::vector<std::string> textFileNames;
  for (int i = 0; i < 3; i++)
  {
    vtkMRMLTextNode* textNode = vtkMRMLTextNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTextNode"));
    textNode->SetText(("Text " + std::to_string(i)).c_str());
    textNodeIDs.push_back(textNode->GetID());
    textFileNames.push_back(directory + "/Text" + std::to_string(i) + ".txt");
  }
  vtkNew<vtkMRMLIOProperties> exportProperties;
  exportProperties->SetStringProperty("fileFormat", "Text (.txt)");
  Recorder.Reset();
  CHECK_BOOL(manager->ExportNodes(textNodeIDs, textFileNames, exportProperties, false), true);
  CHECK_EXIT_SUCCESS(checkOperation(3, "Saving data", 6));
  for (const std::string& textFileName : textFileNames)
  {
    CHECK_BOOL(vtksys::SystemTools::FileExists(textFileName, true), true);
    vtksys::SystemTools::RemoveFile(textFileName);
  }

  // Aborting exporting: it stops after the node that is being exported
  Recorder.Reset();
  Recorder.AbortAtProgress = 0.3;
  abortUserMessages->ClearMessages();
  CHECK_BOOL(manager->ExportNodes(textNodeIDs, textFileNames, exportProperties, false, abortUserMessages), false);
  CHECK_INT(abortUserMessages->GetNumberOfMessagesOfType(vtkCommand::WarningEvent), 1);
  CHECK_BOOL(abortUserMessages->GetAllMessagesAsString().find("1 of 3") != std::string::npos, true);
  CHECK_INT(Recorder.NumberOfStartEvents, 1);
  CHECK_INT(Recorder.NumberOfEndEvents, 1);
  CHECK_BOOL(vtksys::SystemTools::FileExists(textFileNames[0], true), true);
  CHECK_BOOL(vtksys::SystemTools::FileExists(textFileNames[1], true), false);
  CHECK_BOOL(vtksys::SystemTools::FileExists(textFileNames[2], true), false);
  vtksys::SystemTools::RemoveFile(textFileNames[0]);

  // Nodes that cannot be exported do not prevent exporting of the other nodes
  // (export options are invalid for the second node, the third node does not exist)
  vtkNew<vtkCollection> exportPropertiesList;
  for (int i = 0; i < 4; i++)
  {
    vtkNew<vtkMRMLIOProperties> nodeExportProperties;
    nodeExportProperties->SetStringProperty("nodeID", i == 2 ? "vtkMRMLTextNodeNonExistent" : textNodeIDs[i % 3]);
    nodeExportProperties->SetStringProperty("fileName", directory + "/PartialExport" + std::to_string(i) + ".txt");
    if (i != 1)
    {
      nodeExportProperties->SetStringProperty("fileFormat", "Text (.txt)");
    }
    exportPropertiesList->AddItem(nodeExportProperties);
  }
  vtkNew<vtkMRMLMessageCollection> partialExportUserMessages;
  Recorder.Reset();
  TESTING_OUTPUT_RESET();
  CHECK_BOOL(manager->ExportNodes(exportPropertiesList, false, partialExportUserMessages), false);
  TESTING_OUTPUT_ASSERT_ERRORS(1); // missing "fileFormat" property
  TESTING_OUTPUT_RESET();
  CHECK_INT(partialExportUserMessages->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent), 1); // node not found
  CHECK_INT(Recorder.NumberOfStartEvents, 1);
  CHECK_INT(Recorder.NumberOfEndEvents, 1);
  for (int i = 0; i < 4; i++)
  {
    const std::string partialExportFileName = directory + "/PartialExport" + std::to_string(i) + ".txt";
    CHECK_BOOL(vtksys::SystemTools::FileExists(partialExportFileName, true), i == 0 || i == 3);
    vtksys::SystemTools::RemoveFile(partialExportFileName);
  }

  // Limiting the progress update rate: progress events are not more frequent than the maximum update rate,
  // except the last one (which reports the final state)
  manager->SetMaximumProgressUpdatesPerSecond(3.0);
  Recorder.Reset();
  vtkNew<vtkMRMLIOProperties> manyFilesProperties;
  manyFilesProperties->SetStringListProperty("fileName", std::vector<std::string>(40, sceneFile));
  CHECK_BOOL(manager->LoadNodes("TestSceneFile", manyFilesProperties), true);
  CHECK_INT(Recorder.NumberOfStartEvents, 1);
  CHECK_INT(Recorder.NumberOfEndEvents, 1);
  CHECK_BOOL(Recorder.Progress.size() >= 2, true);
  for (size_t i = 1; i + 1 < Recorder.ProgressTimes.size(); i++)
  {
    CHECK_BOOL(Recorder.ProgressTimes[i] - Recorder.ProgressTimes[i - 1] >= 1.0 / 3.0 - 1e-6, true);
  }
  CHECK_DOUBLE(Recorder.Progress.back(), 1.0);
  CHECK_DOUBLE(Recorder.CurrentFileProgress.back(), 1.0);

  // Processing of each new file is reported immediately (regardless of the maximum update rate),
  // because processing of a file may take a long time without any progress reported
  Recorder.Reset();
  std::vector<std::string> differentSceneFiles;
  for (int i = 0; i < 10; i++)
  {
    differentSceneFiles.push_back(createFile(directory, "Scene" + std::to_string(i) + ".testscene"));
  }
  manyFilesProperties->SetStringListProperty("fileName", differentSceneFiles);
  CHECK_BOOL(manager->LoadNodes("TestSceneFile", manyFilesProperties), true);
  for (int i = 0; i < 10; i++)
  {
    const std::string fileName = "Scene" + std::to_string(i) + ".testscene";
    bool fileReported = false;
    for (size_t eventIndex = 0; eventIndex < Recorder.Messages.size(); eventIndex++)
    {
      if (Recorder.Messages[eventIndex].find(fileName) != std::string::npos && Recorder.FileIndices[eventIndex] == i)
      {
        fileReported = true;
        break;
      }
    }
    CHECK_BOOL(fileReported, true);
  }

  vtksys::SystemTools::RemoveADirectory(directory);
  return EXIT_SUCCESS;
}
