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
  static vtkSlicerROIWidget *New();

  /// Standard methods for a VTK class.
  vtkTypeMacro(vtkSlicerROIWidget,vtkSlicerMarkupsWidget);

  /// Widget states
  enum
  {
    WidgetStateSymmetricScale = WidgetStateMarkups_Last,
    WidgetStateMarkupsROI_Last
  };

  // Widget events
  enum
  {
    WidgetEventSymmetricScaleStart = WidgetEventMarkups_Last,
    WidgetEventSymmetricScaleEnd,
    WidgetEventMarkupsROI_Last
  };

  /// Create the default widget representation and initializes the widget and representation.
  void CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer) override;

  /// Flip the selected index across the specified axis.
  /// Ex. Switch between L--R face.
  /// Used when the user drags an ROI handle across the ROI origin.
  void FlipROIHandles(bool flipLRHandle, bool flipAPHandle, bool flipISHandle);

  /// Create instance of the markups widget
  vtkSlicerMarkupsWidgetCreateInstanceMacro(vtkSlicerROIWidget);

protected:
  vtkSlicerROIWidget();
  ~vtkSlicerROIWidget() override;

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;
  bool ProcessWidgetSymmetricScaleStart(vtkMRMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData) override;
  bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData) override;

  void ScaleWidget(double eventPos[2]) override;
  void ScaleWidget(double eventPos[2], bool symmetricScale);

private:
  vtkSlicerROIWidget(const vtkSlicerROIWidget&) = delete;
  void operator=(const vtkSlicerROIWidget&) = delete;
};

#endif
