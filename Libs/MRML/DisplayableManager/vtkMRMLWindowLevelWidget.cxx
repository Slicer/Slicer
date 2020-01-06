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

#include "vtkMRMLWindowLevelWidget.h"

#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLCrosshairDisplayableManager.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceLayerLogic.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLRubberBandWidgetRepresentation.h"

#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkEvent.h"
#include "vtkGeneralTransform.h"
#include "vtkImageData.h"
#include "vtkImageClip.h"
#include "vtkImageHistogramStatistics.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkTransform.h"
#include "vtkWidgetEvent.h"

#include <deque>

vtkStandardNewMacro(vtkMRMLWindowLevelWidget);

//----------------------------------------------------------------------------------
vtkMRMLWindowLevelWidget::vtkMRMLWindowLevelWidget()
{
  this->StartEventPosition[0] = 0;
  this->StartEventPosition[1] = 0;

  this->PreviousEventPosition[0] = 0;
  this->PreviousEventPosition[1] = 0;

  this->VolumeScalarRange[0] = 0;
  this->VolumeScalarRange[1] = 0;

  this->IsStartVolumeAutoWindowLevel = false;
  this->StartVolumeWindowLevel[0] = 0;
  this->StartVolumeWindowLevel[1] = 0;
  this->LastVolumeWindowLevel[0] = 0;
  this->LastVolumeWindowLevel[1] = 0;
  this->WindowLevelAdjustedLayer = vtkMRMLSliceLogic::LayerBackground;

  this->BackgroundVolumeEditable = true;
  this->ForegroundVolumeEditable = true;

  this->CenteredRubberBand = true;

  this->AdjustMode = ModeAdjust;

  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier,
    WidgetStateAdjustWindowLevel, WidgetEventAdjustWindowLevelStart, WidgetEventAdjustWindowLevelEnd);
  this->SetKeyboardEventTranslation(WidgetStateAdjustWindowLevel, vtkEvent::NoModifier, 0, 0, "Escape", WidgetEventAdjustWindowLevelCancel);
  this->SetEventTranslation(WidgetStateAdjustWindowLevel, vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetEventAdjustWindowLevelCancel);

  this->SetEventTranslationClickAndDrag(WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::ControlModifier,
    WidgetStateAdjustWindowLevelAlternative, WidgetEventAdjustWindowLevelAlternativeStart, WidgetEventAdjustWindowLevelAlternativeEnd);
  this->SetKeyboardEventTranslation(WidgetStateAdjustWindowLevelAlternative,
    vtkEvent::AnyModifier, 0, 0, "Escape", WidgetEventAdjustWindowLevelAlternativeCancel);
  this->SetEventTranslation(WidgetStateAdjustWindowLevelAlternative, vtkCommand::RightButtonPressEvent, vtkEvent::AnyModifier,
    WidgetEventAdjustWindowLevelAlternativeCancel);

  this->SetEventTranslation(WidgetStateIdle, vtkCommand::LeftButtonDoubleClickEvent, vtkEvent::NoModifier, WidgetEventResetWindowLevel);
}

//----------------------------------------------------------------------------------
vtkMRMLWindowLevelWidget::~vtkMRMLWindowLevelWidget()
{
  this->SetMRMLApplicationLogic(nullptr);
}

//----------------------------------------------------------------------
void vtkMRMLWindowLevelWidget::CreateDefaultRepresentation()
{
  if (this->WidgetRep)
    {
    // already created
    return;
    }
  vtkNew<vtkMRMLRubberBandWidgetRepresentation> newRep;
  this->WidgetRep = newRep;
  this->WidgetRep->SetViewNode(this->GetSliceNode());
}

//-----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2)
{
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
    {
    return false;
    }
  if (!this->GetRepresentation())
    {
    return false;
    }

  // If we are currently dragging a point then we interact everywhere
  if (this->WidgetState == WidgetStateAdjustWindowLevel
    || this->WidgetState == WidgetStateAdjustWindowLevelAlternative)
    {
    distance2 = 0.0;
    return true;
    }

  // We can process this event but we let more specific widgets to claim it (if they are closer).
  // View adjust actions are set at 1e10, set a lower value in order to override them.
  distance2 = 1e9;
  return true;
}

