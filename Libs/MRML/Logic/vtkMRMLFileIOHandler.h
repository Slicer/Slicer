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

#ifndef __vtkMRMLFileIOHandler_h
#define __vtkMRMLFileIOHandler_h

#include "vtkMRMLIOProperties.h"
#include "vtkMRMLLogicExport.h"

#include <vtkMRMLMessageCollection.h>

#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkWeakPointer.h>

#include <string>
#include <vector>

class vtkMRMLScene;
class vtkMRMLFileIOManager;
class vtkMRMLIOOptionsDescription;

/// Base class of file readers and writers (vtkMRMLFileReader, vtkMRMLFileWriter).
/// It stores the file type, description, and name filters of the reader or writer and describes its options.
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#file-reading-and-writing
class VTK_MRML_LOGIC_EXPORT vtkMRMLFileIOHandler : public vtkObject
{
public:
  static vtkMRMLFileIOHandler* New();
  vtkTypeMacro(vtkMRMLFileIOHandler, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// The kind of file this handles, as the rest of Slicer names it ("VolumeFile", "ModelFile",
  /// "SceneFile", or whatever a module calls its own).
  vtkGetMacro(FileType, std::string);
  vtkSetMacro(FileType, std::string);

  /// What to call this in a file dialog ("Volume", "Mimics project").
  vtkGetMacro(Description, std::string);
  vtkSetMacro(Description, std::string);

  /// The name filters, as Slicer writes them: "Mimics project (*.mcs *.mcs.gz)".
  void SetNameFilters(const std::vector<std::string>& filters);
  const std::vector<std::string>& GetNameFilters() const { return this->NameFilters; }

  /// The extensions of the name filters, lower case and with the dot: ".mcs", ".mcs.gz".
  /// A filter of "*.*" or "*" gives no extension: such a handler takes any file.
  const std::vector<std::string>& GetExtensions() const { return this->Extensions; }

  /// Whether the file name ends in one of the extensions. A handler with no extension of its own
  /// matches nothing here; it has to say what it can do in CanLoadFileConfidence instead.
  virtual bool MatchesExtension(const std::string& filePath) const;
  /// The longest matching extension of *filePath*, or an empty string.
  virtual std::string GetMatchedExtension(const std::string& filePath) const;

  /// Where the nodes go, and where messages for the user are collected.
  virtual void SetScene(vtkMRMLScene* scene);
  vtkMRMLScene* GetScene() const;

  /// Messages the user should see about the last read or write (a vtkMRMLMessageCollection, as
  /// the readers of Slicer use). Never null.
  vtkMRMLMessageCollection* GetUserMessages() const;

  /// Whether this writes rather than reads.
  virtual bool IsWriter() const { return false; }

  /// The manager this handler is registered in (set by the manager), nullptr if it is not registered.
  /// It allows readers and writers to update application-wide IO settings, such as the default scene file type.
  vtkMRMLFileIOManager* GetFileIOManager() const;
  void SetFileIOManager(vtkMRMLFileIOManager* manager);

  //@{
  /// Options of the reader or writer (properties that the user may set).
  /// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#reader-and-writer-options

  /// Describe the options. The default implementation describes no options.
  /// Properties (context and current values) and defaults are already set in the description.
  virtual void GetOptionsDescription(vtkMRMLIOOptionsDescription* description);

  /// Fill the description of options for the specified context (properties may be nullptr).
  /// Sets properties and defaults in the description, then calls GetOptionsDescription().
  void FillOptionsDescription(vtkMRMLIOProperties* properties, vtkMRMLIOOptionsDescription* description);

  /// Get the description of options as JSON (see vtkMRMLIOOptionsDescription).
  /// Returns an empty string if the reader or writer has no options.
  std::string GetOptionsDescriptionJSON(vtkMRMLIOProperties* properties);

  /// Get the values of all options for the specified context (default values, overridden by values in properties).
  /// It can be used for getting option values without displaying a user interface.
  void GetOptionValues(vtkMRMLIOProperties* properties, vtkMRMLIOProperties* values);

  /// Default values of options that the application specifies (for example, from user settings).
  /// These values are used instead of the defaults of the reader or writer. Never null.
  vtkMRMLIOProperties* GetOptionDefaults();
  //@}

  /// Return the name filters that the file name matches. For example, if the file name is "my_image.nrrd"
  /// and the name filters are "Volumes (*.mha *.nrrd *.raw)", "Images (*.png *.jpg)", "DICOM (*)" then it returns
  /// "Volumes (*.mha *.nrrd *.raw)" and "DICOM (*)".
  /// If  requireReadableFile is true then no filters are returned if the file is not a readable file
  /// (or it is a temporary file, with '~' in its extension).
  /// \param longestExtensionMatch If non-zero then it is set to the length of the longest matched extension
  /// (not counting wildcard characters). It can be used to determine how specifically extension matched.
  static std::vector<std::string> GetMatchingNameFilters(const std::string& filePath,
                                                         const std::vector<std::string>& nameFilters,
                                                         bool requireReadableFile,
                                                         int* longestExtensionMatch = nullptr);

  /// Get the list of wildcard patterns from a name filter. For example: "Image (*.jpg *.png)" -> "*.jpg", "*.png".
  /// A name filter without brackets is returned as is if it is a valid wildcard ("*.jpg").
  static std::vector<std::string> NameFilterToWildcards(const std::string& nameFilter);

  /// Case-insensitive wildcard matching of the whole text. Supported wildcards: '*', '?', and character sets ("[abc]", "[!abc]").
  static bool WildcardMatch(const std::string& pattern, const std::string& text);

  /// The lower-case extension of a file name, taking the compound extensions of Slicer into
  /// account (".seg.nrrd" rather than ".nrrd"), against a list of known extensions.
  static std::string LongestMatchingExtension(const std::string& filePath, const std::vector<std::string>& extensions);
  /// The extensions named by a filter such as "Mimics project (*.mcs *.mcs.gz)".
  static std::vector<std::string> ExtensionsFromNameFilter(const std::string& nameFilter);

protected:
  vtkMRMLFileIOHandler();
  ~vtkMRMLFileIOHandler() override;

  std::string FileType;
  std::string Description;
  std::vector<std::string> NameFilters;
  std::vector<std::string> Extensions;
  vtkWeakPointer<vtkMRMLScene> Scene;
  vtkWeakPointer<vtkMRMLFileIOManager> FileIOManager;
  vtkNew<vtkMRMLIOProperties> OptionDefaults;
  vtkNew<vtkMRMLMessageCollection> UserMessages;

private:
  vtkMRMLFileIOHandler(const vtkMRMLFileIOHandler&) = delete;
  void operator=(const vtkMRMLFileIOHandler&) = delete;
};

#endif
