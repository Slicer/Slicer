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
 * @class   vtkSlicerAngleRepresentation3D
 * @brief   Default representation for the angle widget
 *
 * This class provides the default concrete representation for the
 * vtkMRMLAbstractWidget. See vtkMRMLAbstractWidget
 * for details.
 * @sa
 * vtkSlicerMarkupsWidgetRepresentation3D vtkMRMLAbstractWidget
*/

#ifndef vtkSlicerAngleRepresentation3D_h
#define vtkSlicerAngleRepresentation3D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidgetRepresentation3D.h"

class vtkAppendPolyData;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkTubeFilter;
class vtkPropPicker;
class vtkTextActor;
class vtkArcSource;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerAngleRepresentation3D : public vtkSlicerMarkupsWidgetRepresentation3D
{
public:
  /// Instantiate this class.
  static vtkSlicerAngleRepresentation3D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerAngleRepresentation3D,vtkSlicerMarkupsWidgetRepresentation3D);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Subclasses of vtkMRMLAbstractWidgetRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData=NULL) VTK_OVERRIDE;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) VTK_OVERRIDE;
  void ReleaseGraphicsResources(vtkWindow *) VTK_OVERRIDE;
  int RenderOverlay(vtkViewport *viewport) VTK_OVERRIDE;
  int RenderOpaqueGeometry(vtkViewport *viewport) VTK_OVERRIDE;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) VTK_OVERRIDE;
  vtkTypeBool HasTranslucentPolygonalGeometry() VTK_OVERRIDE;

  /// Return the bounds of the representation
  double *GetBounds() VTK_OVERRIDE;

  virtual bool GetTransformationReferencePoint(double referencePointWorld[3]) VTK_OVERRIDE;

  void CanInteract(const int displayPosition[2], const double worldPosition[3],
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) VTK_OVERRIDE;

protected:
  vtkSlicerAngleRepresentation3D();
  ~vtkSlicerAngleRepresentation3D() VTK_OVERRIDE;

  vtkSmartPointer<vtkPolyData>                Line;
  vtkSmartPointer<vtkPolyDataMapper>    LineMapper;
  vtkSmartPointer<vtkActor>             LineActor;

  vtkSmartPointer<vtkArcSource>               Arc;
  vtkSmartPointer<vtkPolyDataMapper>    ArcMapper;
  vtkSmartPointer<vtkActor>             ArcActor;

  vtkSmartPointer<vtkTextActor>         TextActor;

  vtkSmartPointer<vtkTubeFilter>              TubeFilter;
  vtkSmartPointer<vtkTubeFilter>              ArcTubeFilter;

  std::string LabelFormat;

  void BuildArc();

  // Support picking
  vtkSmartPointer<vtkPropPicker> LinePicker;

private:
  vtkSlicerAngleRepresentation3D(const vtkSlicerAngleRepresentation3D&) = delete;
  void operator=(const vtkSlicerAngleRepresentation3D&) = delete;
};

#endif
