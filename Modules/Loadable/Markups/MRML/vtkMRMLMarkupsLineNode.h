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

#ifndef __vtkMRMLMarkupsLineNode_h
#define __vtkMRMLMarkupsLineNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

/// \brief MRML node to represent a line markup
/// Line Markups nodes contain two control points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
///
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsLineNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsLineNode *New();
  vtkTypeMacro(vtkMRMLMarkupsLineNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsLine.png";}
  const char* GetAddIcon() override {return ":/Icons/MarkupsLineMouseModePlace.png";}
  const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsLineMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsLine";}

  /// Get markup type internal name
  const char* GetMarkupType() override {return "Line";};

  // Get markup type GUI display name
  const char* GetTypeDisplayName() override {return "Line";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "L";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLMarkupsLineNode);

  /// Return line length (distance between the two line endpoints) in world coordinate system.
  virtual double GetLineLengthWorld();

  //@{
  /// Convenience method to get the line start or end position
  /// (control point indices 0 and 1).
  /// Returns true if the point position is defined.
  bool GetLineStartPosition(double position[3]);
  bool GetLineEndPosition(double position[3]);
  bool GetLineStartPositionWorld(double position[3]);
  bool GetLineEndPositionWorld(double position[3]);
  //@}

  //@{
  /// Convenience method to get the line start or end position
  /// (control point indices 0 and 1).
  vtkVector3d GetLineStartPosition();
  vtkVector3d GetLineEndPosition();
  vtkVector3d GetLineStartPositionWorld();
  vtkVector3d GetLineEndPositionWorld();
  //@}

  //@{
  /// Convenience method to get the line start or end position
  /// (control point indices 0 and 1).
  void SetLineStartPosition(double position[3]);
  void SetLineEndPosition(double position[3]);
  void SetLineStartPositionWorld(double position[3]);
  void SetLineEndPositionWorld(double position[3]);
  //@}

  // Deprecated methods

  //@{
  /// This method is deprecated (it was added for backward compatibility
  /// with vtkMRMLAnnotationRulerNode and will be removed in the future).
  /// Use GetLineStartPosition/GetLineEndPosition method instead.
  /// Get line start/end point in local coordinate system.
  /// Returns true if the point position is defined.
  bool GetPosition1(double position[3]);
  bool GetPosition2(double position[3]);
  //@}

  //@{
  /// This method is deprecated (it was added for backward compatibility
  /// with vtkMRMLAnnotationRulerNode and will be removed in the future).
  /// Use GetLineStartPosition/GetLineEndPosition method instead.
  /// Get line start/end point in local coordinate system.
  /// Returns nullptr if the point position is not defined.
  double* GetPosition1();
  double* GetPosition2();
  //@}

  //@{
  /// This method is deprecated (it was added for backward compatibility
  /// with vtkMRMLAnnotationRulerNode and will be removed in the future).
  /// Use GetLineStartPositionWorld/GetLineEndPositionWorld method instead.
  /// Get line start/end point in world coordinate system.
  /// Returns true if the point position is defined.
  bool GetPositionWorldCoordinates1(double position[3]);
  bool GetPositionWorldCoordinates2(double position[3]);
  //@}

  //@{
  /// This method is deprecated (it was added for backward compatibility
  /// with vtkMRMLAnnotationRulerNode and will be removed in the future).
  /// Use SetLineStartPosition/SetLineEndPosition method instead.
  /// Set line start/end point in local coordinate system.
  /// Returns true if the operation is successful.
  bool SetPosition1(double newControl[3]);
  bool SetPosition2(double newControl[3]);
  bool SetPosition1(double nC1, double nC2, double nC3);
  bool SetPosition2(double nC1, double nC2, double nC3);
  //@}

  //@{
  /// This method is deprecated (it was added for backward compatibility
  /// with vtkMRMLAnnotationRulerNode and will be removed in the future).
  /// Use SetLineStartPositionWorld/SetLineEndPositionWorld method instead.
  /// Set line start/end point in world coordinate system.
  /// Returns true if the operation is successful.
  bool SetPositionWorldCoordinates1(double newControl[3]);
  bool SetPositionWorldCoordinates2(double newControl[3]);
  //@}

protected:
  vtkMRMLMarkupsLineNode();
  ~vtkMRMLMarkupsLineNode() override;
  vtkMRMLMarkupsLineNode(const vtkMRMLMarkupsLineNode&);
  void operator=(const vtkMRMLMarkupsLineNode&);

  /// Calculates the handle to world matrix based on the current control points
  void UpdateInteractionHandleToWorldMatrix() override;
};

#endif
