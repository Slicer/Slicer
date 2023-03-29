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

#ifndef __vtkMRMLMarkupsNode_h
#define __vtkMRMLMarkupsNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"
#include "vtkCurveGenerator.h"
#include "vtkMRMLMeasurement.h"
#include "vtkMRMLSelectionNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"

// VTK includes
#include <vtkParallelTransportFrame.h>
#include <vtkPointLocator.h>
#include <vtkSmartPointer.h>
#include <vtkVector.h>

class vtkMatrix3x3;
class vtkMRMLUnitNode;

/// \brief Abstract base class to represent an interactive widget.
///
/// Markups nodes contains a list of points (ControlPoint).
/// Each markups node is defined by a certain number of control points:
/// N for fiducials (point lists) and curves, 2 for rulers, 3 for angles.
/// Each ControlPoint has a unique ID, position, orientation, .
/// an associated node id (set when the ControlPoint is placed to the node that was visible
/// at that position). Position of a control point may be undefined, because it is not placed
/// yet or because it cannot be placed (e.g., an anatomical landmark point is not visible),
/// in which cases the position and orientation values of the control point must be ignored.
///
/// Each ControlPoint can also be individually un/selected, un/locked, in/visible,
/// and have a label (short, shown in the viewers) and description (longer, shown in the GUI).
///
/// Each markups node is associated with a vtkSlicerMarkupsWidget, which is responsible for
/// displaying an interactive widget in each view. The representations are handled
/// by the VTKWidget (there is one widget for each markups node per view).
///
/// Visualization parameters for these nodes are controlled by the
/// vtkMRMLMarkupsDisplayNode class.
///
/// Coordinate systems used:
///   - Local: Local coordinates
///   - World: All parent transforms on node applied to local.
///
/// \sa vtkMRMLMarkupsDisplayNode
/// \ingroup Slicer_QtModules_Markups

class vtkAlgorithmOutput;
class vtkCollection;
class vtkDataArray;
class vtkGeneralTransform;
class vtkMatrix4x4;
class vtkMRMLMarkupsDisplayNode;
class vtkPolyData;
class vtkStringArray;
class vtkTransformPolyDataFilter;

class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsNode : public vtkMRMLDisplayableNode
{
  /// Make the storage node a friend so that ReadDataInternal can set the ControlPoint ids
  friend class vtkMRMLMarkupsStorageNode;
  friend class vtkMRMLMarkupsFiducialStorageNode;
  friend class vtkMRMLMarkupsJsonStorageNode;

public:

  struct ControlPoint
    {
    ControlPoint()
      {
      // position is 0
      this->Position[0] = 0.0;
      this->Position[1] = 0.0;
      this->Position[2] = 0.0;

      this->OrientationMatrix[0] = 1.0;
      this->OrientationMatrix[1] = 0.0;
      this->OrientationMatrix[2] = 0.0;

      this->OrientationMatrix[3] = 0.0;
      this->OrientationMatrix[4] = 1.0;
      this->OrientationMatrix[5] = 0.0;

      this->OrientationMatrix[6] = 0.0;
      this->OrientationMatrix[7] = 0.0;
      this->OrientationMatrix[8] = 1.0;

      Selected = true;
      Locked = false;
      Visibility = true;
      PositionStatus = PositionUndefined;
      AutoCreated = false;
      }

    // Positions and orientation in local coordinates.
    // If transform is applied to the markup node then world
    // coordinates may be obtained by applying "to world" transform.
    double Position[3];
    // Orientation of control point in 3x3  matrix.
    // x axis (0, 3, 6), y axis (1, 4, 7), and z axis (2, 5, 8).
    double OrientationMatrix[9];

    std::string ID;
    std::string Label;
    std::string Description;
    std::string AssociatedNodeID;

    bool Selected;
    bool Locked;
    bool Visibility;
    int PositionStatus;
    bool AutoCreated;
    };

  typedef std::vector<ControlPoint*> ControlPointsListType;

  vtkTypeMacro(vtkMRMLMarkupsNode,vtkMRMLDisplayableNode);

  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual const char* GetIcon() {return ":/Icons/MarkupsGeneric.png";}
  virtual const char* GetAddIcon() {return ":/Icons/MarkupsGenericMouseModePlace.png";}
  virtual const char* GetPlaceAddIcon() {return ":/Icons/MarkupsGenericMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  /// Get markup type internal name. This type name is the same regardless of the
  /// chosen application language and should not be displayed to end users.
  virtual const char* GetMarkupType() {return "Markup";};

  /// Get markup short name
  virtual const char* GetDefaultNodeNamePrefix() {return "M";};

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Write this node's information to a vector of strings for passing to a CLI,
  /// precede each datum with the prefix if not an empty string
  /// coordinateSystemFlag = vtkMRMLStorageNode::CoordinateSystemRAS or vtkMRMLStorageNode::CoordinateSystemLPS
  /// multipleFlag = 1 for the whole list, 1 for the first selected control point
  void WriteCLI(std::vector<std::string>& commandLine,
                        std::string prefix, int coordinateSystem = vtkMRMLStorageNode::CoordinateSystemRAS,
                        int multipleFlag = 1) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsNode);

  /// Alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  /// \brief End modifying the node.
  /// Updates pending measurements and other updates.
  /// \sa StartModify()
  int EndModify(int previousDisableModifiedEventState) override;

  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  /// Create and observe default display node(s)
  void CreateDefaultDisplayNodes() override;

