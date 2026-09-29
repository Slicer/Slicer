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

#ifndef __vtkMRMLFileWriter_h
#define __vtkMRMLFileWriter_h

#include "vtkMRMLFileIOHandler.h"

class vtkMRMLIOProperties;

/// Base class of file writers, which write a node (or the scene) to file.
/// Subclasses override Write() and, if checking the node class is not enough to decide if the object can be
/// written, CanWriteObjectConfidence().
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#the-writer
class VTK_MRML_LOGIC_EXPORT vtkMRMLFileWriter : public vtkMRMLFileIOHandler
{
public:
  static vtkMRMLFileWriter* New();
  vtkTypeMacro(vtkMRMLFileWriter, vtkMRMLFileIOHandler);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  bool IsWriter() const override { return true; }

  /// Returns a positive number (>0) if the writer can write the object to file.
  /// The higher the returned value is the more confident the writer it is
  /// the most suitable class to write the object.
  /// By default, the method calls CanWriteObject and if it returns true then
  /// it returns ConfidenceForMatchingClass (0.5 by default).
  virtual double CanWriteObjectConfidence(vtkObject* object);

  /// Return true if the object is handled by the writer.
  /// By default it returns true if the object is an instance of any of the node class names.
  virtual bool CanWriteObject(vtkObject* object);

  /// Get the list of name filters supported for writing a particular object.
  /// Example: "Image (*.jpg *.png *.tiff)", "Model (.vtk)".
  /// By default, it returns the name filters of the writer (see SetNameFilters()).
  virtual std::vector<std::string> GetNameFiltersForObject(vtkObject* object);

  /// Write the node named by the "nodeID" property to the file named by "fileName". What it wrote
  /// is then in GetWrittenNodeIDs. The base class writes nothing: a writer whose work is done
  /// elsewhere is called through its owner.
  virtual bool Write(vtkMRMLIOProperties* properties);

  /// The nodes the last Write wrote, by node ID.
  const std::vector<std::string>& GetWrittenNodeIDs() const;
  void SetWrittenNodeIDs(const std::vector<std::string>& nodeIDs);
  void AddWrittenNodeID(const std::string& nodeID);
  void ClearWrittenNodeIDs();

  /// The class of node this writer is for ("vtkMRMLTextNode"). Setting it replaces all node class names
  /// with this single class name.
  /// Empty for a writer that decides for itself in CanWriteObjectConfidence.
  std::string GetNodeClassName() const;
  void SetNodeClassName(const std::string& className);

  /// The classes of nodes this writer is for.
  void SetNodeClassNames(const std::vector<std::string>& classNames);
  const std::vector<std::string>& GetNodeClassNames() const { return this->NodeClassNames; }
  void AddNodeClassName(const std::string& className);

  /// What a writer that only knows its node class answers for a node of that class.
  vtkGetMacro(ConfidenceForMatchingClass, double);
  vtkSetMacro(ConfidenceForMatchingClass, double);

protected:
  vtkMRMLFileWriter();
  ~vtkMRMLFileWriter() override;

  std::vector<std::string> NodeClassNames;
  double ConfidenceForMatchingClass{ 0.5 };
  std::vector<std::string> WrittenNodeIDs;

private:
  vtkMRMLFileWriter(const vtkMRMLFileWriter&) = delete;
  void operator=(const vtkMRMLFileWriter&) = delete;
};

#endif
