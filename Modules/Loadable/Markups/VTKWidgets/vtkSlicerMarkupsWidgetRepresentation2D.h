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

class vtkActor2D;
class vtkDiscretizableColorTransferFunction;
class vtkGlyph2D;
class vtkLabelPlacementMapper;
class vtkMarkupsGlyphSource2D;
class vtkPlane;
class vtkPolyDataMapper2D;
class vtkProperty2D;

class vtkMRMLInteractionEventData;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerMarkupsWidgetRepresentation2D : public vtkSlicerMarkupsWidgetRepresentation
{
public:
  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerMarkupsWidgetRepresentation2D, vtkSlicerMarkupsWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Position is displayed (slice) position
  void CanInteract(vtkMRMLInteractionEventData* interactionEventData,
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) override;

  /// Check if interaction with the transformation handles is possible
  virtual void CanInteractWithHandles(vtkMRMLInteractionEventData* interactionEventData,
    int& foundComponentType, int& foundComponentIndex, double& closestDistance2);

  /// Checks if interaction with straight line between visible points is possible.
  /// Can be used on the output of CanInteract, as if no better component is found then the input is returned.
  void CanInteractWithLine(vtkMRMLInteractionEventData* interactionEventData,
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2);

  /// Subclasses of vtkSlicerMarkupsWidgetRepresentation2D must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData=nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  int RenderOpaqueGeometry(vtkViewport *viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) override;
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

  void UpdateInteractionPipeline() override;

protected:
  vtkSlicerMarkupsWidgetRepresentation2D();
  ~vtkSlicerMarkupsWidgetRepresentation2D() override;

  /// Reimplemented for 2D specific mapper/actor settings
  void SetupInteractionPipeline() override;

    /// Get MRML view node as slice view node
  vtkMRMLSliceNode *GetSliceNode();

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
  void GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates);
  void GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates);

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
  bool                         CenterVisibilityOnSlice = { false };
  bool                         AnyPointVisibilityOnSlice = { false };  // at least one point is visible

  vtkSmartPointer<vtkTransform> WorldToSliceTransform;
  vtkSmartPointer<vtkPlane> SlicePlane;

  virtual void UpdateAllPointsAndLabelsFromMRML(double labelsOffset);

  double GetWidgetOpacity(int controlPointType);

  class MarkupsInteractionPipeline2D : public MarkupsInteractionPipeline
  {
  public:
    MarkupsInteractionPipeline2D(vtkSlicerMarkupsWidgetRepresentation* representation);
    ~MarkupsInteractionPipeline2D() override = default;;

    void GetViewPlaneNormal(double viewPlaneNormal[3]) override;

    vtkSmartPointer<vtkTransformPolyDataFilter> WorldToSliceTransformFilter;
  };

private:
  vtkSlicerMarkupsWidgetRepresentation2D(const vtkSlicerMarkupsWidgetRepresentation2D&) = delete;
  void operator=(const vtkSlicerMarkupsWidgetRepresentation2D&) = delete;
};

#endif
