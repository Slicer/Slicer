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

  /// Angle calculation and display modes. Default is Minimal
  /// \sa SetAngleMeasurementModeToMinimal()
  /// \sa SetAngleMeasurementModeToOrientedSigned()
  /// \sa SetAngleMeasurementModeToOrientedPositive()
  enum AngleMeasurementModes
    {
    Minimal,
    OrientedSigned,
    OrientedPositive,
    AngleMeasurementMode_Last
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
  /// \return \sa AngleMeasurementMode (Minimal, OrientedSigned, OrientedPositive)
  vtkGetMacro(AngleMeasurementMode, int);
  /// Set angle calculation mode, determining what angle to calculate from the given points.
  /// \param type \sa AngleMeasurementMode (Minimal, OrientedSigned, OrientedPositive)
  void SetAngleMeasurementMode(int type);
  /// Get angle calculation mode as string from ID
  /// \sa AngleMeasurementMode, \sa GetAngleMeasurementMode, \sa GetAngleMeasurementModeFromString
  /// \param id Identifier of the angle mode
  /// \return "Minimal", "OrientedSigned", or "OrientedPositive"
  const char* GetAngleMeasurementModeAsString(int id);
  /// Get angle calculation mode ID from string
  /// \sa AngleMeasurementMode, \sa GetAngleMeasurementMode, \sa GetAngleMeasurementModeAsString
  /// \param name "Minimal", "OrientedSigned", or "OrientedPositive"
  /// \return Identifier of the angle mode
  int GetAngleMeasurementModeFromString(const char* name);
  /// Set angle mode to Minimal.
  /// Calculate minimal angle, between [0..180)
  /// \sa AngleMeasurementMode, \sa SetAngleMeasurementMode
  void SetAngleMeasurementModeToMinimal();
  /// Set angle mode to OrientedSigned.
  /// Calculate oriented angle [-180..180), using OrientationRotationAxis in right-hand rule
  /// \sa AngleMeasurementMode, \sa OrientationRotationAxis, \sa SetAngleMeasurementMode
  void SetAngleMeasurementModeToOrientedSigned();
  /// Set angle mode to OrientedPositive.
  /// Calculate oriented angle [0..360), using OrientationRotationAxis in right-hand rule
  /// \sa AngleMeasurementMode, \sa OrientationRotationAxis, \sa SetAngleMeasurementMode
  void SetAngleMeasurementModeToOrientedPositive();

  /// Get rotation axis for calculating right-hand rule angles.
  /// \sa OrientationRotationAxis, \sa SetOrientationRotationAxis
  vtkGetVector3Macro(OrientationRotationAxis, double);
  /// Set rotation axis for calculating right-hand rule angles.
  /// \sa OrientationRotationAxis, \sa GetOrientationRotationAxis
  virtual void SetOrientationRotationAxis(double r, double a, double s);
  /// Set rotation axis for calculating right-hand rule angles.
  /// \sa OrientationRotationAxis, \sa GetOrientationRotationAxis
  virtual void SetOrientationRotationAxis(double ras[3]);

  /// Get current measured angle
  /// \return Angle in degrees, respecting \sa AngleMeasurementMode
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
  /// Applies if \sa AngleMeasurementMode is oriented (OrientedSigned or OrientedPositive).
  double OrientationRotationAxis[3];

  /// Angle calculation mode, determining what angle to calculate from the given points.
  /// Available options:
  /// - Minimal: Minimal angle, between [0..180)
  /// - OrientedSigned: [-180..180), using \sa OrientationRotationAxis in right-hand rule
  /// - OrientedPositive: [0..360), using OrientationRotationAxis in right-hand rule
  int AngleMeasurementMode;
};

#endif