  ///@{
  /**
   * Get measurement data, such as length, angle, diameter, cross-section area.
   * Add/remove/clear measurements.
   */
  int GetNumberOfMeasurements();
  int GetNumberOfEnabledMeasurements();
  int GetNumberOfEnabledAndDefinedMeasurements();
  vtkMRMLMeasurement* GetNthMeasurement(int id);
  vtkMRMLMeasurement* GetMeasurement(const char* name);
  void AddMeasurement(vtkMRMLMeasurement* measurement);
  std::string GetMeasurementNameFromIndex(int id);
  int GetMeasurementIndexFromName(const char* name);
  void RemoveMeasurement(const char* name);
  void RemoveNthMeasurement(int id);
  void EnableAllMeasurements();
  void EnableMeasurement(const char* name);
  void DisableAllMeasurements();
  void DisableMeasurement(const char* name);
  void ClearValueForAllMeasurements();
  ///@}

  ///@{
  /**
   * Utility method to override the list DefaultMeasurements which in default is empty.
   * DefaultMeasurements values have to be the measurement names.
   * The list will used by the MarkupsMeasurementPanel to set which measurements will
   * be enabled when a markup is added.
   *
   * Python pseudocode
   * curve = slicer.vtkMRMLMarkupsClosedCurveNode()
   * defaultMeasurements = vtk.vtkStringArray()
   * defaultMeasurements.SetNumberOfTuples(2)
   * defaultMeasurements.SetValue(0, "length")
   * defaultMeasurements.SetValue(1, "curvature max")
   * curve.SetDefaultMeasurements(defaultMeasurements)
   * slicer.mrmlScene.AddDefaultNode(curve)
   *
   */
  void SetDefaultMeasurements(vtkStringArray *defaultMeasurements);
  vtkStringArray* GetDefaultMeasurements();
  ///@}

  /// Update all measurements.
  /// It should not be necessary for users to call this method.
  void UpdateAllMeasurements();

  ///@{
  /**
   * Set measurement data, such as length, angle, diameter, cross-section area.
   *
   * IMPORTANT: These functions should not be used any more. Setting measurements now
   * happens in the \sa UpdateMeasurementsInternal functions by calling Compute on the
   * measurements. Similarly, RemoveAllMeasurements should not be called to clear the values,
   * because the measurements are not set externally but added internally in the constructors
   * and only updated (Compute called) as needed.
   */
  void SetNthMeasurement(int id, vtkMRMLMeasurement* measurement);
  void SetNthMeasurement(int id, const std::string& name, double value, const std::string &units,
    std::string printFormat = "", const std::string description = "",
    vtkCodedEntry* quantityCode = nullptr, vtkCodedEntry* derivationCode = nullptr,
    vtkCodedEntry* unitsCode = nullptr, vtkCodedEntry* methodCode = nullptr);
  void RemoveAllMeasurements();
  ///@}

  /// Invoke events when control points change, passing the control point index if applicable.
  ///
  /// Event data for Point* events: Event callData is control point index address (int*). If the pointer is nullptr
  /// then one or more points are added/removed/modified.
  ///
  /// Note: the current active node (control point or line) information are stored in the display node.
  ///
  enum
  {
    LockModifiedEvent = 19000,                  ///< Markups node lock status is changed. Modified event is invoked, too.
    LabelFormatModifiedEvent,                   ///< Markups node label format changed.
    MeasurementsCollectionModifiedEvent,        ///< Markups measurements collection changed.
    PointAddedEvent,                            ///< New control point(s) added. Modified event is NOT invoked.
    PointRemovedEvent,                          ///< Control point(s) deleted. Modified event is NOT invoked.
    PointPositionDefinedEvent,                  ///< Point was not defined (undefined, preview position status,
                                                ///  or non-existent point) before but now it is defined.
    PointPositionUndefinedEvent,                ///< Point position was defined and now it is not defined anymore (point deleted or position is not defined).
    PointPositionMissingEvent,                  ///< Point was not not missing before and now it is missing.
    PointPositionNonMissingEvent,               ///< Point missing before and now it is not missing.
    PointModifiedEvent,                         ///< Existing control point(s) modified, added, or removed. Modified event is NOT invoked.
    PointStartInteractionEvent,                 ///< When starting interacting with a control point.
    PointEndInteractionEvent,                   ///< When an interaction with a control point process finishes.
    CenterOfRotationModifiedEvent,              ///< When position of the center of rotation is changed (used for example for rotating closed curves).
    FixedNumberOfControlPointsModifiedEvent,    ///< When fixed number of points set/unset.
    PointAboutToBeRemovedEvent,                 ///< Point is about to be deleted. Thus it is alive when event is called.
  };

  /// Placement status of a control point.
  enum
  {
    PositionUndefined,      ///< PositionUndefined: position is undefined (coordinate values must not be used).
    PositionPreview,        ///< PositionPreview: new point is being placed, position is tentative.
    PositionDefined,        ///< PositionDefined: position is specified.
    PositionMissing,        ///< PositionMissing: point is undefined and placement should not be attempted.
    PositionStatus_Last     ///< PositionStatus_Last: indicates the end of the enum (int first = 0, int last = PositionStatus_Last)
  };

  static const char* GetPositionStatusAsString(int id);
  static int GetPositionStatusFromString(const char* name);

  /// Clear out the node of all control points
  virtual void RemoveAllControlPoints();
  virtual void UnsetAllControlPoints();

  ///@{
  /// Get/Set the Locked property on the markupNode/list of control points
  /// If set to 1 then parameters should not be changed, and dragging the
  /// control points is disabled in 2d and 3d.
  /// Overrides the Locked flag on individual control points in that when the node is
  /// set to be locked, all the control points in the list are locked. When the node
  /// is unlocked, use the locked flag on the individual control points to determine
  /// their locked state.
  vtkGetMacro(Locked, int);
  void SetLocked(int locked);
  vtkBooleanMacro(Locked, int);
  ///@}

