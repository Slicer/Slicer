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

// Slicer includes
#include "vtkMRMLAbstractLogic.h"
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLFileIOManager.h"
#include "vtkMRMLFileReader.h"
#include "vtkMRMLFileWriter.h"
#include "vtkMRMLIOOptionsDescription.h"
#include "vtkMRMLIOProperties.h"
#include "vtkMRMLModuleLogic.h"
#include "vtkMRMLNodeWriter.h"
#include "vtkMRMLCoreTestingMacros.h"

// MRML includes
#include <vtkMRMLJsonElement.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTextNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkVariantArray.h>
#include <vtkWeakPointer.h>
#include <vtksys/FStream.hxx>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <cmath>
#include <iostream>
#include <limits>
#include <map>

namespace
{

/// A reader that looks inside files: it is sure of the ones whose name says "sure".
class vtkSureReader : public vtkMRMLFileReader
{
public:
  static vtkSureReader* New();
  vtkTypeMacro(vtkSureReader, vtkMRMLFileReader);
  double CanLoadFileConfidence(const std::string& filePath) override
  {
    if (filePath.find("sure") != std::string::npos)
    {
      return 0.9;
    }
    return this->Superclass::CanLoadFileConfidence(filePath);
  }
};
vtkStandardNewMacro(vtkSureReader);

/// A reader that adds a text node to the scene with the content of the file.
class vtkTextFileReader : public vtkMRMLFileReader
{
public:
  static vtkTextFileReader* New();
  vtkTypeMacro(vtkTextFileReader, vtkMRMLFileReader);
  bool Load(vtkMRMLIOProperties* properties) override
  {
    this->ClearLoadedNodeIDs();
    vtkMRMLTextNode* node = vtkMRMLTextNode::SafeDownCast(this->GetScene()->AddNewNodeByClass("vtkMRMLTextNode", properties->GetStringProperty("name", "Text")));
    node->SetText(properties->GetStringProperty("fileName").c_str());
    this->AddLoadedNodeID(node->GetID());
    this->GetUserMessages()->AddMessage(vtkCommand::WarningEvent, "loaded");
    return true;
  }
};
vtkStandardNewMacro(vtkTextFileReader);

/// A writer that writes the text of text nodes.
class vtkTextFileWriter : public vtkMRMLFileWriter
{
public:
  static vtkTextFileWriter* New();
  vtkTypeMacro(vtkTextFileWriter, vtkMRMLFileWriter);
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
vtkStandardNewMacro(vtkTextFileWriter);

/// A reader that describes options, with defaults that depend on the file name and on other options.
class vtkOptionsReader : public vtkMRMLFileReader
{
public:
  static vtkOptionsReader* New();
  vtkTypeMacro(vtkOptionsReader, vtkMRMLFileReader);
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override
  {
    bool isLabel = description->GetFileName().find("label") != std::string::npos;
    description->AddBoolOption("labelmap", "Label map", "Load as label map", isLabel);
    description->AddEnumOption("colorNodeID", "Color", "", vtkVariant(description->GetBoolOptionValue("labelmap") ? "Labels" : "Grey"));
    description->AddEnumChoice("colorNodeID", vtkVariant("Grey"), "Grey \"scale\"");
    description->AddEnumChoice("colorNodeID", vtkVariant("Labels"), "Labels");
    description->AddIntOption("count", "Count", "", 3, 0, 10);
    description->SetOptionEnabled("count", !description->GetBoolOptionValue("labelmap"));
    description->AddStringListOption("name", "", "", std::vector<std::string>{ "a", "b" });
  }
};
vtkStandardNewMacro(vtkOptionsReader);

/// A module logic that provides a reader and a writer
class vtkReaderRegisteringLogic : public vtkMRMLModuleLogic
{
public:
  static vtkReaderRegisteringLogic* New();
  vtkTypeMacro(vtkReaderRegisteringLogic, vtkMRMLModuleLogic);

