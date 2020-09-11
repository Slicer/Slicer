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
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

/**
 * @class   vtkSlicerPlaneRepresentation2D
 * @brief   Default representation for the plane widget
 *
 * This class provides the default concrete representation for the
 * vtkMRMLAbstractWidget. See vtkMRMLAbstractWidget
 * for details.
 * @sa
 * vtkSlicerMarkupsWidgetRepresentation2D vtkMRMLAbstractWidget
*/

#ifndef vtkSlicerPlaneRepresentation2D_h
#define vtkSlicerPlaneRepresentation2D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidgetRepresentation2D.h"
#include "vtkGlyphSource2D.h"

class vtkAppendPolyData;
class vtkClipPolyData;
class vtkCompositeDataGeometryFilter;
class vtkDiscretizableColorTransferFunction;
class vtkFeatureEdges;
class vtkMRMLInteractionEventData;
class vtkPlaneCutter;
class vtkPlaneSource;
class vtkSampleImplicitFunctionFilter;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerPlaneRepresentation2D : public vtkSlicerMarkupsWidgetRepresentation2D
{
public:
  /// Instantiate this class.
  static vtkSlicerPlaneRepresentation2D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerPlaneRepresentation2D,vtkSlicerMarkupsWidgetRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Subclasses of vtkContourCurveRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData = nullptr) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  int RenderOpaqueGeometry(vtkViewport *viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Return the bounds of the representation
  double *GetBounds() override;

  void CanInteract(vtkMRMLInteractionEventData* interactionEventData,
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) override;

  void CanInteractWithPlane(vtkMRMLInteractionEventData* interactionEventData,
    int& foundComponentType, int& foundComponentIndex, double& closestDistance2);

  bool GetTransformationReferencePoint(double referencePointWorld[3]) override;

  void BuildPlane();

  // Update visibility of interaction handles for representation
  void UpdateInteractionPipeline() override;

protected:
  vtkSlicerPlaneRepresentation2D();
  ~vtkSlicerPlaneRepresentation2D() override;

  virtual void UpdatePlaneFillColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3]);
  virtual void UpdatePlaneOutlineColorMap(vtkDiscretizableColorTransferFunction* colormap, double color[3]);

  vtkNew<vtkPlaneSource> PlaneFilter;
  vtkNew<vtkPlaneCutter> PlaneCutter;

  vtkNew<vtkClipPolyData> PlaneClipperSlicePlane;
  vtkNew<vtkClipPolyData> PlaneClipperStartFadeNear;
  vtkNew<vtkClipPolyData> PlaneClipperEndFadeNear;
  vtkNew<vtkClipPolyData> PlaneClipperStartFadeFar;
  vtkNew<vtkClipPolyData> PlaneClipperEndFadeFar;

  vtkNew<vtkCompositeDataGeometryFilter> PlaneCompositeFilter;
  vtkNew<vtkAppendPolyData> PlaneAppend;
  vtkNew<vtkTransformPolyDataFilter> PlaneWorldToSliceTransformer;
  vtkNew<vtkPolyDataMapper2D> PlaneFillMapper;
  vtkNew<vtkActor2D> PlaneFillActor;

  vtkNew<vtkFeatureEdges> PlaneOutlineFilter;
  vtkNew<vtkDiscretizableColorTransferFunction> PlaneOutlineColorMap;
  vtkNew<vtkTransformPolyDataFilter> PlaneOutlineWorldToSliceTransformer;
  vtkNew<vtkPolyDataMapper2D> PlaneOutlineMapper;
  vtkNew<vtkActor2D> PlaneOutlineActor;

  vtkNew<vtkGlyphSource2D> ArrowFilter;
  vtkNew<vtkGlyph2D> ArrowGlypher;
  vtkNew<vtkPolyDataMapper2D> ArrowMapper;
  vtkNew<vtkActor2D> ArrowActor;

  vtkNew<vtkDiscretizableColorTransferFunction> PlaneFillColorMap;
  vtkNew<vtkSampleImplicitFunctionFilter> PlaneSliceDistance;
  std::string LabelFormat;

private:
  vtkSlicerPlaneRepresentation2D(const vtkSlicerPlaneRepresentation2D&) = delete;
  void operator=(const vtkSlicerPlaneRepresentation2D&) = delete;
};

#endif
