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

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"

// VTK includes
#include <vtkPointLocator.h>
#include <vtkSmartPointer.h>
#include <vtkVector.h>

class vtkFrenetSerretFrame;
class vtkMRMLUnitNode;

/// \brief MRML node to represent an interactive widget.
/// MarkupsNodes contains a list of points (ControlPoint).
/// Each markupNode is defined by a certain number of control points:
/// N for fiducials, 2 for rulers, 3 for angles and N for curves.
/// MarkupNodes are stricly connected with the VTKWidget representations. For each
/// MarkupNode there is a representation in each view. The representations are handled
/// by the VTKWidget (there is one widget for each MRMLMarkupsNode per view).
/// Visualization parameters for these nodes are controlled by the
/// vtkMRMLMarkupsDisplayNode class.
/// Each ControlPoint has a unique ID.
/// Each ControlPoint has an orientation defined by a by a 4 element vector:
/// [0] = the angle of rotation in degrees, [1,2,3] = the axis of rotation.
/// Default is 0.0, 0.0, 0.0, 1.0.
/// Each ControlPoint also has an associated node id, set when the ControlPoint
/// is placed on a data set to link the ControlPoint to the volume or model.
/// Each ControlPoint can also be individually un/selected, un/locked, in/visible,
/// and have a label (short, shown in the viewers) and description (longer,
/// shown in the GUI).
/// Coordinate systems used:
///   - Local: Local coordinates
///   - World: All parent transforms on node applied to local.
/// \sa vtkMRMLMarkupsDisplayNode
/// \ingroup Slicer_QtModules_Markups