//-----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = true;

  switch (widgetEvent)
    {
    case WidgetEventMouseMove:
    // click-and-dragging the mouse cursor
      processedEvent = this->ProcessMouseMove(eventData);
      break;
    case WidgetEventAdjustWindowLevelStart:
    case WidgetEventAdjustWindowLevelAlternativeStart:
      {
      vtkMRMLInteractionNode * interactionNode = this->GetInteractionNode();
      this->AdjustMode = -1;
      if (interactionNode)
        {
        this->AdjustMode = GetAdjustWindowLevelModeFromString(
          interactionNode->GetAttribute(GetInteractionNodeAdjustWindowLevelModeAttributeName()));
        }
      if (this->AdjustMode < 0)
        {
        // no valid mode is defined, use default
        this->AdjustMode = ModeAdjust;
        }
      // Control modifier indicates to use the alternative adjustment mode
      if (widgetEvent == WidgetEventAdjustWindowLevelAlternativeStart)
        {
        if (this->AdjustMode == ModeAdjust)
          {
          this->AdjustMode = ModeRectangleCentered;
          }
        else
          {
          this->AdjustMode = ModeAdjust;
          }
        }
      if (this->AdjustMode == ModeAdjust)
        {
        processedEvent = this->ProcessAdjustWindowLevelStart(eventData);
        }
      else
        {
        this->SetCenteredRubberBand(this->AdjustMode == ModeRectangleCentered);
        processedEvent = this->ProcessSetWindowLevelFromRegionStart(eventData);
        }
      }
      break;
    case WidgetEventAdjustWindowLevelEnd:
    case WidgetEventAdjustWindowLevelAlternativeEnd:
      if (this->AdjustMode == ModeAdjust)
        {
        processedEvent = this->ProcessEndMouseDrag(eventData);
        }
      else
        {
        processedEvent = this->ProcessSetWindowLevelFromRegionEnd(eventData);
        }
      break;
    case WidgetEventAdjustWindowLevelCancel:
      processedEvent = this->ProcessEndMouseDrag(eventData);
      this->SetVolumeWindowLevel(this->StartVolumeWindowLevel[0], this->StartVolumeWindowLevel[1], this->IsStartVolumeAutoWindowLevel);
      break;
    case WidgetEventResetWindowLevel:
      processedEvent = this->ProcessResetWindowLevel(eventData);
      break;
    case WidgetEventAdjustWindowLevelAlternativeCancel:
      processedEvent = this->ProcessSetWindowLevelFromRegionEnd(eventData, false);
      break;
    default:
      processedEvent = false;
    }

  return processedEvent;
}

//-------------------------------------------------------------------------
void vtkMRMLWindowLevelWidget::Leave(vtkMRMLInteractionEventData* eventData)
{
  this->SetWidgetState(WidgetStateIdle);
  if (this->WidgetRep)
    {
    this->WidgetRep->SetVisibility(false);
    }
  this->Superclass::Leave(eventData);
}

//-------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep || !eventData)
    {
    return false;
    }

  switch (this->WidgetState)
    {
    case WidgetStateAdjustWindowLevel:
      this->ProcessAdjustWindowLevel(eventData);
      break;
    case WidgetStateAdjustWindowLevelAlternative:
      this->ProcessSetWindowLevelFromRegion(eventData);
      break;
    }

  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  const int* displayPos = eventData->GetDisplayPosition();

  this->StartEventPosition[0] = displayPos[0];
  this->StartEventPosition[1] = displayPos[1];

  this->PreviousEventPosition[0] = this->StartEventPosition[0];
  this->PreviousEventPosition[1] = this->StartEventPosition[1];

  this->ProcessMouseMove(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  /*
  if (this->WidgetState == WidgetStateIdle)
    {
    return false;
    }
  */
  this->SetWidgetState(WidgetStateIdle);
  return true;
}

//----------------------------------------------------------------------------------
void vtkMRMLWindowLevelWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------------
void vtkMRMLWindowLevelWidget::SetMRMLApplicationLogic(vtkMRMLApplicationLogic* appLogic)
{
  if (appLogic == this->ApplicationLogic)
    {
    return;
    }
  this->Superclass::SetMRMLApplicationLogic(appLogic);
}

