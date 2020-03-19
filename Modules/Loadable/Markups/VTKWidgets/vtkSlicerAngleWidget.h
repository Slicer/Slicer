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
 * @class   vtkSlicerAngleWidget
 * @brief   create an angle with a set of 3 points
 *
 * The vtkSlicerAngleWidget is used to create an angle widget with a set of 3 points.
 *
*/

#ifndef vtkSlicerAngleWidget_h
#define vtkSlicerAngleWidget_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidget.h"

class vtkSlicerMarkupsWidgetRepresentation;
class vtkPolyData;
class vtkIdList;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerAngleWidget : public vtkSlicerMarkupsWidget
{
public:
  /// Instantiate this class.
  static vtkSlicerAngleWidget *New();

  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkSlicerAngleWidget,vtkSlicerMarkupsWidget);

  /// Create the default widget representation and initializes the widget and representation.
  void CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer) override;

protected:
  vtkSlicerAngleWidget();
  ~vtkSlicerAngleWidget() override;

private:
  vtkSlicerAngleWidget(const vtkSlicerAngleWidget&) = delete;
  void operator=(const vtkSlicerAngleWidget&) = delete;
};

#endif
