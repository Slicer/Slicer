/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLSequenceStorageNode_h
#define __vtkMRMLSequenceStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLSequenceNode.h"

/// \brief MRML node for storing a sequence node on disk.
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk.
class VTK_MRML_EXPORT vtkMRMLSequenceStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLSequenceStorageNode *New();
  vtkTypeMacro(vtkMRMLSequenceStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Sequence)
  const char* GetNodeTagName() override {return "SequenceStorage";};

  /// Return a default file extension for writting
  const char* GetDefaultWriteFileExtension() override;

  /// Return true if the reference node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

  // filename: fCal_Test_Validation_3NWires_fCal2.0-ProbeToTracker-Seq.seq.mha
  // itemname: ProbeToTracker
  // return: fCal_Test_Validation_3NWires_fCal2.0
  static std::string GetSequenceBaseName(const std::string& fileNameName, const std::string& itemName);

  // baseName: fCal_Test_Validation_3NWires_fCal2.0
  // itemName: Image
  // return: fCal_Test_Validation_3NWires_fCal2.0-Image-Seq
  static std::string GetSequenceNodeName(const std::string& baseName, const std::string& itemName);

protected:
  vtkMRMLSequenceStorageNode();
  ~vtkMRMLSequenceStorageNode() override;
  vtkMRMLSequenceStorageNode(const vtkMRMLSequenceStorageNode&);
  void operator=(const vtkMRMLSequenceStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkMRMLNode *refNode) override;

  bool WriteToMRB(const char* fullName, vtkMRMLScene *scene);

  bool ReadFromMRB(const char* fullName, vtkMRMLScene *scene);

  /// Force each storable node to be saved to a file with a different name, preventing overwriting during saving
  void ForceUniqueDataNodeFileNames(vtkMRMLSequenceNode* sequenceNode);
};

#endif