//----------------------------------------------------------------------------------
vtkMRMLSliceLogic* vtkMRMLWindowLevelWidget::GetSliceLogic()
{
  if (!this->SliceLogic && this->GetMRMLApplicationLogic())
    {
    this->SliceLogic = this->GetMRMLApplicationLogic()->GetSliceLogic(this->SliceNode);
    }
  return this->SliceLogic;
}


//----------------------------------------------------------------------------------
void vtkMRMLWindowLevelWidget::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (this->SliceNode == sliceNode)
    {
    // no change
    return;
    }
  this->SliceNode = sliceNode;
  if (this->WidgetRep)
    {
    this->WidgetRep->SetViewNode(sliceNode);
    }
  // Force update of slice logic
  this->SliceLogic = nullptr;
}

//----------------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLWindowLevelWidget::GetSliceNode()
{
  return this->SliceNode;
}

//----------------------------------------------------------------------------
void vtkMRMLWindowLevelWidget::ProcessAdjustWindowLevel(vtkMRMLInteractionEventData* eventData)
{
  const int* eventPosition = eventData->GetDisplayPosition();

  int deltaX = eventPosition[0] - this->PreviousEventPosition[0];
  int deltaY = eventPosition[1] - this->PreviousEventPosition[1];

  double rangeLow = this->VolumeScalarRange[0];
  double rangeHigh = this->VolumeScalarRange[1];

  int* windowSize = this->GetRenderer()->GetRenderWindow()->GetSize();
  double windowMinSize = std::min(windowSize[0], windowSize[1]);

  double gain = (rangeHigh - rangeLow) / windowMinSize;
  double newWindow = this->LastVolumeWindowLevel[0] + (gain * deltaX);
  if (newWindow < 0)
    {
    newWindow = 0;
    }
  double newLevel = this->LastVolumeWindowLevel[1] + (gain * deltaY);
  if (newLevel < rangeLow - newWindow / 2)
    {
    newLevel = rangeLow - newWindow / 2;
    }
  if (newLevel > rangeHigh + newWindow / 2)
    {
    newLevel = rangeHigh + newWindow / 2;
    }
  this->SetVolumeWindowLevel(newWindow, newLevel, false);
  this->LastVolumeWindowLevel[0] = newWindow;
  this->LastVolumeWindowLevel[1] = newLevel;

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];
  return;
}

//----------------------------------------------------------------------------
int vtkMRMLWindowLevelWidget::GetEditableLayerAtEventPosition(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return vtkMRMLSliceLogic::LayerNone;
    }
  vtkMRMLSliceNode *sliceNode = this->SliceLogic->GetSliceNode();
  if (!sliceNode)
    {
    return vtkMRMLSliceLogic::LayerNone;
    }
  vtkMRMLSliceCompositeNode *sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    return vtkMRMLSliceLogic::LayerNone;
    }

  bool foregroundEditable = this->VolumeWindowLevelEditable(sliceCompositeNode->GetForegroundVolumeID())
    && this->ForegroundVolumeEditable;
  bool backgroundEditable = this->VolumeWindowLevelEditable(sliceCompositeNode->GetBackgroundVolumeID())
    && this->BackgroundVolumeEditable;

  if (!foregroundEditable && !backgroundEditable)
    {
    // window/level editing is disabled on both volumes
    return vtkMRMLSliceLogic::LayerNone;
    }
  // By default adjust background volume, if available
  bool adjustForeground = !backgroundEditable;

  // If both foreground and background volumes are visible then choose adjustment of
  // foreground volume, if foreground volume is visible in current mouse position
  if (foregroundEditable && backgroundEditable)
    {
    adjustForeground = (sliceCompositeNode->GetForegroundOpacity() > 0.0)
      && this->IsEventInsideVolume(true, eventData)   // inside background (used as mask for displaying foreground)
      && this->IsEventInsideVolume(false, eventData); // inside foreground
    }

  return (adjustForeground ? vtkMRMLSliceLogic::LayerForeground : vtkMRMLSliceLogic::LayerBackground);
}

