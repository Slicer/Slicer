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

#ifndef __vtkMRMLMarkupsPointListNode_h
#define __vtkMRMLMarkupsPointListNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

/// \brief MRML node to represent a fiducial markup
/// Fiducial Markups nodes contain a list of control points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
/// There is no specific limit for number of points that can be added to a list,
/// but performance is optimal if there are less than 2000 points.
///
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsPointListNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsPointListNode *New();
  vtkTypeMacro(vtkMRMLMarkupsPointListNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsFiducial.png";}
  const char* GetAddIcon() override {return ":/Icons/MarkupsFiducialMouseModePlace.png";}
  const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsFiducialMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsPointList";}

  /// Get markup type internal name
  const char* GetMarkupType() override {return "PointList";};

  // Get markup type GUI display name
  const char* GetTypeDisplayName() override {return "Point List";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "F";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsPointListNode);

  /// Maximum number of control points limits the number of markups allowed in the node.
  /// If maximum number of control points is set to 0 then no it means there
  /// is no limit (this is the default value).
  /// The value is an indication to the user interface and does not affect
  /// prevent adding markups to a node programmatically.
  /// If value is set to lower value than the number of markups in the node, then
  /// existing markups are not deleted.
  /// 2 for line, and 3 for angle Markups
  vtkSetMacro(MaximumNumberOfControlPoints, int);

  /// Set the number of control points that are required for defining this widget.
  /// Interaction mode remains in "place" mode until this number is reached.
  /// If the number is set to 0 then no it means there is no preference (this is the default value).
  vtkSetMacro(RequiredNumberOfControlPoints, int);

  /// Create and observe default display node(s)
  void CreateDefaultDisplayNodes() override;

protected:
  vtkMRMLMarkupsPointListNode();
  ~vtkMRMLMarkupsPointListNode() override;
  vtkMRMLMarkupsPointListNode(const vtkMRMLMarkupsPointListNode&);
  void operator=(const vtkMRMLMarkupsPointListNode&);

};

#endif