  std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> CreateFileIOHandlers() override
  {
    vtkNew<vtkSureReader> reader;
    reader->SetDescription("Logic reader");
    return { reader.GetPointer(), vtkMRMLNodeWriter::CreateNodeWriter("Logic writer", "LogicFile", { "vtkMRMLTextNode" }) };
  }
};
vtkStandardNewMacro(vtkReaderRegisteringLogic);

/// A logic that is not a vtkMRMLModuleLogic (it cannot register readers and writers)
class vtkPlainLogic : public vtkMRMLAbstractLogic
{
public:
  static vtkPlainLogic* New();
  vtkTypeMacro(vtkPlainLogic, vtkMRMLAbstractLogic);
};
vtkStandardNewMacro(vtkPlainLogic);

/// Count events
int NewFileLoadedEventCount = 0;
int FileSavedEventCount = 0;
void onFileIOManagerEvent(vtkObject*, unsigned long eid, void*, void* callData)
{
  vtkMRMLIOProperties* properties = reinterpret_cast<vtkMRMLIOProperties*>(callData);
  if (eid == vtkMRMLFileIOManager::NewFileLoadedEvent && properties && properties->HasProperty("nodeIDs"))
  {
    ++NewFileLoadedEventCount;
  }
  if (eid == vtkMRMLFileIOManager::FileSavedEvent && properties)
  {
    ++FileSavedEventCount;
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

} // namespace

//-----------------------------------------------------------------------------
int vtkMRMLFileIOManagerTest1(int argc, char* argv[])
{
  // Files that the readers are asked about must exist.
  // They are created in the temporary directory (first argument), or in the current directory.
  const std::string baseDirectory = (argc > 1) ? std::string(argv[1]) : vtksys::SystemTools::GetCurrentWorkingDirectory();
  const std::string directory = baseDirectory + "/vtkMRMLFileIOManagerTest1";
  vtksys::SystemTools::RemoveADirectory(directory);
  vtksys::SystemTools::MakeDirectory(directory);
  const std::string sureSegFile = createFile(directory, "sure.seg.nrrd");
  const std::string segFile = createFile(directory, "Head.seg.nrrd");
  const std::string nrrdFile = createFile(directory, "Head.NRRD");
  const std::string vtkFile = createFile(directory, "Head.vtk");
  const std::string textFile = createFile(directory, "Note.txt");
  const std::string missingFile = directory + "/Missing.nrrd";

  // Name filter parsing and wildcard matching
  CHECK_INT(static_cast<int>(vtkMRMLFileIOHandler::NameFilterToWildcards("Image (*.jpg *.png)").size()), 2);
  CHECK_STD_STRING(vtkMRMLFileIOHandler::NameFilterToWildcards("*.mrml")[0], "*.mrml");
  CHECK_STD_STRING(vtkMRMLFileIOHandler::NameFilterToWildcards("All Files (*)")[0], "*");
  CHECK_BOOL(vtkMRMLFileIOHandler::WildcardMatch("*.seg.nrrd", "Head.SEG.nrrd"), true);
  CHECK_BOOL(vtkMRMLFileIOHandler::WildcardMatch("*.nrrd", "Head.nrrd.gz"), false);
  CHECK_BOOL(vtkMRMLFileIOHandler::WildcardMatch("image?.[pj]ng", "image1.png"), true);
  CHECK_BOOL(vtkMRMLFileIOHandler::WildcardMatch("image?.[!pj]ng", "image1.png"), false);
  CHECK_BOOL(vtkMRMLFileIOHandler::WildcardMatch("*", "anything"), true);

  vtkNew<vtkMRMLFileIOManager> manager;
  vtkNew<vtkMRMLScene> scene;
  manager->SetScene(scene);

  // A reader that goes by the extension, and one that looks inside the file
  vtkNew<vtkMRMLFileReader> extensionReader;
  extensionReader->SetFileType("VolumeFile");
  extensionReader->SetDescription("Volume");
  extensionReader->SetNameFilters(std::vector<std::string>{ "Volume (*.nrrd *.seg.nrrd)" });
  vtkNew<vtkSureReader> sureReader;
  sureReader->SetFileType("SegmentationFile");
  sureReader->SetDescription("Segmentation");
  sureReader->SetNameFilters(std::vector<std::string>{ "Segmentation (*.seg.nrrd)" });

  manager->RegisterReader(extensionReader);
  manager->RegisterReader(sureReader);
  manager->RegisterReader(sureReader); // registering again does nothing
  CHECK_INT(manager->GetNumberOfReaders(), 2);
  CHECK_POINTER(extensionReader->GetFileIOManager(), manager.GetPointer());
  CHECK_POINTER(extensionReader->GetScene(), scene.GetPointer());

  // Extensions from the name filters, compound ones included
  CHECK_STD_STRING(extensionReader->GetMatchedExtension("/data/Head.seg.nrrd"), ".seg.nrrd");
  CHECK_STD_STRING(extensionReader->GetMatchedExtension("/data/Head.NRRD"), ".nrrd");
  CHECK_BOOL(extensionReader->MatchesExtension("/data/Head.vtk"), false);

  // Description may contain brackets, extensions are taken from the last pair of brackets
  vtkNew<vtkMRMLFileReader> bracketReader;
  bracketReader->SetNameFilters(std::vector<std::string>{ "Markups (JSON) (*.mrk.json)" });
  CHECK_INT(static_cast<int>(bracketReader->GetExtensions().size()), 1);
  CHECK_STD_STRING(bracketReader->GetMatchedExtension("/data/F.mrk.json"), ".mrk.json");

  // Default confidence: 0.5 + 0.01 * length of the matched extension, only for existing files
  CHECK_DOUBLE_TOLERANCE(extensionReader->CanLoadFileConfidence(segFile), 0.59, 1e-6);
  CHECK_DOUBLE_TOLERANCE(extensionReader->CanLoadFileConfidence(nrrdFile), 0.55, 1e-6);
  CHECK_DOUBLE(extensionReader->CanLoadFileConfidence(vtkFile), 0.0);
  CHECK_DOUBLE(extensionReader->CanLoadFileConfidence(missingFile), 0.0);
  CHECK_INT(static_cast<int>(extensionReader->GetSupportedNameFilters(segFile).size()), 1);
  int longestExtensionMatch = 0;
  CHECK_INT(static_cast<int>(extensionReader->GetSupportedNameFilters(segFile, longestExtensionMatch).size()), 1);
  CHECK_INT(longestExtensionMatch, 9); // ".seg.nrrd"

  // The reader that is surest comes first; ties keep the order of registration
  std::vector<vtkSmartPointer<vtkMRMLFileReader>> readers = manager->GetReadersForFile(sureSegFile);
  CHECK_INT(static_cast<int>(readers.size()), 2);
  CHECK_POINTER(readers[0].GetPointer(), sureReader.GetPointer());
  CHECK_POINTER(manager->GetReaderForFile(segFile), extensionReader.GetPointer());
  CHECK_STD_STRING(manager->GetFileTypeForFile(sureSegFile), "SegmentationFile");
  CHECK_NULL(manager->GetReaderForFile(vtkFile));
  CHECK_STD_STRING(manager->GetFileTypeForFile(vtkFile), "");
  std::vector<std::string> fileTypes = manager->GetFileTypesForFile(sureSegFile);
  CHECK_INT(static_cast<int>(fileTypes.size()), 2);
  CHECK_STD_STRING(fileTypes[0], "SegmentationFile");
  CHECK_STD_STRING(manager->GetFileDescriptionsForFile(sureSegFile)[0], "Segmentation");
  CHECK_STD_STRING(manager->GetFileTypeFromDescription("Volume"), "VolumeFile");
  CHECK_POINTER(manager->GetReaderByDescription("Segmentation"), sureReader.GetPointer());

  // No reader recognizes a group of files: the list is not changed
  std::vector<std::string> fileList{ segFile, nrrdFile };
  vtkNew<vtkMRMLIOProperties> examineProperties;
  CHECK_NULL(manager->ExamineFileList(fileList, examineProperties));
  CHECK_INT(static_cast<int>(fileList.size()), 2);

  // What the file types are
  CHECK_INT(static_cast<int>(manager->GetReaderFileTypes().size()), 2);
  CHECK_STD_STRING(manager->GetDescriptionForFileType("SegmentationFile", false), "Segmentation");
  CHECK_INT(static_cast<int>(manager->GetExtensionsForFileType("VolumeFile", false).size()), 2);
  CHECK_INT(static_cast<int>(manager->GetNameFiltersForFileType("VolumeFile", false).size()), 1);
  CHECK_INT(static_cast<int>(manager->GetReadersForFileType("VolumeFile").size()), 1);

  // Unregistering removes the reader
  manager->UnregisterHandler(sureReader);
  CHECK_INT(manager->GetNumberOfReaders(), 1);
  CHECK_NULL(sureReader->GetFileIOManager());

  // RegisterHandler() adds a reader or a writer, depending on the type of the handler
  vtkSmartPointer<vtkMRMLNodeWriter> nodeWriter = vtkMRMLNodeWriter::CreateNodeWriter("Texts", "TextFile", { "vtkMRMLTextNode" }, false);
  CHECK_STD_STRING(nodeWriter->GetDescription(), "Texts");
  CHECK_STD_STRING(nodeWriter->GetFileType(), "TextFile");
  CHECK_BOOL(nodeWriter->GetSupportUseCompression(), false);
  CHECK_INT(static_cast<int>(nodeWriter->GetNodeClassNames().size()), 1);
  CHECK_NULL(nodeWriter->GetFileIOManager()); // not registered
  const int numberOfWritersBeforeRegister = manager->GetNumberOfWriters();
  manager->RegisterHandler(nodeWriter);
  CHECK_INT(manager->GetNumberOfWriters(), numberOfWritersBeforeRegister + 1);
  CHECK_POINTER(nodeWriter->GetFileIOManager(), manager.GetPointer());
  manager->RegisterHandler(sureReader);
  CHECK_INT(manager->GetNumberOfReaders(), 2);
  manager->UnregisterHandler(nodeWriter);
  manager->UnregisterHandler(sureReader);
  CHECK_INT(manager->GetNumberOfWriters(), numberOfWritersBeforeRegister);
  CHECK_INT(manager->GetNumberOfReaders(), 1);
  CHECK_POINTER(manager->GetNthReader(0), extensionReader.GetPointer());
  CHECK_NULL(sureReader->GetFileIOManager());
  manager->UnregisterHandler(extensionReader);
  CHECK_INT(manager->GetNumberOfReaders(), 0);

  // Loading
  vtkNew<vtkCallbackCommand> callback;
  callback->SetCallback(onFileIOManagerEvent);
  manager->AddObserver(vtkMRMLFileIOManager::NewFileLoadedEvent, callback);
  manager->AddObserver(vtkMRMLFileIOManager::FileSavedEvent, callback);
  vtkNew<vtkTextFileReader> textReader;
  textReader->SetFileType("TextFile");
  textReader->SetDescription("Text");
  textReader->SetNameFilters(std::vector<std::string>{ "Text (*.txt)" });
  manager->RegisterReader(textReader);
  vtkNew<vtkCollection> loadedNodes;
  vtkNew<vtkMRMLMessageCollection> userMessages;
  CHECK_BOOL(manager->LoadFile(textFile, userMessages), true);
  CHECK_INT(static_cast<int>(textReader->GetLoadedNodeIDs().size()), 1);
  CHECK_INT(scene->GetNumberOfNodesByClass("vtkMRMLTextNode"), 1);
  CHECK_INT(NewFileLoadedEventCount, 1);
  CHECK_INT(userMessages->GetNumberOfMessagesOfType(vtkCommand::WarningEvent), 1);
  // List of files, with names
  vtkNew<vtkMRMLIOProperties> loadProperties;
  loadProperties->SetStringListProperty("fileName", std::vector<std::string>{ textFile, textFile });
  loadProperties->SetStringListProperty("name", std::vector<std::string>{ "First", "Second" });
  CHECK_BOOL(manager->LoadNodes("TextFile", loadProperties, loadedNodes), true);
  CHECK_INT(loadedNodes->GetNumberOfItems(), 2);
  CHECK_STRING(vtkMRMLNode::SafeDownCast(loadedNodes->GetItemAsObject(1))->GetName(), "Second");
  CHECK_INT(NewFileLoadedEventCount, 3);
  // Files that do not exist or that no reader can read
  userMessages->ClearMessages();
  CHECK_BOOL(manager->LoadFile(missingFile, userMessages), false);
  CHECK_BOOL(userMessages->GetNumberOfMessagesOfType(vtkCommand::ErrorEvent) > 0, true);

  // Writers
  vtkNew<vtkTextFileWriter> writer;
  writer->SetFileType("TextFile");
  writer->SetDescription("Text");
  writer->SetNameFilters(std::vector<std::string>{ "Text (.txt)" });
  writer->SetNodeClassName("vtkMRMLTextNode");
  manager->RegisterWriter(writer);
  CHECK_INT(manager->GetNumberOfWriters(), 1);
  CHECK_BOOL(writer->IsWriter(), true);
  vtkMRMLNode* textNode = vtkMRMLNode::SafeDownCast(loadedNodes->GetItemAsObject(0));
  CHECK_POINTER(manager->GetWriterForObject(textNode), writer.GetPointer());
  CHECK_POINTER(manager->GetWriterForObject(textNode, "Text (.txt)"), writer.GetPointer());
  CHECK_STD_STRING(manager->GetFileWriterFileType(textNode), "TextFile");
  CHECK_STD_STRING(manager->GetFileWriterFileType(scene), "");
  CHECK_INT(static_cast<int>(manager->GetWritersForObject(textNode).size()), 1);
  CHECK_INT(static_cast<int>(manager->GetWritersForFileType("TextFile").size()), 1);
  CHECK_STD_STRING(manager->GetFileWriterExtensions(textNode)[0], "Text (.txt)");
  CHECK_STD_STRING(manager->GetFileWriterDescriptions("TextFile")[0], "Text");
  CHECK_STD_STRING(writer->GetNodeClassName(), "vtkMRMLTextNode");
  CHECK_STD_STRING(manager->ExtractKnownExtension("Note.txt", textNode), ".txt");
  CHECK_STD_STRING(manager->StripKnownExtension("Note.txt.txt", textNode), "Note");

  vtkNew<vtkMRMLIOProperties> saveProperties;
  const std::string savedFile = directory + "/subdirectory/Saved.txt";
  saveProperties->SetStringProperty("fileName", savedFile);
  saveProperties->SetStringProperty("nodeID", textNode->GetID());
  CHECK_BOOL(manager->SaveNodes("TextFile", saveProperties), true);
  CHECK_STD_STRING(writer->GetWrittenNodeIDs()[0], textNode->GetID());
  CHECK_BOOL(vtksys::SystemTools::FileExists(savedFile, true), true);
  CHECK_INT(FileSavedEventCount, 1);
  // Writing to a file that the writer does not support
  saveProperties->SetStringProperty("fileName", directory + "/Saved.nrrd");
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  CHECK_BOOL(manager->SaveNodes("TextFile", saveProperties), false);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  // File name utilities
  CHECK_STD_STRING(vtkMRMLFileIOManager::ForceFileNameValidCharacters(" a/b:c*d?(e) "), "abcd(e)");
  manager->SetDefaultSceneFileType("MRML Scene (.mrml)");
  CHECK_STD_STRING(manager->GetDefaultSceneFileType(), "MRML Scene (.mrml)");

  // Properties: typed values, defaults, order, copy
  vtkNew<vtkMRMLIOProperties> properties;
  properties->SetStringProperty("fileName", "/data/Head.nrrd");
  properties->SetBoolProperty("labelmap", true);
  properties->SetIntProperty("count", 3);
  properties->SetDoubleProperty("spacing", 0.5);
  properties->SetStringListProperty("fileNames", std::vector<std::string>{ "a.png", "b.png" });
  vtkNew<vtkImageData> image;
  properties->SetObjectProperty("screenShot", image);
  CHECK_STD_STRING(properties->GetStringProperty("fileName"), "/data/Head.nrrd");
  CHECK_BOOL(properties->GetBoolProperty("labelmap"), true);
  CHECK_BOOL(properties->IsBoolProperty("labelmap"), true);
  CHECK_BOOL(properties->IsBoolProperty("count"), false);
  CHECK_INT(properties->GetIntProperty("count"), 3);
  CHECK_DOUBLE(properties->GetDoubleProperty("spacing"), 0.5);
  CHECK_INT(properties->GetIntProperty("missing", 7), 7);
  CHECK_BOOL(properties->HasProperty("missing"), false);
  CHECK_BOOL(properties->IsStringListProperty("fileNames"), true);
  CHECK_INT(static_cast<int>(properties->GetStringListProperty("fileNames").size()), 2);
  CHECK_BOOL(properties->IsObjectProperty("screenShot"), true);
  CHECK_POINTER(properties->GetObjectProperty("screenShot"), image.GetPointer());
  CHECK_STD_STRING(properties->GetPropertyNames()[0], "fileName");
  vtkNew<vtkMRMLIOProperties> copy;
  copy->Copy(properties);
  CHECK_INT(static_cast<int>(copy->GetPropertyNames().size()), 6);
  CHECK_BOOL(copy->IsBoolProperty("labelmap"), true);
  CHECK_INT(static_cast<int>(copy->GetStringListProperty("fileNames").size()), 2);
  properties->RemoveProperty("count");
  CHECK_BOOL(properties->HasProperty("count"), false);

  // List and map (nested properties) values
  vtkNew<vtkVariantArray> listValues;
  listValues->InsertNextValue(vtkVariant("text"));
  listValues->InsertNextValue(vtkVariant(12));
  listValues->InsertNextValue(vtkVariant(2.5));
  listValues->InsertNextValue(vtkVariant(true));
  properties->SetListProperty("list", listValues);
  vtkNew<vtkMRMLIOProperties> mapValues;
  mapValues->SetStringListProperty("path", std::vector<std::string>{ "a", "b" });
  mapValues->SetListProperty("nestedList", listValues);
  properties->SetMapProperty("map", mapValues);
  CHECK_BOOL(properties->IsListProperty("list"), true);
  CHECK_BOOL(properties->IsObjectProperty("list"), false);
  CHECK_BOOL(properties->IsMapProperty("map"), true);
  CHECK_BOOL(properties->IsObjectProperty("map"), false);
  vtkNew<vtkVariantArray> retrievedList;
  CHECK_BOOL(properties->GetListProperty("list", retrievedList), true);
  CHECK_INT(static_cast<int>(retrievedList->GetNumberOfValues()), 4);
  CHECK_STD_STRING(retrievedList->GetValue(0).ToString(), "text");
  CHECK_INT(retrievedList->GetValue(1).ToInt(), 12);
  CHECK_DOUBLE(retrievedList->GetValue(2).ToDouble(), 2.5);
  CHECK_BOOL(retrievedList->GetValue(3).IsChar(), true); // boolean
  CHECK_INT(static_cast<int>(properties->GetStringListProperty("list").size()), 4);
  CHECK_NOT_NULL(properties->GetMapProperty("map"));
  CHECK_INT(static_cast<int>(properties->GetMapProperty("map")->GetStringListProperty("path").size()), 2);
  // Values are copied, changing the original does not change the stored values
  mapValues->RemoveAllProperties();
  CHECK_BOOL(properties->GetMapProperty("map")->IsListProperty("nestedList"), true);
  vtkNew<vtkMRMLIOProperties> copyWithNested;
  copyWithNested->Copy(properties);
  CHECK_BOOL(copyWithNested->IsListProperty("list"), true);
  CHECK_BOOL(copyWithNested->IsMapProperty("map"), true);
  CHECK_BOOL(copyWithNested->GetMapProperty("map") != properties->GetMapProperty("map"), true);
  CHECK_INT(static_cast<int>(copyWithNested->GetMapProperty("map")->GetStringListProperty("path").size()), 2);
  properties->RemoveProperty("list");
  properties->RemoveProperty("map");

  // Options description
  vtkNew<vtkOptionsReader> optionsReader;
  vtkNew<vtkMRMLIOProperties> context;
  context->SetStringProperty("fileName", "/data/mylabel.nrrd");
  vtkNew<vtkMRMLIOProperties> optionValues;
  optionsReader->GetOptionValues(context, optionValues);
  CHECK_BOOL(optionValues->GetBoolProperty("labelmap"), true);
  CHECK_STD_STRING(optionValues->GetStringProperty("colorNodeID"), "Labels");
  CHECK_INT(optionValues->GetIntProperty("count"), 3);
  CHECK_INT(static_cast<int>(optionValues->GetStringListProperty("name").size()), 2);
  // Value set by the user overrides the default, dependent default follows it
  context->SetBoolProperty("labelmap", false);
  optionsReader->GetOptionValues(context, optionValues);
  CHECK_STD_STRING(optionValues->GetStringProperty("colorNodeID"), "Grey");
  // Application default is used if the user did not set a value
  optionsReader->GetOptionDefaults()->SetIntProperty("count", 7);
  optionsReader->GetOptionValues(context, optionValues);
  CHECK_INT(optionValues->GetIntProperty("count"), 7);
  std::string json = optionsReader->GetOptionsDescriptionJSON(context);
  std::cout << json << std::endl;
  {
    vtkNew<vtkMRMLJsonReader> jsonReader;
    vtkSmartPointer<vtkMRMLJsonElement> jsonRoot = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonReader->ReadFromString(json));
    CHECK_NOT_NULL(jsonRoot);
    CHECK_BOOL(jsonReader->HasErrors(), false);
    vtkSmartPointer<vtkMRMLJsonElement> jsonOptions = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonRoot->GetArrayProperty("options"));
    CHECK_NOT_NULL(jsonOptions);
    std::map<std::string, vtkSmartPointer<vtkMRMLJsonElement>> jsonOptionsByProperty;
    for (int optionIndex = 0; optionIndex < jsonOptions->GetArraySize(); ++optionIndex)
    {
      vtkSmartPointer<vtkMRMLJsonElement> jsonOption = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonOptions->GetArrayItem(optionIndex));
      jsonOptionsByProperty[jsonOption->GetStringProperty("property")] = jsonOption;
    }
    CHECK_BOOL(jsonOptionsByProperty.count("labelmap") > 0, true);
    CHECK_STD_STRING(jsonOptionsByProperty["colorNodeID"]->GetStringProperty("value"), "Grey");
    // Quotes in labels are restored by parsing
    vtkMRMLJsonElement* jsonColorOption = jsonOptionsByProperty["colorNodeID"];
    vtkSmartPointer<vtkMRMLJsonElement> jsonChoices = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonColorOption->GetArrayProperty("choices"));
    CHECK_NOT_NULL(jsonChoices);
    CHECK_INT(jsonChoices->GetArraySize(), 2);
    vtkSmartPointer<vtkMRMLJsonElement> jsonFirstChoice = vtkSmartPointer<vtkMRMLJsonElement>::Take(jsonChoices->GetArrayItem(0));
    CHECK_STD_STRING(jsonFirstChoice->GetStringProperty("label"), "Grey \"scale\"");
    CHECK_INT(jsonOptionsByProperty["count"]->GetMemberType("value"), vtkMRMLJsonElement::INT);
    CHECK_INT(jsonOptionsByProperty["count"]->GetIntProperty("value"), 7);
    std::vector<std::string> nameValues;
    CHECK_BOOL(jsonOptionsByProperty["name"]->GetStringVectorProperty("value", nameValues), true);
    CHECK_INT(static_cast<int>(nameValues.size()), 2);
    CHECK_STD_STRING(nameValues[0], "a");
    CHECK_STD_STRING(nameValues[1], "b");
  }
  CHECK_STD_STRING(textReader->GetOptionsDescriptionJSON(context), ""); // no options

  // Options description accessors (used by native user interfaces)
  {
    vtkNew<vtkMRMLIOOptionsDescription> description;
    optionsReader->FillOptionsDescription(context, description);
    CHECK_INT(description->GetNumberOfOptions(), 4);
    CHECK_INT(description->GetOptionIndex("labelmap"), 0);
    CHECK_INT(description->GetOptionIndex("colorNodeID"), 1);
    CHECK_INT(description->GetOptionIndex("count"), 2);
    CHECK_INT(description->GetOptionIndex("name"), 3);
    CHECK_INT(description->GetOptionIndex("nonexistent"), -1);
    // bool: value from properties (context sets labelmap to false)
    CHECK_STD_STRING(description->GetNthOptionProperty(0), "labelmap");
    CHECK_STD_STRING(description->GetNthOptionType(0), "bool");
    CHECK_STD_STRING(description->GetNthOptionLabel(0), "Label map");
    CHECK_STD_STRING(description->GetNthOptionToolTip(0), "Load as label map");
    CHECK_INT(description->GetNthOptionValue(0).ToInt(), 0);
    CHECK_BOOL(description->GetNthOptionEnabled(0), true);
    CHECK_BOOL(description->GetNthOptionVisible(0), true);
    CHECK_BOOL(description->GetNthOptionHasRange(0), false);
    // enum: option default that depends on another option, choices keep their type
    CHECK_STD_STRING(description->GetNthOptionType(1), "enum");
    CHECK_BOOL(description->GetNthOptionValue(1).IsString(), true);
    CHECK_STD_STRING(description->GetNthOptionValue(1).ToString(), "Grey");
    CHECK_INT(description->GetNthOptionNumberOfChoices(1), 2);
    CHECK_STD_STRING(description->GetNthOptionChoiceValue(1, 0).ToString(), "Grey");
    CHECK_STD_STRING(description->GetNthOptionChoiceLabel(1, 0), "Grey \"scale\"");
    CHECK_STD_STRING(description->GetNthOptionChoiceLabel(1, 1), "Labels");
    CHECK_INT(description->GetNthOptionNumberOfChoices(0), 0);
    // int: value from application defaults, range, enabled state
    CHECK_STD_STRING(description->GetNthOptionType(2), "int");
    CHECK_INT(description->GetNthOptionValue(2).ToInt(), 7);
    CHECK_BOOL(description->GetNthOptionValue(2).IsInt(), true);
    CHECK_BOOL(description->GetNthOptionHasRange(2), true);
    CHECK_DOUBLE(description->GetNthOptionMinimum(2), 0.0);
    CHECK_DOUBLE(description->GetNthOptionMaximum(2), 10.0);
    CHECK_INT(description->GetNthOptionDecimals(2), -1);
    CHECK_BOOL(description->GetNthOptionEnabled(2), true); // enabled because labelmap is false
    // string list: option default
    CHECK_STD_STRING(description->GetNthOptionType(3), "stringList");
    std::vector<std::string> nameValue = description->GetNthOptionStringListValue(3);
    CHECK_INT(static_cast<int>(nameValue.size()), 2);
    CHECK_STD_STRING(nameValue[1], "b");
    CHECK_STD_STRING(description->GetNthOptionSeparator(3), ";");
    // value set in properties takes precedence over application defaults
    context->SetIntProperty("count", 5);
    optionsReader->FillOptionsDescription(context, description);
    CHECK_INT(description->GetNumberOfOptions(), 4); // options are replaced, not added
    CHECK_INT(description->GetNthOptionValue(2).ToInt(), 5);
    context->RemoveProperty("count");
    // options without properties
    optionsReader->FillOptionsDescription(nullptr, description);
    CHECK_INT(description->GetNthOptionValue(0).ToInt(), 0); // no "label" in the file name
    CHECK_INT(description->GetNthOptionValue(2).ToInt(), 7);

    // Node option, double option with decimals and infinite range
    vtkNew<vtkMRMLIOOptionsDescription> otherDescription;
    otherDescription->AddNodeOption("colorNode", "Color", "", "vtkMRMLColorTableNodeGrey", { "vtkMRMLColorTableNode" }, true);
    otherDescription->SetOptionShowHidden("colorNode", true);
    otherDescription->SetOptionWidget("colorNode", "colorTable");
    otherDescription->AddDoubleOption("spacing", "Spacing", "", 1.5, 0.0, std::numeric_limits<double>::infinity(), 3);
    CHECK_STD_STRING(otherDescription->GetNthOptionType(0), "node");
    CHECK_STD_STRING(otherDescription->GetNthOptionWidget(0), "colorTable");
    CHECK_STD_STRING(otherDescription->GetNthOptionValue(0).ToString(), "vtkMRMLColorTableNodeGrey");
    CHECK_INT(static_cast<int>(otherDescription->GetNthOptionNodeClasses(0).size()), 1);
    CHECK_STD_STRING(otherDescription->GetNthOptionNodeClasses(0)[0], "vtkMRMLColorTableNode");
    CHECK_BOOL(otherDescription->GetNthOptionNoneEnabled(0), true);
    CHECK_BOOL(otherDescription->GetNthOptionShowHidden(0), true);
    CHECK_STD_STRING(otherDescription->GetNthOptionType(1), "double");
    CHECK_DOUBLE(otherDescription->GetNthOptionValue(1).ToDouble(), 1.5);
    CHECK_INT(otherDescription->GetNthOptionDecimals(1), 3);
    CHECK_BOOL(std::isinf(otherDescription->GetNthOptionMaximum(1)), true);

    // Invalid index
    TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
    CHECK_STD_STRING(otherDescription->GetNthOptionProperty(2), "");
    CHECK_STD_STRING(otherDescription->GetNthOptionType(-1), "");
    CHECK_BOOL(otherDescription->GetNthOptionValue(5).IsValid(), false);
    CHECK_BOOL(otherDescription->GetNthOptionChoiceValue(0, 0).IsValid(), false); // node option has no choices
    TESTING_OUTPUT_ASSERT_ERRORS_END();
  }

  // Readers and writers of module logics are registered and unregistered by the application logic
  {
    vtkNew<vtkReaderRegisteringLogic> moduleLogic; // deleted after the application logic
    vtkNew<vtkMRMLApplicationLogic> appLogic;
    vtkMRMLFileIOManager* appFileIOManager = appLogic->GetFileIOManager();
    const int numberOfReaders = appFileIOManager->GetNumberOfReaders();
    const int numberOfWriters = appFileIOManager->GetNumberOfWriters();
    appLogic->SetModuleLogic("MyModule", moduleLogic);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders + 1);
    CHECK_INT(appFileIOManager->GetNumberOfWriters(), numberOfWriters + 1);
    CHECK_NOT_NULL(appFileIOManager->GetReaderByDescription("Logic reader"));
    // Setting the same logic again does not register the reader again
    appLogic->SetModuleLogic("MyModule", moduleLogic);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders + 1);
    CHECK_INT(appFileIOManager->GetNumberOfWriters(), numberOfWriters + 1);
    // Other instances of the logic class do not register readers
    vtkNew<vtkReaderRegisteringLogic> otherLogic;
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders + 1);
    // Creating the readers and writers directly does not register them
    CHECK_INT(static_cast<int>(otherLogic->CreateFileIOHandlers().size()), 2);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders + 1);
    CHECK_INT(appFileIOManager->GetNumberOfWriters(), numberOfWriters + 1);
    // Replacing the module logic replaces the readers
    vtkWeakPointer<vtkMRMLFileReader> firstLogicReader = appFileIOManager->GetReaderByDescription("Logic reader");
    appLogic->SetModuleLogic("MyModule", otherLogic);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders + 1);
    CHECK_INT(appFileIOManager->GetNumberOfWriters(), numberOfWriters + 1);
    CHECK_NULL(firstLogicReader.GetPointer()); // the reader of the previous logic is unregistered and deleted
    CHECK_NOT_NULL(appFileIOManager->GetReaderByDescription("Logic reader"));
    // Removing the module logic removes the readers
    appLogic->SetModuleLogic("MyModule", nullptr);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders);
    CHECK_INT(appFileIOManager->GetNumberOfWriters(), numberOfWriters);
    // Deleting the module logic removes the readers
    vtkReaderRegisteringLogic* deletedLogic = vtkReaderRegisteringLogic::New();
    appLogic->SetModuleLogic("MyModule", deletedLogic);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders + 1);
    deletedLogic->Delete();
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders);
    CHECK_NULL(appLogic->GetModuleLogic("MyModule"));
    // A logic that is not a vtkMRMLModuleLogic does not register readers
    vtkNew<vtkPlainLogic> plainLogic;
    appLogic->SetModuleLogic("MyModule", plainLogic);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders);
    CHECK_POINTER(appLogic->GetModuleLogic("MyModule"), plainLogic.GetPointer());
    // Replacing it with a module logic registers the readers
    appLogic->SetModuleLogic("MyModule", otherLogic);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders + 1);
    // Replacing the module logic with a plain logic unregisters the readers
    appLogic->SetModuleLogic("MyModule", plainLogic);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders);
    // Removing the plain logic does not change the readers
    appLogic->SetModuleLogic("MyModule", nullptr);
    CHECK_INT(appFileIOManager->GetNumberOfReaders(), numberOfReaders);
    // Module logic may be deleted after the application logic
    appLogic->SetModuleLogic("MyModule", moduleLogic);
  }

  vtksys::SystemTools::RemoveADirectory(directory);

  std::cout << "vtkMRMLFileIOManagerTest1 passed" << std::endl;
  return EXIT_SUCCESS;
}