  /// Return a cast display node, returns null if none
  vtkMRMLMarkupsDisplayNode *GetMarkupsDisplayNode();

  /// Return true if n is a valid control point, false otherwise.
  bool ControlPointExists(int n);

  /// Return the number of control points that are stored in this node
  int GetNumberOfControlPoints();
  /// Return the number of control points that are already placed (not being previewed or undefined).
  int GetNumberOfDefinedControlPoints(bool includePreview=false);
  /// Return the number of control points that have not been placed (not being previewed or skipped).
  int GetNumberOfUndefinedControlPoints(bool includePreview = false);

  /// Return a pointer to the Nth control point stored in this node, null if n is out of bounds
  ControlPoint* GetNthControlPoint(int n);
  /// Return a pointer to the std::vector of control points stored in this node
  std::vector<ControlPoint*>* GetControlPoints();

  ///@{
  /// Add n control points.
  /// If point is specified then all control point positions will be initialized to that position,
  /// otherwise control point positions are initialized to (0,0,0).
  /// If requested number of points would result more points than the maximum allowed number of points
  /// then no points are added at all.
  /// Return index of the last placed control point, -1 on failure.
  int AddNControlPoints(int n, std::string label = std::string(), vtkVector3d* point = nullptr);
  int AddNControlPoints(int n, std::string label, double point[3]);
  ///@}

  /// Set all control point positions from a point list.
  /// If points is nullptr then all control points are removed.
  /// New control points are added if needed.
  /// Existing control points are updated with the new positions.
  /// Any extra existing control points are removed.
  void SetControlPointPositionsWorld(vtkPoints* points);

  /// Get a copy of all control point positions in world coordinate system
  void GetControlPointPositionsWorld(vtkPoints* points);

  ///@{
  /// Add a new control point, returning the point index, -1 on failure.
  int AddControlPoint(vtkVector3d point, std::string label = std::string());
  int AddControlPoint(double point[3], std::string label = std::string());
  int AddControlPoint(double x, double y, double z, std::string label = std::string());
  ///@}

  /// Add a controlPoint to the end of the list. Return index
  /// of new controlPoint, -1 on failure.
  /// Markups node takes over ownership of the pointer (markups node will delete it)
  /// \param autoLabel: if enabled (by default it is) then empty point label will be
  /// replaced with automatically generated label.
  int AddControlPoint(ControlPoint *controlPoint, bool autoLabel=true);

  ///@{
  /// Add a new control point, defined in the world coordinate system.
  /// Return index of point index, -1 on failure.
  int AddControlPointWorld(vtkVector3d point, std::string label = std::string());
  int AddControlPointWorld(double point[3], std::string label = std::string());
  int AddControlPointWorld(double x, double y, double z, std::string label = std::string());
  ///@}

  ///@{
  /// Insert a control point in this list at targetIndex.
  /// If targetIndex is < 0, insert at the start of the list.
  /// If targetIndex is > list size - 1, append to end of list.
  /// If the insertion is successful, ownership of the controlPoint
  /// is transferred to the markups node.
  /// Returns true on success, false on failure.
  bool InsertControlPoint(ControlPoint* controlPoint, int targetIndex);
  bool InsertControlPoint(int n, vtkVector3d point, std::string label = std::string());
  bool InsertControlPoint(int n, double point[3], std::string label = std::string());
  ///@}

  ///@{
  //Add and insert control point at index, defined in the world coordinate system.
  //\sa InsertControlPoint
  bool InsertControlPointWorld(int n, vtkVector3d pointWorld, std::string label = std::string());
  bool InsertControlPointWorld(int n, double pointWorld[3], std::string label = std::string());
  ///@}

  /// Remove Nth Control Point
  void RemoveNthControlPoint(int pointIndex);

  /// Swap two control points (position data and all other properties).
  void SwapControlPoints(int m1, int m2);

  ///@{
  /// Get/Set control point auto-created status. Set to true if point was generated automatically
  bool GetNthControlPointAutoCreated(int n);
  void SetNthControlPointAutoCreated(int n, bool flag);
  ///@}

  ///@{
  /// Get the position of the Nth control point
  /// setting the elements of point
  void GetNthControlPointPosition(int pointIndex, double point[3]);
  double* GetNthControlPointPosition(int pointIndex) VTK_SIZEHINT(3);
  ///@}

  /// Get the position of the Nth control point
  /// returning it as a vtkVector3d, return (0,0,0) if not found
  // Note: this method is not redundant because GetNthControlPointPosition returns a double*
  // (as it is safe to do so) and so the method that returns a vtkVector3d cannot have the same name.
  vtkVector3d GetNthControlPointPositionVector(int pointIndex);

  ///@{
  /// Get the position of the Nth control point in World coordinate system
  /// Returns 0 on failure, 1 on success.
  int GetNthControlPointPositionWorld(int pointIndex, double worldxyz[3]);
  vtkVector3d GetNthControlPointPositionWorld(int pointIndex);
  ///@}


    ///@{
  /// Set of the Nth control point position from coordinates
  void SetNthControlPointPosition(const int pointIndex, const double x, const double y, const double z, int positionStatus = PositionDefined);
  void SetNthControlPointPosition(const int pointIndex, const double position[3], int positionStatus = PositionDefined);
  ///@}

