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
 * @class   vtkSlicerAngleRepresentation2D
 * @brief   Default representation for the line widget
 *
 * This class provides the default concrete representation for the
 * vtkMRMLAbstractWidget. See vtkMRMLAbstractWidget
 * for details.
 * @sa
 * vtkSlicerMarkupsWidgetRepresentation2D vtkMRMLAbstractWidget
*/

#ifndef vtkSlicerAngleRepresentation2D_h
#define vtkSlicerAngleRepresentation2D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidgetRepresentation2D.h"

class vtkActor2D;
class vtkAppendPolyData;
class vtkPolyDataMapper2D;
class vtkPolyData;
class vtkProperty2D;
class vtkTubeFilter;
class vtkArcSource;
class vtkTextActor;
class vtkVectorText;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerAngleRepresentation2D : public vtkSlicerMarkupsWidgetRepresentation2D
{
public:
  /// Instantiate this class.
  static vtkSlicerAngleRepresentation2D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerAngleRepresentation2D,vtkSlicerMarkupsWidgetRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Subclasses of vtkContourCurveRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData = NULL) VTK_OVERRIDE;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) override;
  void ReleaseGraphicsResources(vtkWindow *) override;
  int RenderOverlay(vtkViewport *viewport) override;
  int RenderOpaqueGeometry(vtkViewport *viewport) override;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) override;
  vtkTypeBool HasTranslucentPolygonalGeometry() override;

  /// Return the bounds of the representation
  double *GetBounds() override;

  void CanInteract(const int displayPosition[2], const double worldPosition[3],
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) VTK_OVERRIDE;

  virtual bool GetTransformationReferencePoint(double referencePointWorld[3]) VTK_OVERRIDE;

protected:
  vtkSlicerAngleRepresentation2D();
  ~vtkSlicerAngleRepresentation2D() override;

  vtkSmartPointer<vtkPolyData>                  Line;
  vtkSmartPointer<vtkPolyDataMapper2D>    LineMapper;
  vtkSmartPointer<vtkActor2D>                   LineActor;

  vtkSmartPointer<vtkArcSource>                 Arc;
  vtkSmartPointer<vtkPolyDataMapper2D>    ArcMapper;
  vtkSmartPointer<vtkActor2D>                   ArcActor;

  vtkSmartPointer<vtkTextActor>           TextActor;

  vtkSmartPointer<vtkTubeFilter>                TubeFilter;
  vtkSmartPointer<vtkTubeFilter>                ArcTubeFilter;

  std::string LabelFormat;

  void BuildArc();

private:
  vtkSlicerAngleRepresentation2D(const vtkSlicerAngleRepresentation2D&) = delete;
  void operator=(const vtkSlicerAngleRepresentation2D&) = delete;
};

#endif
