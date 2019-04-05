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
 * @class   vtkMRMLWindowLevelWidget
 * @brief   Show slice intersection lines
 *
 * The vtkMRMLWindowLevelWidget allows moving slices by interacting with
 * displayed slice intersecrion lines.
 *
 *
*/

#ifndef vtkMRMLWindowLevelWidget_h
#define vtkMRMLWindowLevelWidget_h

#include "vtkMRMLDisplayableManagerExport.h" // For export macro
#include "vtkMRMLAbstractWidget.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"

#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkSliceIntersectionRepresentation2D;
class vtkMRMLApplicationLogic;
class vtkMRMLSegmentationDisplayNode;


class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLWindowLevelWidget : public vtkMRMLAbstractWidget
{
public:
  /**
   * Instantiate this class.
   */
  static vtkMRMLWindowLevelWidget *New();

  //@{
  /**
   * Standard VTK class macros.
   */
  vtkTypeMacro(vtkMRMLWindowLevelWidget, vtkMRMLAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
   * Create the default widget representation if one is not set.
   */
  void CreateDefaultRepresentation();

  void SetSliceNode(vtkMRMLSliceNode* sliceNode);
  vtkMRMLSliceNode* GetSliceNode();

  vtkMRMLSliceLogic* GetSliceLogic();

  vtkGetMacro(BackgroundVolumeEditable, bool);
  vtkSetMacro(BackgroundVolumeEditable, bool);
  vtkBooleanMacro(BackgroundVolumeEditable, bool);

  vtkGetMacro(ForegroundVolumeEditable, bool);
  vtkSetMacro(ForegroundVolumeEditable, bool);
  vtkBooleanMacro(ForegroundVolumeEditable, bool);

  void SetMRMLApplicationLogic(vtkMRMLApplicationLogic* applicationLogic) override;

  /// Return true if the widget can process the event.
  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2) override;

  /// Process interaction event.
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  /// Called when the the widget loses the focus.
  void Leave() override;

  /// Widget states
  enum
    {
    WidgetStateAdjustWindowLevel = WidgetStateUser,
    WidgetStateSetWindowLevelFromRegion,
    };

  /// Widget events
  enum
    {
    WidgetEventResetWindowLevel = WidgetEventUser,
    WidgetEventAdjustWindowLevelStart,
    WidgetEventAdjustWindowLevelEnd,
    WidgetEventAdjustWindowLevelCancel,
    WidgetEventSetWindowLevelFromRegionStart,
    WidgetEventSetWindowLevelFromRegionEnd,
    WidgetEventSetWindowLevelFromRegionCancel,
    };

  bool UpdateWindowLevelFromRectangle(int layer, int cornerPoint1[2], int cornerPoint2[2]);

  /// Rubberband is centered around the click position
  vtkGetMacro(CenteredRubberBand, bool);
  vtkSetMacro(CenteredRubberBand, bool);
  vtkBooleanMacro(CenteredRubberBand, bool);

protected:
  vtkMRMLWindowLevelWidget();
  ~vtkMRMLWindowLevelWidget() override;

  bool ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData);
  bool ProcessMouseMove(vtkMRMLInteractionEventData* eventData);
  bool ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData);

  bool ProcessAdjustWindowLevelStart(vtkMRMLInteractionEventData* eventData);
  void ProcessAdjustWindowLevel(vtkMRMLInteractionEventData* eventData);

  bool ProcessSetWindowLevelFromRegionStart(vtkMRMLInteractionEventData* eventData);
  void ProcessSetWindowLevelFromRegion(vtkMRMLInteractionEventData* eventData);
  // If updateWindowLevel is set to false then the operation is cancelled without changing the window/level
  bool ProcessSetWindowLevelFromRegionEnd(vtkMRMLInteractionEventData* eventData, bool updateWindowLevel=true);

  bool ProcessResetWindowLevel(vtkMRMLInteractionEventData* eventData);

  int GetEditableLayerAtEventPosition(vtkMRMLInteractionEventData* eventData);

  /// Returns true if mouse is inside the selected layer volume.
  /// Use background flag to choose between foreground/background layer.
  bool IsEventInsideVolume(bool background, vtkMRMLInteractionEventData* eventData);

  /// Returns true if the volume's window/level values are editable
  /// on the GUI
  bool VolumeWindowLevelEditable(const char* volumeNodeID);

  vtkMRMLVolumeNode* GetVolumeNodeFromSliceLayer(int editedLayer);

  bool SetVolumeWindowLevel(double window, double level, bool isAutoWindowLevel);

  vtkWeakPointer<vtkMRMLSliceNode> SliceNode;
  vtkWeakPointer<vtkMRMLSliceLogic> SliceLogic;

  bool CenteredRubberBand;

  int StartEventPosition[2];
  int PreviousEventPosition[2];

  double VolumeScalarRange[2];

  bool IsStartVolumeAutoWindowLevel;
  double StartVolumeWindowLevel[2];
  double LastVolumeWindowLevel[2];

  int WindowLevelAdjustedLayer;

  bool BackgroundVolumeEditable;
  bool ForegroundVolumeEditable;

private:
  vtkMRMLWindowLevelWidget(const vtkMRMLWindowLevelWidget&) = delete;
  void operator=(const vtkMRMLWindowLevelWidget&) = delete;
};

#endif
