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
 * @class   vtkSlicerLineWidget
 * @brief   create a line with a set of 2 points
 *
 * The vtkSlicerLineWidget is used to create a line widget with a set of 2 points.
 *
*/

#ifndef vtkSlicerLineWidget_h
#define vtkSlicerLineWidget_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidget.h"

class vtkSlicerMarkupsWidgetRepresentation;
class vtkPolyData;
class vtkIdList;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerLineWidget : public vtkSlicerMarkupsWidget
{
public:
  /// Instantiate this class.
  static vtkSlicerLineWidget *New();

  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkSlicerLineWidget,vtkSlicerMarkupsWidget);

  /// Create the default widget representation and initializes the widget and representation.
  void CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer) override;

protected:
  vtkSlicerLineWidget();
  ~vtkSlicerLineWidget() override;

private:
  vtkSlicerLineWidget(const vtkSlicerLineWidget&) = delete;
  void operator=(const vtkSlicerLineWidget&) = delete;
};

#endif
