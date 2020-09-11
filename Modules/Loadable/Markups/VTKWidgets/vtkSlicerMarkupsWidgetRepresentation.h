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
 * @class   vtkSlicerMarkupsWidgetRepresentation
 * @brief   Class for rendering a markups node
 *
 * This class can display a markups node in the scene.
 * It plays a similar role to vtkWidgetRepresentation, but it is
 * simplified and specialized for optimal use in Slicer.
 * It state is stored in the associated MRML display node to
 * avoid extra synchronization mechanisms.
 * The representation only observes MRML node changes,
 * it does not directly process any interaction events directly
 * (interaction events are processed by vtkMRMLAbstractWidget,
 * which then modifies MRML nodes).
 *
 * This class (and subclasses) are a type of
 * vtkProp; meaning that they can be associated with a vtkRenderer end
 * embedded in a scene like any other vtkActor.
*
 * @sa
 * vtkSlicerMarkupsWidgetRepresentation vtkMRMLAbstractWidget vtkPointPlacer
*/

#ifndef vtkSlicerMarkupsRepresentation_h
#define vtkSlicerMarkupsRepresentation_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkMRMLAbstractWidgetRepresentation.h"

#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsNode.h"

#include "vtkSmartPointer.h"

class vtkActor2D;
class vtkAppendPolyData;
class vtkArcSource;
class vtkArrayCalculator;
class vtkArrowSource;
class vtkConeSource;
class vtkGlyph3D;
class vtkMarkupsGlyphSource2D;
class vtkMRMLInteractionEventData;
class vtkPointPlacer;
class vtkPointSetToLabelHierarchy;
class vtkPolyDataMapper2D;
class vtkProperty2D;
class vtkRegularPolygonSource;
class vtkSphereSource;
class vtkTextActor;
class vtkTextProperty;
class vtkTensorGlyph;
class vtkTubeFilter;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerMarkupsWidgetRepresentation : public vtkMRMLAbstractWidgetRepresentation
{
public:
  enum
  {
    Unselected,
    Selected,
    Active,
    Project,
    ProjectBack,
    NumberOfControlPointTypes
  };

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerMarkupsWidgetRepresentation, vtkMRMLAbstractWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Update the representation from markups node
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData = nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection*) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOverlay(vtkViewport* viewport) override;
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport* viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Get number of control points.
  virtual int GetNumberOfControlPoints();

  /// Get the nth node's display position. Will return
  /// 1 on success, or 0 if there are not at least
  /// (n+1) nodes (0 based counting).
  virtual int GetNthControlPointDisplayPosition(int n, double pos[2]);

  /// Get the nth control point.
  virtual vtkMRMLMarkupsNode::ControlPoint *GetNthControlPoint(int n);

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
  virtual void CanInteract(vtkMRMLInteractionEventData* interactionEventData,
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2);

  virtual int FindClosestPointOnWidget(const int displayPos[2], double worldPos[3], int *idx);

  virtual vtkPointPlacer* GetPointPlacer();

  /// Get internal control points polydata - for testing purposes.
  /// controlPointType can be Unselected, Selected, Active, Project, ProjectBack.
  virtual vtkPolyData* GetControlPointsPolyData(int controlPointType);
  /// Get internal label control points polydata - for testing purposes.
  /// controlPointType can be Unselected, Selected, Active, Project, ProjectBack.
  virtual vtkPolyData* GetLabelControlPointsPolyData(int pipeline);
  /// Get internal labels list - for testing purposes.
  /// controlPointType can be Unselected, Selected, Active, Project, ProjectBack.
  virtual vtkStringArray* GetLabels(int pipeline);

  //@{
  /**
  * Returns true if the representation is displayable in the current view.
  * It takes into account current view node's display node and parent folder's visibility.
  */
  bool IsDisplayable();
  //@}

  /// Get the axis for the handle specified by the index
  virtual void GetInteractionHandleAxisWorld(int index, double axis[3]);
  /// Get the origin of the interaction handle widget
  virtual void GetInteractionHandleOriginWorld(double origin[3]);
  /// Get the position of the interaction handle in world coordinates
  /// Type is specified using vtkMRMLMarkupsDisplayNode::ComponentType
  virtual void GetInteractionHandlePositionWorld(int type, int index, double position[3]);
  /// Get the direction vector of the interaction handle from the interaction origin
  /// Type is specified using vtkMRMLMarkupsDisplayNode::ComponentType
  virtual void GetInteractionHandleVector(int type, int index, double axis[3]);
  /// Get the direction vector of the interaction handle from the interaction origin in world coordinates
  virtual void GetInteractionHandleVectorWorld(int type, int index, double axis[3]);

