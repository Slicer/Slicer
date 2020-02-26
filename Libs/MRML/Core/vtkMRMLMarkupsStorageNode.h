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
/// vtkMRMLMarkupsStorageNode - MRML node for handling markups storage
///
/// vtkMRMLMarkupsStorageNode nodes describe the markups storage
/// node that allows to read/write point data from/to file.
/// In MRML/Core to facilitate passing markups to CLIs via file.

#ifndef __vtkMRMLMarkupsStorageNode_h
#define __vtkMRMLMarkupsStorageNode_h

// MRML includes
#include "vtkMRMLStorageNode.h"

/// Define a default buffer size for parsing files during read, number of characters
#define MARKUPS_BUFFER_SIZE 1024

class VTK_MRML_EXPORT vtkMRMLMarkupsStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLMarkupsStorageNode *New();
  vtkTypeMacro(vtkMRMLMarkupsStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "MarkupsStorage";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

  /// Get/Set flag that controls if points are to be written in various coordinate systems
  vtkSetClampMacro(CoordinateSystem, int, 0, vtkMRMLStorageNode::CoordinateSystemType_Last-1);
  vtkGetMacro(CoordinateSystem, int);
  std::string GetCoordinateSystemAsString();
  static const char* GetCoordinateSystemAsString(int id);
  static int GetCoordinateSystemFromString(const char* name);
  /// Convenience methods to get/set various coordinate system values
  /// \sa SetCoordinateSystem, GetCoordinateSystem
  void UseRASOn();
  bool GetUseRAS();
  void UseLPSOn();
  bool GetUseLPS();

  /// Convert between user input strings and strings safe to be
  /// written to the storage file. Since the current storage node
  /// file format is CSV, puts double quotes around strings if there
  /// there are commas or double quotes in them, and replace occurrences
  /// of double quotes with two double quotes
  std::string ConvertStringToStorageFormat(std::string input);
  std::string ConvertStringFromStorageFormat(std::string input);

protected:
  vtkMRMLMarkupsStorageNode();
  ~vtkMRMLMarkupsStorageNode() override;
  vtkMRMLMarkupsStorageNode(const vtkMRMLMarkupsStorageNode&);
  void operator=(const vtkMRMLMarkupsStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  /// Needs to be implemented by subclasses
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a  referenced node
  /// There can be any number of points associated with a
  /// markup, so subclasses need to implement this for their markup type
  int WriteDataInternal(vtkMRMLNode *refNode) override;

  /// Extract the first quoted string from an input string and return it.
  /// Used in parsing the input strings in \sa ReadDataInternal.
  /// Returns output string with the quotes around it, but no ending comma.
  /// Returns the location of the ending comma in the input string for further parsing
  /// With no starting or ending quote, returns an empty string.
  std::string GetFirstQuotedString(std::string inputString, size_t *endCommaPos);

private:

  /// Flag set to enum RAS if the points are to be written out/read in using
  /// the RAS coordinate system, enum LPS if the points are to be written
  /// out/read in using LPS coordinate system, enum IJK if the points are
  /// to be written out in the IJK coordinates for the associated volume node.
  int CoordinateSystem;
};

#endif
