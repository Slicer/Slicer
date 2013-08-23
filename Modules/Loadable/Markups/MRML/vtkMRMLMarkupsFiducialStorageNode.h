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
/// vtkMRMLMarkupsFiducialStorageNode - MRML node for markups fiducial storage
///
/// vtkMRMLMarkupsFiducialStorageNode nodes describe the markups storage
/// node that allows to read/write fiducial point data from/to file.

#ifndef __vtkMRMLMarkupsFiducialStorageNode_h
#define __vtkMRMLMarkupsFiducialStorageNode_h

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsStorageNode.h"

/// \ingroup Slicer_QtModules_Markups
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsFiducialStorageNode : public vtkMRMLMarkupsStorageNode
{
public:
  static vtkMRMLMarkupsFiducialStorageNode *New();
  vtkTypeMacro(vtkMRMLMarkupsFiducialStorageNode,vtkMRMLMarkupsStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "MarkupsFiducialStorage";};

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Return a default file extension for writing
  virtual const char* GetDefaultWriteFileExtension();

  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

protected:
  vtkMRMLMarkupsFiducialStorageNode();
  ~vtkMRMLMarkupsFiducialStorageNode();
  vtkMRMLMarkupsFiducialStorageNode(const vtkMRMLMarkupsFiducialStorageNode&);
  void operator=(const vtkMRMLMarkupsFiducialStorageNode&);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node.
  /// Assumes 1 point per markup for a fiducial referenced node:
  /// x,y,z,ow,ox,oy,oz,vis,sel,lock,label,id,desc,associatedNodeID
  /// orientation is a quaternion, angle and axis
  /// associatedNodeID and description can be empty strings
  /// x,y,z,ow,ox,oy,oz,vis,sel,lock,label,id,,
  /// label can have spaces, everything up to next comma is used, no quotes
  /// necessary, same with the description
  virtual int WriteDataInternal(vtkMRMLNode *refNode);
};

#endif