  ///@{
  /// Set of the Nth control point position using World coordinate system
  /// Calls SetNthControlPointPosition after transforming the passed in coordinate
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionWorld(const int pointIndex, const double x, const double y, const double z, int positionStatus = PositionDefined);
  void SetNthControlPointPositionWorld(const int pointIndex, const double position[3], int positionStatus = PositionDefined);
  ///@}

  /// Set of the Nth control point position and orientation from an array using World coordinate system.
  /// Orientation: x (0, 3, 6), y (1, 4, 7), z (2, 5, 8)
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionOrientationWorld(const int pointIndex,
    const double pos[3], const double orientationMatrix[9], const char* associatedNodeID, int positionStatus = PositionDefined);

  ///@{
  /// Set the orientation for the Nth control point from a WXYZ orientation.
  /// The value W is in degrees.
  void SetNthControlPointOrientation(int n, double w, double x, double y, double z);
  void SetNthControlPointOrientation(int n, const double wxyz[4]);
  ///@}

  /// Get the WXYZ orientation for the Nth control point
  /// The value W is in degrees.
  void GetNthControlPointOrientation(int n, double orientationWXYZ[4]);

  ///@{
  /// Get/Set orientation as 9 values: x, y, and z axis directions, respectively:
  /// x (0, 3, 6), y (1, 4, 7), z (2, 5, 8)
  double* GetNthControlPointOrientationMatrix(int n) VTK_SIZEHINT(9);
  void SetNthControlPointOrientationMatrix(int n, double orientationMatrix[9]);
  ///@}

  ///@{
  /// Get/Set orientation as a vtkMatrix3x3.
  void GetNthControlPointOrientationMatrix(int n, vtkMatrix3x3* matrix);
  void SetNthControlPointOrientationMatrix(int n, vtkMatrix3x3* matrix);
  ///@}

  ///@{
  /// Get/Set orientation in world coordinate system as 9 values: x, y, and z axis directions, respectively:
  /// x (0, 3, 6), y (1, 4, 7), z (2, 5, 8)
  void GetNthControlPointOrientationMatrixWorld(int n, double orientationMatrix[9]);
  vtkVector<double, 9> GetNthControlPointOrientationMatrixWorld(int n);
  void SetNthControlPointOrientationMatrixWorld(int n, const double orientationMatrix[9]);
  ///@}

  ///@{
  /// Get/Set orientation in world coordinate system as a vtkMatrix3x3.
  void GetNthControlPointOrientationMatrixWorld(int n, vtkMatrix3x3* matrix);
  void SetNthControlPointOrientationMatrixWorld(int n, vtkMatrix3x3* matrix);
  ///@}

  /// Get control point position status (PositionUndefined, PositionPreview, PositionDefined)
  int GetNthControlPointPositionStatus(int pointIndex);

  /// Get index of N-th control point of the specified status.
  /// (for example, get index of N-th placed control point).
  /// pointIndex is zero-based, so to get index of the first control point, use pointIndex=0.
  /// Return -1 if no such control point is found.
  int GetNthControlPointIndexByPositionStatus(int pointIndex, int positionStatus);

  /// Set control point status to undefined.
  void UnsetNthControlPointPosition(int pointIndex);

  /// Set control point status to ignored.
  void SetNthControlPointPositionMissing(int pointIndex);

  /// Set control point status to preview
  void ResetNthControlPointPosition(int n);

  /// Set control point status to defined and return to the previous position
  void RestoreNthControlPointPosition(int n);

  /// Get the center position of the transformations, such as rotation and scaling.
  /// Return (0,0,0) if undefined.
  /// \sa GetCenterOfRotationWorld
  vtkVector3d GetCenterOfRotation();

  /// Get the center position of the transformations, such as rotation and scaling.
  /// Returns false if center position is undefined.
  /// \sa GetCenterOfRotationWorld
  bool GetCenterOfRotation(double point[3]);

  /// Get the center position of the transformations, such as rotation and scaling,
  /// in World coordinate system.
  /// Returns true on success.
  // \sa GetCenterOfRotation
  bool GetCenterOfRotationWorld(double worldxyz[3]);

  ///@{
  /// Set the center position of the transformations, such as rotation and scaling.
  /// \sa SetCenterOfRotationWorld
  void SetCenterOfRotation(const double x, const double y, const double z);
  void SetCenterOfRotation(const double position[3]);
  ///@}

  ///@{
  /// Set the center position of the transformations, such as rotation and scaling.
  /// \sa SetCenterOfRotation
  void SetCenterOfRotationWorld(const double x, const double y, const double z);
  void SetCenterOfRotationWorld(const double positionWorld[3]);
  ///@}

  ///@{
  /// Get/Set normal direction (orientation of z axis) in local coordinate system.
  void GetNthControlPointNormal(int n, double normal[3]);
  vtkVector3d GetNthControlPointNormal(int n);
  ///@}

  ///@{
  /// Get normal direction (orientation of z axis) in world coordinate system.
  void GetNthControlPointNormalWorld(int n, double normalWorld[3]);
  vtkVector3d GetNthControlPointNormalWorld(int n);
  ///@}

  ///@{
  /// Get/Set the associated node id for the Nth control point
  std::string GetNthControlPointAssociatedNodeID(int n = 0);
  void SetNthControlPointAssociatedNodeID(int n, std::string id);
  ///@}

  /// Get the id for the Nth control point
  std::string GetNthControlPointID(int n);

  /// Get the Nth control point index based on it's ID
  int GetNthControlPointIndexByID(const char* controlPointID);
  /// Get the Nth control point based on it's ID
  ControlPoint* GetNthControlPointByID(const char* controlPointID);