class vtkAlgorithmOutput;
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
      }

    // Positions and orientation in local coordinates.
    // If transform is applied to the markup node then world
    // coordinates may be obtained by applying "to world" transform.
    double Position[3];
    // Orientation of x axis (0, 1, 2), y axis (3, 4, 5), and z axis (6, 7, 8).
    // This memory layout is chosen so that the normal (z axis) can be retrieved quickly.
    double OrientationMatrix[9];

    std::string ID;
    std::string Label;
    std::string Description;
    std::string AssociatedNodeID;

    bool Selected;
    bool Locked;
    bool Visibility;
    int PositionStatus;
    };

  typedef std::vector<ControlPoint*> ControlPointsListType;

  static vtkMRMLMarkupsNode *New();
  vtkTypeMacro(vtkMRMLMarkupsNode,vtkMRMLDisplayableNode);

  void PrintSelf(ostream& os, vtkIndent indent) override;

  virtual const char* GetIcon() {return "";};

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Markups";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Write this node's information to a vector of strings for passing to a CLI,
  /// precede each datum with the prefix if not an empty string
  /// coordinateSystemFlag = 0 for RAS, 1 for LPS
  /// multipleFlag = 1 for the whole list, 1 for the first selected control point
  void WriteCLI(std::vector<std::string>& commandLine,
                        std::string prefix, int coordinateSystem = 0,
                        int multipleFlag = 1) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsNode);

  /// Alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;


  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  /// Create and observe default display node(s)
  void CreateDefaultDisplayNodes() override;

  //@{
  /**
   * Get/set measurement data, such as length, angle, diameter, cross-section area.
   */
  int GetNumberOfMeasurements();
  vtkMRMLMeasurement* GetNthMeasurement(int id);
  void AddMeasurement(vtkMRMLMeasurement* measurement);
  void SetNthMeasurement(int id, vtkMRMLMeasurement* measurement);
  void SetNthMeasurement(int id, const std::string& name, double value, const std::string &units,
    std::string printFormat = "", const std::string description = "",
    vtkCodedEntry* quantityCode = nullptr, vtkCodedEntry* derivationCode = nullptr,
    vtkCodedEntry* unitsCode = nullptr, vtkCodedEntry* methodCode = nullptr);
  void RemoveNthMeasurement(int id);
  void RemoveAllMeasurements();
  //@}

  /// Invoke events when control points change, passing the control point index if applicable.
  /// - LockModifiedEvent: markups node lock status is changed. Modified event is invoked, too.
  /// - LabelFormatModifiedEvent: markups node label format changed. Modified event is invoked, too.
  /// - PointAddedEvent: new control point(s) added. Modified event is NOT invoked.
  /// - PointRemovedEvent: control point(s) deleted. Modified event is NOT invoked.
  /// - PointModifiedEvent: existing control point(s) modified, added, or removed. Modified event is NOT invoked.
  /// - PointStartInteractionEvent when starting interacting with a control point.
  /// - PointEndInteractionEvent when an interaction eith a control point process finishes.
  /// - CenterPointModifiedEvent when position of the centerpoint is changed (displayed for example for closed curves)
  ///
  /// Event data for Point* events: Event callData is control point index address (int*). If the pointer is nullptr
  /// then one or more points are added/removed/modified.
  ///
  /// Note: the current active node (control point or line) information are stored in the display node.
  ///
  enum
  {
    LockModifiedEvent = 19000,
    LabelFormatModifiedEvent,
    PointAddedEvent,
    PointRemovedEvent,
    PointPositionDefinedEvent, // point was not defined (undefined, preview position status, or non-existent point) before but now it is defined
    PointPositionUndefinedEvent, // point position was defined and now it is not defined anymore (point deleted or position is not defined)
    PointModifiedEvent,
    PointStartInteractionEvent,
    PointEndInteractionEvent,
    CenterPointModifiedEvent,
  };

  /// Placement status of a control point.
  /// - Undefined: position is undefined (coordinate values must not be used).
  /// - Preview: point is being placed, position is tentative.
  /// - Defined: position is specified.
  enum
  {
    PositionUndefined,
    PositionPreview,
    PositionDefined,
    PositionStatus_Last
  };

  static const char* GetPositionStatusAsString(int id);
  static int GetPositionStatusFromString(const char* name);

  /// Clear out the node of all control points
  virtual void RemoveAllControlPoints();

  /// \deprecated Use RemoveAllControlPoints instead.
  void RemoveAllMarkups() { this->RemoveAllControlPoints(); };

  /// Get the Locked property on the markupNode/list of control points.
  vtkGetMacro(Locked, int);
  /// Set the Locked property on the markupNode/list of control points
  /// If set to 1 then parameters should not be changed, and dragging the
  /// control points is disabled in 2d and 3d.
  /// Overrides the Locked flag on individual control points in that when the node is
  /// set to be locked, all the control points in the list are locked. When the node
  /// is unlocked, use the locked flag on the individual control points to determine
  /// their locked state.
  void SetLocked(int locked);
  /// Get/Set the Locked property on the markupNode.
  /// If set to 1 then parameters should not be changed
  vtkBooleanMacro(Locked, int);

  /// Return a cast display node, returns null if none
  vtkMRMLMarkupsDisplayNode *GetMarkupsDisplayNode();

  /// Return true if n is a valid control point, false otherwise.
  bool ControlPointExists(int n);

  /// Depreceated. Use ControlPointExists instead.
  bool MarkupExists(int n) { return this->ControlPointExists(n); }
  /// Return the number of control points that are stored in this node
  int GetNumberOfControlPoints();
  /// Return the number of control points that are already placed (not being previewed or undefined).
  int GetNumberOfDefinedControlPoints(bool includePreview=false);
  /// \deprecated Use GetNumberOfControlPoints() instead.
  int GetNumberOfMarkups() { return this->GetNumberOfControlPoints(); };
  /// \deprecated Use GetNumberOfControlPoints() instead.
  int GetNumberOfPointsInNthMarkup(int) { return this->GetNumberOfControlPoints(); };
  /// Return a pointer to the Nth control point stored in this node, null if n is out of bounds
  ControlPoint* GetNthControlPoint(int n);
  /// Return a pointer to the std::vector of control points stored in this node
  std::vector<ControlPoint*>* GetControlPoints();
  /// Add n control points.
  /// If point is specified then all control point positions will be initialized to that position,
  /// otherwise control poin positions are initialized to (0,0,0).
  /// If requested number of points would result more points than the maximum allowed number of points
  /// then no points are added at all.
  /// Return index of the last placed control point, -1 on failure.
  int AddNControlPoints(int n, std::string label = std::string(), vtkVector3d* point = nullptr);
  /// Add a new control point, defined in the world coordinate system.
  /// Return index of point index, -1 on failure.
  int AddControlPointWorld(vtkVector3d point, std::string label = std::string());
  /// Add a new control point, returning the point index, -1 on failure.
  int AddControlPoint(vtkVector3d point, std::string label = std::string());
  /// Add a controlPoint to the end of the list. Return index
  /// of new controlPoint, -1 on failure.
  /// Markups node takes over ownership of the pointer (markups node will delete it)
  /// \param autoLabel: if enabled (by default it is) then empty point label will be
  /// replaced with automatically generated label.
  int AddControlPoint(ControlPoint *controlPoint, bool autoLabel=true);

  /// Get the position of the Nth control point
  /// returning it as a vtkVector3d, return (0,0,0) if not found
  vtkVector3d GetNthControlPointPositionVector(int pointIndex);

  /// \deprecated Use GetNthControlPointPositionVector() method instead.
  vtkVector3d GetMarkupPointVector(int markupIndex, int) { return this->GetNthControlPointPositionVector(markupIndex); };

  /// \deprecated Use GetNthControlPointPosition method instead.
  void GetMarkupPoint(int markupIndex, int pointIndex, double point[3]);

  /// Get the position of the Nth control point
  /// setting the elements of point
  void GetNthControlPointPosition(int pointIndex, double point[3]);
  double* GetNthControlPointPosition(int pointIndex);
  /// Get the position of the Nth control point in World coordinate system
  /// Returns 0 on failure, 1 on success.
  int GetNthControlPointPositionWorld(int pointIndex, double worldxyz[3]);

  /// Get control point position status (PositionUndefined, PositionPreview, PositionDefined)
  int GetNthControlPointPositionStatus(int pointIndex);

  /// Set control point status to undefined.
  void UnsetNthControlPointPosition(int pointIndex);

  /// Remove Nth Control Point
  void RemoveNthControlPoint(int pointIndex);

  /// \deprecated Use RemoveNthControlPoint instead.
  void RemoveMarkup(int pointIndex) { this->RemoveNthControlPoint(pointIndex); };

  /// Insert a control point in this list at targetIndex.
  /// If targetIndex is < 0, insert at the start of the list.
  /// If targetIndex is > list size - 1, append to end of list.
  /// If the insertion is successful, ownership of the controlPoint
  /// is transferred to the markups node.
  /// Returns true on success, false on failure.
  bool InsertControlPoint(ControlPoint* controlPoint, int targetIndex);

  //Add and insert control point at index, defined in the world coordinate system.
  //\sa InsertControlPoint
  bool InsertControlPointWorld(int n, vtkVector3d pointWorld, std::string label = std::string());

  //Add and insert control point at index
  //\sa InsertControlPoint
  bool InsertControlPoint(int n, vtkVector3d point, std::string label = std::string());

  /// Swap the position of two control points
  void SwapControlPoints(int m1, int m2);

  /// Set of the Nth control point position from a pointer to an array
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionFromPointer(const int pointIndex, const double *pos);
  /// Set of the Nth control point position from an array
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionFromArray(const int pointIndex, const double pos[3], int positionStatus = PositionDefined);
  /// Set of the Nth control point position from coordinates
  /// \sa SetNthControlPointPositionFromPointer, SetNthControlPointPositionFromArray
  void SetNthControlPointPosition(const int pointIndex, const double x, const double y, const double z, int positionStatus = PositionDefined);
  /// Set of the Nth control point position using World coordinate system
  /// Calls SetNthControlPointPosition after transforming the passed in coordinate
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionWorld(const int pointIndex, const double x, const double y, const double z);
  /// Set of the Nth control point position from an array using World coordinate system
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionWorldFromArray(const int pointIndex, const double pos[3], int positionStatus = PositionDefined);
  /// Set of the Nth control point position and orientation from an array using World coordinate system.
  /// \sa SetNthControlPointPosition
  void SetNthControlPointPositionOrientationWorldFromArray(const int pointIndex,
    const double pos[3], const double orientationMatrix[9], const char* associatedNodeID, int positionStatus = PositionDefined);

  /// Get the position of the center
  /// returning it as a vtkVector3d, return (0,0,0) if not found
  vtkVector3d GetCenterPositionVector();
  /// Get the position of the center
  /// setting the elements of point
  void GetCenterPosition(double point[3]);
  /// Get the position of the center in World coordinate system
  /// Returns 0 on failure, 1 on success.
  int GetCenterPositionWorld(double worldxyz[3]);
  /// Set the center position from a pointer to an array
  /// \sa SetCenterPosition
  void SetCenterPositionFromPointer(const double *pos);
  /// Set the center position position from an array
  /// \sa SetCenterPosition
  void SetCenterPositionFromArray(const double pos[3]);
  /// Set the center position position from coordinates
  /// \sa SetCenterPositionFromPointer, SetCenterPositionFromArray
  void SetCenterPosition(const double x, const double y, const double z);
  /// Set the center position position using World coordinate system
  /// Calls SetCenterPosition after transforming the passed in coordinate
  /// \sa SetCenterPosition
  void SetCenterPositionWorld(const double x, const double y, const double z);

  /// Set the orientation for the Nth control point from a pointer to a double array WXYZ
  void SetNthControlPointOrientationFromPointer(int n, const double *orientationWXYZ);
  /// Set the orientation for the Nth control point from a double array WXYZ
  void SetNthControlPointOrientationFromArray(int n, const double orientationWXYZ[4]);
  /// Set the orientation for the Nth control point from passed parameters
  void SetNthControlPointOrientation(int n, double w, double x, double y, double z);
  /// Get the WXYZ orientation for the Nth control point
  void GetNthControlPointOrientation(int n, double orientationWXYZ[4]);
  /// Get orientation as 9 values: x, y, and z axis directions, respectively.
  double* GetNthControlPointOrientationMatrix(int n);
  /// Set orientation as 9 values: x, y, and z axis directions, respectively.
  void SetNthControlPointOrientationMatrix(int n, double orientationMatrix[9]);
  /// Set orientation as 9 values: x, y, and z axis directions, respectively.
  /// Important: this method just calls SetNthControlPointOrientationMatrix and
  /// does not convert from world coordinate yet.
  void SetNthControlPointOrientationMatrixWorld(int n, double orientationMatrix[9]);
  /// Get normal direction (orientation of z axis) in local coordinate system.
  double* GetNthControlPointNormal(int n);
  /// Get normal direction (orientation of z axis) in world coordinate system.
  void GetNthControlPointNormalWorld(int n, double normalWorld[3]);
  /// Get the WXYZ orientation for the Nth control point
  /// returning it as a vtkVector4d, return (0,0,0,0) if not found.
  /// Note that vtkVector4d stores components in the order XYZW
  /// (in all other methods we get/set components in WXYZ order).
  vtkVector4d GetNthControlPointOrientationVector(int pointIndex);

  /// Get/Set the associated node id for the Nth control point
  std::string GetNthControlPointAssociatedNodeID(int n = 0);
  void SetNthControlPointAssociatedNodeID(int n, std::string id);

  /// \deprecated Use GetNthControlPointAssociatedNodeID instead.
  std::string GetNthMarkupAssociatedNodeID(int n = 0) { return this->GetNthControlPointAssociatedNodeID(n); }
  /// \deprecated Use SetNthControlPointAssociatedNodeID instead.
  void SetNthMarkupAssociatedNodeID(int n, std::string id) { this->SetNthControlPointAssociatedNodeID(n,id); }

  /// Get the id for the Nth control point
  std::string GetNthControlPointID(int n);

  /// \deprecated Use GetNthControlPointID instead.
  std::string GetNthMarkupID(int n = 0) { return this->GetNthControlPointID(n); }

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

  /// \deprecated Use GetNthControlPointLocked instead.
  bool GetNthMarkupLocked(int n = 0) { return this->GetNthControlPointLocked(n); };
  /// \deprecated Use SetNthControlPointLocked instead.
  void SetNthMarkupLocked(int n, bool flag) { this->SetNthControlPointLocked(n, flag);  }

  /// Get the Visibility flag on the Nth control point,
  /// returns false if control point doesn't exist
  bool GetNthControlPointVisibility(int n = 0);
  /// Set Visibility property on Nth control point. If the visibility is set to
  /// true on the node/list as a whole, the Nth control point visibility is used to
  /// determine if it is visible. If the visibility is set to false on the node
  /// as a whole, all control points are hidden but keep this value for when the
  /// list as a whole is turned visible.
  /// \sa vtkMRMLDisplayableNode::SetDisplayVisibility
  /// \sa vtkMRMLDisplayNode::SetVisibility
  void SetNthControlPointVisibility(int n, bool flag);

  /// Get the Label on the Nth control point,
  /// returns false if control point doesn't exist
  std::string GetNthControlPointLabel(int n = 0);
  /// Set the Label on the Nth control point
  void SetNthControlPointLabel(int n, std::string label);

  /// \deprecated Use GetNthControlPointLabel instead.
  std::string GetNthMarkupLabel(int n = 0) { return this->GetNthControlPointLabel(n); }
  /// \deprecated Use SetNthControlPointLabel instead.
  void SetNthMarkupLabel(int n, std::string label) { this->SetNthControlPointLabel(n, label); }

  /// Get the Description flag on the Nth control point,
  /// returns false if control point doesn't exist
  std::string GetNthControlPointDescription(int n = 0);
  /// Set the Description on the Nth control point
  void SetNthControlPointDescription(int n, std::string description);

  /// Returns true since can apply non linear transforms
  /// \sa ApplyTransform
  bool CanApplyNonLinearTransforms()const override;
  /// Apply the passed transformation to all of the control points
  /// \sa CanApplyNonLinearTransforms
  void ApplyTransform(vtkAbstractTransform* transform) override;

  /// Get the markup node label format string that defines the markup names.
  /// \sa SetMarkupLabelFormat
  std::string GetMarkupLabelFormat();
  /// Set the markup node label format strign that defines the markup names,
  /// then invoke the LabelFormatModifedEvent
  /// In standard printf notation, with the addition of %N being replaced
  /// by the list name.
  /// %d will resolve to the highest not yet used list index integer.
  /// Character strings will otherwise pass through
  /// Defaults to %N-%d which will yield markup names of Name-0, Name-1,
  /// Name-2
  /// \sa GetMarkupLabelFormat
  void SetMarkupLabelFormat(std::string format);

  /// If the MarkupLabelFormat contains the string %N, return a string
  /// in which that has been replaced with the list name. If the list name is
  /// nullptr, replace it with an empty string. If the MarkupLabelFormat doesn't
  /// contain %N, return MarkupLabelFormat
  std::string ReplaceListNameInMarkupLabelFormat();

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

  /// Return the number of control points that are required for defining this widget.
  /// Interaction mode remains in "place" mode until this number is reached.
  /// If the number is set to 0 then no it means there is no preference (this is the default value).
  vtkGetMacro(RequiredNumberOfControlPoints, int);

  /// Maximum number of control points limits the number of markups allowed in the node.
  /// If maximum number of control points is set to 0 then no it means there
  /// is no limit (this is the default value).
  /// The value is an indication to the user interface and does not affect
  /// prevent adding markups to a node programmatically.
  /// If value is set to lower value than the number of markups in the node, then
  /// existing markups are not deleted.
  /// 2 for line, and 3 for angle Markups
  vtkGetMacro(MaximumNumberOfControlPoints, int);

  // WXYZ: W rotation angle in degrees, XYZ is rotation axis.
  static void ConvertOrientationMatrixToWXYZ(const double orientationMatrix[9], double orientationWXYZ[4]);
  static void ConvertOrientationWXYZToMatrix(double orientationWXYZ[4], double orientationMatrix[9]);

  void GetControlPointLabels(vtkStringArray* labels);

  vtkPoints* GetCurvePoints();
  vtkPoints* GetCurvePointsWorld();

  vtkPolyData* GetCurve();
  vtkPolyData* GetCurveWorld();

  vtkAlgorithmOutput* GetCurveWorldConnection();

  vtkGetMacro(CurveClosed, bool);

  int GetControlPointIndexFromInterpolatedPointIndex(vtkIdType interpolatedPointIndex);

  /// The internal instance of the curve generator to allow
  /// use of the curve for other computations.
  /// Any custom overrides of the interpolation modes are not persisted in MRML.
  vtkCurveGenerator* GetCurveGenerator() { return this->CurveGenerator.GetPointer(); };

  void GetRASBounds(double bounds[6]) override;
  void GetBounds(double bounds[6]) override;

  /// Get the index of the closest control point to the world coordinates
  int GetClosestControlPointIndexToPositionWorld(double pos[3]);

  /// Set all control point positions from a point list.
  /// If points is nullptr then all control points are removed.
  /// New control points are added if needed.
  /// Existing control points are updated with the new positions.
  /// Any extra existing control points are removed.
  void SetControlPointPositionsWorld(vtkPoints* points);

  /// Get a copy of all control point positions in world coordinate system
  void GetControlPointPositionsWorld(vtkPoints* points);

  /// 4x4 matrix detailing the orientation and position in world coordinates of the interaction handles.
  virtual vtkMatrix4x4* GetInteractionHandleToWorldMatrix();

  virtual std::string GetPropertiesLabelText();

