/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __vtkMRMLTextStorageNode_h
#define __vtkMRMLTextStorageNode_h

// MRML includes
#include <vtkMRMLStorageNode.h>

class VTK_MRML_EXPORT vtkMRMLTextStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLTextStorageNode* New();
  vtkTypeMacro(vtkMRMLTextStorageNode, vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLTextStorageNode);

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "TextStorage"; };

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  /// Return true if the node can be written by using the writer.
  bool CanWriteFromReferenceNode(vtkMRMLNode* refNode) override;
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  /// Return a default file extension for writing.
  /// It corresponds to the first format in the list of supported write file formats.
  const char* GetDefaultWriteFileExtension() override;

  //@{
  /// Get/Set file extensions that this storage node can read and write.
  /// The first write file extension is also used as DefaultWriteFileExtension.
  /// This feature is useful to allow using this storage node for storing text files
  /// with any extension. For example, developers may want to save configuration files in .yaml format,
  /// which extension is not recognized by default by this node. In that case, a custom file reader
  /// can be added, which recognizes the .yaml file and instantiates a text storage node with custom
  /// supported read/write file extensions.
  /// If all file extensions are set to empty then the class default file extensions are used.
  /// fileExtensions is a string list containing each supported file extension, without a leading "." character
  /// (for example: ["yml", "yaml"]).
  void SetSupportedReadFileExtensions(const std::vector<std::string> fileExtensions);
  void SetSupportedWriteFileExtensions(const std::vector<std::string> fileExtensions);
  std::vector<std::string> GetSupportedReadFileExtensions();
  std::vector<std::string> GetSupportedWriteFileExtensions();
  //@}

protected:
  vtkMRMLTextStorageNode();
  ~vtkMRMLTextStorageNode() override;
  vtkMRMLTextStorageNode(const vtkMRMLTextStorageNode&);
  void operator=(const vtkMRMLTextStorageNode&);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is incompatible.
  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  // Customizable list of file extensions that this class can read/write
  std::vector<std::string> SupportedReadFileExtensions;
  std::vector<std::string> SupportedWriteFileExtensions;
};

#endif
