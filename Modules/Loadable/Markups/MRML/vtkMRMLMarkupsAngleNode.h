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

#ifndef __vtkMRMLMarkupsAngleNode_h
#define __vtkMRMLMarkupsAngleNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

/// \brief MRML node to represent an angle markup
/// Angle Markups nodes contain three control points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
///
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsAngleNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsAngleNode *New();
  vtkTypeMacro(vtkMRMLMarkupsAngleNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsAngleMouseModePlace.png";}

  /// Angle calculation and display modes. Default is Unsigned
  /// \sa SetAngleModeToUnsigned()
  /// \sa SetAngleModeToOrientedSigned()
  /// \sa SetAngleModeToOrientedPositive()
  enum AngleModes
    {
    Unsigned,
    OrientedSigned,
    OrientedPositive,
    AngleMode_Last
    };

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsAngle";}

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsAngleNode);

  /// Get angle calculation mode, determining what angle to calculate from the given points.
  /// \return \sa AngleMode (Unsigned, OrientedSigned, OrientedPositive)
  vtkGetMacro(AngleMode, int);
  /// Set angle calculation mode, determining what angle to calculate from the given points.
  /// \param type \sa AngleMode (Unsigned, OrientedSigned, OrientedPositive)
  void SetAngleMode(int type);
  /// Get angle calculation mode as string from ID
  /// \sa AngleMode, \sa GetAngleMode, \sa GetAngleModeFromString
  /// \param id Identifier of the angle mode
  /// \return "Unsigned", "OrientedSigned", or "OrientedPositive"
  const char* GetAngleModeAsString(int id);
  /// Get angle calculation mode ID from string
  /// \sa AngleMode, \sa GetAngleMode, \sa GetAngleModeAsString
  /// \param name "Unsigned", "OrientedSigned", or "OrientedPositive"
  /// \return Identifier of the angle mode
  int GetAngleModeFromString(const char* name);
  /// Set angle mode to Unsigned.
  /// Calculate minimal angle, between [0..180)
  /// \sa AngleMode, \sa SetAngleMode
  void SetAngleModeToUnsigned();
  /// Set angle mode to OrientedSigned.
  /// Calculate oriented angle [-180..180), using OrientationRotationAxis in right-hand rule
  /// \sa AngleMode, \sa OrientationRotationAxis, \sa SetAngleMode
  void SetAngleModeToOrientedSigned();
  /// Set angle mode to OrientedPositive.
  /// Calculate oriented angle [0..360), using OrientationRotationAxis in right-hand rule
  /// \sa AngleMode, \sa OrientationRotationAxis, \sa SetAngleMode
  void SetAngleModeToOrientedPositive();

  /// Get rotation axis for calculating right-hand rule angles.
  /// \sa OrientationRotationAxis, \sa SetOrientationRotationAxis
  vtkGetVector3Macro(OrientationRotationAxis, double);
  /// Set rotation axis for calculating right-hand rule angles.
  /// \sa OrientationRotationAxis, \sa GetOrientationRotationAxis
  vtkSetVector3Macro(OrientationRotationAxis, double);

  /// Get current measured angle
  /// \return Angle in degrees, respecting \sa AngleMode
  double GetAngleDegrees();

protected:
  vtkMRMLMarkupsAngleNode();
  ~vtkMRMLMarkupsAngleNode() override;
  vtkMRMLMarkupsAngleNode(const vtkMRMLMarkupsAngleNode&);
  void operator=(const vtkMRMLMarkupsAngleNode&);

  /// Calculates the handle to world matrix based on the current control points
  void UpdateInteractionHandleToWorldMatrix() override;

  void UpdateMeasurementsInternal() override;

protected:
  /// Rotation axis for calculating right-hand rule angles.
  /// Right-hand rule: angle is positive if p2->p1 is rotated to p2->p3 by counter-clockwise rotation
  /// Applies if \sa AngleMode is oriented (OrientedSigned or OrientedPositive).
  double OrientationRotationAxis[3];

  /// Angle calculation mode, determining what angle to calculate from the given points.
  /// Available options:
  /// 0: Unsigned: Minimal angle, between [0..180)
  /// 1: OrientedSigned: [-180..180), using \sa OrientationRotationAxis in right-hand rule
  /// 2: OrientedPositive: [0..360), using OrientationRotationAxis in right-hand rule
  int AngleMode;
};

#endif
