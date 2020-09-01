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
 * @class   vtkMRMLAbstractWidgetRepresentation
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
 * vtkMRMLAbstractWidgetRepresentation vtkMRMLAbstractWidget
*/

#ifndef vtkMRMLAbstractRepresentation_h
#define vtkMRMLAbstractRepresentation_h

#include "vtkMRMLDisplayableManagerExport.h"
#include "vtkWidgetRepresentation.h"

#include "vtkMRMLAbstractViewNode.h"

#include <vector>

class vtkMapper;

#include "vtkBoundingBox.h"

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractWidgetRepresentation : public vtkProp
{
public:
  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkMRMLAbstractWidgetRepresentation, vtkProp);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //@{
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

  //@{
  /**
  * Set the renderer in which the representations draws itself.
  * Typically the renderer is set by the associated widget.
  * Use the widget's SetCurrentRenderer() method in most cases;
  * otherwise there is a risk of inconsistent behavior as events
  * and drawing may be performed in different viewports.
  * WARNING: The renderer is NOT reference counted by the representation,
  * in order to avoid reference loops.  Be sure that the representation
  * lifetime does not extend beyond the renderer lifetime.
  */
  virtual void SetRenderer(vtkRenderer *ren);
  virtual vtkRenderer* GetRenderer();
  //@}

  //@{
  /**
  * Set the view node where this widget is displayed.
  */
  virtual void SetViewNode(vtkMRMLAbstractViewNode* viewNode);
  virtual vtkMRMLAbstractViewNode* GetViewNode();
  //@}

  /**
  * UpdateFromMRML() - update the widget from its state stored in MRML.
  * if event is non-zero then a specific update (faster, smaller scope) is performed instead
  * of a full update.
  */
  virtual void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData = nullptr);

  /// Specify tolerance for performing pick operations of points.
  /// For display renderers it is defined in pixels. The specified value is scaled with ScreenScaleFactor.
  /// For VR renderer it is defined in millimeters. The specified value is scaled with WorldToPhysicalScale.
  vtkSetMacro(PickingTolerance, double);
  vtkGetMacro(PickingTolerance, double);

  /// Controls whether the widget should always appear on top
  /// of other actors in the scene. (In effect, this will disable OpenGL
  /// Depth buffer tests while rendering the widget).
  /// Default is to set it to false.
  vtkSetMacro(AlwaysOnTop, bool);
  vtkGetMacro(AlwaysOnTop, bool);
  vtkBooleanMacro(AlwaysOnTop, bool);

  //@{
  /**
  * The widget representation can set this data member to true to indicate that it needs to be re-rendered.
  * If the rendering request is processed then the flag is cleared.
  */
  vtkGetMacro(NeedToRender, bool);
  vtkSetMacro(NeedToRender, bool);
  vtkBooleanMacro(NeedToRender, bool);
  //@}

 protected:
  vtkMRMLAbstractWidgetRepresentation();
  ~vtkMRMLAbstractWidgetRepresentation() override;

  /// Helper function to add bounds of all listed actors to the supplied bounding box.
  /// additionalBounds is for convenience only, it allows defining additional bounds.
  void AddActorsBounds(vtkBoundingBox& bounds, const std::vector<vtkProp*> &actors, double* additionalBounds = nullptr);

  /// Given a world position and orientation, this computes the display position
  /// using the renderer of this class.
  void GetRendererComputedDisplayPositionFromWorldPosition(const double worldPos[3], double displayPos[2]);

  virtual void UpdateRelativeCoincidentTopologyOffsets(vtkMapper* mapper);

  /// The renderer in which this widget is placed
  vtkWeakPointer<vtkRenderer> Renderer;

  bool NeedToRender;

  /// Tolerance for performing pick operations of points.
  /// For display renderers it is defined in pixels. The specified value is scaled with ScreenScaleFactor.
  /// For VR renderer it is defined in millimeters. The specified value is scaled with WorldToPhysicalScale.
  double PickingTolerance;

  /// Allows global rescaling of all widgets (to compensate for larger or smaller physical screen size)
  double ScreenScaleFactor;

  vtkWeakPointer<vtkMRMLAbstractViewNode> ViewNode;

  bool AlwaysOnTop;

  /// Temporary variable to store GetBounds() result
  double Bounds[6];

private:
  vtkMRMLAbstractWidgetRepresentation(const vtkMRMLAbstractWidgetRepresentation&) = delete;
  void operator=(const vtkMRMLAbstractWidgetRepresentation&) = delete;
};

#endif
