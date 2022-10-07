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

#ifndef __vtkMRMLMarkupsCurveNode_h
#define __vtkMRMLMarkupsCurveNode_h

// MRML includes
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLModelNode.h"

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

// VTK includes
#include <vtkStringArray.h>

// std includes
#include <vector>

class vtkArrayCalculator;
class vtkAssignAttribute;
class vtkCallbackCommand;
class vtkCleanPolyData;
class vtkCurveMeasurementsCalculator;
class vtkPassThrough;
class vtkPlane;
class vtkProjectMarkupsCurvePointsFilter;
class vtkTransformPolyDataFilter;
class vtkTriangleFilter;

/// \brief MRML node to represent a curve markup
/// Curve Markups nodes contain N control points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
///
/// Coordinate systems used:
///   - Local: Current node's coordinate system where the position of the control and curve points are defined.
///            Local coordinates can be converted to world by concatenating all parent transforms on the current node.
///   - Surface: Model node's coordinate system where the polydata used for ShortestDistanceOnSurface pathfinding is defined.
///            Surface coordinates can be converted to world by concatenating all parent transforms on the surface node.
///   - World: Patient coordinate system (RAS)
///
/// Markups measurement pipeline:
///
/// +------------------------------------------vtkMarkupsCurveNode-----------------------------------------------+
/// |  +-vtkMRMLMarkupsNode-+                                                             === : Display pipeline |
/// |  |                    |        +-vtkCurveGenerator-+                                                       |
/// |  |   CurveInputPoly=============                   | : Generate interpolated curve from the control points |
/// |  |   Measurements--------+     +---------║---------+                                                       |
/// |  |                    |  |               ║                                                                 |
/// |  +--------------------+  |               ║                                                                 |
/// |                         ++-vtkCurveMeasurementsCalculator-+                                                |
/// |                         |                                 | : Add measurements as data arrays to the curve |
/// |                         +----------------║----------------+   poly data                                    |
/// |                                          ║                                                                 |
/// |                                          ║                                                                 |
/// |                         +---vtkTransformPolyDataFilter----+                                                |
/// |                         |  (CurvePolyToWorldTransformer)  |                                                |
/// |                         +----------------║----------------+                                                |
/// |                                          ║                                                                 |
/// +------------------------------------------║-----------------------------------------------------------------+
///                                            ║
///                          +-----------------║-------vtkSlicerCurveRepresentation3D----------------------+
///                          |                 ║                                                           |
///                          |                 ║(vtkCleaner in between)                                    |
///                          |        +--vtkTubeFilter--+                                                  |
///                          |        |                 =========> vtkPolyDataMapper => vtkActor           |
///                          |        +-----------------+                                                  |
///                          +-----------------------------------------------------------------------------+
///
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsCurveNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsCurveNode *New();
  vtkTypeMacro(vtkMRMLMarkupsCurveNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsOpenCurve.png";}
  const char* GetAddIcon() override {return ":/Icons/MarkupsCurveMouseModePlace.png";}
  const char* GetPlaceAddIcon() override {return ":/Icons/MarkupsCurveMouseModePlaceAdd.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsCurve";}

  /// Get markup type internal name
  const char* GetMarkupType() override {return "Curve";};

  // Get markup type GUI display name
  const char* GetTypeDisplayName() override {return "Curve";};

  /// Get markup short name
  const char* GetDefaultNodeNamePrefix() override {return "OC";};

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLMarkupsCurveNode);

  ///@{
  /// Get curve points positions in world coordinate system.
  vtkPoints* GetCurvePointsWorld() override;
  vtkPolyData* GetCurveWorld() override;
  vtkAlgorithmOutput* GetCurveWorldConnection() override;
  ///@}

  /// Get length of the curve or a section of the curve.
  /// \param startCurvePointIndex length computation starts from this curve point index
  /// \param numberOfCurvePoints if specified then distances up to the first n points are computed.
  ///   If <0 then all the points are used.
  /// \return sum of distances between the curve points, returns 0 in case of an error
  double GetCurveLengthWorld(vtkIdType startCurvePointIndex=0, vtkIdType numberOfCurvePoints=-1);

  /// Utility function to get curve length from a point list.
  /// \sa GetCurveLengthWorld
  static double GetCurveLength(vtkPoints* curvePoints, bool closedCurve, vtkIdType startCurvePointIndex=0, vtkIdType numberOfCurvePoints=-1);

  /// Get length of a section of the curve between startPointIndex and endPointIndex.
  /// If endPointIndex < startPointIndex then length outside of the section is computed.
  /// \param startCurvePointIndex length computation starts from this curve point index
  /// \param endCurvePointIndex length computation starts from this curve point index
  /// \return sum of distances between the curve points, returns 0 in case of an error
  double GetCurveLengthBetweenStartEndPointsWorld(vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex);

  /// Provides access to protected vtkMRMLMarkupsNode::SetControlPointLabelsWorld
  bool SetControlPointLabels(vtkStringArray* labels, vtkPoints* points);

  /// Constrain points to a specified model surface
  /// Projection to surface is constrained by maximumSearchRadius, specified as a percentage of the model's
  /// bounding box diagonal in world coordinate system.
  /// maximumSearchRadius is valid in the range between 0 and 1.
  /// returns true if successful, false in case of error
  ///
  /// \sa vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurface
  static bool ConstrainPointsToSurface(vtkPoints* originalPoints, vtkPoints* normalVectors, vtkPolyData* surfacePolydata,
    vtkPoints* surfacePoints, double maximumSearchRadius=.25);

  /// Resample control points to have equal distances in the world coordinate system.
  void ResampleCurveWorld(double controlPointDistance);

  /// Resample control points to have equal distances in the node's coordinate system.
  // If pedigreeIdsArray is specified then the method returns the fractional point index of the original point for each new point.
  // Fractional point index is a value between the point indices of the two original points it was between, the fractional part
  // specifies the distance from those two points. It can be used for interpolating point data.
  static bool ResamplePoints(vtkPoints* originalPoints, vtkPoints* interpolatedPoints,
    double samplingDistance, bool closedCurve, vtkDoubleArray* pedigreeIdsArray=nullptr);

  /// Resample static control point measurements using linear interpolation, based on fractional pedigreeIds.
  static bool ResampleStaticControlPointMeasurements(vtkCollection* measurements, vtkDoubleArray* curvePointsPedigreeIdsArray, int curvePointsPerControlPoint);

  /// Samples points along the curve at equal distances.
  /// If endPointIndex < startPointIndex then after the last point, the curve is assumed to continue at the first point.
  bool GetSampledCurvePointsBetweenStartEndPointsWorld(vtkPoints* sampledPoints,
    double samplingDistance, vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex);

  /// Get the index of the closest curve point to the world coordinates
  vtkIdType GetClosestCurvePointIndexToPositionWorld(const double posWorld[3]);

  /// Get position of the closest point along the curve in world coordinates.
  /// The found position may be between two curve points.
  /// Returns index of the found line segment. -1 if failed.
  /// \param posWorld: input position
  /// \param closestPosWorld: output found closest position
  vtkIdType GetClosestPointPositionAlongCurveWorld(const double posWorld[3], double closestPosWorld[3]);

  /// Get index of the farthest curve point from the specified reference point.
  /// Distance is Euclidean distance, not distance along the curve.
  /// \param posWorld Reference point position in world coordinate system
  /// \return index of the farthest curve point from refPoint, -1 in case of error
  vtkIdType GetFarthestCurvePointIndexToPositionWorld(const double posWorld[3]);

  /// Get curve point index corresponding to a control point.
  /// It is useful for calling methods that require curve point index as input.
  vtkIdType GetCurvePointIndexFromControlPointIndex(int controlPointIndex);

  /// Get point position along curve. Position is found along the curve and not snapped to closest curve point.
  static bool GetPositionAndClosestPointIndexAlongCurve(double foundCurvePosition[3], vtkIdType& foundClosestPointIndex,
    vtkIdType startCurvePointId, double distanceFromStartPoint, vtkPoints* curvePoints, bool closedCurve);

  /// Get position of a curve point along the curve relative to the specified start point index.
  /// \param startCurvePointId index of the curve point to start the distance measurement from
  /// \param distanceFromStartPoint distance from the start point
  /// \return found point index, -1 in case of an error
  vtkIdType GetCurvePointIndexAlongCurveWorld(vtkIdType startCurvePointId, double distanceFromStartPoint);

  /// Get position of a point along the curve relative to the specified start point index.
  /// The returned position can be between curve points (to match the requested distance as accurately as possible).
  /// \param startCurvePointId index of the curve point to start the distance measurement from
  /// \param distanceFromStartPoint distance from the start point
  /// \return true if a point found exactly at the requested distance
  bool GetPositionAlongCurveWorld(double foundCurvePosition[3], vtkIdType startCurvePointId, double distanceFromStartPoint);

  /// Get direction vector at specified curve point index, in World coordinate system.
  /// \return true on success.
  bool GetCurveDirectionAtPointIndexWorld(vtkIdType curvePointIndex, double directionVectorWorld[3]);

  /// Get transformation from CurvePoint to World coordinate system at the specified curve point index.
  /// CurvePoint coordinate system:
  /// - Origin: position of the curve point.
  /// - X axis: curve normal direction.
  /// - Y axis: binormal direction (tangent x normal).
  /// - Z axis: curve tangent direction.
  /// \return true on success.
  bool GetCurvePointToWorldTransformAtPointIndex(vtkIdType curvePointIndex, vtkMatrix4x4* curvePointToWorld);

  bool GetPointsOnPlaneWorld(vtkPlane* plane, vtkPoints* intersectionPoints);

  /// Type of curve to generate
  int GetCurveType();
  void SetCurveType(int type);
  const char* GetCurveTypeAsString(int id);
  int GetCurveTypeFromString(const char* name);
  void SetCurveTypeToLinear();
  void SetCurveTypeToCardinalSpline();
  void SetCurveTypeToKochanekSpline();
  void SetCurveTypeToPolynomial();
  void SetCurveTypeToShortestDistanceOnSurface(vtkMRMLModelNode* modelNode=nullptr);

  /// \deprecated GetShortestDistanceSurfaceNodeReferenceRole
  /// \sa GetSurfaceConstraintNodeReferenceRole
  const char* GetShortestDistanceSurfaceNodeReferenceRole() { return this->GetSurfaceConstraintNodeReferenceRole(); }
  /// \deprecated GetShortestDistanceSurfaceNodeReferenceMRMLAttributeName
  /// \sa GetSurfaceConstraintNodeReferenceMRMLAttributeName
  const char* GetShortestDistanceSurfaceNodeReferenceMRMLAttributeName() { return this->GetSurfaceConstraintNodeReferenceMRMLAttributeName(); }

  /// Node reference role for the surface that is used to project the curve onto
  const char* GetSurfaceConstraintNodeReferenceRole() { return "shortestDistanceSurface"; }
  const char* GetSurfaceConstraintNodeReferenceMRMLAttributeName() { return "shortestDistanceSurfaceRef"; };

  /// The model node that is used as the surface mesh for finding the shortest distance path on the surface mesh.
  /// Used by the ShortestDistanceOnSurface curve type.
  /// \deprecated SetAndObserveShortestDistanceSurfaceNode
  /// \sa SetAndObserveSurfaceConstraintNode
  void SetAndObserveShortestDistanceSurfaceNode(vtkMRMLModelNode* modelNode) { this->SetAndObserveSurfaceConstraintNode(modelNode); }
  /// \deprecated GetShortestDistanceSurfaceNode
  /// \sa GetSurfaceConstraintNode
  vtkMRMLModelNode* GetShortestDistanceSurfaceNode() { return this->GetSurfaceConstraintNode(); }

  ///@{
  /// Set/Get the model node that is used as the surface mesh for finding the shortest distance path on the surface mesh.
  void SetAndObserveSurfaceConstraintNode(vtkMRMLModelNode* modelNode);
  vtkMRMLModelNode* GetSurfaceConstraintNode();
  ///@}

  ///@{
  /// Set/Get maximumSearchRadiusTolerance defining the allowable projection distance when projecting
  /// curve to surface.
  ///
  /// It is specified as a percentage of the model's bounding box diagonal in world coordinate system.
  void SetSurfaceConstraintMaximumSearchRadiusTolerance(double tolerance);
  double GetSurfaceConstraintMaximumSearchRadiusTolerance() const;
  ///@}

  /// The method that should be used to combine the distance with the scalar value for ShortestDistanceOnSurface curve type.
  /// Uses the COST_FUNCTION_X enums from vtkSlicerDijkstraGraphGeodesicPath.
  int GetSurfaceCostFunctionType();
  void SetSurfaceCostFunctionType(int surfaceCostFunctionType);
  static const char* GetSurfaceCostFunctionTypeAsString(int surfaceCostFunctionType);
  static int GetSurfaceCostFunctionTypeFromString(const char* name);

  /// The scalar weight function that is used for modifying the weight on each vertex for ShortestDistanceOnSurface curve type.
  /// The the currently active point scalar array is available as the "activeScalar" variable.
  const char* GetSurfaceDistanceWeightingFunction();
  void SetSurfaceDistanceWeightingFunction(const char* function);

  //@{
  /// Get/set how many curve points are inserted between control points.
  /// Higher values are recommended if distance between control points is large.
  int GetNumberOfPointsPerInterpolatingSegment();
  void SetNumberOfPointsPerInterpolatingSegment(int pointsPerSegment);
  //@}

  /// Update scalar range and update markups pipeline when the active scalar array is changed
  virtual void UpdateAssignedAttribute() override;

