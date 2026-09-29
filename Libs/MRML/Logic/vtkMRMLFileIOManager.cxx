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

#include "vtkMRMLFileIOManager.h"

#include "vtkMRMLApplicationLogic.h"

#include "vtkMRMLFileIOHandler.h"
#include "vtkMRMLFileReader.h"
#include "vtkMRMLFileWriter.h"
#include "vtkMRMLIOProperties.h"
#include "vtkMRMLNodeWriter.h"

// MRML includes
#include <vtkDataFileFormatHelper.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLI18N.h>
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLTransformableNode.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkLoggingMacros.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkTimerLog.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iomanip>
#include <numeric>
#include <sstream>

vtkStandardNewMacro(vtkMRMLFileIOManager);

namespace
{
/// Add a value to a list of strings unless it is empty or already in the list.
void AddOnce(std::vector<std::string>& values, const std::string& value)
{
  if (value.empty() || std::find(values.begin(), values.end(), value) != values.end())
  {
    return;
  }
  values.push_back(value);
}

bool StartsWith(const std::string& text, const std::string& prefix)
{
  return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
}

bool EndsWith(const std::string& text, const std::string& suffix)
{
  return text.size() >= suffix.size() && text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}
} // namespace

//----------------------------------------------------------------------------
vtkMRMLFileIOManager::vtkMRMLFileIOManager() = default;

