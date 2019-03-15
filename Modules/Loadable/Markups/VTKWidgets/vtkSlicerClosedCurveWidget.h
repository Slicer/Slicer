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
 * @class   vtkSlicerClosedCurveWidget
 * @brief   create a curve with a set of N points
 *
 * The vtkSlicerClosedCurveWidget is used to create a closed
 * curve widget with a set of N points.
 * The widget handles all processing of widget
 * events (that are triggered by VTK events). The vtkSlicerCurveRepresentation is
 * responsible for all placement of the points, and
 * curve manipulation. This is done through a main helper class:
 * vtkPointPlacer. The representation is also
 * responsible for drawing the points.
 *
 * @par Event Bindings:
 * By default, the widget responds to the following VTK events (i.e., it
 * watches the vtkRenderWindowInteractor for these events):
 * <pre>
 *   LeftButtonPressEvent - triggers a Select event
 *   Alt + LeftButtonPressEvent - triggers a Rotate event
 *   MiddleButtonPressEvent - triggers a Translate event
 *   RightButtonPressEvent - triggers a Pick event
 *   Alt + RightButtonPressEvent - triggers a Scale event
 *
 *   MouseMoveEvent - triggers a Move event
 *
 *   LeftButtonReleaseEvent - triggers an EndAction event
 *   MiddleButtonReleaseEvent - triggers an EndAction event
 *   RightButtonReleaseEvent - triggers an EndAction event
 *
 *   LeftButtonDoubleClickEvent - triggers an Pick event
 *   MiddleButtonDoubleClickEvent - triggers an Pick event
 *   RightButtonDoubleClickEvent - triggers an Pick event
 *
 *   Delete key event - triggers a Delete event
 *   Shift + Delete key event - triggers a Reset event
 * </pre>
 *
 *
*/

#ifndef vtkSlicerClosedCurveWidget_h
#define vtkSlicerClosedCurveWidget_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidget.h"

class vtkPolyData;
class vtkIdList;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerClosedCurveWidget : public vtkSlicerMarkupsWidget
{
public:
  /// Instantiate this class.
  static vtkSlicerClosedCurveWidget *New();

  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkSlicerClosedCurveWidget, vtkSlicerMarkupsWidget);

  /// Create the default widget representation and initializes the widget and representation.
  void CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer) override;

protected:
  vtkSlicerClosedCurveWidget();
  ~vtkSlicerClosedCurveWidget() override;

  // Callback interface to capture evets when
  // placing the widget.
  static void AddPointOnCurveAction(vtkAbstractWidget*);

private:
  vtkSlicerClosedCurveWidget(const vtkSlicerClosedCurveWidget&) = delete;
  void operator=(const vtkSlicerClosedCurveWidget&) = delete;
};

#endif
