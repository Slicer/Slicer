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
 * @class   vtkSlicerLineRepresentation3D
 * @brief   Default representation for the line widget
 *
 * This class provides the default concrete representation for the
 * vtkSlicerAbstractWidget. See vtkSlicerAbstractWidget
 * for details.
 * @sa
 * vtkSlicerAbstractWidgetRepresentation3D vtkSlicerAbstractWidget
*/

#ifndef vtkSlicerLineRepresentation3D_h
#define vtkSlicerLineRepresentation3D_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerAbstractWidgetRepresentation3D.h"

class vtkAppendPolyData;
class vtkOpenGLActor;
class vtkOpenGLPolyDataMapper;
class vtkPolyData;
class vtkTubeFilter;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerLineRepresentation3D : public vtkSlicerAbstractWidgetRepresentation3D
{
public:
  /// Instantiate this class.
  static vtkSlicerLineRepresentation3D *New();

  /// Standard methods for instances of this class.
  vtkTypeMacro(vtkSlicerLineRepresentation3D,vtkSlicerAbstractWidgetRepresentation3D);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Subclasses of vtkSlicerAbstractWidgetRepresentation must implement these methods. These
  /// are the methods that the widget and its representation use to
  /// communicate with each other.
  void UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void *callData=NULL) VTK_OVERRIDE;

  void CanInteract(const int displayPosition[2], const double worldPosition[3],
    int &foundComponentType, int &foundComponentIndex, double &closestDistance2) VTK_OVERRIDE;

  /// Methods to make this class behave as a vtkProp.
  void GetActors(vtkPropCollection *) VTK_OVERRIDE;
  void ReleaseGraphicsResources(vtkWindow *) VTK_OVERRIDE;
  int RenderOverlay(vtkViewport *viewport) VTK_OVERRIDE;
  int RenderOpaqueGeometry(vtkViewport *viewport) VTK_OVERRIDE;
  int RenderTranslucentPolygonalGeometry(vtkViewport *viewport) VTK_OVERRIDE;
  vtkTypeBool HasTranslucentPolygonalGeometry() VTK_OVERRIDE;

  /// Return the bounds of the representation
  double *GetBounds() VTK_OVERRIDE;

protected:
  vtkSlicerLineRepresentation3D();
  ~vtkSlicerLineRepresentation3D() VTK_OVERRIDE;

  vtkSmartPointer<vtkPolyData> Line;
  vtkSmartPointer<vtkOpenGLPolyDataMapper> LineMapper;
  vtkSmartPointer<vtkOpenGLActor> LineActor;

  vtkSmartPointer<vtkTubeFilter> TubeFilter;

private:
  vtkSlicerLineRepresentation3D(const vtkSlicerLineRepresentation3D&) = delete;
  void operator=(const vtkSlicerLineRepresentation3D&) = delete;
};

#endif
