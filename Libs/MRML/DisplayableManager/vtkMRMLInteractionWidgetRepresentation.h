/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

///
/// \class vtkMRMLInteractionWidgetRepresentation
/// \brief Class for rendering interaction handles
///
/// This class can display interaction handles in the scene.
/// It plays a similar role to vtkWidgetRepresentation, but it is
/// simplified and specialized for optimal use in Slicer.
/// It state is stored in the associated MRML display node to
/// avoid extra synchronization mechanisms.
/// The representation only observes MRML node changes,
/// it does not directly process any interaction events directly
/// (interaction events are processed by vtkMRMLAbstractWidget,
/// which then modifies MRML nodes).
///
/// This class (and subclasses) are a type of vtkProp; meaning that
/// they can be associated with a vtkRenderer and embedded in a scene
/// like any other vtkActor.
///
/// \sa vtkMRMLInteractionWidgetRepresentation vtkMRMLAbstractWidget vtkPointPlacer
///

#ifndef vtkSlicerInteractionRepresentation_h
#define vtkSlicerInteractionRepresentation_h

// MRMLDM includes
#include "vtkMRMLDisplayableManagerExport.h"
#include "vtkMRMLAbstractWidgetRepresentation.h"

// MRML includes
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkActor.h>
#include <vtkFloatArray.h>
#include <vtkGlyph3DMapper.h>
#include <vtkIntArray.h>
#include <vtkPlane.h>
#include <vtkPointPlacer.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkUnsignedCharArray.h>
#include <vtkWeakPointer.h>

// STD includes
#include <map>
#include <memory>
#include <vector>

class vtkMRMLInteractionEventData;
class vtkMRMLDisplayNode;
class vtkMRMLDisplayableNode;
struct SharedHandleRenderer;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLInteractionWidgetRepresentation : public vtkMRMLAbstractWidgetRepresentation
{
public:
  ///@{
  /// Standard VTK class macros.
  vtkTypeMacro(vtkMRMLInteractionWidgetRepresentation, vtkMRMLAbstractWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  ///@}

  /// Update the representation from display node
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void* callData = nullptr) override;