//----------------------------------------------------------------------------
vtkMRMLFileIOManager::~vtkMRMLFileIOManager()
{
  for (const auto& reader : this->Readers)
  {
    reader->SetFileIOManager(nullptr);
  }
  for (const auto& writer : this->Writers)
  {
    writer->SetFileIOManager(nullptr);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "DefaultSceneFileType: " << this->DefaultSceneFileType << "\n";
  os << indent << "DefaultMaximumFileNameLength: " << this->DefaultMaximumFileNameLength << "\n";
  os << indent << "Readers: " << this->Readers.size() << "\n";
  for (const auto& reader : this->Readers)
  {
    os << indent << "  " << reader->GetFileType() << " (" << reader->GetDescription() << ")\n";
  }
  os << indent << "Writers: " << this->Writers.size() << "\n";
  for (const auto& writer : this->Writers)
  {
    os << indent << "  " << writer->GetFileType() << " (" << writer->GetDescription() << ")\n";
  }
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::SetScene(vtkMRMLScene* scene)
{
  if (this->Scene == scene)
  {
    return;
  }
  this->Scene = scene;
  for (const auto& reader : this->Readers)
  {
    reader->SetScene(scene);
  }
  for (const auto& writer : this->Writers)
  {
    writer->SetScene(scene);
  }
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLScene* vtkMRMLFileIOManager::GetScene() const
{
  return this->Scene;
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::SetApplicationLogic(vtkMRMLApplicationLogic* applicationLogic)
{
  this->ApplicationLogic = applicationLogic;
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic* vtkMRMLFileIOManager::GetApplicationLogic() const
{
  return this->ApplicationLogic;
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::RegisterReader(vtkMRMLFileReader* reader)
{
  if (!reader)
  {
    vtkErrorMacro("RegisterReader: no reader given");
    return;
  }
  if (std::find(this->Readers.begin(), this->Readers.end(), reader) != this->Readers.end())
  {
    return;
  }
  reader->SetScene(this->Scene);
  reader->SetFileIOManager(this);
  this->Readers.emplace_back(reader);
  this->InvokeEvent(HandlerRegisteredEvent, reader);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::RegisterWriter(vtkMRMLFileWriter* writer)
{
  if (!writer)
  {
    vtkErrorMacro("RegisterWriter: no writer given");
    return;
  }
  if (std::find(this->Writers.begin(), this->Writers.end(), writer) != this->Writers.end())
  {
    return;
  }
  writer->SetScene(this->Scene);
  writer->SetFileIOManager(this);
  this->Writers.emplace_back(writer);
  this->InvokeEvent(HandlerRegisteredEvent, writer);
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLNodeWriter* vtkMRMLFileIOManager::RegisterNodeWriter(const std::string& description,
                                                            const std::string& fileType,
                                                            const std::vector<std::string>& nodeClassNames,
                                                            bool supportUseCompression /*=true*/)
{
  vtkSmartPointer<vtkMRMLNodeWriter> writer = vtkMRMLNodeWriter::CreateNodeWriter(description, fileType, nodeClassNames, supportUseCompression);
  this->RegisterWriter(writer);
  return writer;
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::Register(vtkMRMLFileIOHandler* handler)
{
  if (vtkMRMLFileReader* reader = vtkMRMLFileReader::SafeDownCast(handler))
  {
    this->RegisterReader(reader);
  }
  else if (vtkMRMLFileWriter* writer = vtkMRMLFileWriter::SafeDownCast(handler))
  {
    this->RegisterWriter(writer);
  }
  else
  {
    vtkErrorMacro("Register failed: " << (handler ? handler->GetClassName() : "(null)") << " is neither a file reader nor a file writer");
  }
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::Unregister(vtkMRMLFileIOHandler* handler)
{
  if (!handler)
  {
    return;
  }
  vtkSmartPointer<vtkMRMLFileIOHandler> kept(handler); // outlives the removal, for the event
  bool removed = false;
  vtkMRMLFileReader* asReader = vtkMRMLFileReader::SafeDownCast(handler);
  if (asReader)
  {
    auto found = std::find(this->Readers.begin(), this->Readers.end(), asReader);
    if (found != this->Readers.end())
    {
      this->Readers.erase(found);
      removed = true;
    }
  }
  vtkMRMLFileWriter* asWriter = vtkMRMLFileWriter::SafeDownCast(handler);
  if (asWriter)
  {
    auto found = std::find(this->Writers.begin(), this->Writers.end(), asWriter);
    if (found != this->Writers.end())
    {
      this->Writers.erase(found);
      removed = true;
    }
  }
  if (removed)
  {
    if (handler->GetFileIOManager() == this)
    {
      handler->SetFileIOManager(nullptr);
    }
    this->InvokeEvent(HandlerUnregisteredEvent, kept);
    this->Modified();
  }
}

//----------------------------------------------------------------------------
vtkMRMLFileReader* vtkMRMLFileIOManager::GetReaderByClassName(const std::string& className) const
{
  for (const auto& reader : this->Readers)
  {
    if (className == reader->GetClassName())
    {
      return reader;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLFileWriter* vtkMRMLFileIOManager::GetWriterByClassName(const std::string& className) const
{
  for (const auto& writer : this->Writers)
  {
    if (className == writer->GetClassName())
    {
      return writer;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
int vtkMRMLFileIOManager::GetNumberOfReaders() const
{
  return static_cast<int>(this->Readers.size());
}

//----------------------------------------------------------------------------
vtkMRMLFileReader* vtkMRMLFileIOManager::GetNthReader(int index) const
{
  if (index < 0 || index >= static_cast<int>(this->Readers.size()))
  {
    return nullptr;
  }
  return this->Readers[index];
}

//----------------------------------------------------------------------------
int vtkMRMLFileIOManager::GetNumberOfWriters() const
{
  return static_cast<int>(this->Writers.size());
}

//----------------------------------------------------------------------------
vtkMRMLFileWriter* vtkMRMLFileIOManager::GetNthWriter(int index) const
{
  if (index < 0 || index >= static_cast<int>(this->Writers.size()))
  {
    return nullptr;
  }
  return this->Writers[index];
}

//----------------------------------------------------------------------------
template <typename HandlerType>
std::vector<vtkSmartPointer<HandlerType>> vtkMRMLFileIOManager::SortedByConfidence(const std::vector<vtkSmartPointer<HandlerType>>& handlers,
                                                                                   const std::vector<double>& confidences)
{
  std::vector<size_t> order(handlers.size());
  std::iota(order.begin(), order.end(), 0);
  // A stable sort keeps the order of registration where two handlers are equally sure of themselves
  std::stable_sort(order.begin(), order.end(), [&confidences](size_t a, size_t b) { return confidences[a] > confidences[b]; });
  std::vector<vtkSmartPointer<HandlerType>> result;
  for (size_t index : order)
  {
    if (confidences[index] > 0.0)
    {
      result.push_back(handlers[index]);
    }
  }
  return result;
}

//----------------------------------------------------------------------------
std::vector<vtkSmartPointer<vtkMRMLFileReader>> vtkMRMLFileIOManager::GetReadersForFile(const std::string& filePath)
{
  if (filePath.empty())
  {
    return std::vector<vtkSmartPointer<vtkMRMLFileReader>>();
  }
  // Copy the list, as a reader may register or unregister handlers
  std::vector<vtkSmartPointer<vtkMRMLFileReader>> allReaders = this->Readers;
  std::vector<double> confidences;
  confidences.reserve(allReaders.size());
  for (const auto& reader : allReaders)
  {
    confidences.push_back(reader->CanLoadFileConfidence(filePath));
  }
  return vtkMRMLFileIOManager::SortedByConfidence(allReaders, confidences);
}

//----------------------------------------------------------------------------
vtkMRMLFileReader* vtkMRMLFileIOManager::GetReaderForFile(const std::string& filePath)
{
  std::vector<vtkSmartPointer<vtkMRMLFileReader>> readers = this->GetReadersForFile(filePath);
  // The returned reader is kept alive by this manager (readers that are not registered are not returned)
  return readers.empty() ? nullptr : readers[0].GetPointer();
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::GetFileTypeForFile(const std::string& filePath)
{
  vtkMRMLFileReader* reader = this->GetReaderForFile(filePath);
  return reader ? reader->GetFileType() : std::string();
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetFileTypesForFile(const std::string& filePath)
{
  std::vector<std::string> fileTypes;
  for (const auto& reader : this->GetReadersForFile(filePath))
  {
    fileTypes.push_back(reader->GetFileType());
  }
  return fileTypes;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetFileDescriptionsForFile(const std::string& filePath)
{
  std::vector<std::string> descriptions;
  for (const auto& reader : this->GetReadersForFile(filePath))
  {
    descriptions.push_back(reader->GetDescription());
  }
  return descriptions;
}

//----------------------------------------------------------------------------
vtkMRMLFileReader* vtkMRMLFileIOManager::GetReaderByDescription(const std::string& description)
{
  if (description.empty())
  {
    return nullptr;
  }
  for (const auto& reader : this->Readers)
  {
    if (reader->GetDescription() == description)
    {
      return reader;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::GetFileTypeFromDescription(const std::string& description)
{
  vtkMRMLFileReader* reader = this->GetReaderByDescription(description);
  return reader ? reader->GetFileType() : std::string();
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetFileDescriptionsByType(const std::string& fileType)
{
  std::vector<std::string> descriptions;
  for (const auto& reader : this->GetReadersForFileType(fileType))
  {
    descriptions.push_back(reader->GetDescription());
  }
  return descriptions;
}

//----------------------------------------------------------------------------
vtkMRMLFileReader* vtkMRMLFileIOManager::ExamineFileList(std::vector<std::string>& fileList, vtkMRMLIOProperties* ioProperties)
{
  if (!ioProperties)
  {
    vtkErrorMacro("ExamineFileList failed: invalid properties");
    return nullptr;
  }
  // Each reader examines a copy of the file list and properties. The group of files that is recognized
  // with the highest confidence is used (if the confidence is the same then the larger group is used,
  // if the group size is the same then the reader that was registered first).
  // Copy the list of readers, as a reader may register or unregister handlers.
  std::vector<vtkSmartPointer<vtkMRMLFileReader>> allReaders = this->Readers;
  vtkSmartPointer<vtkMRMLFileReader> bestReader;
  std::string bestArchetypeFile;
  std::vector<std::string> bestFileList;
  vtkNew<vtkMRMLIOProperties> bestProperties;
  double bestConfidence = 0.0;
  size_t bestNumberOfGroupedFiles = 0;
  for (const auto& reader : allReaders)
  {
    std::vector<std::string> candidateFileList = fileList;
    vtkNew<vtkMRMLIOProperties> candidateProperties;
    candidateProperties->Copy(ioProperties);
    candidateProperties->RemoveProperty("fileName");
    const double confidence = reader->ExamineFileListConfidence(candidateFileList, candidateProperties);
    const std::string archetypeFile = candidateProperties->GetStringProperty("fileName");
    if (archetypeFile.empty())
    {
      continue;
    }
    // number of files that were removed from the list (loaded with the archetype)
    const size_t numberOfGroupedFiles = fileList.size() > candidateFileList.size() ? fileList.size() - candidateFileList.size() : 0;
    if (confidence <= 0.0)
    {
      continue;
    }
    if (bestReader && (confidence < bestConfidence || (confidence == bestConfidence && numberOfGroupedFiles <= bestNumberOfGroupedFiles)))
    {
      continue;
    }
    bestReader = reader;
    bestArchetypeFile = archetypeFile;
    bestFileList = candidateFileList;
    bestProperties->Copy(candidateProperties);
    bestConfidence = confidence;
    bestNumberOfGroupedFiles = numberOfGroupedFiles;
  }
  if (!bestReader)
  {
    return nullptr;
  }
  fileList = bestFileList;
  ioProperties->Copy(bestProperties);
  ioProperties->SetStringProperty("fileName", bestArchetypeFile);
  return bestReader;
}

//----------------------------------------------------------------------------
std::vector<vtkSmartPointer<vtkMRMLFileWriter>> vtkMRMLFileIOManager::GetWritersForObject(vtkObject* object)
{
  if (!object)
  {
    return std::vector<vtkSmartPointer<vtkMRMLFileWriter>>();
  }
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> allWriters = this->Writers;
  std::vector<double> confidences;
  confidences.reserve(allWriters.size());
  for (const auto& writer : allWriters)
  {
    confidences.push_back(writer->CanWriteObjectConfidence(object));
  }
  return vtkMRMLFileIOManager::SortedByConfidence(allWriters, confidences);
}

//----------------------------------------------------------------------------
vtkMRMLFileWriter* vtkMRMLFileIOManager::GetWriterForObject(vtkObject* object, const std::string& format /*=std::string()*/)
{
  // best match: the writer that supports the node type and the specific format
  // closest match: the writer that supports the node type but not that specific format
  //
  // If there are multiple matches then the one with the highest confidence is returned.
  vtkMRMLFileWriter* bestMatch = nullptr;
  double bestMatchConfidence = 0.0;
  vtkMRMLFileWriter* closestMatch = nullptr;
  double closestMatchConfidence = 0.0;

  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> allWriters = this->Writers;
  for (const auto& writer : allWriters)
  {
    double confidence = writer->CanWriteObjectConfidence(object);
    if (confidence <= 0.0)
    {
      continue;
    }
    if (confidence > closestMatchConfidence)
    {
      closestMatch = writer;
      closestMatchConfidence = confidence;
    }
    if (format.empty())
    {
      if (confidence > bestMatchConfidence)
      {
        bestMatch = writer;
        bestMatchConfidence = confidence;
      }
      continue;
    }
    std::vector<std::string> nameFilters = writer->GetNameFiltersForObject(object);
    if (std::find(nameFilters.begin(), nameFilters.end(), format) != nameFilters.end())
    {
      if (confidence > bestMatchConfidence)
      {
        bestMatch = writer;
        bestMatchConfidence = confidence;
      }
    }
  }
  return bestMatch ? bestMatch : closestMatch;
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::GetFileWriterFileType(vtkObject* object, const std::string& format /*=std::string()*/)
{
  vtkMRMLFileWriter* writer = this->GetWriterForObject(object, format);
  return writer ? writer->GetFileType() : std::string();
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::GetFileTypeForObject(vtkObject* object)
{
  return this->GetFileWriterFileType(object);
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetFileWriterDescriptions(const std::string& fileType)
{
  std::vector<std::string> descriptions;
  for (const auto& writer : this->GetWritersForFileType(fileType))
  {
    descriptions.push_back(writer->GetDescription());
  }
  return descriptions;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetFileWriterExtensions(vtkObject* object)
{
  std::vector<std::string> nameFilters;
  // Writers sorted by confidence, highest first
  for (const auto& writer : this->GetWritersForObject(object))
  {
    for (const std::string& nameFilter : writer->GetNameFiltersForObject(object))
    {
      AddOnce(nameFilters, nameFilter);
    }
  }
  return nameFilters;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetReaderFileTypes()
{
  std::vector<std::string> fileTypes;
  for (const auto& reader : this->Readers)
  {
    AddOnce(fileTypes, reader->GetFileType());
  }
  return fileTypes;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetWriterFileTypes()
{
  std::vector<std::string> fileTypes;
  for (const auto& writer : this->Writers)
  {
    AddOnce(fileTypes, writer->GetFileType());
  }
  return fileTypes;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetExtensionsForFileType(const std::string& fileType, bool writers)
{
  std::vector<std::string> extensions;
  std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> handlers;
  if (writers)
  {
    std::vector<vtkSmartPointer<vtkMRMLFileWriter>> fileTypeWriters = this->GetWritersForFileType(fileType);
    handlers.assign(fileTypeWriters.begin(), fileTypeWriters.end());
  }
  else
  {
    std::vector<vtkSmartPointer<vtkMRMLFileReader>> fileTypeReaders = this->GetReadersForFileType(fileType);
    handlers.assign(fileTypeReaders.begin(), fileTypeReaders.end());
  }
  for (const auto& handler : handlers)
  {
    for (const std::string& extension : handler->GetExtensions())
    {
      AddOnce(extensions, extension);
    }
  }
  return extensions;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetNameFiltersForFileType(const std::string& fileType, bool writers)
{
  std::vector<std::string> nameFilters;
  std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> handlers;
  if (writers)
  {
    std::vector<vtkSmartPointer<vtkMRMLFileWriter>> fileTypeWriters = this->GetWritersForFileType(fileType);
    handlers.assign(fileTypeWriters.begin(), fileTypeWriters.end());
  }
  else
  {
    std::vector<vtkSmartPointer<vtkMRMLFileReader>> fileTypeReaders = this->GetReadersForFileType(fileType);
    handlers.assign(fileTypeReaders.begin(), fileTypeReaders.end());
  }
  for (const auto& handler : handlers)
  {
    for (const std::string& filter : handler->GetNameFilters())
    {
      AddOnce(nameFilters, filter);
    }
  }
  return nameFilters;
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::GetDescriptionForFileType(const std::string& fileType, bool writers)
{
  std::vector<std::string> descriptions = writers ? this->GetFileWriterDescriptions(fileType) : this->GetFileDescriptionsByType(fileType);
  for (const std::string& description : descriptions)
  {
    if (!description.empty())
    {
      return description;
    }
  }
  return std::string();
}

//----------------------------------------------------------------------------
std::vector<vtkSmartPointer<vtkMRMLFileReader>> vtkMRMLFileIOManager::GetReadersForFileType(const std::string& fileType)
{
  std::vector<vtkSmartPointer<vtkMRMLFileReader>> readers;
  if (fileType.empty())
  {
    return readers;
  }
  for (const auto& reader : this->Readers)
  {
    if (reader->GetFileType() == fileType)
    {
      readers.push_back(reader);
    }
  }
  return readers;
}

//----------------------------------------------------------------------------
std::vector<vtkSmartPointer<vtkMRMLFileWriter>> vtkMRMLFileIOManager::GetWritersForFileType(const std::string& fileType)
{
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> writers;
  if (fileType.empty())
  {
    return writers;
  }
  for (const auto& writer : this->Writers)
  {
    if (writer->GetFileType() == fileType)
    {
      writers.push_back(writer);
    }
  }
  return writers;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetAllWritableFileExtensions()
{
  std::vector<std::string> extensions;
  if (!this->Scene)
  {
    vtkWarningMacro("GetAllWritableFileExtensions: manager has no scene defined");
    return extensions;
  }
  // check for all extensions that can be used to write storable nodes
  int numRegisteredNodeClasses = this->Scene->GetNumberOfRegisteredNodeClasses();
  for (int i = 0; i < numRegisteredNodeClasses; ++i)
  {
    vtkMRMLStorageNode* storageNode = vtkMRMLStorageNode::SafeDownCast(this->Scene->GetNthRegisteredNodeClass(i));
    if (!storageNode)
    {
      continue;
    }
    vtkNew<vtkStringArray> supportedFileExtensions;
    storageNode->GetFileExtensionsFromFileTypes(storageNode->GetSupportedWriteFileTypes(), supportedFileExtensions);
    for (vtkIdType formatIt = 0; formatIt < supportedFileExtensions->GetNumberOfValues(); ++formatIt)
    {
      AddOnce(extensions, supportedFileExtensions->GetValue(formatIt));
    }
  }
  return extensions;
}

//----------------------------------------------------------------------------
std::vector<std::string> vtkMRMLFileIOManager::GetAllReadableFileExtensions()
{
  std::vector<std::string> extensions;
  if (!this->Scene)
  {
    vtkWarningMacro("GetAllReadableFileExtensions: manager has no scene defined");
    return extensions;
  }
  // check for all extensions that can be used to read storable nodes
  int numRegisteredNodeClasses = this->Scene->GetNumberOfRegisteredNodeClasses();
  for (int i = 0; i < numRegisteredNodeClasses; ++i)
  {
    vtkMRMLStorageNode* storageNode = vtkMRMLStorageNode::SafeDownCast(this->Scene->GetNthRegisteredNodeClass(i));
    if (!storageNode)
    {
      continue;
    }
    vtkNew<vtkStringArray> supportedFileExtensions;
    storageNode->GetFileExtensionsFromFileTypes(storageNode->GetSupportedReadFileTypes(), supportedFileExtensions);
    for (vtkIdType formatIt = 0; formatIt < supportedFileExtensions->GetNumberOfValues(); ++formatIt)
    {
      AddOnce(extensions, supportedFileExtensions->GetValue(formatIt));
    }
  }
  return extensions;
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::GetCompleteSlicerWritableFileNameSuffix(vtkMRMLStorableNode* node)
{
  vtkMRMLStorageNode* storageNode = node ? node->GetStorageNode() : nullptr;
  if (!storageNode)
  {
    vtkWarningMacro("GetCompleteSlicerWritableFileNameSuffix failed: no storage node is available");
    return ".";
  }
  std::string ext = storageNode->GetSupportedFileExtension(nullptr, false, true);
  if (!ext.empty())
  {
    return ext;
  }
  // otherwise return an empty suffix
  return ".";
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::GetFileNameValidCharactersPattern()
{
  return "[A-Za-z0-9\\ \\-\\_\\.\\(\\)\\$\\!\\~\\#\\'\\%\\^\\{\\}]";
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::ForceFileNameValidCharacters(const std::string& filename)
{
  // Remove characters that are likely to cause problems in filename
  // (same set of characters as in GetFileNameValidCharactersPattern)
  static const std::string validSpecialCharacters = " -_.()$!~#'%^{}";
  std::string sanitizedFilename;
  for (char c : filename)
  {
    unsigned char uc = static_cast<unsigned char>(c);
    if ((uc < 128 && std::isalnum(uc)) || validSpecialCharacters.find(c) != std::string::npos)
    {
      sanitizedFilename += c;
    }
  }
  // Remove leading and trailing spaces
  std::string::size_type first = sanitizedFilename.find_first_not_of(' ');
  if (first == std::string::npos)
  {
    return std::string();
  }
  std::string::size_type last = sanitizedFilename.find_last_not_of(' ');
  return sanitizedFilename.substr(first, last - first + 1);
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::ForceFileNameMaxLength(const std::string& filename, int extensionLength, int maxLength /*=-1*/)
{
  if (maxLength < 0)
  {
    maxLength = this->DefaultMaximumFileNameLength;
  }
  return vtkMRMLStorageNode::ClampFileName(filename, extensionLength, maxLength);
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::ExtractKnownExtension(const std::string& fileName, vtkObject* object)
{
  std::string longestMatchedExtension;
  for (const std::string& nameFilter : this->GetFileWriterExtensions(object))
  {
    std::string extension = vtkDataFileFormatHelper::GetFileExtensionFromFormatString(nameFilter.c_str());
    if (!extension.empty() && EndsWith(fileName, extension) && extension.size() > longestMatchedExtension.size())
    {
      longestMatchedExtension = extension;
    }
  }
  return longestMatchedExtension;
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::StripKnownExtension(const std::string& fileName, vtkObject* object)
{
  std::string strippedFileName(fileName);
  std::string knownExtension = this->ExtractKnownExtension(fileName, object);
  if (!knownExtension.empty())
  {
    strippedFileName.resize(strippedFileName.size() - knownExtension.size());
    // recursively chop any further copies of the extension,
    // which sometimes appear when the filename+extension is
    // constructed from a filename that already had an extension
    if (EndsWith(strippedFileName, knownExtension))
    {
      return this->StripKnownExtension(strippedFileName, object);
    }
  }
  return strippedFileName;
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOManager::LoadNodes(const std::string& fileType,
                                     vtkMRMLIOProperties* properties,
                                     vtkCollection* loadedNodes /*=nullptr*/,
                                     vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  if (!properties || !properties->HasProperty("fileName"))
  {
    vtkErrorMacro("LoadNodes failed: \"fileName\" property is required");
    if (userMessages)
    {
      userMessages->AddMessage(vtkCommand::ErrorEvent, vtkMRMLTr("qSlicerCoreIOManager", "File name is not specified."));
    }
    return false;
  }
  if (!this->Scene)
  {
    vtkErrorMacro("LoadNodes failed: scene is invalid");
    return false;
  }

  if (properties->IsStringListProperty("fileName"))
  {
    // Load each file
    bool res = true;
    std::vector<std::string> fileNames = properties->GetStringListProperty("fileName");
    std::vector<std::string> names = properties->GetStringListProperty("name");
    size_t nameId = 0;
    for (const std::string& fileName : fileNames)
    {
      vtkNew<vtkMRMLIOProperties> fileProperties;
      fileProperties->Copy(properties);
      fileProperties->SetStringProperty("fileName", fileName);
      if (!names.empty())
      {
        fileProperties->SetStringProperty("name", nameId < names.size() ? names[nameId] : names.back());
        ++nameId;
      }
      res = this->LoadNodes(fileType, fileProperties, loadedNodes, userMessages) && res;
    }
    return res;
  }

  const std::string fileName = properties->GetStringProperty("fileName");

  vtkNew<vtkMRMLIOProperties> loadedFileProperties;
  loadedFileProperties->Copy(properties);
  loadedFileProperties->SetStringProperty("fileType", fileType);

  std::vector<vtkSmartPointer<vtkMRMLFileReader>> readers = this->GetReadersForFileType(fileType);

  // If no readers were able to read and load the file(s), success will remain false
  bool success = false;
  int numberOfUserMessagesBefore = userMessages ? userMessages->GetNumberOfMessages() : 0;
  //: %1 is the filename
  const std::string userMessagePrefix = vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Loading %1"), fileName.c_str()) + " - ";

  std::vector<std::string> nodeIDs;
  for (vtkMRMLFileReader* reader : readers)
  {
    double startTime = vtkTimerLog::GetUniversalTime();
    reader->GetUserMessages()->ClearMessages();
    reader->SetScene(this->Scene);
    double confidence = reader->CanLoadFileConfidence(fileName);
    if (confidence <= 0.0)
    {
      continue;
    }
    reader->ClearLoadedNodeIDs();
    bool currentFileSuccess = reader->Load(properties);
    if (userMessages)
    {
      userMessages->AddMessages(reader->GetUserMessages(), userMessagePrefix);
    }
    if (!currentFileSuccess)
    {
      continue;
    }
    double elapsedTimeInSeconds = vtkTimerLog::GetUniversalTime() - startTime;
    std::ostringstream elapsedTimeStr;
    elapsedTimeStr << std::fixed << std::setprecision(2) << elapsedTimeInSeconds;
    vtkInfoMacro("\"" << reader->GetDescription() << "\" Reader has successfully read the file \"" << fileName << "\" [" << elapsedTimeStr.str() << "s]");
    nodeIDs = reader->GetLoadedNodeIDs();
    success = true;
    break;
  }

  if (!success && userMessages != nullptr && userMessages->GetNumberOfMessages() == numberOfUserMessagesBefore)
  {
    // Make sure that at least one message is logged if reading failed.
    userMessages->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "%1 load failed."), userMessagePrefix.c_str()));
  }

  loadedFileProperties->SetStringListProperty("nodeIDs", nodeIDs);
  this->InvokeNewFileLoadedEvent(loadedFileProperties);

  if (loadedNodes && this->Scene)
  {
    for (const std::string& nodeID : nodeIDs)
    {
      vtkMRMLNode* loadedNode = this->Scene->GetNodeByID(nodeID);
      if (!loadedNode)
      {
        vtkWarningMacro("LoadNodes: cannot find node by ID " << nodeID);
        continue;
      }
      loadedNodes->AddItem(loadedNode);
    }
  }

  return success;
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOManager::LoadNodesFromList(vtkCollection* propertiesList, vtkCollection* loadedNodes /*=nullptr*/, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  if (!propertiesList)
  {
    return false;
  }
  bool success = true;
  for (int i = 0; i < propertiesList->GetNumberOfItems(); ++i)
  {
    vtkMRMLIOProperties* fileProperties = vtkMRMLIOProperties::SafeDownCast(propertiesList->GetItemAsObject(i));
    if (!fileProperties)
    {
      vtkErrorMacro("LoadNodesFromList: item " << i << " is not a vtkMRMLIOProperties object");
      success = false;
      continue;
    }
    int numberOfUserMessagesBefore = userMessages ? userMessages->GetNumberOfMessages() : 0;
    success = this->LoadNodes(fileProperties->GetStringProperty("fileType"), fileProperties, loadedNodes, userMessages) && success;
    // Add a separator between nodes
    if (userMessages && userMessages->GetNumberOfMessages() > numberOfUserMessagesBefore)
    {
      userMessages->AddSeparator();
    }
  }
  return success;
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLFileIOManager::LoadNodesAndGetFirst(const std::string& fileType, //
                                                        vtkMRMLIOProperties* properties,
                                                        vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  vtkNew<vtkCollection> loadedNodes;
  this->LoadNodes(fileType, properties, loadedNodes, userMessages);
  return vtkMRMLNode::SafeDownCast(loadedNodes->GetItemAsObject(0));
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOManager::LoadScene(const std::string& fileName, bool clear /*=true*/, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  vtkNew<vtkMRMLIOProperties> properties;
  properties->SetStringProperty("fileName", fileName);
  properties->SetBoolProperty("clear", clear);
  return this->LoadNodes("SceneFile", properties, nullptr, userMessages);
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOManager::LoadFile(const std::string& fileName, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  vtkNew<vtkMRMLIOProperties> properties;
  properties->SetStringProperty("fileName", fileName);
  std::string fileType = this->GetFileTypeForFile(fileName);
  return this->LoadNodes(fileType.empty() ? "NoFile" : fileType, properties, nullptr, userMessages);
}

//----------------------------------------------------------------------------
std::vector<vtkSmartPointer<vtkMRMLFileWriter>> vtkMRMLFileIOManager::GetWritersForFile(const std::string& fileType, //
                                                                                        vtkMRMLIOProperties* properties,
                                                                                        vtkMRMLScene* scene)
{
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> matchingWriters;
  if (fileType.empty() || !properties || !scene)
  {
    return matchingWriters;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  std::string nodeID = properties->GetStringProperty("nodeID");

  vtkObject* object = nullptr;
  // empty nodeID means saving the scene
  if (!nodeID.empty())
  {
    object = scene->GetNodeByID(nodeID);
    if (!object)
    {
      vtkWarningMacro("GetWritersForFile: Unable to find node with ID " << nodeID << " in the given scene.");
    }
  }
  std::string fileNameWithoutPath = vtksys::SystemTools::GetFilenameName(fileName);

  // Some writers ("Slicer Data Bundle (*)" can support any file,
  // they are called generic writers. The following code ensures
  // that writers associated with specific file extension are
  // considered first.
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> genericWriters;
  for (const auto& writer : this->Writers)
  {
    if (writer->GetFileType() != fileType)
    {
      continue;
    }
    std::vector<std::string> matchingNameFilters;
    for (const std::string& nameFilter : writer->GetNameFiltersForObject(object))
    {
      for (const std::string& extension : vtkMRMLFileIOHandler::NameFilterToWildcards(nameFilter))
      {
        // HACK - See https://github.com/Slicer/Slicer/issues/3322
        std::string extensionWithStar = StartsWith(extension, "*") ? extension : "*" + extension;
        if (vtkMRMLFileIOHandler::WildcardMatch(extensionWithStar, fileNameWithoutPath))
        {
          matchingNameFilters.push_back(nameFilter);
        }
      }
    }
    // Generic writers must be added to the end
    for (const std::string& nameFilter : matchingNameFilters)
    {
      if (nameFilter.find("*.*") != std::string::npos || nameFilter.find("(*)") != std::string::npos)
      {
        genericWriters.push_back(writer);
        continue;
      }
      if (std::find(matchingWriters.begin(), matchingWriters.end(), writer) == matchingWriters.end())
      {
        matchingWriters.push_back(writer);
      }
    }
  }
  for (const auto& writer : genericWriters)
  {
    if (std::find(matchingWriters.begin(), matchingWriters.end(), writer) == matchingWriters.end())
    {
      matchingWriters.push_back(writer);
    }
  }
  return matchingWriters;
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOManager::SaveNodes(const std::string& fileType,
                                     vtkMRMLIOProperties* properties,
                                     vtkMRMLMessageCollection* userMessages /*=nullptr*/,
                                     vtkMRMLScene* scene /*=nullptr*/)
{
  if (!scene)
  {
    scene = this->Scene;
  }
  if (!scene)
  {
    vtkErrorMacro("SaveNodes failed: invalid scene");
    return false;
  }
  if (!properties || !properties->HasProperty("fileName"))
  {
    vtkErrorMacro("SaveNodes failed: \"fileName\" must be included as a string parameter.");
    return false;
  }
  std::string fileName = properties->GetStringProperty("fileName");
  if (fileName.empty())
  {
    vtkErrorMacro("SaveNodes failed: \"fileName\" parameter must not be empty.");
    return false;
  }
  // HACK - See https://github.com/Slicer/Slicer/issues/3322
  //        Sort writers to ensure generic ones are last.
  // The returned list keeps a reference to all the writers, as a writer may be unregistered while writing (e.g., by a Python writer)
  std::vector<vtkSmartPointer<vtkMRMLFileWriter>> writers = this->GetWritersForFile(fileType, properties, scene);
  if (writers.empty())
  {
    vtkErrorMacro("SaveNodes error: No writer found to write file " << fileName << " of type " << fileType);
    if (userMessages)
    {
      userMessages->AddMessage(vtkCommand::ErrorEvent,
                               vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "No writer found to write file %1 of type %2."), //
                                                   fileName.c_str(),
                                                   fileType.c_str()));
    }
    return false;
  }

  // Create the directory that the file will be saved to, if it does not exist.
  std::string directory = vtksys::SystemTools::GetFilenamePath(vtksys::SystemTools::CollapseFullPath(fileName));
  if (!directory.empty() && !vtksys::SystemTools::FileIsDirectory(directory))
  {
    if (!vtksys::SystemTools::MakeDirectory(directory).IsSuccess())
    {
      vtkErrorMacro("SaveNodes error: Unable to create directory " << directory);
      if (userMessages)
      {
        userMessages->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Unable to create directory '%1'"), directory.c_str()));
      }
      return false;
    }
  }

  std::vector<std::string> nodeIDs;
  bool writeSuccess = false;
  for (vtkMRMLFileWriter* writer : writers)
  {
    vtkWeakPointer<vtkMRMLScene> writerScene = writer->GetScene();
    writer->SetScene(scene);
    writer->GetUserMessages()->ClearMessages();
    writer->ClearWrittenNodeIDs();
    bool currentWriterSuccess = writer->Write(properties);
    if (userMessages)
    {
      userMessages->AddMessages(writer->GetUserMessages());
    }
    if (currentWriterSuccess)
    {
      const std::vector<std::string>& writtenNodeIDs = writer->GetWrittenNodeIDs();
      nodeIDs.insert(nodeIDs.end(), writtenNodeIDs.begin(), writtenNodeIDs.end());
    }
    // Restore the scene of the writer (the scene may be a temporary scene)
    writer->SetScene(writerScene);
    if (!currentWriterSuccess)
    {
      continue;
    }
    this->InvokeFileSavedEvent(properties);
    writeSuccess = true;
    break;
  }

  if (!writeSuccess)
  {
    // no appropriate writer was found
    vtkErrorMacro("SaveNodes error: Saving failed with all writers found for file " << fileName << " of type " << fileType);
    if (userMessages)
    {
      userMessages->AddMessage(vtkCommand::ErrorEvent,
                               vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Saving failed with all writers found for file '%1' of type '%2'."), //
                                                   fileName.c_str(),
                                                   fileType.c_str()));
    }
    return false;
  }

  if (nodeIDs.empty() && fileType != "SceneFile")
  {
    // the writer did not report error
    // but did not report any successfully written nodes either
    vtkErrorMacro("SaveNodes error: No nodes were saved in scene");
    if (userMessages)
    {
      userMessages->AddMessage(vtkCommand::ErrorEvent, vtkMRMLTr("qSlicerCoreIOManager", "No nodes were saved in the scene"));
    }
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOManager::ExportNodes(const std::vector<std::string>& nodeIDs,
                                       const std::vector<std::string>& fileNames,
                                       vtkMRMLIOProperties* commonProperties,
                                       bool hardenTransforms,
                                       vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  if (nodeIDs.size() != fileNames.size())
  {
    vtkErrorMacro("ExportNodes failed: Mismatch in number of nodeIDs and filenames");
    return false;
  }
  vtkNew<vtkCollection> propertiesList;
  for (size_t nodeIndex = 0; nodeIndex < nodeIDs.size(); ++nodeIndex)
  {
    vtkNew<vtkMRMLIOProperties> nodeProperties;
    nodeProperties->Copy(commonProperties);
    nodeProperties->SetStringProperty("nodeID", nodeIDs[nodeIndex]);
    nodeProperties->SetStringProperty("fileName", fileNames[nodeIndex]);
    propertiesList->AddItem(nodeProperties);
  }
  return this->ExportNodes(propertiesList, hardenTransforms, userMessages);
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOManager::ExportNodes(vtkCollection* propertiesList, bool hardenTransforms, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  if (!this->Scene || !propertiesList)
  {
    vtkErrorMacro("ExportNodes failed: invalid scene or properties");
    return false;
  }

  // Create a temporary scene to use for exporting only and to be destroyed when done exporting
  vtkNew<vtkMRMLScene> temporaryScene;
  this->Scene->CopyDefaultNodesToScene(temporaryScene);
  this->Scene->CopyRegisteredNodesToScene(temporaryScene);
  this->Scene->CopySingletonNodesToScene(temporaryScene);
  temporaryScene->SetDataIOManager(this->Scene->GetDataIOManager());

  bool success = true;
  for (int propertiesIndex = 0; propertiesIndex < propertiesList->GetNumberOfItems(); ++propertiesIndex)
  {
    vtkMRMLIOProperties* properties = vtkMRMLIOProperties::SafeDownCast(propertiesList->GetItemAsObject(propertiesIndex));
    if (!properties)
    {
      vtkErrorMacro("ExportNodes failed: item " << propertiesIndex << " is not a vtkMRMLIOProperties object");
      return false;
    }
    // Validate parameters
    for (const char* requiredKey : { "nodeID", "fileName", "fileFormat" })
    {
      if (!properties->HasProperty(requiredKey))
      {
        vtkErrorMacro("ExportNodes failed: " << requiredKey << " must be included as a string parameter.");
        return false;
      }
    }
    std::string nodeID = properties->GetStringProperty("nodeID");

    // Copy over each node to be exported
    vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(this->Scene->GetNodeByID(nodeID));
    if (!storableNode)
    {
      if (userMessages)
      {
        userMessages->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Unable to find a storable node with ID %1"), nodeID.c_str()));
      }
      success = false;
      continue;
    }
    const char* nodeName = storableNode->GetName() ? storableNode->GetName() : "";
    vtkMRMLStorableNode* temporaryStorableNode = vtkMRMLStorableNode::SafeDownCast(temporaryScene->AddNewNodeByClass(storableNode->GetClassName()));
    if (!temporaryStorableNode)
    {
      vtkErrorMacro("ExportNodes error: Unable to add node to temporary scene");
      if (userMessages)
      {
        userMessages->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Error encountered while exporting %1."), nodeName));
      }
      success = false;
      continue;
    }
    // We will do a shallow copy, unless transform hardening was requested. Transform hardening
    // can sometimes affect the underlying data of the transformable node, so it's worth doing
    // a deep copy to be certain that the original node is not modified during export.
    temporaryStorableNode->CopyContent(storableNode, /*deepCopy=*/hardenTransforms);

    // If transform hardening was requested and node is transformable, then put transforms in the temporaryScene and apply hardening.
    vtkMRMLTransformableNode* nodeAsTransformable = vtkMRMLTransformableNode::SafeDownCast(storableNode);
    if (hardenTransforms && nodeAsTransformable)
    {
      vtkMRMLTransformableNode* temporaryNodeAsTransformable = vtkMRMLTransformableNode::SafeDownCast(temporaryStorableNode);
      if (!temporaryNodeAsTransformable)
      {
        vtkErrorMacro("ExportNodes failed: Node is transformable but its copy is not... this should never happen.");
        return false;
      }
      vtkMRMLTransformNode* parentTransform = nodeAsTransformable->GetParentTransformNode();
      if (parentTransform)
      {
        vtkNew<vtkGeneralTransform> generalTransform;
        parentTransform->GetTransformFromWorld(generalTransform);
        vtkMRMLTransformNode* compositeTransformNode = vtkMRMLTransformNode::SafeDownCast(temporaryScene->AddNewNodeByClass("vtkMRMLTransformNode"));
        if (!compositeTransformNode)
        {
          vtkErrorMacro("ExportNodes: Unable to add a transform node to temporary scene");
          if (userMessages)
          {
            userMessages->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Error encountered while exporting %1."), nodeName));
          }
          success = false;
          continue;
        }
        compositeTransformNode->SetAndObserveTransformFromParent(generalTransform);
        temporaryNodeAsTransformable->SetAndObserveTransformNodeID(compositeTransformNode->GetID());
        temporaryNodeAsTransformable->HardenTransform();
      }
    }

    // Copy parameters map; we will need to set the nodeID parameter to correspond to the node in the temporary scene
    vtkNew<vtkMRMLIOProperties> temporarySceneProperties;
    temporarySceneProperties->Copy(properties);
    temporarySceneProperties->SetStringProperty("nodeID", temporaryStorableNode->GetID());

    // Deduce "fileType" from "fileFormat" parameter; SaveNodes will want both
    std::string fileType = this->GetFileWriterFileType(storableNode, properties->GetStringProperty("fileFormat"));
    if (fileType.empty())
    {
      // Same file type name as used by the Qt-based IO manager (appears in error messages)
      fileType = "NoFile";
    }

    // Add default storage node into the temporary scene. This is sometimes needed.
    if (!temporaryStorableNode->AddDefaultStorageNode())
    {
      vtkErrorMacro("ExportNodes error: Unable to create default storage in temporary scene");
      if (userMessages)
      {
        userMessages->AddMessage(vtkCommand::ErrorEvent,
                                 vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Unable to create default storage node for %1 in temporary scene."), nodeName));
      }
      success = false;
      continue;
    }

    // Some data files store display properties (for example: markups), therefore we need to copy the display node as well.
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(storableNode);
    vtkMRMLDisplayableNode* temporaryDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast(temporaryStorableNode);
    if (displayableNode && temporaryDisplayableNode && displayableNode->GetDisplayNode())
    {
      vtkMRMLDisplayNode* temporaryDisplayNode = vtkMRMLDisplayNode::SafeDownCast(temporaryScene->AddNewNodeByClass(displayableNode->GetDisplayNode()->GetClassName()));
      if (temporaryDisplayNode)
      {
        temporaryDisplayNode->CopyContent(displayableNode->GetDisplayNode(), false);
        temporaryDisplayableNode->SetAndObserveDisplayNodeID(temporaryDisplayNode->GetID());
      }
      else if (userMessages)
      {
        userMessages->AddMessage(vtkCommand::WarningEvent,
                                 vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Unable to save display properties for %1 in temporary scene."), nodeName));
      }
    }

    // Finally, applying saving logic to the the temporary scene
    if (!this->SaveNodes(fileType, temporarySceneProperties, userMessages, temporaryScene))
    {
      if (userMessages)
      {
        userMessages->AddMessage(vtkCommand::ErrorEvent, vtkMRMLI18N::Format(vtkMRMLTr("qSlicerCoreIOManager", "Error encountered while exporting %1."), nodeName));
      }
      success = false;
    }

    // Pick up any user messages that were saved to temporaryStorableNode's storage node
    if (userMessages && temporaryStorableNode->GetStorageNode() && temporaryStorableNode->GetStorageNode()->GetUserMessages())
    {
      userMessages->AddMessages(temporaryStorableNode->GetStorageNode()->GetUserMessages());
    }
  }
  return success;
}

//----------------------------------------------------------------------------
bool vtkMRMLFileIOManager::SaveScene(const std::string& fileName, vtkImageData* screenShot /*=nullptr*/, vtkMRMLMessageCollection* userMessages /*=nullptr*/)
{
  vtkNew<vtkMRMLIOProperties> properties;
  properties->SetStringProperty("fileName", fileName);
  if (screenShot)
  {
    properties->SetObjectProperty("screenShot", screenShot);
  }
  return this->SaveNodes("SceneFile", properties, userMessages);
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::AddDefaultStorageNodes()
{
  if (!this->Scene)
  {
    vtkErrorMacro("AddDefaultStorageNodes failed: invalid scene");
    return;
  }
  int numNodes = this->Scene->GetNumberOfNodes();
  for (int i = 0; i < numNodes; ++i)
  {
    vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(this->Scene->GetNthNode(i));
    if (!storableNode || !storableNode->GetSaveWithScene())
    {
      continue;
    }
    if (storableNode->GetStorageNode())
    {
      // this node already has a storage node
      continue;
    }
    storableNode->AddDefaultStorageNode();
  }
}

//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLFileIOManager::CreateAndAddDefaultStorageNode(vtkMRMLStorableNode* node)
{
  if (!node)
  {
    vtkGenericWarningMacro("vtkMRMLFileIOManager::CreateAndAddDefaultStorageNode failed: invalid input node");
    return nullptr;
  }
  if (!node->AddDefaultStorageNode())
  {
    vtkGenericWarningMacro("vtkMRMLFileIOManager::CreateAndAddDefaultStorageNode failed: error while adding default storage node");
    return nullptr;
  }
  return node->GetStorageNode();
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::InvokeNewFileLoadedEvent(vtkMRMLIOProperties* loadedFileProperties)
{
  this->InvokeEvent(NewFileLoadedEvent, loadedFileProperties);
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::InvokeFileSavedEvent(vtkMRMLIOProperties* savedFileProperties)
{
  this->InvokeEvent(FileSavedEvent, savedFileProperties);
}

//----------------------------------------------------------------------------
std::string vtkMRMLFileIOManager::GetDefaultSceneFileType() const
{
  return this->DefaultSceneFileType;
}

//----------------------------------------------------------------------------
void vtkMRMLFileIOManager::SetDefaultSceneFileType(const std::string& fileType)
{
  if (this->DefaultSceneFileType == fileType)
  {
    return;
  }
  this->DefaultSceneFileType = fileType;
  this->Modified();
}