//----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::VolumeWindowLevelEditable(const char* volumeNodeID)
{
  if (!volumeNodeID)
    {
    return false;
    }
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }
  vtkMRMLScene *scene = sliceLogic->GetMRMLScene();
  if (!scene)
    {
    return false;
    }
  vtkMRMLVolumeNode* volumeNode =
    vtkMRMLVolumeNode::SafeDownCast(scene->GetNodeByID(volumeNodeID));
  if (volumeNode == nullptr)
    {
    return false;
    }
  vtkMRMLScalarVolumeDisplayNode* scalarVolumeDisplayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetDisplayNode());
  if (!scalarVolumeDisplayNode)
    {
    return false;
    }
  return !scalarVolumeDisplayNode->GetWindowLevelLocked();
}

//----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::ProcessAdjustWindowLevelStart(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }
  this->WindowLevelAdjustedLayer = vtkMRMLSliceLogic::LayerNone;
  vtkMRMLSliceCompositeNode *sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    return false;
    }
  int editedLayer = this->GetEditableLayerAtEventPosition(eventData);
  if (editedLayer != vtkMRMLSliceLogic::LayerForeground
    && editedLayer != vtkMRMLSliceLogic::LayerBackground)
    {
    return false;
    }
  sliceLogic->GetMRMLScene()->SaveStateForUndo();
  this->WindowLevelAdjustedLayer = editedLayer;
  if (editedLayer == vtkMRMLSliceLogic::LayerForeground)
    {
    sliceLogic->GetForegroundWindowLevelAndRange(
      this->LastVolumeWindowLevel[0], this->LastVolumeWindowLevel[1],
      this->VolumeScalarRange[0], this->VolumeScalarRange[1],
      this->IsStartVolumeAutoWindowLevel);
    }
  else if (editedLayer == vtkMRMLSliceLogic::LayerBackground)
    {
    sliceLogic->GetBackgroundWindowLevelAndRange(
      this->LastVolumeWindowLevel[0], this->LastVolumeWindowLevel[1],
      this->VolumeScalarRange[0], this->VolumeScalarRange[1],
      this->IsStartVolumeAutoWindowLevel);
    }
  this->StartVolumeWindowLevel[0] = this->LastVolumeWindowLevel[0];
  this->StartVolumeWindowLevel[1] = this->LastVolumeWindowLevel[1];
  this->SetWidgetState(WidgetStateAdjustWindowLevel);
  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkMRMLWindowLevelWidget::GetVolumeNodeFromSliceLayer(int editedLayer)
{
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return nullptr;
    }
  vtkMRMLSliceCompositeNode *sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  if (!sliceCompositeNode)
    {
    return nullptr;
    }
  vtkMRMLVolumeNode* volumeNode = nullptr;
  if (editedLayer == vtkMRMLSliceLogic::LayerForeground)
    {
    volumeNode = vtkMRMLVolumeNode::SafeDownCast(sliceLogic->GetMRMLScene()->GetNodeByID(sliceCompositeNode->GetForegroundVolumeID()));
    }
  else if (editedLayer == vtkMRMLSliceLogic::LayerBackground)
    {
    volumeNode = vtkMRMLVolumeNode::SafeDownCast(sliceLogic->GetMRMLScene()->GetNodeByID(sliceCompositeNode->GetBackgroundVolumeID()));
    }
  return volumeNode;
}

