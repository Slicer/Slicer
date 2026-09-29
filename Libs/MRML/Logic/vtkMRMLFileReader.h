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

#ifndef __vtkMRMLFileReader_h
#define __vtkMRMLFileReader_h

#include "vtkMRMLFileIOHandler.h"

class vtkMRMLIOProperties;

/// Base class of file readers, which read files into the scene.
/// Subclasses override Load() and, if the file extension is not enough to decide if the file can be loaded,
/// CanLoadFileConfidence().
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#the-reader
class VTK_MRML_LOGIC_EXPORT vtkMRMLFileReader : public vtkMRMLFileIOHandler
{
public:
  static vtkMRMLFileReader* New();
  vtkTypeMacro(vtkMRMLFileReader, vtkMRMLFileIOHandler);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Returns a positive number (>0) if the reader can load this file.
  /// The higher the returned value is the more confident the reader it is
  /// the most suitable class to load the file.
  /// By default, the method calls CanLoadFile and if it returns true then
  /// the returned confidence value is ConfidenceForMatchingExtension (0.5 by default)
  /// + 0.01 * matchedFileExtensionLength.
  /// The additional confidence for longer matched file extensions allow prioritization of
  /// more specific readers. For example, "*.seg.nrrd" is more specific than "*.nrrd";
  /// "*.nrrd" is more specific than "*.*".
  virtual double CanLoadFileConfidence(const std::string& filePath);

  /// Returns true if the reader can load this file.
  /// Default implementation is a simple and fast, it just checks
  /// if file extension matches any of the name filters.
  virtual bool CanLoadFile(const std::string& filePath);

  /// Return the name filters that match the file. For example, if the file name is "my_image.nrrd"
  /// and the supported name filters are "Volumes (*.mha *.nrrd *.raw)", "Images (*.png *.jpg)", "DICOM (*)"
  /// then it returns "Volumes (*.mha *.nrrd *.raw)" and "DICOM (*)".
  /// No name filters are returned if the file is not a readable file.
  std::vector<std::string> GetSupportedNameFilters(const std::string& filePath);
  /// Same as GetSupportedNameFilters(filePath) but it also returns the length of the longest matched extension
  /// in longestExtensionMatch (it can be used to determine how specifically extension matched).
  std::vector<std::string> GetSupportedNameFilters(const std::string& filePath, int& longestExtensionMatch);

  /// Read the file specified by the "fileName" property into the scene.
  /// Loaded nodes are reported by AddLoadedNodeID(). Returns true on success.
  /// The base class reads nothing and returns false.
  virtual bool Load(vtkMRMLIOProperties* properties);

  /// Examine the list of files to see if there is an entry that can serve as an archetype for loading multiple files.
  /// If so, the reader removes the recognized files (except the archetype) from the list, sets the ioProperties
  /// so that the reader will read these files, and returns the archetype file path.
  /// If no pattern is recognized then the method returns an empty string.
  /// The specific motivating use case is when the file list contains a set of related files, such as a list of image
  /// files that are recognized as a volume.
  virtual std::string ExamineFileList(std::vector<std::string>& fileList, vtkMRMLIOProperties* ioProperties);

  /// The nodes the last Load added, by node ID.
  const std::vector<std::string>& GetLoadedNodeIDs() const;
  void SetLoadedNodeIDs(const std::vector<std::string>& nodeIDs);
  void AddLoadedNodeID(const std::string& nodeID);
  void ClearLoadedNodeIDs();

  /// Confidence value returned by the default CanLoadFileConfidence implementation when the file matches
  /// one of the name filters. A small value is added for longer matched extensions.
  vtkGetMacro(ConfidenceForMatchingExtension, double);
  vtkSetMacro(ConfidenceForMatchingExtension, double);

protected:
  vtkMRMLFileReader();
  ~vtkMRMLFileReader() override;

  /// Read the first maxLength characters of a text file (to inspect a file header).
  /// Returns empty string if the file cannot be read.
  static std::string ReadFileHeader(const std::string& filePath, size_t maxLength);

  /// Returns true if the string ends with the suffix (case-insensitive).
  static bool EndsWithNoCase(const std::string& text, const std::string& suffix);

  double ConfidenceForMatchingExtension{ 0.5 };
  std::vector<std::string> LoadedNodeIDs;

private:
  vtkMRMLFileReader(const vtkMRMLFileReader&) = delete;
  void operator=(const vtkMRMLFileReader&) = delete;
};

#endif
