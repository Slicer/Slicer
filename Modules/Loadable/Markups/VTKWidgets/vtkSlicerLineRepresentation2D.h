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
 * @class   vtkSlicerLineRepresentation2D
 * @brief   Default representation for the line widget
 *
 * This class provides the default concrete representation for the
 * vtkMRMLAbstractWidget. See vtkMRMLAbstractWidget
 * for details.
 * @sa
 * vtkSlicerMarkupsWidgetRepresentation2D vtkMRMLAbstractWidget
*/

#ifndef vtkSlicerLineRepresentation2D_h
#define vtkSlicerLineRepresentation2D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidgetRepresentation2D.h"

class vtkTubeFilter;
class vtkSampleImplicitFunctionFilter;

class vtkMRMLInteractionEventData;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerLineRepresentation2D : public vtkSlicerMarkupsWidgetRepresentation2D
{
public:
  /// Instantiate this class.
  static vtkSlicerLineRepresentation2D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerLineRepresentation2D,vtkSlicerMarkupsWidgetRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Subclasses of vtkContourCurveRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData=nullptr) override;

  void CanInteract(vtkMRMLInteractionEventData* interactionEventData,
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) override;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  int RenderOpaqueGeometry(vtkViewport *viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;


  /// Return the bounds of the representation
  double *GetBounds() override;

protected:
  vtkSlicerLineRepresentation2D();
  ~vtkSlicerLineRepresentation2D() override;

  void SetMarkupsNode(vtkMRMLMarkupsNode *markupsNode) override;

  /// Update interaction handle visibility for representation
  void UpdateInteractionPipeline() override;

  vtkSmartPointer<vtkPolyData> Line;
  vtkSmartPointer<vtkPolyDataMapper2D> LineMapper;
  vtkSmartPointer<vtkActor2D> LineActor;
  vtkSmartPointer<vtkDiscretizableColorTransferFunction> LineColorMap;

  vtkSmartPointer<vtkTubeFilter> TubeFilter;

  vtkSmartPointer<vtkTransformPolyDataFilter> WorldToSliceTransformer;
  vtkSmartPointer<vtkSampleImplicitFunctionFilter> SliceDistance;

private:
  vtkSlicerLineRepresentation2D(const vtkSlicerLineRepresentation2D&) = delete;
  void operator=(const vtkSlicerLineRepresentation2D&) = delete;
};

#endif