//----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::ProcessResetWindowLevel(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkMRMLVolumeNode* volumeNode = this->GetVolumeNodeFromSliceLayer(this->WindowLevelAdjustedLayer);
  if (!volumeNode)
    {
    return false;
    }
  vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetVolumeDisplayNode());
  if (!displayNode)
    {
    return false;
    }
  if (displayNode->GetAutoWindowLevel())
    {
    // auto-window/level is already enabled
    return true;
    }
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (sliceLogic)
    {
    sliceLogic->GetMRMLScene()->SaveStateForUndo();
    }
  displayNode->AutoWindowLevelOn();
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::ProcessSetWindowLevelFromRegionStart(vtkMRMLInteractionEventData* eventData)
{
  this->WindowLevelAdjustedLayer = this->GetEditableLayerAtEventPosition(eventData);
  if (this->WindowLevelAdjustedLayer == vtkMRMLSliceLogic::LayerNone)
    {
    return false;
    }
  vtkMRMLRubberBandWidgetRepresentation* rubberBand = vtkMRMLRubberBandWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (rubberBand)
    {
    const int* displayPos = eventData->GetDisplayPosition();
    rubberBand->SetCornerPoint1((int*)displayPos);
    rubberBand->SetCornerPoint2((int*)displayPos);
    rubberBand->SetVisibility(true);
    rubberBand->NeedToRenderOn();
    }
  this->SetWidgetState(WidgetStateAdjustWindowLevelAlternative);
  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------------
void vtkMRMLWindowLevelWidget::ProcessSetWindowLevelFromRegion(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLRubberBandWidgetRepresentation* rubberBand = vtkMRMLRubberBandWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rubberBand)
    {
    return;
    }
  const int* displayPos = eventData->GetDisplayPosition();

  if (this->CenteredRubberBand)
    {
    int radius[2] =
      {
      abs(displayPos[0] - this->StartEventPosition[0]),
      abs(displayPos[1] - this->StartEventPosition[1])
      };
    rubberBand->SetCornerPoint1(this->StartEventPosition[0] - radius[0], this->StartEventPosition[1] - radius[1]);
    rubberBand->SetCornerPoint2(this->StartEventPosition[0] + radius[0], this->StartEventPosition[1] + radius[1]);
    }
  else
    {
    rubberBand->SetCornerPoint1(this->StartEventPosition);
    rubberBand->SetCornerPoint2((int*)displayPos);
    }

  rubberBand->NeedToRenderOn();
}

//----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::ProcessSetWindowLevelFromRegionEnd(vtkMRMLInteractionEventData* eventData, bool updateWindowLevel/*=true*/)
{
  if (!this->ProcessEndMouseDrag(eventData))
    {
    return false;
    }
  vtkMRMLRubberBandWidgetRepresentation* rubberBand = vtkMRMLRubberBandWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (rubberBand)
    {
    rubberBand->SetVisibility(false);
    rubberBand->NeedToRenderOn();
    }
  if (updateWindowLevel)
    {
    return this->UpdateWindowLevelFromRectangle(this->WindowLevelAdjustedLayer, rubberBand->GetCornerPoint1(), rubberBand->GetCornerPoint2());
    }
  else
    {
    // cancelled
    return true;
    }
}