  /// Get the Selected flag on the Nth control point,
  /// returns false if control point doesn't exist
  bool GetNthControlPointSelected(int n = 0);
  /// Set the Selected flag on the Nth control point
  /// \sa vtkMRMLNode::SetSelected
  void SetNthControlPointSelected(int n, bool flag);

  /// Get the Lock flag on the Nth control point,
  /// returns false if control point doesn't exist
  bool GetNthControlPointLocked(int n = 0);
  /// Set Locked property on Nth control point. If locked is set to
  /// true on the node/list as a whole, the Nth control point locked flag is used to
  /// determine if it is locked. If the locked flag is set to false on the node
  /// as a whole, all control point are locked but keep this value for when the
  /// list as a whole is turned unlocked.
  /// \sa vtMRMLMarkupsNode::SetLocked
  void SetNthControlPointLocked(int n, bool flag);

  /// Get the Visibility flag on the Nth control point,
  /// returns false if control point doesn't exist
  bool GetNthControlPointVisibility(int n = 0);

  /// Get point visibility and visibility of point position status
  /// returns true if point visibility is enabled and the position is defined or
  /// in preview mode.
  bool GetNthControlPointPositionVisibility(int n = 0);

  /// Set Visibility property on Nth control point. If the visibility is set to
  /// true on the node/list as a whole, the Nth control point visibility is used to
  /// determine if it is visible. If the visibility is set to false on the node
  /// as a whole, all control points are hidden but keep this value for when the
  /// list as a whole is turned visible.
  /// \sa vtkMRMLDisplayableNode::SetDisplayVisibility
  /// \sa vtkMRMLDisplayNode::SetVisibility
  void SetNthControlPointVisibility(int n, bool flag);

  ///@{
  /// Get/Set the Label on the Nth control point.
  std::string GetNthControlPointLabel(int n = 0);
  void SetNthControlPointLabel(int n, std::string label);
  ///@}

  /// Get all control point labels at once.
  void GetControlPointLabels(vtkStringArray* labels);

  ///@{
  /// Get/Set the Description flag on the Nth control point,
  /// returns false if control point doesn't exist
  std::string GetNthControlPointDescription(int n = 0);
  void SetNthControlPointDescription(int n, std::string description);
  ///@}

  /// Returns true since can apply non linear transforms
  /// \sa ApplyTransform
  bool CanApplyNonLinearTransforms()const override;

  /// Apply the passed transformation to all of the control points
  /// \sa CanApplyNonLinearTransforms
  void ApplyTransform(vtkAbstractTransform* transform) override;

  ///@{
  /// Get/Set the ControlPointLabelFormat string that defines the control point names.
  /// In standard printf notation, with the addition of %N being replaced
  /// by the list name.
  /// %d will resolve to the highest not yet used list index integer.
  /// Character strings will otherwise pass through
  /// Defaults to %N-%d which will yield control point names of Name-0, Name-1, Name-2.
  /// If format string is changed then LabelFormatModifedEvent event is invoked.
  std::string GetControlPointLabelFormat();
  void SetControlPointLabelFormat(std::string format);
  ///@}

  /// If the ControlPointLabelFormat contains the string %N, return a string
  /// in which that has been replaced with the list name. If the list name is
  /// nullptr, replace it with an empty string. If the ControlPointLabelFormat doesn't
  /// contain %N, return ControlPointLabelFormat
  std::string ReplaceListNameInControlPointLabelFormat();

  /// Reimplemented to take into account the modified time of the markups
  /// Returns true if the node (default behavior) or the markups are modified
  /// since read/written.
  /// Note: The MTime of the markups node is used to know if it has been modified.
  /// So if you invoke class specific modified events without calling Modified() on the
  /// markups, GetModifiedSinceRead() won't return true.
  /// \sa vtkMRMLStorableNode::GetModifiedSinceRead()
  bool GetModifiedSinceRead() override;

  /// Reset the id of the Nth control point according to the local policy
  /// Called after an already initialised markup has been added to the
  /// scene. Returns false if n out of bounds, true on success.
  bool ResetNthControlPointID(int n);

  ///@{
  /// Get/Set locking of control point count.
  /// If number of control points is fixed then points cannot be added or removed
  /// only their position can be set/unset.
  bool GetFixedNumberOfControlPoints();
  void SetFixedNumberOfControlPoints(bool fixed);
  ///@}

  /// Return the number of control points that are required for defining this widget.
  /// Interaction mode remains in "place" mode until this number is reached.
  /// If the number is set to 0 then no it means there is no preference (this is the default value).
  vtkGetMacro(RequiredNumberOfControlPoints, int);

  /// Maximum number of control points limits the number of markups allowed in the node.
  /// If maximum number of control points is set to -1 then no it means there
  /// is no limit (this is the default value).
  /// The value is an indication to the user interface and does not affect
  /// prevent adding markups to a node programmatically.
  /// If value is set to lower value than the number of markups in the node, then
  /// existing markups are not deleted.
  /// 2 for line, and 3 for angle Markups
  vtkGetMacro(MaximumNumberOfControlPoints, int);

  ///@{
  /// Helper methods for converting orientation between WXYZ quaternion and 3x3 matrix representation.
  /// WXYZ: W rotation angle in degrees, XYZ is rotation axis.
  static void ConvertOrientationMatrixToWXYZ(const double orientationMatrix[9], double orientationWXYZ[4]);
  static void ConvertOrientationWXYZToMatrix(const double orientationWXYZ[4], double orientationMatrix[9]);
  ///@}

  ///@{
  /// Get markup control points.
  virtual vtkPoints* GetCurvePoints();
  virtual vtkPoints* GetCurvePointsWorld();
  virtual vtkPolyData* GetCurve();
  virtual vtkPolyData* GetCurveWorld();
  virtual vtkAlgorithmOutput* GetCurveWorldConnection();
  ///@}