  /// Set the renderer that the representation is displayed in.
  /// Unregisters the representation from the renderer that it was previously displayed in, so that its
  /// interaction handles are no longer rendered there.
  void SetRenderer(vtkRenderer* renderer) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection*) override;
  void ReleaseGraphicsResources(vtkWindow*) override;
  int RenderOverlay(vtkViewport* viewport) override;
  int RenderOpaqueGeometry(vtkViewport* viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport* viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Translation, rotation, scaling will happen around this position
  virtual bool GetTransformationReferencePoint(double referencePointWorld[3]);

  /// Return found component type (as vtkMRMLInteractionDisplayNode::ComponentType).
  /// closestDistance2 is the squared distance in display coordinates from the closest position where interaction is possible.
  /// componentIndex returns index of the found component (e.g., if control point is found then control point index is returned).
  virtual void CanInteract(vtkMRMLInteractionEventData* interactionEventData, int& foundComponentType, int& foundComponentIndex, double& closestDistance2);

  virtual vtkPointPlacer* GetPointPlacer();

  /// Returns the per-representation actor for the interaction widget.
  /// In slice views this is the actor that renders the handles.
  /// In 3D views the handles are drawn by a shared per-renderer actor and only
  /// this actor's visibility is honored; property, mapper, or transform changes
  /// on the returned actor have no effect on the rendered handles.
  vtkProp* GetInteractionActor();

  /// Returns true if the representation is displayable in the current view.
  /// It takes into account current view node's display node and parent folder's visibility.
  virtual bool IsDisplayable() = 0;

  virtual vtkMRMLSliceNode* GetSliceNode();
  virtual void GetSliceToWorldCoordinates(const double slicePos[2], double worldPos[3]);

  virtual void UpdateSlicePlaneFromSliceNode();

  /// Update the interaction pipeline
  virtual void UpdateInteractionPipeline();

  /// Get the axis for the handle specified by the index in local coordinates
  virtual void GetInteractionHandleAxisLocal(int type, int index, double axis[3]);

  /// Get the axis for the handle specified by the index
  virtual void GetInteractionHandleAxisWorld(int type, int index, double axis[3]);
  /// Get the origin of the interaction handle widget
  virtual void GetInteractionHandleOriginWorld(double origin[3]);

  enum
  {
    InteractionNone,
    InteractionTranslationHandle,
    InteractionRotationHandle,
    InteractionScaleHandle,
    Interaction_Last
  };

  enum
  {
    GlyphArrow,
    GlyphCircle,
    GlyphRing,
    GlyphCrosshair,
    GlyphArrowOutline,
    GlyphCircleOutline,
    GlyphRingOutline,
    GlyphCrosshairOutline
  };

  virtual int GetActiveComponentType() = 0;
  virtual void SetActiveComponentType(int type) = 0;

  virtual int GetActiveComponentIndex() = 0;
  virtual void SetActiveComponentIndex(int index) = 0;

  virtual double GetMaximumHandlePickingDistance2();

  virtual vtkTransform* GetHandleToWorldTransform();

  virtual int GetNumberOfHandles(int type);
  virtual int GetNumberOfHandles();

  vtkSetMacro(Interacting, bool);
  vtkGetMacro(Interacting, bool);
  vtkBooleanMacro(Interacting, bool);

  /// Get the color of the specified handle
  /// Type is specified using InteractionType enum
  virtual void GetHandleColor(int type, int index, double color[4]);
  /// Get the opacity of the specified handle
  virtual double GetHandleOpacity(int type, int index);
  /// Get the overall opacity multiplier applied to all interaction handles. Default is 1.0.
  virtual double GetInteractionHandleOpacity();
  /// Get the factor that the translation vector is scaled by during an interactive translate
  /// operation. Default is 1.0 (no scaling).
  virtual double GetTranslationScaleFactor();
  /// Get the visibility of the specified handle
  virtual bool GetHandleVisibility(int type, int index);
  ///  Get the type of glyph (Arrow, Circle, Ring, etc.) of the specified handle.
  virtual int GetHandleGlyphType(int type, int index);
  /// Get if the view scaling should be applied to the position of the handle.
  virtual bool GetApplyScaleToPosition(int type, int index);
  /// Get the position of the interaction handle in local coordinates
  /// Type is specified using vtkMRMLInteractionDisplayNode::ComponentType
  virtual void GetInteractionHandlePositionLocal(int type, int index, double position[3]);
  /// Get the position of the interaction handle in world coordinates
  /// Type is specified using vtkMRMLInteractionDisplayNode::ComponentType
  virtual void GetInteractionHandlePositionWorld(int type, int index, double position[3]);

  ///@{
  /// Update HandleToWorldTransform. This controls the position and orientation of the interaction handles.
  virtual void UpdateHandleToWorldTransform();
  virtual void UpdateHandleToWorldTransform(vtkTransform* handleToWorldTransform) = 0;
  ///@}

protected:
  vtkMRMLInteractionWidgetRepresentation();
  ~vtkMRMLInteractionWidgetRepresentation() override;

  class VTK_MRML_DISPLAYABLEMANAGER_EXPORT InteractionPipeline
  {
  public:
    InteractionPipeline();
    virtual ~InteractionPipeline();

    vtkSmartPointer<vtkPolyData> ArrowPolyData;
    vtkSmartPointer<vtkPolyData> CirclePolyData;
    vtkSmartPointer<vtkPolyData> RingPolyData;
    vtkSmartPointer<vtkPolyData> CrosshairPolyData;
    vtkSmartPointer<vtkPolyData> ArrowOutlinePolyData;
    vtkSmartPointer<vtkPolyData> CircleOutlinePolyData;
    vtkSmartPointer<vtkPolyData> RingOutlinePolyData;
    vtkSmartPointer<vtkPolyData> CrosshairOutlinePolyData;

    vtkSmartPointer<vtkPolyData> RotationHandlePoints;
    vtkSmartPointer<vtkPolyData> TranslationHandlePoints;
    vtkSmartPointer<vtkPolyData> ScaleHandlePoints;

    vtkSmartPointer<vtkTransform> HandleToWorldTransform;

    vtkSmartPointer<vtkPolyData> InstancePolyData;
    vtkSmartPointer<vtkFloatArray> GlyphOrientationArray;
    vtkSmartPointer<vtkFloatArray> GlyphScaleArray;
    vtkSmartPointer<vtkIntArray> GlyphSourceIndexArray;
    vtkSmartPointer<vtkUnsignedCharArray> ColorArray;

    vtkSmartPointer<vtkGlyph3DMapper> Mapper;
    vtkSmartPointer<vtkProperty> Property;
    vtkSmartPointer<vtkActor> Actor;

    vtkSmartPointer<vtkTransform> WorldToSliceTransform;
    vtkSmartPointer<vtkTransform> ActorTransform;
  };

  struct HandleInfo
  {
    HandleInfo(int index, int componentType, double positionWorld[3], double positionLocal[3], double color[4], int glyphType, bool applyScaleToPosition)
      : Index(index)
      , ComponentType(componentType)
      , GlyphType(glyphType)
      , ApplyScaleToPosition(applyScaleToPosition)
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
    int GlyphType;
    double PositionLocal[4];
    double PositionWorld[4];
    double Color[4];
    bool ApplyScaleToPosition{ true };
    bool IsVisible()
    {
      double epsilon = 0.001;
      return this->Color[3] > epsilon;
    }
  };

  /// Get the list of info for all interaction handles
  typedef std::vector<HandleInfo> HandleInfoList;
  virtual HandleInfoList GetHandleInfoList();
  virtual HandleInfo GetHandleInfo(int type, int index);

  virtual void InitializePipeline();
  virtual void CreateRotationHandles();
  virtual void CreateTranslationHandles();
  virtual void CreateScaleHandles();
  virtual void UpdateInstanceArrays();
  virtual void UpdateActorTransform();
  virtual vtkPolyData* GetHandlePolydata(int type);

  /// Cache camera state (direction, position, view up) for use by per-handle computations.
  virtual void UpdateCameraState();

  /// Set the scale of the interaction handles in world coordinates
  virtual void SetWidgetScale(double scale);

  /// Get the vector from the interaction handle to the camera in world coordinates.
  /// In slice views and in 3D with parallel projection this is the same as the camera view direction.
  /// Uses cached camera state from UpdateCameraState().
  virtual void GetHandleToCameraVectorWorld(double handlePosition_World[3], double normal_World[3]);

  /// Orthogonalize the transform axes. The Z-axis will not be changed.
  virtual void OrthoganalizeTransform(vtkTransform* transform);

  /// Calculate view size and scale factor
  virtual void UpdateViewScaleFactor();

  virtual void UpdateHandleSize();
  virtual double GetInteractionScalePercent();
  virtual double GetInteractionSizeMm();
  virtual bool GetInteractionSizeAbsolute();

  virtual void CanInteractWithCircleHandle(vtkMRMLInteractionEventData* interactionEventData,
                                           int& foundComponentType,
                                           int& foundComponentIndex,
                                           double& closestDistance2,
                                           HandleInfo& handleInfo);
  virtual void CanInteractWithArrowHandle(vtkMRMLInteractionEventData* interactionEventData,
                                          int& foundComponentType,
                                          int& foundComponentIndex,
                                          double& closestDistance2,
                                          HandleInfo& handleInfo);
  virtual void CanInteractWithRingHandle(vtkMRMLInteractionEventData* interactionEventData,
                                         int& foundComponentType,
                                         int& foundComponentIndex,
                                         double& closestDistance2,
                                         HandleInfo& handleInfo);

  /// The angle between the camera and the interaction axis at which point the interaction handle starts to fade out.
  double StartFadeAngleDegrees{ 10.0 };
  /// The angle between the camera and the interaction axis at which point the interaction handle is completely faded out.
  double EndFadeAngleDegrees{ 8.0 };

  double ViewScaleFactorMmPerPixel;
  double ScreenSizePixel; // diagonal size of the screen

  vtkSmartPointer<vtkPlane> SlicePlane;

  /// Handle size, specified in renderer world coordinate system.
  /// For slice views, renderer world coordinate system is the display coordinate system, so it is measured in pixels.
  /// For 3D views, renderer world coordinate system is the Slicer world coordinate system, so it is measured in the
  /// scene length unit (typically millimeters).
  double InteractionSize{ 1.0 };

  double WidgetScale{ 1.0 };

  bool CachedParallelProjection{ true };
  double CachedCameraPosition[3]{ 0.0, 0.0, 0.0 };
  double CachedCameraDirection[3]{ 0.0, 0.0, 1.0 };
  double CachedViewUp_World[3]{ 0.0, 1.0, 0.0 };

  /// State that the handle instance arrays were last built for. Rebuilding the arrays is relatively
  /// expensive, so it is only done when this state changes.
  /// The camera state is compared by value rather than by MTime, because the camera is modified on
  /// every render by clipping range updates, which do not affect the appearance of the handles.
  /// Clipping range is intentionally excluded: the clamp in UpdateViewScaleFactor only changes the
  /// result when the handle is behind/inside the near plane (not visible), and the camera pose change
  /// that causes that already reopens the gate.
  struct RenderState
  {
    vtkMTimeType RepresentationMTime{ 0 };
    vtkMTimeType SliceXYToRASMTime{ 0 };
    double CameraPosition[3]{ 0.0, 0.0, 0.0 };
    double CameraDirectionOfProjection[3]{ 0.0, 0.0, 0.0 };
    double CameraViewUp[3]{ 0.0, 0.0, 0.0 };
    double CameraParallelScale{ 0.0 };
    double CameraViewAngle{ 0.0 };
    bool CameraParallelProjection{ false };
    int RendererSize[2]{ 0, 0 };
    int ScreenSize[2]{ 0, 0 };
    int ActiveComponentType{ -1 };
    int ActiveComponentIndex{ -1 };

    bool operator==(const RenderState& other) const;
    bool operator!=(const RenderState& other) const { return !(*this == other); }
  };

  /// Get the state that the handle instance arrays would be built for if they were rebuilt now.
  RenderState GetCurrentRenderState();

  /// Rebuild the handle instance arrays if the state that they were built for has changed.
  /// Returns true if the arrays have been rebuilt.
  bool UpdatePipelineIfNeeded();

  RenderState LastRenderState;
  bool HasVisibleHandles{ false };

  bool Interacting{ false };

  vtkSmartPointer<vtkPointPlacer> PointPlacer;

  virtual void SetupInteractionPipeline();
  InteractionPipeline* Pipeline;

  ///@{
  /// Shared handle renderer: in 3D views, all interaction handle instances are rendered by a single
  /// shared actor per vtkRenderer (one instanced draw per glyph source instead of one actor per
  /// handle). The map is keyed by renderer pointer; entries are created on first use and removed
  /// when the last representation unregisters or when the renderer fires DeleteEvent.
  static std::map<vtkRenderer*, std::unique_ptr<SharedHandleRenderer>> SharedHandleRenderers;
  static SharedHandleRenderer* GetSharedHandleRenderer(vtkRenderer* renderer, bool create = false);
  static void OnSharedRendererStartEvent(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
  static void OnSharedRendererDelete(vtkObject* caller, unsigned long eid, void* clientData, void* callData);
  void RegisterWithSharedHandleRenderer();
  void UnregisterFromSharedHandleRenderer();
  bool IsSharedRenderingRepresentation(SharedHandleRenderer* shared) const;
  static void ConfigureHandleInstancePipeline(vtkSmartPointer<vtkPolyData>& instancePolyData,
                                              vtkSmartPointer<vtkFloatArray>& orientationArray,
                                              vtkSmartPointer<vtkFloatArray>& scaleArray,
                                              vtkSmartPointer<vtkIntArray>& sourceIndexArray,
                                              vtkSmartPointer<vtkUnsignedCharArray>& colorArray,
                                              vtkSmartPointer<vtkGlyph3DMapper>& mapper,
                                              vtkSmartPointer<vtkProperty>& property,
                                              vtkSmartPointer<vtkActor>& actor);
  static unsigned char ColorComponentToUChar(double c);
  ///@}

private:
  vtkMRMLInteractionWidgetRepresentation(const vtkMRMLInteractionWidgetRepresentation&) = delete;
  void operator=(const vtkMRMLInteractionWidgetRepresentation&) = delete;
};

#endif