protected:
  vtkMRMLMarkupsNode();
  ~vtkMRMLMarkupsNode() override;
  vtkMRMLMarkupsNode(const vtkMRMLMarkupsNode&);
  void operator=(const vtkMRMLMarkupsNode&);

  vtkSmartPointer<vtkStringArray> TextList;

  /// Utility function to get unit node from scene
  vtkMRMLUnitNode* GetUnitNode(const char* quantity);

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

  /// Calls UpdateMeasurementsInternal()
  void UpdateMeasurements();

  /// Calculates the updated measurements
  /// Should be overwritten by child classes to compute measurements
  virtual void UpdateMeasurementsInternal();

  /// Helper function to write measurements to node Description property.
  /// This is a short-term solution until measurements display is properly implemented.
  virtual void WriteMeasurementsToDescription();

  /// Calculates the handle to world matrix based on the current control points
  virtual void UpdateInteractionHandleToWorldMatrix();

  // Used for limiting number of markups that may be placed.
  int MaximumNumberOfControlPoints;
  int RequiredNumberOfControlPoints;

  bool CurveClosed;

  // Vector of control points
  ControlPointsListType ControlPoints;

  // Converts curve control points to curve points.
  vtkSmartPointer<vtkCurveGenerator> CurveGenerator;

  // Computes tangent and smooth normal for each curve point.
  // It provides a fully specified coordinate system at each point of the curve,
  // which is useful for image reslicing or defining camera pose.
  // Curve is defined in the world coordinate sytem.
  vtkSmartPointer<vtkFrenetSerretFrame> CurveCoordinateSystemGeneratorWorld;

  // Stores control point positions in a polydata (in local coordinate system).
  // Line cells connect all points into a curve.
  vtkSmartPointer<vtkPolyData> CurveInputPoly;

  vtkSmartPointer<vtkTransformPolyDataFilter> CurvePolyToWorldTransformer;
  vtkSmartPointer<vtkGeneralTransform> CurvePolyToWorldTransform;

  // Point locator that allows quick finding of interpolated point in the world
  // coordinate system (in transformed CurvePoly).
  vtkSmartPointer<vtkPointLocator> TransformedCurvePolyLocator;

  // Locks all the points and GUI
  int Locked;

  std::string MarkupLabelFormat;

  // Keep track of the number of markups that were added to the list, always
  // incrementing, not decreasing when they're removed. Used to help create
  // unique names and ids. Reset to 0 when \sa RemoveAllControlPoints called
  int LastUsedControlPointNumber;

  // Markup centerpoint (in local coordinates).
  // It may be used as rotation center or as a handle to grab the widget by.
  vtkVector3d CenterPos;

  std::vector< vtkSmartPointer<vtkMRMLMeasurement> > Measurements;

  std::string PropertiesLabelText;

  // Transform that moves the xyz unit vectors and origin of the interaction handles to local coordinates
  vtkSmartPointer<vtkMatrix4x4> InteractionHandleToWorldMatrix;

  /// Flag set from SetControlPointPositionsWorld that pauses update of measurements until the update is complete.
  bool IsUpdatingPoints;
};

#endif