protected:
  vtkSmartPointer<vtkCleanPolyData> CleanFilter;
  vtkSmartPointer<vtkTriangleFilter> TriangleFilter;
  vtkSmartPointer<vtkTransformPolyDataFilter> SurfaceToLocalTransformer;
  vtkSmartPointer<vtkArrayCalculator> SurfaceScalarCalculator;
  vtkSmartPointer<vtkPassThrough> SurfaceScalarPassThroughFilter;
  vtkSmartPointer<vtkCurveMeasurementsCalculator> CurveMeasurementsCalculator;
  vtkSmartPointer<vtkPassThrough> WorldOutput;
  const char* ShortestDistanceSurfaceActiveScalar;

  /// Filter that changes the active scalar of the input mesh using the ActiveScalarName
  /// and ActiveAttributeLocation properties. This can be useful to specify what field
  /// array is the color array that needs to be used by the VTK mappers.
  vtkSmartPointer<vtkAssignAttribute> ScalarDisplayAssignAttribute;

  /// Command handling curvature measurement modified events to propagate enabled state
  vtkCallbackCommand* CurvatureMeasurementModifiedCallbackCommand;

protected:
  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;
  void OnNodeReferenceAdded(vtkMRMLNodeReference* reference) override;
  void OnNodeReferenceModified(vtkMRMLNodeReference* reference) override;
  void OnNodeReferenceRemoved(vtkMRMLNodeReference* reference) override;

  virtual void UpdateSurfaceScalarVariables();
  virtual void OnSurfaceModelNodeChanged();
  virtual void OnSurfaceModelTransformChanged();

  vtkMRMLMarkupsCurveNode();
  ~vtkMRMLMarkupsCurveNode() override;
  vtkMRMLMarkupsCurveNode(const vtkMRMLMarkupsCurveNode&);
  void operator=(const vtkMRMLMarkupsCurveNode&);

  void UpdateMeasurementsInternal() override;

  /// Callback function observing curvature measurement modified events to propagate enabled state
  static void OnCurvatureMeasurementModified(vtkObject* caller, unsigned long eid, void* clientData, void* callData);

private:
  vtkSmartPointer<vtkProjectMarkupsCurvePointsFilter> ProjectPointsFilter;
};

#endif
