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

class vtkMRMLMarkupsJsonElement;
class vtkMRMLMarkupsJsonWriter;
class vtkMRMLMarkupsDisplayNode;
class vtkMRMLMarkupsNode;

class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsJsonStorageNode : public vtkMRMLMarkupsStorageNode
{
public:
  static vtkMRMLMarkupsJsonStorageNode* New();
  vtkTypeMacro(vtkMRMLMarkupsJsonStorageNode, vtkMRMLMarkupsStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Read a markups node from a file.
  vtkMRMLMarkupsNode* AddNewMarkupsNodeFromFile(const char* filePath,
                                                const char* nodeName = nullptr,
                                                int markupIndex = 0);

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "MarkupsJsonStorage"; };

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  /// Returns a list of all markup types ("Curve", "ClosedCurve", "Angle", "Plane", etc.) in the json file.
  /// The types are ordered by the index in which they appear in the Json file.
  void GetMarkupsTypesInFile(const char* filePath, std::vector<std::string>& outputMarkupsTypes);

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
  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Write data from a  referenced node.
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  /// Only in C++: The caller must take ownership of the returned object.
  VTK_NEWINSTANCE
  vtkMRMLMarkupsJsonElement* ReadMarkupsFile(const char* filePath);

  std::string GetMarkupsClassNameFromMarkupsType(std::string markupsType);

  virtual bool UpdateMarkupsNodeFromJsonValue(vtkMRMLMarkupsNode* markupsNode, vtkMRMLMarkupsJsonElement* markupObject);
  virtual bool UpdateMarkupsDisplayNodeFromJsonValue(vtkMRMLMarkupsDisplayNode* displayNode,
                                                     vtkMRMLMarkupsJsonElement* markupObject);

  virtual bool ReadControlPoints(vtkMRMLMarkupsJsonElement* controlPointsArray,
                                 int coordinateSystem,
                                 vtkMRMLMarkupsNode* markupsNode);
  virtual bool ReadMeasurements(vtkMRMLMarkupsJsonElement* measurementsArray, vtkMRMLMarkupsNode* markupsNode);

  virtual bool WriteMarkup(vtkMRMLMarkupsJsonWriter* writer, vtkMRMLMarkupsNode* markupsNode);
  virtual bool WriteBasicProperties(vtkMRMLMarkupsJsonWriter* writer, vtkMRMLMarkupsNode* markupsNode);
  virtual bool WriteControlPoints(vtkMRMLMarkupsJsonWriter* writer, vtkMRMLMarkupsNode* markupsNode);
  virtual bool WriteMeasurements(vtkMRMLMarkupsJsonWriter* writer, vtkMRMLMarkupsNode* markupsNode);
  virtual bool WriteDisplayProperties(vtkMRMLMarkupsJsonWriter* writer, vtkMRMLMarkupsDisplayNode* markupsDisplayNode);

  std::string GetCoordinateUnitsFromSceneAsString(vtkMRMLMarkupsNode* markupsNode);
};

#endif
