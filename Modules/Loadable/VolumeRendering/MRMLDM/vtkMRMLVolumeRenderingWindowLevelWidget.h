/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

/**
 * @class   vtkMRMLVolumeRenderingWindowLevelWidget
 * @brief   Adjust volume rendering transfer functions with mouse interactions
 *
 * The vtkMRMLVolumeRenderingWindowLevelWidget allows adjusting volume rendering
 * transfer functions by dragging the mouse in 3D views. Horizontal drag shifts
 * the transfer functions, vertical drag scales them.
 *
 */

#ifndef vtkMRMLVolumeRenderingWindowLevelWidget_h
#define vtkMRMLVolumeRenderingWindowLevelWidget_h

#include "vtkSlicerVolumeRenderingModuleMRMLDisplayableManagerExport.h"
#include "vtkMRMLAbstractWidget.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkMRMLViewNode;
class vtkMRMLVolumeRenderingDisplayNode;
class vtkMRMLVolumeRenderingDisplayableManager;
class vtkMRMLVolumePropertyNode;
class vtkPiecewiseFunction;
class vtkColorTransferFunction;

class VTK_SLICER_VOLUMERENDERING_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLVolumeRenderingWindowLevelWidget : public vtkMRMLAbstractWidget
{
public:
  static vtkMRMLVolumeRenderingWindowLevelWidget* New();
  vtkTypeMacro(vtkMRMLVolumeRenderingWindowLevelWidget, vtkMRMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create the default widget representation
  void SetMRMLViewNode(vtkMRMLViewNode* viewNode);

  /// Set the volume rendering displayable manager
  void SetVolumeRenderingDisplayableManager(vtkMRMLVolumeRenderingDisplayableManager* manager);

  /// Return true if the widget can process the interaction event
  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2) override;

  /// Process interaction event
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  /// Called when the widget loses input focus
  void Leave(vtkMRMLInteractionEventData* eventData) override;

  /// Widget states
  enum
  {
    WidgetStateWindowLevelAdjust = WidgetStateUser, // Normal window/level adjustment mode
    WidgetStateOpacityAdjust,                       // Ctrl+drag opacity adjustment mode
  };

  /// Widget events
  enum
  {
    WidgetEventClickAndDragStart = WidgetEventUser,
    WidgetEventClickAndDragEnd,
  };

protected:
  vtkMRMLVolumeRenderingWindowLevelWidget();
  ~vtkMRMLVolumeRenderingWindowLevelWidget() override;

  bool ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData);
  bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData);
  bool ProcessReset(vtkMRMLInteractionEventData* eventData);

  bool ProcessAdjustTransferFunctionStart(vtkMRMLInteractionEventData* eventData);
  void ProcessAdjustTransferFunction(vtkMRMLInteractionEventData* eventData);

  /// Adjust transfer functions for a specific volume property node
  void AdjustVolumeRenderingTransferFunctions(vtkMRMLVolumePropertyNode* volumePropertyNode, double deltaX, double deltaY, int modifiers = 0);

private:
  vtkMRMLVolumeRenderingWindowLevelWidget(const vtkMRMLVolumeRenderingWindowLevelWidget&) = delete;
  void operator=(const vtkMRMLVolumeRenderingWindowLevelWidget&) = delete;

  int StartEventPosition[2];
  int PreviousEventPosition[2];

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
