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

/// Markups Module MRML storage nodes
///
/// vtkMRMLMarkupsJsonStorageNode - MRML node for storing markups in JSON file
///

#ifndef __vtkMRMLMarkupsJsonStorageNode_h
#define __vtkMRMLMarkupsJsonStorageNode_h

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsStorageNode.h"

class vtkMRMLMarkupsNode;
class vtkMRMLMarkupsDisplayNode;

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsJsonStorageNode : public vtkMRMLMarkupsStorageNode
{
public:
  static vtkMRMLMarkupsJsonStorageNode *New();
  vtkTypeMacro(vtkMRMLMarkupsJsonStorageNode,vtkMRMLMarkupsStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  vtkMRMLMarkupsNode* AddNewMarkupsNodeFromFile(const char* filePath, const char* nodeName=nullptr, int markupIndex=0);

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "MarkupsJsonStorage";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

protected:
  vtkMRMLMarkupsJsonStorageNode();
  ~vtkMRMLMarkupsJsonStorageNode() override;
  vtkMRMLMarkupsJsonStorageNode(const vtkMRMLMarkupsJsonStorageNode&);
  void operator=(const vtkMRMLMarkupsJsonStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a  referenced node.
  int WriteDataInternal(vtkMRMLNode *refNode) override;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
