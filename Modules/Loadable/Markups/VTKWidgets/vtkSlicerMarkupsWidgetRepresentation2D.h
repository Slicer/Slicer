/*=========================================================================

 Copyright (c) ProxSim ltd., Kwun Tong, Hong Kong. All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
 and development was supported by ProxSim ltd.

=========================================================================*/

/**
 * @class   vtkSlicerMarkupsWidgetRepresentation2D
 * @brief   Default representation for the slicer markups widget
 *
 * This class provides the default concrete representation for the
 * vtkMRMLAbstractWidget. See vtkMRMLAbstractWidget
 * for details.
 * @sa
 * vtkSlicerMarkupsWidgetRepresentation2D vtkMRMLAbstractWidget
 */

#ifndef vtkSlicerMarkupsWidgetRepresentation2D_h
#define vtkSlicerMarkupsWidgetRepresentation2D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidgetRepresentation.h"

#include "vtkMRMLSliceNode.h"

#include <memory>

class vtkActor2D;
class vtkAlgorithmOutput;
class vtkDoubleArray;
class vtkDiscretizableColorTransferFunction;
class vtkFloatArray;
class vtkGlyph2D;
class vtkLabelPlacementMapper;
class vtkMarkupsGlyphSource2D;
class vtkPlane;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataMapper2D;
class vtkProperty2D;
class vtkScalarsToColors;

class vtkMRMLInteractionEventData;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerMarkupsWidgetRepresentation2D : public vtkSlicerMarkupsWidgetRepresentation
{
public:
  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerMarkupsWidgetRepresentation2D, vtkSlicerMarkupsWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Position is displayed (slice) position
  void CanInteract(vtkMRMLInteractionEventData* interactionEventData, int& foundComponentType, int& foundComponentIndex, double& closestDistance2) override;

  /// Checks if interaction with straight line between visible points is possible.
  /// Can be used on the output of CanInteract, as if no better component is found then the input is returned.
  void CanInteractWithLine(vtkMRMLInteractionEventData* interactionEventData, int& foundComponentType, int& foundComponentIndex, double& closestDistance2);

  /// Subclasses of vtkSlicerMarkupsWidgetRepresentation2D must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRMLInternal(vtkMRMLNode* caller, unsigned long event, void* callData = nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection*) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOverlay(vtkViewport* viewport) override;
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport* viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Get the nth node's position on the slice. Will return
  /// 1 on success, or 0 if there are not at least
  /// (n+1) nodes (0 based counting).
  int GetNthControlPointDisplayPosition(int n, double pos[2]) override;

  /// Set the Nth node visibility in this view (markup visibility is enabled
  /// and markup is on current slice).
  /// Useful for non-regression tests that need to inspect internal state of the widget.
  virtual bool GetNthControlPointViewVisibility(int n);

  /// Set the Nth node slice visibility (i.e. if it is on the slice).
  virtual void SetNthControlPointSliceVisibility(int n, bool visibility);

  /// Set the center slice visibility (i.e. if it is on the slice).
  virtual void SetCenterSliceVisibility(bool visibility);

  void GetSliceToWorldCoordinates(const double[2], double[3]);
  void GetWorldToSliceCoordinates(const double worldPos[3], double slicePos[2]);

protected:
  vtkSlicerMarkupsWidgetRepresentation2D();
  ~vtkSlicerMarkupsWidgetRepresentation2D() override;

  /// Get MRML view node as slice view node
  vtkMRMLSliceNode* GetSliceNode();

  void UpdatePlaneFromSliceNode();

  void UpdateViewScaleFactor() override;
  void UpdateControlPointSize() override;

  // Return squared distance of maximum distance for picking a control point,
  // in pixels.
  double GetMaximumControlPointPickingDistance2();

  bool GetAllControlPointsVisible() override;

  /// Check, if the point is displayable in the current slice geometry
  virtual bool IsControlPointDisplayableOnSlice(vtkMRMLMarkupsNode* node, int pointIndex = 0);