  /// Converts curve point index to control point index.
  int GetControlPointIndexFromInterpolatedPointIndex(vtkIdType interpolatedPointIndex);

  /// Returns true if the curve generator creates a closed curve.
  vtkGetMacro(CurveClosed, bool);

  /// The internal instance of the curve generator to allow
  /// use of the curve for other computations.
  /// Any custom overrides of the interpolation modes are not persisted in MRML.
  vtkCurveGenerator* GetCurveGenerator() { return this->CurveGenerator.GetPointer(); };

  /// The internal instance of the curve coordinate system generator to allow
  /// use of the coordinate systems computed for curve point for other computations.
  vtkParallelTransportFrame* GetCurveCoordinateSystemGeneratorWorld() { return this->CurveCoordinateSystemGeneratorWorld.GetPointer(); };

  void GetRASBounds(double bounds[6]) override;
  void GetBounds(double bounds[6]) override;

  /// Get the index of the closest control point to the world coordinates.
  /// If visibleOnly is set to true then index of the closest visible control point will be returned.
  int GetClosestControlPointIndexToPositionWorld(double pos[3], bool visibleOnly=false);

  /// 4x4 matrix detailing the orientation and position in world coordinates of the interaction handles.
  virtual vtkMatrix4x4* GetInteractionHandleToWorldMatrix();

  /// Get displayable string of the properties label (containing name, measurements, etc.) that
  /// identifies the node and provides basic information.
  virtual std::string GetPropertiesLabelText();

  /// Utility function to get unit node from scene
  vtkMRMLUnitNode* GetUnitNode(const char* quantity);

  /// Update the AssignAttribute filter based on its ActiveScalarName and its ActiveAttributeLocation
  /// To be re-implemented in subclasses
  virtual void UpdateAssignedAttribute() {};

  /// Returns true if no additional control points can be added to this node.
  virtual bool GetControlPointPlacementComplete();

  ///@{
  /// Set the index of the control point that will be placed next.
  ///
  /// Currently, this property is not stored persistently in the scene and modifying it does not trigger
  /// a node modification event, because it is considered to be a temporary value. For example, it would
  /// not be desirable to store this value for each item in a markups node sequence, or include it in
  /// undo/redo.
  int GetControlPointPlacementStartIndex();
  void SetControlPointPlacementStartIndex(int);
  ///@}

  ///@{
  /// This value is used for generating number in the control point's name when a new point is added.
  /// The value is not decremented when a control point is deleted to keep the control point names unique.
  /// The value is reset to 0 when \sa RemoveAllControlPoints is called.
  vtkGetMacro(LastUsedControlPointNumber, int);
  vtkSetMacro(LastUsedControlPointNumber, int);
  ///@}

  //-----------------------------------------------------------
  // All public methods below are deprecated
  //
  // These methods are either deprecated because they use old terms (markup instead of control point),
  // or include "array", "vector", "pointer" in the name (it is redundant, as input arguments can be
  // deduced from the type; and return type for vectors is always vtkVectorNd).
  //