//----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::UpdateWindowLevelFromRectangle(int layer, int cornerPoint1[2], int cornerPoint2[2])
{
  if (cornerPoint1[0] == cornerPoint2[0]
    || cornerPoint1[1] == cornerPoint2[1])
    {
    // empty box
    return false;
    }
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return vtkMRMLSliceLogic::LayerNone;
    }
  vtkMRMLSliceNode *sliceNode = sliceLogic->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }
  vtkMRMLSliceLayerLogic* layerLogic = nullptr;
  if (layer == vtkMRMLSliceLogic::LayerBackground)
    {
    layerLogic = sliceLogic->GetBackgroundLayer();
    }
  else if (layer == vtkMRMLSliceLogic::LayerForeground)
    {
    layerLogic = sliceLogic->GetForegroundLayer();
    }
  if (!layerLogic)
    {
    return false;
    }

  // get the rubberband bounding box in ijk coordinates
  vtkGeneralTransform* xyToIJK = layerLogic->GetXYToIJKTransform();
  if (!xyToIJK)
    {
    return false;
    }
  vtkBoundingBox ijkBounds;
  ijkBounds.AddPoint(xyToIJK->TransformPoint(cornerPoint1[0], cornerPoint1[1], 0.0));
  ijkBounds.AddPoint(xyToIJK->TransformPoint(cornerPoint2[0], cornerPoint1[1], 0.0));
  ijkBounds.AddPoint(xyToIJK->TransformPoint(cornerPoint2[0], cornerPoint2[1], 0.0));
  ijkBounds.AddPoint(xyToIJK->TransformPoint(cornerPoint1[0], cornerPoint2[1], 0.0));

  // clamp the bounds to the dimensions of the label image
  vtkMRMLVolumeNode* volumeNode = layerLogic->GetVolumeNode();
  vtkImageData* imageData = (volumeNode ? volumeNode->GetImageData() : nullptr);
  if (!imageData || !imageData->GetPointData() || !imageData->GetPointData()->GetScalars())
    {
    // vtkImageHistogramStatistics crashes if there are no scalars
    return false;
    }
  double bounds[6] = { 0.0 };
  ijkBounds.GetBounds(bounds);
  int extent[6] = { 0 };
  for (int i = 0; i < 3; i++)
    {
    extent[i * 2] = std::max(static_cast<int>(std::floor(bounds[i * 2])), imageData->GetExtent()[i * 2]);
    extent[i * 2 + 1] = std::min(static_cast<int>(std::floor(bounds[i * 2 + 1])), imageData->GetExtent()[i * 2 + 1]);
    }

   // calculate the statistics for the selected region
  vtkNew<vtkImageClip> clip;
  clip->SetOutputWholeExtent(extent);
  clip->SetInputData(imageData);
  clip->ClipDataOn();
  vtkNew<vtkImageHistogramStatistics> stats;
  stats->SetInputConnection(clip->GetOutputPort());
  stats->Update();

  vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetVolumeDisplayNode());
  if (!displayNode)
    {
    return false;
    }
  displayNode->AutoWindowLevelOff();
  // Compute intensity range as 1th and 99th percentile, expanded by 1%.
  // It is more robust than taking the minimum and maximum - a few outlier voxels do not throw off the range.
  double* intensityRange = stats->GetAutoRange();
  displayNode->SetWindowLevelMinMax(intensityRange[0], intensityRange[1]);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::IsEventInsideVolume(bool background, vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLSliceLogic* sliceLogic = this->GetSliceLogic();
  if (!sliceLogic)
    {
    return false;
    }
  vtkMRMLSliceNode *sliceNode = sliceLogic->GetSliceNode();
  if (!sliceNode)
    {
    return false;
    }
  vtkMRMLSliceLayerLogic* layerLogic = background ?
    sliceLogic->GetBackgroundLayer() : sliceLogic->GetForegroundLayer();
  if (!layerLogic)
    {
    return false;
    }
  vtkMRMLVolumeNode* volumeNode = layerLogic->GetVolumeNode();
  if (!volumeNode || !volumeNode->GetImageData())
    {
    return false;
    }
  const int* eventPosition = eventData->GetDisplayPosition();
  double xyz[3] = { 0 };
  vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(this->GetRenderer(), sliceNode, eventPosition[0], eventPosition[1], xyz);
  vtkGeneralTransform* xyToBackgroundIJK = layerLogic->GetXYToIJKTransform();
  double mousePositionIJK[3] = { 0 };
  xyToBackgroundIJK->TransformPoint(xyz, mousePositionIJK);
  int volumeExtent[6] = { 0 };
  volumeNode->GetImageData()->GetExtent(volumeExtent);
  for (int i = 0; i < 3; i++)
    {
    if (mousePositionIJK[i]<volumeExtent[i * 2] || mousePositionIJK[i]>volumeExtent[i * 2 + 1])
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLWindowLevelWidget::SetVolumeWindowLevel(double window, double level, bool isAutoWindowLevel)
{
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    this->GetVolumeNodeFromSliceLayer(this->WindowLevelAdjustedLayer));
  if (!volumeNode)
    {
    return false;
    }
  vtkMRMLScalarVolumeDisplayNode* volumeDisplayNode = volumeNode->GetScalarVolumeDisplayNode();
  if (!volumeDisplayNode)
    {
    return false;
    }
  if (isAutoWindowLevel)
    {
    int disabledModify = volumeDisplayNode->StartModify();
    volumeDisplayNode->SetWindowLevel(window, level);
    volumeDisplayNode->SetAutoWindowLevel(1);
    volumeDisplayNode->EndModify(disabledModify);
    }
  else
    {
    int disabledModify = volumeDisplayNode->StartModify();
    volumeDisplayNode->SetAutoWindowLevel(0);
    volumeDisplayNode->SetWindowLevel(window, level);
    volumeDisplayNode->EndModify(disabledModify);
    }
  return true;
}

//-----------------------------------------------------------
const char* vtkMRMLWindowLevelWidget::GetAdjustWindowLevelModeAsString(int id)
{
  switch (id)
    {
    case ModeAdjust: return "Adjust";
    case ModeRectangle: return "Rectangle";
    case ModeRectangleCentered: return "RectangleCentered";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLWindowLevelWidget::GetAdjustWindowLevelModeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < Mode_Last; ii++)
    {
    if (strcmp(name, GetAdjustWindowLevelModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
}
