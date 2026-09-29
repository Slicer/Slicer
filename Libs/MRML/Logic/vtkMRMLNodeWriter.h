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

#ifndef __vtkMRMLNodeWriter_h
#define __vtkMRMLNodeWriter_h

#include "vtkMRMLFileWriter.h"

// VTK includes
#include <vtkSmartPointer.h>

class vtkMRMLNode;
class vtkMRMLStorableNode;

/// Writer that is ready to use for most of the nodes: it writes a storable node using its storage node.
///
/// qSlicerNodeWriter is the Qt interface of this class, kept for backward compatibility.
class VTK_MRML_LOGIC_EXPORT vtkMRMLNodeWriter : public vtkMRMLFileWriter
{
public:
  static vtkMRMLNodeWriter* New();
  vtkTypeMacro(vtkMRMLNodeWriter, vtkMRMLFileWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create a writer that writes nodes of the specified classes using their storage nodes.
  /// The writer is not registered in any file IO manager.
  /// \sa vtkMRMLFileIOManager::RegisterNodeWriter()
  static vtkSmartPointer<vtkMRMLNodeWriter> CreateNodeWriter(const std::string& description,
                                                             const std::string& fileType,
                                                             const std::vector<std::string>& nodeClassNames,
                                                             bool supportUseCompression = true);

  /// Some storage nodes don't support the compression option
  vtkGetMacro(SupportUseCompression, bool);
  vtkSetMacro(SupportUseCompression, bool);
  vtkBooleanMacro(SupportUseCompression, bool);

  /// Return true if the object is a storable node of any of the node class names.
  bool CanWriteObject(vtkObject* object) override;

  /// Return the list of the supported file formats for a particular object.
  /// Example: "NRRD (.nrrd)", "Model (.vtk)".
  std::vector<std::string> GetNameFiltersForObject(vtkObject* object) override;

  /// Write the node referenced by "nodeID" into the "fileName" file.
  /// Optionally, "useCompression", "compressionParameter", and "fileFormat" can be specified.
  /// Return true on success, false otherwise.
  /// Create a storage node if the storable node doesn't have any.
  bool Write(vtkMRMLIOProperties* properties) override;

  /// Write the specified node into the "fileName" file, without checking if the writer can write the node.
  /// Optionally, "useCompression", "compressionParameter", and "fileFormat" can be specified.
  /// The node is added to the written node IDs on success. Written node IDs are not cleared.
  /// It allows subclasses and Qt-based writers (qSlicerNodeWriter) to use a custom method for finding the node
  /// and checking if it can be written.
  /// Return true on success, false otherwise.
  bool WriteNode(vtkMRMLStorableNode* node, vtkMRMLIOProperties* properties);

  /// Options: "useCompression", "compressionParameter" (if SupportUseCompression is enabled).
  /// Default values are taken from the storage node of the node that is written ("nodeID" property).
  void GetOptionsDescription(vtkMRMLIOOptionsDescription* description) override;

  /// Get node from the scene by node ID.
  virtual vtkMRMLNode* GetNodeByID(const std::string& id);

protected:
  vtkMRMLNodeWriter();
  ~vtkMRMLNodeWriter() override;

  bool SupportUseCompression{ true };

private:
  vtkMRMLNodeWriter(const vtkMRMLNodeWriter&) = delete;
  void operator=(const vtkMRMLNodeWriter&) = delete;
};

#endif