  /// \deprecated Use RemoveAllControlPoints instead.
  void RemoveAllMarkups()
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::RemoveAllMarkups method is deprecated, please use RemoveAllControlPoints instead");
    this->RemoveAllControlPoints();
  };

  /// \deprecated Use ControlPointExists instead.
  bool MarkupExists(int n)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::MarkupExists method is deprecated, please use ControlPointExists instead");
    return this->ControlPointExists(n);
  };

  /// \deprecated Use GetNumberOfControlPoints() instead.
  int GetNumberOfMarkups()
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetNumberOfMarkups method is deprecated, please use GetNumberOfControlPoints instead");
    return this->GetNumberOfControlPoints();
  };
  /// \deprecated Use GetNumberOfControlPoints() instead.
  int GetNumberOfPointsInNthMarkup(int)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetNumberOfPointsInNthMarkup method is deprecated, please use GetNumberOfControlPoints instead");
    return this->GetNumberOfControlPoints();
  };

  /// \deprecated Use GetNthControlPointPositionVector() method instead.
  vtkVector3d GetMarkupPointVector(int markupIndex, int)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetMarkupPointVector method is deprecated, please use GetNthControlPointPositionVector instead");
    return this->GetNthControlPointPositionVector(markupIndex);
  };

  /// \deprecated Use GetNthControlPointPosition method instead.
  void GetMarkupPoint(int markupIndex, int pointIndex, double point[3]);

  /// \deprecated Use RemoveNthControlPoint instead.
  void RemoveMarkup(int pointIndex)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::RemoveMarkup method is deprecated, please use RemoveNthControlPoint instead");
    this->RemoveNthControlPoint(pointIndex);
  };

  /// Set of the Nth control point position from a pointer to an array
  /// \deprecated Use SetNthControlPointPosition instead.
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionFromPointer(const int pointIndex, const double* pos);

  /// Set of the Nth control point position from an array
  /// \deprecated Use SetNthControlPointPosition instead.
  void SetNthControlPointPositionFromArray(const int pointIndex, const double pos[3], int positionStatus = PositionDefined)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::SetNthControlPointPositionFromArray method is deprecated, please use SetNthControlPointPosition instead");
    this->SetNthControlPointPosition(pointIndex, pos[0], pos[1], pos[2], positionStatus);
  }

  /// Set of the Nth control point position from an array using World coordinate system
  /// \deprecated Use SetNthControlPointPositionWorld instead.
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionWorldFromArray(const int pointIndex, const double pos[3], int positionStatus = PositionDefined);

  /// Set of the Nth control point position and orientation from an array using World coordinate system.
  /// \deprecated Use SetNthControlPointPositionOrientationWorld instead.
  /// Orientation: x (0, 3, 6), y (1, 4, 7), z (2, 5, 8)
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionOrientationWorldFromArray(const int pointIndex,
    const double positionWorld[3], const double orientationMatrix_World[9],
    const char* associatedNodeID, int positionStatus = PositionDefined)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::SetNthControlPointPositionOrientationWorldFromArray method is deprecated,"
      << " please use SetNthControlPointPositionOrientationWorld instead");
    this->SetNthControlPointPositionOrientationWorld(
      pointIndex, positionWorld, orientationMatrix_World, associatedNodeID, positionStatus);
  }

  /// Get the WXYZ orientation for the Nth control point
  /// returning it as a vtkVector4d, return (0,0,0,0) if not found.
  /// Note that vtkVector4d stores components in the order XYZW
  /// (in all other methods we get/set components in WXYZ order).
  /// \deprecated Use GetNthControlPointOrientation instead - with a different XYZW/WXYZ component order!
  vtkVector4d GetNthControlPointOrientationVector(int pointIndex);

  /// Get the position of the center.
  /// \deprecated Use GetCenterOfRotation instead.
  /// Return (0,0,0) if center position is undefined.
  vtkVector3d GetCenterOfRotationVector()
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetCenterOfRotationVector method is deprecated, please use GetCenterOfRotation instead");
    return this->GetCenterOfRotation();
  }

  /// Set the center position from a pointer to an array
  /// \deprecated Use SetCenterOfRotation instead.
  /// \sa SetCenterOfRotation
  void SetCenterOfRotationFromPointer(const double* pos);
  /// Set the center position position from an array
  /// \deprecated Use SetCenterOfRotation instead.
  /// \sa SetCenterOfRotation
  void SetCenterOfRotationFromArray(const double pos[3])
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::SetCenterOfRotationFromArray method is deprecated, please use SetCenterOfRotation instead");
    this->SetCenterOfRotation(pos[0], pos[1], pos[2]);
  }

  ///@{
  /// Set the orientation for the Nth control point from a WXYZ orientation.
  /// The value W is in degrees.
  /// \deprecated Use SetNthControlPointOrientation instead.
  void SetNthControlPointOrientationFromPointer(int n, const double* orientationWXYZ);
  void SetNthControlPointOrientationFromArray(int n, const double orientationWXYZ[4])
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::SetNthControlPointOrientationFromArray method is deprecated, please use SetNthControlPointOrientation instead");
    this->SetNthControlPointOrientation(n, orientationWXYZ[0], orientationWXYZ[1], orientationWXYZ[2], orientationWXYZ[3]);
  }
  ///@}

  /// \deprecated Use GetNthControlPointAssociatedNodeID instead.
  std::string GetNthMarkupAssociatedNodeID(int n = 0)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetNthMarkupAssociatedNodeID method is deprecated, please use GetNthControlPointAssociatedNodeID instead");
    return this->GetNthControlPointAssociatedNodeID(n);
  };
  /// \deprecated Use SetNthControlPointAssociatedNodeID instead.
  void SetNthMarkupAssociatedNodeID(int n, std::string id)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::SetNthMarkupAssociatedNodeID method is deprecated, please use SetNthControlPointAssociatedNodeID instead");
    this->SetNthControlPointAssociatedNodeID(n, id);
  };

  /// \deprecated Use GetNthControlPointID instead.
  std::string GetNthMarkupID(int n = 0)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetNthMarkupID method is deprecated, please use GetNthControlPointID instead");
    return this->GetNthControlPointID(n);
  };

  /// \deprecated Use GetNthControlPointLocked instead.
  bool GetNthMarkupLocked(int n = 0)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetNthMarkupLocked method is deprecated, please use GetNthControlPointLocked instead");
    return this->GetNthControlPointLocked(n);
  };
  /// \deprecated Use SetNthControlPointLocked instead.
  void SetNthMarkupLocked(int n, bool flag)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::SetNthMarkupLocked method is deprecated, please use SetNthControlPointLocked instead");
    this->SetNthControlPointLocked(n, flag);
  };

  /// \deprecated Use GetNthControlPointLabel instead.
  std::string GetNthMarkupLabel(int n = 0)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetNthMarkupLabel method is deprecated, please use GetNthControlPointLabel instead");
    return this->GetNthControlPointLabel(n);
  };
  /// \deprecated Use SetNthControlPointLabel instead.
  void SetNthMarkupLabel(int n, std::string label)
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::SetNthMarkupLabel method is deprecated, please use SetNthControlPointLabel instead");
    this->SetNthControlPointLabel(n, label);
  };
  /// \deprecated Use GetControlPointLabelFormat instead.
  std::string GetMarkupLabelFormat()
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::GetMarkupLabelFormat method is deprecated, please use GetControlPointLabelFormat instead");
    return this->GetControlPointLabelFormat();
  };
  /// \deprecated Use SetControlPointLabelFormat instead.
  void SetMarkupLabelFormat(std::string format)
  {
    // Not warning this at the moment as existing scene files will contain the markupLabelFormat attribute name and would warn on load
    // vtkWarningMacro("vtkMRMLMarkupsNode::SetMarkupLabelFormat method is deprecated, please use SetControlPointLabelFormat instead");
    return this->SetControlPointLabelFormat(format);
  };
  /// \deprecated Use ReplaceListNameInControlPointLabelFormat instead.
  std::string ReplaceListNameInMarkupLabelFormat()
  {
    vtkWarningMacro("vtkMRMLMarkupsNode::ReplaceListNameInMarkupLabelFormat method is deprecated, please use ReplaceListNameInControlPointLabelFormat instead");
    return this->ReplaceListNameInControlPointLabelFormat();
  };

