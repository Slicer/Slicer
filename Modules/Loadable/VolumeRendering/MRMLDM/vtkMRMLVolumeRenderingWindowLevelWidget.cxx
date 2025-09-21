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

#include "vtkMRMLVolumeRenderingWindowLevelWidget.h"

// VolumeRendering MRMLDM includes
#include "vtkMRMLVolumeRenderingDisplayableManager.h"

// VolumeRendering MRML includes
#include <vtkMRMLVolumePropertyNode.h>
#include <vtkMRMLVolumeRenderingDisplayNode.h>

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLInteractionEventData.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkColorTransferFunction.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkWidgetEvent.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>

vtkStandardNewMacro(vtkMRMLVolumeRenderingWindowLevelWidget);

//----------------------------------------------------------------------
class vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal
{
public:
  // Structure to store original transfer function state
  struct OriginalTransferFunctionState
  {
    std::vector<double> ScalarOpacityPoints; // x, y pairs
    std::vector<double> ColorTransferPoints; // x, r, g, b quadruples
    double ScalarRange[2]{ 0.0, 0.0 };
    double CenterPoint{ 0.0 };
  };

  vtkInternal(vtkMRMLVolumeRenderingWindowLevelWidget* external);
  ~vtkInternal();

  vtkMRMLScene* GetMRMLScene();

  /// Save original state of transfer functions for all modified display nodes
  void SaveOriginalTransferFunctionStates();

  /// Restore original state of transfer functions for all modified display nodes
  void RestoreOriginalTransferFunctionStates();

  /// Restore the state of a single volume property node's transfer functions
  void RestoreTransferFunctionState(vtkMRMLVolumePropertyNode* volumePropertyNode, const OriginalTransferFunctionState& state);

  /// Save the state of a single volume property node's transfer functions
  void SaveTransferFunctionState(vtkMRMLVolumeNode* volumeNode, vtkMRMLVolumePropertyNode* volumePropertyNode, OriginalTransferFunctionState& state);

  /// Get all visible volume rendering display nodes in the current view
  void GetVisibleVolumeRenderingDisplayNodes(std::vector<vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>>& displayNodes, int maximumNumberOfItems);

  vtkMRMLVolumeRenderingWindowLevelWidget* External;
  vtkWeakPointer<vtkMRMLViewNode> ViewNode;
  vtkWeakPointer<vtkMRMLVolumeRenderingDisplayableManager> DisplayableManager;
  std::vector<vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>> ModifiedDisplayNodes;
  std::map<vtkMRMLVolumePropertyNode*, OriginalTransferFunctionState> OriginalStates;
};

//----------------------------------------------------------------------
vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::vtkInternal(vtkMRMLVolumeRenderingWindowLevelWidget* external)
{
  this->External = external;
}

//----------------------------------------------------------------------
vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::~vtkInternal() = default;

