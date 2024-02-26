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
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

/**
 * @class   vtkSlicerROIWidget
 * @brief   Create an ROI representation
 *
 * The vtkSlicerROIWidget is used to create an ROI widget.
 *
 */

#ifndef vtkSlicerROIWidget_h
#define vtkSlicerROIWidget_h

#include "vtkSlicerMarkupsModuleVTKWidgetsExport.h"
#include "vtkSlicerMarkupsWidget.h"

class VTK_SLICER_MARKUPS_MODULE_VTKWIDGETS_EXPORT vtkSlicerROIWidget : public vtkSlicerMarkupsWidget
{
public:
  /// Instantiate this class.
  static vtkSlicerROIWidget* New();

  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkSlicerROIWidget, vtkSlicerMarkupsWidget);

  /// Create the default widget representation and initializes the widget and representation.
  void CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode,
                                   vtkMRMLAbstractViewNode* viewNode,
                                   vtkRenderer* renderer) override;

  /// Create instance of the markups widget
  vtkSlicerMarkupsWidgetCreateInstanceMacro(vtkSlicerROIWidget);

protected:
  vtkSlicerROIWidget();
  ~vtkSlicerROIWidget() override;

private:
  vtkSlicerROIWidget(const vtkSlicerROIWidget&) = delete;
  void operator=(const vtkSlicerROIWidget&) = delete;
};

#endif