protected:
  vtkMRMLMarkupsNode();
  ~vtkMRMLMarkupsNode() override;
  vtkMRMLMarkupsNode(const vtkMRMLMarkupsNode&);
  void operator=(const vtkMRMLMarkupsNode&);

  vtkSmartPointer<vtkStringArray> TextList;

  /// Set label of closest control point.
  /// If one control point is closest to multiple labels then all of them will be assigned to the same control point,
  /// separated with the provided "separator" string.
  /// Erase labels of all other control points.
  /// The method is protected because the API may still change.
  bool SetControlPointLabelsWorld(vtkStringArray* labels, vtkPoints* points, std::string separator = "");

  /// Utility function to be used internally for safe access to a control point's data.
  /// Return a pointer to the Nth control point stored in this node, nullptr if n is out of bounds
  /// If control point does not exist then an error is logged with the supplied failedMethodName.
  ControlPoint* GetNthControlPointCustomLog(int n, const char* failedMethodName);

  /// Set the id of the nth control point.
  /// The goal is to keep this ID unique, so it's
  /// managed by the markups node.
  void SetNthControlPointID(int n, std::string id);

  /// Generate a scene unique ID for a ControlPoint. If the scene is not set,
  /// returns a number based on the max number of ControlPoints that
  /// have been in this list
  std::string GenerateUniqueControlPointID();

  std::string GenerateControlPointLabel(int controlPointIndex);

  virtual void UpdateCurvePolyFromControlPoints();

  void OnTransformNodeReferenceChanged(vtkMRMLTransformNode* transformNode) override;

  /// Calculate the updated measurements.
  /// May be overridden in subclasses to compute special measurements (for example that apply on the curve polydata).
  virtual void UpdateMeasurementsInternal();

  /// Helper function to write measurements to node Description property.
  /// This is a short-term solution until measurements display is properly implemented.
  virtual void WriteMeasurementsToDescription();

  /// Calculates the handle to world matrix based on the current control points
  virtual void UpdateInteractionHandleToWorldMatrix();

  /// Transform the orientation matrix from node to world coordinates
  virtual void TransformOrientationMatrixFromNodeToWorld(
    const double position_Node[3], const double orientationMatrix_Node[9], double orientationMatrix_World[9]);

  /// Transform the orientation matrix from world to node coordinates
  virtual void TransformOrientationMatrixFromWorldToNode(
    const double position_World[3], const double orientationMatrix_World[9], double orientationMatrix_Node[9]);

  /// Used for limiting number of control points that may be placed.
  /// This is a soft limit at which automatic placement stops.
  int RequiredNumberOfControlPoints{0};
  /// Used for limiting number of control points that may be placed.
  /// This is a hard limit at which new control points cannot be added.
  int MaximumNumberOfControlPoints{-1};

  bool CurveClosed{false};

  /// Vector of control points
  ControlPointsListType ControlPoints;

  /// Converts curve control points to curve points.
  vtkSmartPointer<vtkCurveGenerator> CurveGenerator;

  /// Computes tangent and smooth normal for each curve point.
  /// It provides a fully specified coordinate system at each point of the curve,
  /// which is useful for image reslicing or defining camera pose.
  /// Curve is defined in the world coordinate system.
  vtkSmartPointer<vtkParallelTransportFrame> CurveCoordinateSystemGeneratorWorld;

  /// Stores control point positions in a polydata (in local coordinate system).
  /// Line cells connect all points into a curve.
  vtkSmartPointer<vtkPolyData> CurveInputPoly;

  vtkSmartPointer<vtkTransformPolyDataFilter> CurvePolyToWorldTransformer;
  vtkSmartPointer<vtkGeneralTransform> CurvePolyToWorldTransform;

  /// Point locator that allows quick finding of interpolated point in the world
  /// coordinate system (in transformed CurvePoly).
  vtkSmartPointer<vtkPointLocator> TransformedCurvePolyLocator;

  /// Locks all the points and GUI
  int Locked{0};

  /// Locks number of control points. If enabled then points cannot be added or removed.
  /// Point position can be unset instead of deleting the point.
  bool FixedNumberOfControlPoints{false};

  std::string ControlPointLabelFormat{"%N-%d"};

  /// Keep track of the number of markups that were added to the list, always
  /// incrementing, not decreasing when they're removed. Used to help create
  /// unique names and ids. Reset to 0 when \sa RemoveAllControlPoints called
  int LastUsedControlPointNumber{0};

  /// Index of the control point index that placement is started from (if no other point is requested specifically).
  int ControlPointPlacementStartIndex{-1};

  /// Markup centerpoint (in local coordinates).
  /// It may be used as rotation center or as a handle to grab the widget by.
  vtkVector3d CenterOfRotation;

  /// List of measurements stored for the markup
  vtkCollection* Measurements;

  /// List of default measurements for the MarkupsMeasurement panel
  vtkSmartPointer<vtkStringArray> DefaultMeasurements;

  std::string PropertiesLabelText;

  /// Transform that moves the xyz unit vectors and origin of the interaction handles to local coordinates
  vtkSmartPointer<vtkMatrix4x4> InteractionHandleToWorldMatrix;

  /// Flag set from SetControlPointPositionsWorld that pauses update of measurements until the update is complete.
  bool IsUpdatingPoints{false};

  friend class qSlicerMarkupsModuleWidget; // To directly access measurements
};

#endif
