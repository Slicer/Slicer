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
 * @class   vtkSlicerAbstractWidgetRepresentation
 * @brief   Class for rendering a markups node
 *
 * This class can display a markups node in the scene.
 * It plays a similar role to vtkWidgetRepresentation, but it is
 * simplified and specialized for optimal use in Slicer.
 * It state is stored in the associated MRML display node to
 * avoid extra synchronization mechanisms.
 * The representation only observes MRML node changes,
 * it does not directly process any interaction events directly
 * (interaction events are processed by vtkSlicerAbstractWidget,
 * which then modifies MRML nodes).
 *
 * This class (and subclasses) are a type of
 * vtkProp; meaning that they can be associated with a vtkRenderer end
 * embedded in a scene like any other vtkActor.
*
 * @sa
 * vtkSlicerAbstractWidgetRepresentation vtkSlicerAbstractWidget vtkPointPlacer
*/

#ifndef vtkSlicerAbstractRepresentation_h
#define vtkSlicerAbstractRepresentation_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkWidgetRepresentation.h"

#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

#include <vector> // STL Header; Required for vector

class vtkMapper;
class vtkMarkupsGlyphSource2D;
class vtkPolyData;
class vtkPoints;

class vtkPointPlacer;
class vtkPolyData;
class vtkIdList;
class vtkPointSetToLabelHierarchy;
class vtkSphereSource;
class vtkStringArray;
class vtkTextProperty;

#include "vtkBoundingBox.h"

class ControlPointsPipeline;
class vtkMRMLAbstractViewNode;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerAbstractWidgetRepresentation : public vtkProp
{
public:
  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerAbstractWidgetRepresentation, vtkProp);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;


  //@{
  /**
  * Subclasses of vtkWidgetRepresentation must implement these methods. This is
  * considered the minimum API for a widget representation.
  * <pre>
  * SetRenderer() - the renderer in which the representations draws itself.
  * Typically the renderer is set by the associated widget.
  * Use the widget's SetCurrentRenderer() method in most cases;
  * otherwise there is a risk of inconsistent behavior as events
  * and drawing may be performed in different viewports.
  * UpdateFromMRML() - update the widget from its state stored in MRML.
  * if event is non-zero then a specific update (faster, smaller scope) is performed instead
  * of a full update.
  * </pre>
  * WARNING: The renderer is NOT reference counted by the representation,
  * in order to avoid reference loops.  Be sure that the representation
  * lifetime does not extend beyond the renderer lifetime.
  */
  virtual void SetRenderer(vtkRenderer *ren);
  virtual vtkRenderer* GetRenderer();
  virtual void SetViewNode(vtkMRMLAbstractViewNode* viewNode);
  virtual vtkMRMLAbstractViewNode* GetViewNode();
  virtual void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData = NULL);
  //@}


  /**
  * Methods to make this class behave as a vtkProp. They are repeated here (from the
  * vtkProp superclass) as a reminder to the widget implementor. Failure to implement
  * these methods properly may result in the representation not appearing in the scene
  * (i.e., not implementing the Render() methods properly) or leaking graphics resources
  * (i.e., not implementing ReleaseGraphicsResources() properly).
  */
  double *GetBounds() VTK_SIZEHINT(6) override { return nullptr; }
  void GetActors(vtkPropCollection *) override {}
  void GetActors2D(vtkPropCollection *) override {}
  void GetVolumes(vtkPropCollection *) override {}
  void ReleaseGraphicsResources(vtkWindow *) override {}
  int RenderOverlay(vtkViewport *vtkNotUsed(viewport)) override { return 0; }
  int RenderOpaqueGeometry(vtkViewport *vtkNotUsed(viewport)) override { return 0; }
  int RenderTranslucentPolygonalGeometry(vtkViewport *vtkNotUsed(viewport)) override { return 0; }
  int RenderVolumetricGeometry(vtkViewport *vtkNotUsed(viewport)) override { return 0; }
  vtkTypeBool HasTranslucentPolygonalGeometry() override { return 0; }
  //@}

  /// Get the nth node's display position. Will return
  /// 1 on success, or 0 if there are not at least
  /// (n+1) nodes (0 based counting).
  virtual int GetNthNodeDisplayPosition(int n, double pos[2]);

  /// Get the nth node.
  virtual vtkMRMLMarkupsNode::ControlPoint *GetNthNode(int n);

  /// Specify tolerance for performing pick operations of points
  /// (see ActivateNode).
  /// Tolerance is defined in terms of percentage of the handle size.
  /// Default value is 0.5
  vtkSetMacro(Tolerance, double);
  vtkGetMacro(Tolerance, double);

  /// Set / get the Point Placer. The point placer is
  /// responsible for converting display coordinates into
  /// world coordinates according to some constraints, and
  /// for validating world positions.
  void SetPointPlacer(vtkPointPlacer *);
  vtkPointPlacer* GetPointPlacer();

  /// Controls whether the widget should always appear on top
  /// of other actors in the scene. (In effect, this will disable OpenGL
  /// Depth buffer tests while rendering the widget).
  /// Default is to set it to false.
  vtkSetMacro(AlwaysOnTop, vtkTypeBool);
  vtkGetMacro(AlwaysOnTop, vtkTypeBool);
  vtkBooleanMacro(AlwaysOnTop, vtkTypeBool);

  /// Set/Get the vtkMRMLMarkipsNode connected with this representation
  virtual void SetMarkupsDisplayNode(vtkMRMLMarkupsDisplayNode *markupsDisplayNode);
  virtual vtkMRMLMarkupsDisplayNode* GetMarkupsDisplayNode();
  virtual vtkMRMLMarkupsNode* GetMarkupsNode();

  /// Compute the center by sampling the points along
  /// the polyline of the widget at equal distances.
  /// and it also updates automatically the center pos stored in the Markups node
  virtual void UpdateCenter();

  /// Translation, rotation, scaling will happen around this position
  virtual bool GetTransformationReferencePoint(double referencePointWorld[3]);

  /// Return found component type (as vtkMRMLMarkupsDisplayNode::ComponentType).
  /// closestDistance2 is the squared distance in display coordinates from the closest position where interaction is possible.
  /// componentIndex returns index of the found component (e.g., if control point is found then control point index is returned).
  virtual void CanInteract(const int displayPosition[2], const double worldPosition[3],
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2);

  //@{
  /**
  * This data member is used to keep track of whether to render
  * or not (i.e., to minimize the total number of renders).
  */
  vtkGetMacro(NeedToRender, bool);
  vtkSetMacro(NeedToRender, bool);
  vtkBooleanMacro(NeedToRender, bool);
  //@}

  virtual int FindClosestPointOnWidget(const int displayPos[2], double worldPos[3], int *idx);