protected:
  vtkSlicerMarkupsWidgetRepresentation();
  ~vtkSlicerMarkupsWidgetRepresentation() override;

  // Convert glyph types from display node enums to 2D glyph source enums
  static int GetGlyphTypeSourceFromDisplay(int glyphTypeDisplay);

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

  class MarkupsInteractionPipeline
  {
  public:
    MarkupsInteractionPipeline(vtkMRMLAbstractWidgetRepresentation* representation);
    virtual ~MarkupsInteractionPipeline();

    vtkWeakPointer<vtkMRMLAbstractWidgetRepresentation> Representation;

    vtkSmartPointer<vtkSphereSource>                    AxisRotationHandleSource;
    vtkSmartPointer<vtkArcSource>                       AxisRotationArcSource;
    vtkSmartPointer<vtkTubeFilter>                      AxisRotationTubeFilter;
    vtkSmartPointer<vtkAppendPolyData>                  AxisRotationGlyphSource;

    vtkSmartPointer<vtkArrowSource>                     AxisTranslationGlyphSource;
    vtkSmartPointer<vtkTransformPolyDataFilter>         AxisTranslationGlyphTransformer;

    vtkSmartPointer<vtkPolyData>                        RotationHandlePoints;
    vtkSmartPointer<vtkPolyData>                        TranslationHandlePoints;

    vtkSmartPointer<vtkTransformPolyDataFilter>         RotationScaleTransform;
    vtkSmartPointer<vtkTransformPolyDataFilter>         TranslationScaleTransform;

    vtkSmartPointer<vtkTensorGlyph>                     AxisRotationGlypher;
    vtkSmartPointer<vtkGlyph3D>                         AxisTranslationGlypher;

    vtkSmartPointer<vtkAppendPolyData>                  Append;
    vtkSmartPointer<vtkTransformPolyDataFilter>         HandleToWorldTransformFilter;
    vtkSmartPointer<vtkTransform>                       HandleToWorldTransform;
    vtkSmartPointer<vtkLookupTable>                     ColorTable;
    vtkSmartPointer<vtkPolyDataMapper2D>                Mapper;
    vtkSmartPointer<vtkActor2D>                         Actor;
    vtkSmartPointer<vtkProperty2D>                      Property;

    double                                              StartFadeAngle;
    double                                              EndFadeAngle;

    virtual void InitializePipeline();
    virtual void CreateRotationHandles();
    virtual void CreateTranslationHandles();
    virtual void UpdateHandleColors();

    /// Set the scale of the interaction handles in world coordinates
    virtual void SetWidgetScale(double scale);
    /// Get the color of the specified handle
    /// Type is specified using vtkMRMLMarkupsDisplayNode::ComponentType
    virtual void GetHandleColor(int type, int index, double color[4]);
    /// Get the opacity of the specified handle
    virtual double GetOpacity(int type, int index);

    /// Get the direction of the specified axis in world coordinates
    virtual void GetInteractionHandleAxisWorld(int index, double axis[3]);
    /// Get the origin of the interaction widget in world coordinates
    virtual void GetInteractionHandleOriginWorld(double origin[3]);
    /// Get the view plane normal for the widget in world coordinates
    virtual void GetViewPlaneNormal(double normal[3]);

    struct HandleInfo
    {
      HandleInfo(int index, int componentType, double positionWorld[3], double positionLocal[3], double color[4])
        : Index(index)
        , ComponentType(componentType)
      {
      for (int i = 0; i < 3; ++i)
        {
        this->PositionWorld[i] = positionWorld[i];
        }
      this->PositionWorld[3] = 1.0;
      for (int i = 0; i < 3; ++i)
        {
        this->PositionLocal[i] = positionLocal[i];
        }
      this->PositionLocal[3] = 1.0;
      for (int i = 0; i < 4; ++i)
        {
        this->Color[i] = color[i];
        }
      }
      int Index;
      int ComponentType;
      double PositionLocal[4];
      double PositionWorld[4];
      double Color[4];
      bool IsVisible()
        {
        double epsilon = 0.001;
        return this->Color[3] > epsilon;
        }
    };

    /// Get the list of info for all interaction handles
    std::vector<HandleInfo> GetHandleInfoList();
  };
  typedef std::vector<MarkupsInteractionPipeline::HandleInfo> HandleInfoList;

  // Calculate view size and scale factor
  virtual void UpdateViewScaleFactor() = 0;

  virtual void UpdateControlPointSize() = 0;

  double ViewScaleFactorMmPerPixel;
  double ScreenSizePixel; // diagonal size of the screen

  // Control point size, specified in renderer world coordinate system.
  // For slice views, renderer world coordinate system is the display coordinate system, so it is measured in pixels.
  // For 3D views, renderer world coordinate system is the Slicer world coordinate system, so it is measured in the
  // scene length unit (typically millimeters).
  double ControlPointSize;

  virtual void SetMarkupsNode(vtkMRMLMarkupsNode *markupsNode);

  vtkWeakPointer<vtkMRMLMarkupsDisplayNode> MarkupsDisplayNode;
  vtkWeakPointer<vtkMRMLMarkupsNode> MarkupsNode;

  vtkSmartPointer<vtkPointPlacer> PointPlacer;

  vtkSmartPointer<vtkTextActor> TextActor;

  vtkTypeBool ClosedLoop;

  /// Convenience method.
  virtual bool GetAllControlPointsVisible();

  /// Convenience method.
  bool GetAllControlPointsSelected();

  // Utility function to build straight lines between control points.
  // If displayPosition is true then positions will be computed in display coordinate system,
  // otherwise in world coordinate system.
  // displayPosition is normally set to true in 2D, and to false in 3D representations.
  void BuildLine(vtkPolyData* linePolyData, bool displayPosition);

  vtkTimeStamp MarkupsTransformModifiedTime;

  double* GetWidgetColor(int controlPointType);

  ControlPointsPipeline* ControlPoints[NumberOfControlPointTypes]; // Unselected, Selected, Active, Project, ProjectBehind

  virtual void SetupInteractionPipeline();
  MarkupsInteractionPipeline* InteractionPipeline;

  /// Update the interaction pipeline
  virtual void UpdateInteractionPipeline();

private:
  vtkSlicerMarkupsWidgetRepresentation(const vtkSlicerMarkupsWidgetRepresentation&) = delete;
  void operator=(const vtkSlicerMarkupsWidgetRepresentation&) = delete;
};

#endif
