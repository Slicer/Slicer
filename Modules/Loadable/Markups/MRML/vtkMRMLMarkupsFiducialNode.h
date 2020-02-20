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

#ifndef __vtkMRMLMarkupsFiducialNode_h
#define __vtkMRMLMarkupsFiducialNode_h

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
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsFiducialNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsFiducialNode *New();
  vtkTypeMacro(vtkMRMLMarkupsFiducialNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsMouseModePlace.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsFiducial";}

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

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

  /// Get the number of fiducials in this node
  int GetNumberOfFiducials() { return this->GetNumberOfControlPoints(); } ;
  /// Add a new fiducial from x,y,z coordinates and return the fiducial index
  int AddFiducial(double x, double y, double z);
  int AddFiducial(double x, double y, double z, std::string label);
  /// Add a new fiducial from an array and return the fiducial index
  int AddFiducialFromArray(double pos[3], std::string label = std::string());
  /// Get the position of the nth fiducial, returning it in the pos array
  void GetNthFiducialPosition(int n, double pos[3]);
  /// Set the position of the nth fiducial from x, y, z coordinates
  void SetNthFiducialPosition(int n, double x, double y, double z);
  /// Set the position of the nth fiducial from a double array
  void SetNthFiducialPositionFromArray(int n, double pos[3]);
  /// Get selected property on Nth fiducial
  bool GetNthFiducialSelected(int n = 0);
  /// Set selected property on Nth fiducial
  void SetNthFiducialSelected(int n, bool flag);
  /// Get locked property on Nth fiducial
  bool GetNthFiducialLocked(int n = 0);
  /// Set locked property on Nth fiducial
  void SetNthFiducialLocked(int n, bool flag);
  /// Get visibility property on Nth fiducial
  bool GetNthFiducialVisibility(int n = 0);
  /// Set visibility property on Nth fiducial. If the visibility is set to
  /// true on the node/list as a whole, the nth fiducial visibility is used to
  /// determine if it is visible. If the visibility is set to false on the node
  /// as a whole, all fiducials are hidden but keep this value for when the
  /// list as a whole is turned visible.
  /// \sa vtkMRMLDisplayableNode::SetDisplayVisibility
  /// \sa vtkMRMLDisplayNode::SetVisibility
  void SetNthFiducialVisibility(int n, bool flag);
  /// Get label on nth fiducial
  std::string GetNthFiducialLabel(int n = 0);
  /// Set label on nth fiducial
  void SetNthFiducialLabel(int n, std::string label);
  /// Get associated node id on nth fiducial
  std::string GetNthFiducialAssociatedNodeID(int n = 0);
  /// Set associated node id on nth fiducial
  void SetNthFiducialAssociatedNodeID(int n, const char* id);
  /// Set world coordinates on nth fiducial
  void SetNthFiducialWorldCoordinates(int n, double coords[4]);
  /// Get world coordinates on nth fiducial
  void GetNthFiducialWorldCoordinates(int n, double coords[4]);

  /// Create and observe default display node(s)
  void CreateDefaultDisplayNodes() override;

protected:
  vtkMRMLMarkupsFiducialNode();
  ~vtkMRMLMarkupsFiducialNode() override;
  vtkMRMLMarkupsFiducialNode(const vtkMRMLMarkupsFiducialNode&);
  void operator=(const vtkMRMLMarkupsFiducialNode&);

};

#endif