//----------------------------------------------------------------------
vtkMRMLScene* vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::GetMRMLScene()
{
  if (this->ViewNode)
  {
    return this->ViewNode->GetScene();
  }
  return nullptr;
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::SaveOriginalTransferFunctionStates()
{
  this->OriginalStates.clear();
  for (vtkMRMLVolumeRenderingDisplayNode* displayNode : this->ModifiedDisplayNodes)
  {
    vtkMRMLVolumeNode* volumeNode = displayNode->GetVolumeNode();
    vtkMRMLVolumePropertyNode* volumePropertyNode = displayNode->GetVolumePropertyNode();
    if (volumeNode && volumeNode->GetImageData() && volumePropertyNode)
    {
      OriginalTransferFunctionState state;
      this->SaveTransferFunctionState(volumeNode, volumePropertyNode, state);
      this->OriginalStates[volumePropertyNode] = state;
    }
  }
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::RestoreOriginalTransferFunctionStates()
{
  for (vtkMRMLVolumeRenderingDisplayNode* displayNode : this->ModifiedDisplayNodes)
  {
    vtkMRMLVolumePropertyNode* volumePropertyNode = displayNode->GetVolumePropertyNode();
    if (volumePropertyNode)
    {
      auto it = this->OriginalStates.find(volumePropertyNode);
      if (it != this->OriginalStates.end())
      {
        this->RestoreTransferFunctionState(volumePropertyNode, it->second);
      }
    }
  }
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::RestoreTransferFunctionState(vtkMRMLVolumePropertyNode* volumePropertyNode, const OriginalTransferFunctionState& state)
{
  if (!volumePropertyNode)
  {
    return;
  }

  vtkPiecewiseFunction* scalarOpacity = volumePropertyNode->GetScalarOpacity();
  vtkColorTransferFunction* colorTransferFunction = volumePropertyNode->GetColor();

  if (scalarOpacity && !state.ScalarOpacityPoints.empty())
  {
    // Restore scalar opacity function from original state
    scalarOpacity->RemoveAllPoints();
    int numOpacityPoints = static_cast<int>(state.ScalarOpacityPoints.size()) / 2;
    scalarOpacity->FillFromDataPointer(numOpacityPoints, const_cast<double*>(state.ScalarOpacityPoints.data()));
  }

  if (colorTransferFunction && !state.ColorTransferPoints.empty())
  {
    // Restore color transfer function from original state
    colorTransferFunction->RemoveAllPoints();
    int numColorPoints = static_cast<int>(state.ColorTransferPoints.size()) / 4;
    colorTransferFunction->FillFromDataPointer(numColorPoints, const_cast<double*>(state.ColorTransferPoints.data()));
  }

  // Trigger update
  volumePropertyNode->Modified();
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::SaveTransferFunctionState(vtkMRMLVolumeNode* volumeNode,
                                                                                     vtkMRMLVolumePropertyNode* volumePropertyNode,
                                                                                     OriginalTransferFunctionState& state)
{
  if (!volumePropertyNode)
  {
    return;
  }

  vtkPiecewiseFunction* scalarOpacity = volumePropertyNode->GetScalarOpacity();
  vtkColorTransferFunction* colorTransferFunction = volumePropertyNode->GetColor();

  if (scalarOpacity)
  {
    // Save scalar opacity function
    int numPoints = scalarOpacity->GetSize();
    state.ScalarOpacityPoints.clear();
    state.ScalarOpacityPoints.reserve(numPoints * 2);
    for (int i = 0; i < numPoints; i++)
    {
      double point[4];
      scalarOpacity->GetNodeValue(i, point);
      state.ScalarOpacityPoints.push_back(point[0]); // x
      state.ScalarOpacityPoints.push_back(point[1]); // y
    }

    volumeNode->GetImageData()->GetScalarRange(state.ScalarRange);

    double scalarRange[2];
    scalarOpacity->GetRange(scalarRange);
    state.CenterPoint = (scalarRange[0] + scalarRange[1]) / 2.0;
  }

  if (colorTransferFunction)
  {
    // Save color transfer function
    int numPoints = colorTransferFunction->GetSize();
    state.ColorTransferPoints.clear();
    state.ColorTransferPoints.reserve(numPoints * 4);
    for (int i = 0; i < numPoints; i++)
    {
      double point[6];
      colorTransferFunction->GetNodeValue(i, point);
      state.ColorTransferPoints.push_back(point[0]); // x
      state.ColorTransferPoints.push_back(point[1]); // r
      state.ColorTransferPoints.push_back(point[2]); // g
      state.ColorTransferPoints.push_back(point[3]); // b
    }
  }
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::GetVisibleVolumeRenderingDisplayNodes( //
  std::vector<vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>>& displayNodes,
  int maximumNumberOfItems)
{
  displayNodes.clear();

  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    return;
  }

  // Get all volume rendering display nodes and filter for visible ones
  std::vector<vtkMRMLNode*> nodes;
  scene->GetNodesByClass("vtkMRMLVolumeRenderingDisplayNode", nodes);

  for (vtkMRMLNode* node : nodes)
  {
    vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
    if (displayNode && displayNode->IsDisplayableInView(this->ViewNode->GetID()))
    {
      displayNodes.push_back(displayNode);
      if (displayNodes.size() >= static_cast<size_t>(maximumNumberOfItems))
      {
        break;
      }
    }
  }
}

//----------------------------------------------------------------------
vtkMRMLVolumeRenderingWindowLevelWidget::vtkMRMLVolumeRenderingWindowLevelWidget()
{
  this->Internal = new vtkInternal(this);

  this->StartEventPosition[0] = 0;
  this->StartEventPosition[1] = 0;
  this->PreviousEventPosition[0] = 0;
  this->PreviousEventPosition[1] = 0;

  // Set up event translations for window/level adjustment
  this->SetEventTranslationClickAndDrag(
    WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier, WidgetStateWindowLevelAdjust, WidgetEventClickAndDragStart, WidgetEventClickAndDragEnd);

  // Set up event translation for Ctrl+click+drag (Y-axis opacity scaling)
  this->SetEventTranslationClickAndDrag(
    WidgetStateIdle, vtkCommand::LeftButtonPressEvent, vtkEvent::ControlModifier, WidgetStateOpacityAdjust, WidgetEventClickAndDragStart, WidgetEventClickAndDragEnd);

  this->SetKeyboardEventTranslation(WidgetStateWindowLevelAdjust, vtkEvent::NoModifier, 0, 0, "Escape", WidgetEventReset);
  this->SetKeyboardEventTranslation(WidgetStateOpacityAdjust, vtkEvent::NoModifier, 0, 0, "Escape", WidgetEventReset);
}

//----------------------------------------------------------------------
vtkMRMLVolumeRenderingWindowLevelWidget::~vtkMRMLVolumeRenderingWindowLevelWidget()
{
  delete this->Internal;
  this->Internal = nullptr;
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::SetMRMLViewNode(vtkMRMLViewNode* viewNode)
{
  this->Internal->ViewNode = viewNode;
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::SetVolumeRenderingDisplayableManager(vtkMRMLVolumeRenderingDisplayableManager* manager)
{
  this->Internal->DisplayableManager = manager;
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
bool vtkMRMLVolumeRenderingWindowLevelWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  // Check if we're in window/level interaction mode
  if (!this->Internal->ViewNode || !this->Internal->ViewNode->GetInteractionNode()
      || this->Internal->ViewNode->GetInteractionNode()->GetCurrentInteractionMode() != vtkMRMLInteractionNode::AdjustWindowLevel)
  {
    return false;
  }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
  {
    return false;
  }

  vtkMRMLScene* scene = this->Internal->GetMRMLScene();
  if (!scene)
  {
    return false;
  }

  // If we are currently adjusting then we interact everywhere
  if (this->WidgetState == WidgetStateWindowLevelAdjust || this->WidgetState == WidgetStateOpacityAdjust)
  {
    distance2 = 0.0;
    return true;
  }

  // We accept the event if volume rendering displays an image in this view
  std::vector<vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>> vrDisplayNodes;
  this->Internal->GetVisibleVolumeRenderingDisplayNodes(vrDisplayNodes, 1);
  if (!vrDisplayNodes.empty())
  {
    // We found a volume at this position, we can process this event
    distance2 = 1e9; // Let more specific widgets claim it if they are closer
    return true;
  }

  // No volume found at current position
  return false;
}

//----------------------------------------------------------------------
bool vtkMRMLVolumeRenderingWindowLevelWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLScene* scene = this->Internal->GetMRMLScene();
  if (!scene)
  {
    return false;
  }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = true;

  switch (widgetEvent)
  {
    case WidgetEventMouseMove: processedEvent = this->ProcessMouseMove(eventData); break;
    case WidgetEventClickAndDragStart: processedEvent = this->ProcessAdjustTransferFunctionStart(eventData); break;
    case WidgetEventClickAndDragEnd: processedEvent = this->ProcessEndMouseDrag(eventData); break;
    case WidgetEventReset: processedEvent = this->ProcessReset(eventData); break;
    default: processedEvent = false; break;
  }

  return processedEvent;
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::Leave(vtkMRMLInteractionEventData* eventData)
{
  this->SetWidgetState(WidgetStateIdle);
  this->Superclass::Leave(eventData);
}

//----------------------------------------------------------------------
bool vtkMRMLVolumeRenderingWindowLevelWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  if (!eventData)
  {
    return false;
  }

  switch (this->WidgetState)
  {
    case WidgetStateWindowLevelAdjust:
    case WidgetStateOpacityAdjust: this->ProcessAdjustTransferFunction(eventData); break;
  }

  return true;
}

//----------------------------------------------------------------------
bool vtkMRMLVolumeRenderingWindowLevelWidget::ProcessStartMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  const int* displayPos = eventData->GetDisplayPosition();

  this->StartEventPosition[0] = displayPos[0];
  this->StartEventPosition[1] = displayPos[1];

  this->PreviousEventPosition[0] = this->StartEventPosition[0];
  this->PreviousEventPosition[1] = this->StartEventPosition[1];

  // Fire start interaction event to enable interactive rendering
  for (vtkMRMLVolumeRenderingDisplayNode* displayNode : this->Internal->ModifiedDisplayNodes)
  {
    if (displayNode)
    {
      // displayNode->InvokeEvent(vtkCommand::StartInteractionEvent);
      break;
    }
  }

  this->ProcessMouseMove(eventData);
  return true;
}

//----------------------------------------------------------------------
bool vtkMRMLVolumeRenderingWindowLevelWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  // Fire end interaction event to disable interactive rendering
  for (vtkMRMLVolumeRenderingDisplayNode* displayNode : this->Internal->ModifiedDisplayNodes)
  {
    if (displayNode)
    {
      // displayNode->InvokeEvent(vtkCommand::EndInteractionEvent);
      break;
    }
  }

  this->SetWidgetState(WidgetStateIdle);
  return true;
}

//----------------------------------------------------------------------
bool vtkMRMLVolumeRenderingWindowLevelWidget::ProcessReset(vtkMRMLInteractionEventData* eventData)
{
  // Restore original transfer function states
  this->Internal->RestoreOriginalTransferFunctionStates();

  // Fire end interaction event to disable interactive rendering
  for (vtkMRMLVolumeRenderingDisplayNode* displayNode : this->Internal->ModifiedDisplayNodes)
  {
    if (displayNode)
    {
      // displayNode->InvokeEvent(vtkCommand::EndInteractionEvent);
      break;
    }
  }

  this->SetWidgetState(WidgetStateIdle);
  return true;
}

//----------------------------------------------------------------------
bool vtkMRMLVolumeRenderingWindowLevelWidget::ProcessAdjustTransferFunctionStart(vtkMRMLInteractionEventData* eventData)
{
  // Clear previous modified display nodes
  this->Internal->ModifiedDisplayNodes.clear();

  this->Internal->GetVisibleVolumeRenderingDisplayNodes(this->Internal->ModifiedDisplayNodes, 2);
  if (this->Internal->ModifiedDisplayNodes.empty())
  {
    return false;
  }

  if (this->Internal->ModifiedDisplayNodes.size() > 1)
  {
    // There are multiple visible volumes then we pick the one under the mouse cursor
    if (this->Internal->DisplayableManager)
    {
      const int* displayPos = eventData->GetDisplayPosition();
      if (this->Internal->DisplayableManager->Pick(displayPos[0], displayPos[1]))
      {
        const char* pickedNodeID = this->Internal->DisplayableManager->GetPickedNodeID();
        if (pickedNodeID && strlen(pickedNodeID) > 0)
        {
          vtkMRMLScene* scene = this->Internal->GetMRMLScene();
          if (scene)
          {
            vtkMRMLNode* node = scene->GetNodeByID(pickedNodeID);
            vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);
            if (displayNode)
            {
              this->Internal->ModifiedDisplayNodes.clear();
              this->Internal->ModifiedDisplayNodes.push_back(displayNode);
            }
          }
        }
      }
    }
  }

  if (this->Internal->ModifiedDisplayNodes.size() > 1)
  {
    // Always modify only one volume at a time
    this->Internal->ModifiedDisplayNodes.resize(1);
  }

  // Save state for undo
  vtkMRMLScene* scene = this->Internal->GetMRMLScene();
  if (scene)
  {
    scene->SaveStateForUndo();
  }

  // Save original transfer function states
  this->Internal->SaveOriginalTransferFunctionStates();

  // Set widget state based on modifiers
  int modifiers = eventData->GetModifiers();
  bool isCtrlPressed = (modifiers & vtkEvent::ControlModifier) != 0;

  if (isCtrlPressed)
  {
    this->SetWidgetState(WidgetStateOpacityAdjust);
  }
  else
  {
    this->SetWidgetState(WidgetStateWindowLevelAdjust);
  }

  return this->ProcessStartMouseDrag(eventData);
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::ProcessAdjustTransferFunction(vtkMRMLInteractionEventData* eventData)
{
  const int* eventPosition = eventData->GetDisplayPosition();

  // Calculate total displacement from start position
  int totalDeltaX = eventPosition[0] - this->StartEventPosition[0];
  int totalDeltaY = eventPosition[1] - this->StartEventPosition[1];

  // Get modifiers to determine interaction mode
  int modifiers = eventData->GetModifiers();

  // Apply transfer function adjustments to all visible volume rendering nodes
  for (vtkMRMLVolumeRenderingDisplayNode* displayNode : this->Internal->ModifiedDisplayNodes)
  {
    vtkMRMLVolumePropertyNode* volumePropertyNode = displayNode->GetVolumePropertyNode();
    if (volumePropertyNode)
    {
      this->AdjustVolumeRenderingTransferFunctions(volumePropertyNode, totalDeltaX, totalDeltaY, modifiers);
    }
  }

  this->PreviousEventPosition[0] = eventPosition[0];
  this->PreviousEventPosition[1] = eventPosition[1];
}

//----------------------------------------------------------------------
void vtkMRMLVolumeRenderingWindowLevelWidget::AdjustVolumeRenderingTransferFunctions(vtkMRMLVolumePropertyNode* volumePropertyNode,
                                                                                     double totalDeltaX,
                                                                                     double totalDeltaY,
                                                                                     int modifiers)
{
  if (!volumePropertyNode)
  {
    return;
  }

  // Find the original state for this volume property node
  auto it = this->Internal->OriginalStates.find(volumePropertyNode);
  if (it == this->Internal->OriginalStates.end())
  {
    return; // No original state saved
  }

  const vtkMRMLVolumeRenderingWindowLevelWidget::vtkInternal::OriginalTransferFunctionState& originalState = it->second;

  // Get the scalar opacity and color transfer functions
  vtkPiecewiseFunction* scalarOpacity = volumePropertyNode->GetScalarOpacity();
  vtkColorTransferFunction* colorTransferFunction = volumePropertyNode->GetColor();

  if (!scalarOpacity || !colorTransferFunction)
  {
    return;
  }

  // Get the render window size to calculate scaling factors
  const int* windowSize = this->GetRenderer()->GetRenderWindow()->GetSize();
  double windowMinSize = std::min(windowSize[0], windowSize[1]);

  // Get the original scalar range for scaling
  double scalarRange = originalState.ScalarRange[1] - originalState.ScalarRange[0];
  if (scalarRange <= 0)
  {
    return;
  }

  // Calculate shift and scale factors based on total displacement and modifiers
  double shiftFactor = 0.0;
  double scaleFactor = 1.0;
  double opacityYScaleFactor = 1.0;

  bool isCtrlPressed = (modifiers & vtkEvent::ControlModifier) != 0;

  if (isCtrlPressed)
  {
    // Ctrl+drag mode: Scale Y axis of scalar opacity transfer function
    // Horizontal drag (totalDeltaX) - not used in Ctrl mode
    // Vertical drag (totalDeltaY) scales the Y values of scalar opacity function
    opacityYScaleFactor = 1.0 + (totalDeltaY * 2.0 / windowMinSize);
    // Clamp the opacity Y scale factor to reasonable bounds
    opacityYScaleFactor = std::max(0.1, std::min(5.0, opacityYScaleFactor));
  }
  else
  {
    // Normal mode: X-axis shifting and scaling
    // Horizontal drag (totalDeltaX) shifts the transfer functions along X axis
    shiftFactor = (scalarRange / windowMinSize) * totalDeltaY * 0.5;

    // Vertical drag (totalDeltaY) scales the transfer functions along X axis
    // Use a scaling approach where positive deltaY expands and negative deltaY contracts
    scaleFactor = 1.0 + (totalDeltaX * 0.5 / windowMinSize);

    // Clamp the scale factor to reasonable bounds
    scaleFactor = std::max(0.1, std::min(10.0, scaleFactor));
  }

  double centerPoint = originalState.CenterPoint;

  // Restore and transform scalar opacity function from original state

  int numOpacityPoints = static_cast<int>(originalState.ScalarOpacityPoints.size()) / 2;
  if (numOpacityPoints > 0)
  {
    // Prepare transformed points for batch insertion
    std::vector<double> transformedOpacityPoints(numOpacityPoints * 2);
    for (int i = 0; i < numOpacityPoints; i++)
    {
      double x = originalState.ScalarOpacityPoints[i * 2];
      double y = originalState.ScalarOpacityPoints[i * 2 + 1];

      double newX, newY;

      if (isCtrlPressed)
      {
        // Ctrl+drag mode: Scale Y values of opacity function, keep X unchanged
        newX = x;
        newY = y * opacityYScaleFactor;
        // Clamp Y values to [0, 1] range for opacity
        newY = std::max(0.0, std::min(1.0, newY));
      }
      else
      {
        // Normal mode: Scale around center point and then shift along X axis
        newX = centerPoint + (x - centerPoint) * scaleFactor + shiftFactor;
        newY = y;
      }

      transformedOpacityPoints[i * 2] = newX;
      transformedOpacityPoints[i * 2 + 1] = newY;
    }

    // Use FillFromDataPointer for efficient bulk insertion
    scalarOpacity->FillFromDataPointer(numOpacityPoints, transformedOpacityPoints.data());
  }
  else
  {
    scalarOpacity->RemoveAllPoints();
  }

  // Restore and transform color transfer function from original state

  int numColorPoints = static_cast<int>(originalState.ColorTransferPoints.size()) / 4;
  if (numColorPoints > 0 && !isCtrlPressed)
  {
    // Only transform color transfer function in normal mode (not Ctrl+drag mode)
    // Prepare transformed points for batch insertion
    std::vector<double> transformedColorPoints(numColorPoints * 4);
    for (int i = 0; i < numColorPoints; i++)
    {
      double x = originalState.ColorTransferPoints[i * 4];
      double r = originalState.ColorTransferPoints[i * 4 + 1];
      double g = originalState.ColorTransferPoints[i * 4 + 2];
      double b = originalState.ColorTransferPoints[i * 4 + 3];

      // Scale around center point and then shift
      double newX = centerPoint + (x - centerPoint) * scaleFactor + shiftFactor;
      transformedColorPoints[i * 4] = newX;
      transformedColorPoints[i * 4 + 1] = r;
      transformedColorPoints[i * 4 + 2] = g;
      transformedColorPoints[i * 4 + 3] = b;
    }

    // Use FillFromDataPointer for efficient bulk insertion
    colorTransferFunction->FillFromDataPointer(numColorPoints, transformedColorPoints.data());
  }
  else if (numColorPoints > 0 && isCtrlPressed)
  {
    // In Ctrl+drag mode, restore original color transfer function without changes
    colorTransferFunction->FillFromDataPointer(numColorPoints, const_cast<double*>(originalState.ColorTransferPoints.data()));
  }
  else
  {
    colorTransferFunction->RemoveAllPoints();
  }

  // Trigger update
  volumePropertyNode->Modified();
}
