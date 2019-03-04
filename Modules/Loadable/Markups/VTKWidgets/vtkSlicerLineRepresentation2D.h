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
 * vtkSlicerAbstractWidget. See vtkSlicerAbstractWidget
 * for details.
 * @sa
 * vtkSlicerAbstractWidgetRepresentation2D vtkSlicerAbstractWidget
*/

#ifndef vtkSlicerLineRepresentation2D_h
#define vtkSlicerLineRepresentation2D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerAbstractWidgetRepresentation2D.h"

class vtkActor2D;
class vtkAppendPolyData;
class vtkOpenGLPolyDataMapper2D;
class vtkPolyData;
class vtkProperty2D;
class vtkTubeFilter;
class vtkPropPicker;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerLineRepresentation2D : public vtkSlicerAbstractWidgetRepresentation2D
{
public:
  /// Instantiate this class.
  static vtkSlicerLineRepresentation2D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerLineRepresentation2D,vtkSlicerAbstractWidgetRepresentation2D);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Subclasses of vtkContourCurveRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData=NULL) VTK_OVERRIDE;

  void CanInteract(const int displayPosition[2], const double worldPosition[3],
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) VTK_OVERRIDE;

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

  vtkSmartPointer<vtkPolyData> Line;
  vtkSmartPointer<vtkOpenGLPolyDataMapper2D> LineMapper;
  vtkSmartPointer<vtkActor2D> LineActor;

  vtkSmartPointer<vtkTubeFilter> TubeFilter;

private:
  vtkSlicerLineRepresentation2D(const vtkSlicerLineRepresentation2D&) = delete;
  void operator=(const vtkSlicerLineRepresentation2D&) = delete;
};

#endif