protected:
  vtkSlicerAbstractWidgetRepresentation();
  ~vtkSlicerAbstractWidgetRepresentation() VTK_OVERRIDE;

  /*
  static void MRMLNodesCallback(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData);
  */

  // The renderer in which this widget is placed
  vtkWeakPointer<vtkRenderer> Renderer;

  class ControlPointsPipeline
  {
  public:
    ControlPointsPipeline();
    virtual ~ControlPointsPipeline();

    /// Specify the glyph that is displayed at each control point position.
    /// Keep in mind that the shape will be
    /// aligned with the constraining plane by orienting it such that
    /// the x axis of the geometry lies along the normal of the plane.
    //vtkSmartPointer<vtkPolyData> PointMarkerShape;
    vtkSmartPointer<vtkMarkupsGlyphSource2D> GlyphSource2D;
    vtkSmartPointer<vtkSphereSource> GlyphSourceSphere;

    vtkSmartPointer<vtkPolyData> ControlPointsPolyData;
    vtkSmartPointer<vtkPoints> ControlPoints;
    vtkSmartPointer<vtkPolyData> LabelControlPointsPolyData;
    vtkSmartPointer<vtkPoints> LabelControlPoints;
    vtkSmartPointer<vtkPointSetToLabelHierarchy> PointSetToLabelHierarchyFilter;
    vtkSmartPointer<vtkStringArray> Labels;
    vtkSmartPointer<vtkStringArray> LabelsPriority;
    vtkSmartPointer<vtkTextProperty> TextProperty;
  };

  double ControlPointSize;

  /// Helper function to add bounds of all listed actors to the supplied bounding box.
  /// additionalBounds is for convenience only, it allows defining additional bounds.
  void AddActorsBounds(vtkBoundingBox& bounds, const std::vector<vtkProp*> &actors, double* additionalBounds = nullptr);

  virtual void SetMarkupsNode(vtkMRMLMarkupsNode *markupsNode);

  vtkWeakPointer<vtkMRMLMarkupsDisplayNode> MarkupsDisplayNode;
  vtkWeakPointer<vtkMRMLMarkupsNode> MarkupsNode;

  // Selection tolerance for the picking of points
  double Tolerance;
  double PixelTolerance;

  bool NeedToRender;

  vtkSmartPointer<vtkPointPlacer> PointPlacer;

  int CurrentOperation;
  vtkTypeBool ClosedLoop;

  /// Convenience method.
  virtual bool GetAllControlPointsVisible();

  /// Convenience method.
  bool GetAllControlPointsSelected();

  // Given a world position and orientation, this computes the display position
  // using the renderer of this class.
  void GetRendererComputedDisplayPositionFromWorldPosition(const double worldPos[3],
                                                           double displayPos[2]);

  // Utility function to build straight lines between control points.
  // If displayPosition is true then positions will be computed in display coordinate system,
  // otherwise in world coordinate system.
  // displayPosition is normally set to true in 2D, and to false in 3D representations.
  void BuildLine(vtkPolyData* linePolyData, bool displayPosition);

  // This method is called when something changes in the point placer.
  // It will cause all points to be updated, and all lines to be regenerated.
  // It should be extended to detect changes in the line interpolator too.
//  virtual int  UpdateWidget(bool force = false);
  vtkTimeStamp WidgetBuildTime;

  vtkTimeStamp MarkupsTransformModifiedTime;

  void ComputeMidpoint(double p1[3], double p2[3], double mid[3])
  {
      mid[0] = (p1[0] + p2[0])/2;
      mid[1] = (p1[1] + p2[1])/2;
      mid[2] = (p1[2] + p2[2])/2;
  }

  vtkWeakPointer<vtkMRMLAbstractViewNode> ViewNode;

  // Calculate view scale factor
  void UpdateViewScaleFactor();

  double ViewScaleFactor;

  void UpdateRelativeCoincidentTopologyOffsets(vtkMapper* mapper);

  enum
  {
    Unselected,
    Selected,
    Active,
    NumberOfControlPointTypes
  };

  double* GetWidgetColor(int controlPointType);

  ControlPointsPipeline* ControlPoints[3]; // Unselected, Selected, Active

  vtkTypeBool AlwaysOnTop;

  // Temporary variable to store GetBounds() result
  double Bounds[6];

private:
  vtkSlicerAbstractWidgetRepresentation(const vtkSlicerAbstractWidgetRepresentation&) = delete;
  void operator=(const vtkSlicerAbstractWidgetRepresentation&) = delete;
};

#endif
