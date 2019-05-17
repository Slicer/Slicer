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

// Markups includes
#include "vtkSlicerMarkupsModuleMRMLExport.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

class vtkPlane;

/// \brief MRML node to represent a curve markup
/// Curve Markups nodes contain N control points.
/// Visualization parameters are set in the vtkMRMLMarkupsDisplayNode class.
///
/// Markups is intended to be used for manual marking/editing of point positions.
///
/// \ingroup Slicer_QtModules_Markups
class  VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkMRMLMarkupsCurveNode : public vtkMRMLMarkupsNode
{
public:
  static vtkMRMLMarkupsCurveNode *New();
  vtkTypeMacro(vtkMRMLMarkupsCurveNode,vtkMRMLMarkupsNode);
  /// Print out the node information to the output stream
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetIcon() override {return ":/Icons/MarkupsCurveMouseModePlace.png";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MarkupsCurve";}

  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  /// Get curve points positions in world coordinate system.
  vtkPoints* GetCurvePointsWorld();

  /// Get length of the curve or a section of the curve.
  /// \param startCurvePointIndex length computation starts from this curve point index
  /// \param numberOfCurvePoints if specified then distances up to the first n points are computed
  /// \return sum of distances between the curve points, returns 0 in case of an error
  double GetCurveLengthWorld(vtkIdType startCurvePointIndex=0, vtkIdType numberOfCurvePoints=-1);

  /// Get length of a section of the curve between startPointIndex and endPointIndex.
  /// If endPointIndex < startPointIndex then length outside of the section is computed.
  /// \param startCurvePointIndex length computation starts from this curve point index
  /// \param endCurvePointIndex length computation starts from this curve point index
  /// \return sum of distances between the curve points, returns 0 in case of an error
  double GetCurveLengthBetweenStartEndPointsWorld(vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex);

  void ResampleCurveWorld(double controlPointDistance);

  static bool ResamplePoints(vtkPoints* originalPoints, vtkPoints* interpolatedPoints, double samplingDistance, bool closedCurve);

  /// Samples points along the curve at equal distances.
  /// If endPointIndex < startPointIndex then after the last point, the curve is assumed to continue at the first point.
  bool GetSampledCurvePointsBetweenStartEndPointsWorld(vtkPoints* sampledPoints,
    double samplingDistance, vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex);

  /// Get the index of the closest curve point to the world coordinates
  vtkIdType GetClosestCurvePointIndexToPositionWorld(double posWorld[3]);

  /// Get index of the farthest curve point from the specified reference point.
  /// Distance is Euclidean distance, not distance along the curve.
  /// \param posWorld Reference point position in world coordinate system
  /// \return index of the farthest curve point from refPoint, -1 in case of error
  vtkIdType GetFarthestCurvePointIndexToPositionWorld(double posWorld[3]);

  vtkIdType GetCurvePointIndexFromControlPointIndex(int controlPointIndex);

  /// Get position of a curve point along the curve relative to the specified start point index.
  /// \param startCurvePointId index of the curve point to start the distance measurement from
  /// \param distanceFromStartPoint distance from the start point
  /// \return founod point index, -1 in case of an error
  vtkIdType GetCurvePointIndexAlongCurveWorld(vtkIdType startCurvePointId, double distanceFromStartPoint);

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

protected:
  vtkMRMLMarkupsCurveNode();
  ~vtkMRMLMarkupsCurveNode() override;
  vtkMRMLMarkupsCurveNode(const vtkMRMLMarkupsCurveNode&);
  void operator=(const vtkMRMLMarkupsCurveNode&);
};

#endif