  // Update colormap based on provided base color (modulated with settings stored in the display node)
  virtual void UpdateDistanceColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3]);

  /// Check, if the point is behind in the current slice geometry
  virtual bool IsPointBehindSlice(vtkMRMLMarkupsNode* node, int pointIndex = 0);

  /// Check, if the point is in front in the current slice geometry
  virtual bool IsPointInFrontSlice(vtkMRMLMarkupsNode* node, int pointIndex = 0);

  /// Check, if the point is displayable in the current slice geometry
  virtual bool IsCenterDisplayableOnSlice(vtkMRMLMarkupsNode* node);

  /// Convert display to world coordinates
  void GetWorldToDisplayCoordinates(double r, double a, double s, double* displayCoordinates);
  void GetWorldToDisplayCoordinates(double* worldCoordinates, double* displayCoordinates);

  /// Check if the representation polydata intersects the slice
  bool IsRepresentationIntersectingSlice(vtkPolyData* representation, const char* arrayName);

  /// Compute fading scalars for accurate line/curve projection on the 2D slice.
  /// 1. Clips the polydata to the fading range (|distance| <= fadingEnd), discarding
  ///    invisible portions entirely.
  /// 2. Inserts exact intersection points where the curve crosses the slice plane.
  /// 3. Subdivides the visible portion into enough vertices that per-vertex opacity
  ///    mapping accurately represents the nonlinear transfer function.
  /// If intersectionPoints is non-null, detected zero-crossings are appended to it.
  /// The caller is responsible for resetting intersectionPoints before the first call.
  /// When the MTime guard triggers an early return, intersectionPoints is left untouched
  /// (still valid from the previous computation with the same inputs).
  virtual void ComputeIntersectionFadingScalars(vtkPolyData* worldPolyData,
                                                vtkPlane* slicePlane,
                                                vtkPolyData* outputPolyData,
                                                vtkFloatArray* distanceArray = nullptr,
                                                vtkPoints* intersectionPoints = nullptr);

  /// Project accumulated SliceIntersectionWorldPoints to display coordinates and update
  /// the intersection point glyph pipeline (actor visibility, color, scale, LUT).
  virtual void UpdateSliceIntersectionPointDisplay(double glyphSize, vtkScalarsToColors* colorMap);

  class ControlPointsPipeline2D : public ControlPointsPipeline
  {
  public:
    ControlPointsPipeline2D();
    ~ControlPointsPipeline2D() override;

    vtkSmartPointer<vtkActor2D> Actor;
    vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
    vtkSmartPointer<vtkGlyph2D> Glypher;
    vtkSmartPointer<vtkActor2D> LabelsActor;
    vtkSmartPointer<vtkLabelPlacementMapper> LabelsMapper;
    // Properties used to control the appearance of selected objects and
    // the manipulator in general.
    vtkSmartPointer<vtkProperty2D> Property;
  };

  ControlPointsPipeline2D* GetControlPointsPipeline(int controlPointType);

  vtkSmartPointer<vtkIntArray> PointsVisibilityOnSlice;
  bool CenterVisibilityOnSlice = { false };
  bool AnyPointVisibilityOnSlice = { false }; // at least one point is visible

  vtkSmartPointer<vtkTransform> WorldToSliceTransform;
  vtkSmartPointer<vtkPlane> SlicePlane;

  /// Per-glyph rendering pipeline for slice intersection points.
  /// Each instance holds the display points, polydata, glypher, mapper, and actor
  /// for one glyph type (entering = CircledCross, exiting = CircledPoint).
  class LineIntersectionPointsPipeline2D
  {
  public:
    LineIntersectionPointsPipeline2D(vtkAlgorithmOutput* glyphSourcePort, vtkProperty2D* property);
    ~LineIntersectionPointsPipeline2D() = default;

    vtkSmartPointer<vtkPoints> DisplayPoints;
    vtkSmartPointer<vtkPolyData> PointsPoly;
    vtkSmartPointer<vtkGlyph2D> Glypher;
    vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
    vtkSmartPointer<vtkProperty2D> Property;
    vtkSmartPointer<vtkActor2D> Actor;
  };

  /// Per-glyph rendering pipeline for direction arrow markers (2D).
  /// Holds the display-space points, normals, slice-distance scalars, polydata,
  /// glyph2D, mapper, and actor for cone-shaped direction markers.
  /// The mapper's lookup table is not set here — callers must configure it after
  /// construction via LineDirectionArrowPipeline->Mapper->SetLookupTable(...).
  class LineDirectionArrowPipeline2D
  {
  public:
    LineDirectionArrowPipeline2D();
    ~LineDirectionArrowPipeline2D() = default;

    vtkSmartPointer<vtkPoints> Points;
    vtkSmartPointer<vtkDoubleArray> Normals;
    vtkSmartPointer<vtkFloatArray> SliceDistances; ///< signed distance to slice per marker, drives opacity fading
    vtkSmartPointer<vtkPolyData> PointsPoly;
    vtkSmartPointer<vtkGlyph2D> Glypher;
    vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
    vtkSmartPointer<vtkActor2D> Actor;
  };

  /// Direction arrow glyph pipeline (2D).
  std::unique_ptr<LineDirectionArrowPipeline2D> LineDirectionArrowPipeline;

  /// Cached BuildLineDirectionMarkers output; rebuilt only when spacing or geometry changes.
  /// The view-dependent projection loop (world -> display) always runs.
  vtkSmartPointer<vtkPoints> LineDirectionMarkerCachedWorldPositions;
  vtkSmartPointer<vtkDoubleArray> LineDirectionMarkerCachedWorldTangents;
  double LineDirectionMarkerLastSpacing = -1.0;
  vtkMTimeType LineDirectionMarkerLastGeometryMTime = 0;
  vtkMTimeType LineDirectionMarkerLastSlicePlaneMTime = 0;
  vtkMTimeType LineDirectionMarkerLastMarkupsDisplayMTime = 0;
  bool LineDirectionFirstToLastControlPoint = true;

  // Line slice intersection point glyph pipelines (2D).
  // Mark the exact positions where a curve/line crosses the slice plane.
  // Entering (CircledCross) and exiting (CircledPoint) are driven by
  // SliceIntersectionApproachingSigns.
  vtkSmartPointer<vtkPoints> LineSliceIntersectionWorldPoints;
  vtkSmartPointer<vtkFloatArray> LineSliceIntersectionApproachingSigns;
  vtkSmartPointer<vtkProperty2D> LineSliceIntersectionProperty;
  std::unique_ptr<LineIntersectionPointsPipeline2D> LineSliceIntersectionEnteringPipeline;
  std::unique_ptr<LineIntersectionPointsPipeline2D> LineSliceIntersectionExitingPipeline;

  /// Cached input timestamps for ComputeIntersectionFadingScalars MTime guard.
  /// Avoids expensive polyline walking when neither curve geometry, slice plane,
  /// nor fading range have changed.
  vtkMTimeType FadingScalarsLastWorldDataMTime = 0;
  vtkMTimeType FadingScalarsLastSlicePlaneMTime = 0;
  double FadingScalarsLastFadingEnd = -1.0;

  virtual void UpdateAllPointsAndLabelsFromMRML(double labelsOffset);

  double GetWidgetOpacity(int controlPointType);

private:
  vtkSlicerMarkupsWidgetRepresentation2D(const vtkSlicerMarkupsWidgetRepresentation2D&) = delete;
  void operator=(const vtkSlicerMarkupsWidgetRepresentation2D&) = delete;
};

#endif
