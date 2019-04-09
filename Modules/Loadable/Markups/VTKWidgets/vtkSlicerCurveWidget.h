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
 * @class   vtkSlicerCurveWidget
 * @brief   create a curve with a set of N points
 *
 * The vtkSlicerCurveWidget is used to create a curve widget with a set of N points.
 *
*/

#ifndef vtkSlicerCurveWidget_h
#define vtkSlicerCurveWidget_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidget.h"

class vtkPolyData;
class vtkIdList;
class vtkMRMLMarkupsCurveNode;

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerCurveWidget : public vtkSlicerMarkupsWidget
{
public:
  /// Instantiate this class.
  static vtkSlicerCurveWidget *New();

  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkSlicerCurveWidget, vtkSlicerMarkupsWidget);

  /// Create the default widget representation and initializes the widget and representation.
  void CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer) override;

protected:
  vtkSlicerCurveWidget();
  ~vtkSlicerCurveWidget() override;

  bool ProcessControlPointInsert(vtkMRMLInteractionEventData* eventData) override;

  vtkMRMLMarkupsCurveNode* GetMarkupsCurveNode();

private:
  vtkSlicerCurveWidget(const vtkSlicerCurveWidget&) = delete;
  void operator=(const vtkSlicerCurveWidget&) = delete;
};

#endif
