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

// MRML includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsROIDisplayNode.h"
#include "vtkMRMLMarkupsROINode.h"
#include "vtkMRMLSliceNode.h"

// Markups VTK widgets includes
#include "vtkSlicerROIWidget.h"
#include "vtkSlicerROIRepresentation2D.h"
#include "vtkSlicerROIRepresentation3D.h"

// VTK includes
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkPointPlacer.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

vtkStandardNewMacro(vtkSlicerROIWidget);

//----------------------------------------------------------------------
vtkSlicerROIWidget::vtkSlicerROIWidget()
{
  this->SetEventTranslationClickAndDrag(WidgetStateOnScaleHandle, vtkCommand::LeftButtonPressEvent, vtkEvent::AltModifier,
    WidgetStateSymmetricScale, WidgetEventSymmetricScaleStart, WidgetEventSymmetricScaleEnd);
}

//----------------------------------------------------------------------
vtkSlicerROIWidget::~vtkSlicerROIWidget() = default;

//----------------------------------------------------------------------
void vtkSlicerROIWidget::CreateDefaultRepresentation(
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer)
{
  vtkSmartPointer<vtkSlicerMarkupsWidgetRepresentation> rep = nullptr;
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
    {
    rep = vtkSmartPointer<vtkSlicerROIRepresentation2D>::New();
    }
  else
    {
    rep = vtkSmartPointer<vtkSlicerROIRepresentation3D>::New();
    }
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetMarkupsDisplayNode(markupsDisplayNode);
  rep->UpdateFromMRML(nullptr, 0); // full update
}

//-----------------------------------------------------------------------------
bool vtkSlicerROIWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &distance2)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
    {
    return false;
    }
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
    {
    return false;
    }

  // If we are placing markups or dragging the mouse then we interact everywhere
  if (this->WidgetState == WidgetStateSymmetricScale)
    {
    distance2 = 0.0;
    return true;
    }

  return Superclass::CanProcessInteractionEvent(eventData, distance2);
}

//-----------------------------------------------------------------------------
bool vtkSlicerROIWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = false;
  switch (widgetEvent)
    {
    case WidgetEventSymmetricScaleStart:
      processedEvent = ProcessWidgetSymmetricScaleStart(eventData);
      break;
    case WidgetEventSymmetricScaleEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    }

  if (!processedEvent)
    {
    processedEvent = Superclass::ProcessInteractionEvent(eventData);
    }
  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkSlicerROIWidget::ProcessWidgetSymmetricScaleStart(vtkMRMLInteractionEventData* eventData)
{
  if ((this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateOnWidget && this->WidgetState != vtkSlicerMarkupsWidget::WidgetStateOnScaleHandle)
    || this->IsAnyControlPointLocked())
    {
    return false;
    }

  this->SetWidgetState(WidgetStateSymmetricScale);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkSlicerROIWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
    {
    return false;
    }

  if (this->WidgetState != vtkSlicerROIWidget::WidgetStateSymmetricScale)
    {
    return Superclass::ProcessEndMouseDrag(eventData);
    }

  int activeComponentType = this->GetActiveComponentType();
  if (activeComponentType == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    this->SetWidgetState(WidgetStateOnScaleHandle);
    }
  else
    {
    this->SetWidgetState(WidgetStateOnWidget);
    }

  this->EndWidgetInteraction();
  return true;
}


//-------------------------------------------------------------------------
bool vtkSlicerROIWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep || !markupsNode || !eventData)
    {
    return false;
    }

  int state = this->WidgetState;
  if (state != WidgetStateSymmetricScale)
    {
    return Superclass::ProcessMouseMove(eventData);
    }

  // Process the motion
  // Based on the displacement vector (computed in display coordinates) and
  // the cursor state (which corresponds to which part of the widget has been
  // selected), the widget points are modified.
  // First construct a local coordinate system based on the display coordinates
  // of the widget.
  double eventPos[2]
    {
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1]),
    };

  this->ScaleWidget(eventPos, true);

  this->LastEventPosition[0] = eventPos[0];
  this->LastEventPosition[1] = eventPos[1];

  return true;
}

//----------------------------------------------------------------------
void vtkSlicerROIWidget::ScaleWidget(double eventPos[2])
{
  this->ScaleWidget(eventPos, false);
}

//----------------------------------------------------------------------
void vtkSlicerROIWidget::ScaleWidget(double eventPos[2], bool symmetricScale)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  vtkMRMLMarkupsROINode* markupsNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !displayNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(markupsNode);

  double lastEventPos_World[3] = { 0.0 };
  double eventPos_World[3] = { 0.0 };
  double orientation_World[9] = { 0.0 };

  vtkSlicerROIRepresentation2D* rep2d = vtkSlicerROIRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerROIRepresentation3D* rep3d = vtkSlicerROIRepresentation3D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    // 2D view
    double eventPos_Slice[3] = { 0. };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep2d->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);
    }
  else if (rep3d)
    {
    // 3D view
    double eventPos_Display[2] = { 0. };
    eventPos_Display[0] = this->LastEventPosition[0];
    eventPos_Display[1] = this->LastEventPosition[1];

    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      this->LastEventPosition, lastEventPos_World, orientation_World))
      {
      return;
      }

    if (!rep3d->GetPointPlacer()->ComputeWorldPosition(this->Renderer,
      eventPos, lastEventPos_World, eventPos_World, orientation_World))
      {
      return;
      }
    }

  if (this->GetActiveComponentType() == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle)
    {
    vtkNew<vtkMatrix4x4> worldToObjectMatrix;
    worldToObjectMatrix->DeepCopy(markupsNode->GetObjectToWorldMatrix());
    worldToObjectMatrix->Invert();
    vtkNew<vtkTransform> worldToObjectTransform;
    worldToObjectTransform->SetMatrix(worldToObjectMatrix);

    int index = displayNode->GetActiveComponentIndex();
    if (index < 6 && rep3d)
      {
      this->GetClosestPointOnInteractionAxis(
        vtkMRMLMarkupsDisplayNode::ComponentScaleHandle, index, this->LastEventPosition, lastEventPos_World);
      this->GetClosestPointOnInteractionAxis(
        vtkMRMLMarkupsDisplayNode::ComponentScaleHandle, index, eventPos, eventPos_World);
      }

    double scaleVector_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Subtract(eventPos_World, lastEventPos_World, scaleVector_World);

    double scaleVector_ROI[3] = { 0.0, 0.0, 0.0 };
    worldToObjectTransform->TransformVector(scaleVector_World, scaleVector_ROI);

    double radius_ROI[3] = { 0.0, 0.0, 0.0 };
    markupsNode->GetSize(radius_ROI);
    vtkMath::MultiplyScalar(radius_ROI, 0.5);
    double bounds_ROI[6] = { -radius_ROI[0], radius_ROI[0], -radius_ROI[1], radius_ROI[1], -radius_ROI[2], radius_ROI[2] };

    switch (index)
      {
      case vtkMRMLMarkupsROIDisplayNode::HandleLFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleLPICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLAICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLASCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLAEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleLPEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleLIEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleLSEdge:
        bounds_ROI[0] += scaleVector_ROI[0];
        if (symmetricScale)
          {
          bounds_ROI[1] -= scaleVector_ROI[0];
          }
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleRPICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRAICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRASCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRAEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleRPEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleRIEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleRSEdge:
        bounds_ROI[1] += scaleVector_ROI[0];
        if (symmetricScale)
          {
          bounds_ROI[0] -= scaleVector_ROI[0];
          }
        break;
      default:
        break;
      }

    switch (index)
      {
      case vtkMRMLMarkupsROIDisplayNode::HandlePFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleLPICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRPICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLPEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleRPEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandlePIEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandlePSEdge:
        bounds_ROI[2] += scaleVector_ROI[1];
        if (symmetricScale)
          {
          bounds_ROI[3] -= scaleVector_ROI[1];
          }
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleAFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleLAICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRAICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLASCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRASCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLAEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleRAEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleAIEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleASEdge:
        bounds_ROI[3] += scaleVector_ROI[1];
        if (symmetricScale)
          {
          bounds_ROI[2] -= scaleVector_ROI[1];
          }
        break;
      default:
        break;
      }

    switch (index)
      {
      case vtkMRMLMarkupsROIDisplayNode::HandleIFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleLPICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRPICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLAICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRAICorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLIEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleRIEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleAIEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandlePIEdge:
        bounds_ROI[4] += scaleVector_ROI[2];
        if (symmetricScale)
          {
          bounds_ROI[5] -= scaleVector_ROI[2];
          }
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleSFace:
      case vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLASCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleRASCorner:
      case vtkMRMLMarkupsROIDisplayNode::HandleLSEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleRSEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandleASEdge:
      case vtkMRMLMarkupsROIDisplayNode::HandlePSEdge:
        bounds_ROI[5] += scaleVector_ROI[2];
        if (symmetricScale)
          {
          bounds_ROI[4] -= scaleVector_ROI[2];
          }
        break;
      default:
        break;
      }

    double newSize[3] = { 0.0, 0.0, 0.0 };
    double newOrigin_ROI[3] = { 0.0, 0.0, 0.0 };
    for (int i = 0; i < 3; ++i)
      {
      newSize[i] = std::abs(bounds_ROI[2 * i + 1] - bounds_ROI[2 * i]);
      newOrigin_ROI[i] = (bounds_ROI[2 * i + 1] + bounds_ROI[2 * i]) / 2.0;
      }

    vtkNew<vtkTransform> objectToWorldTransform;
    objectToWorldTransform->SetMatrix(markupsNode->GetObjectToWorldMatrix());
    double newOrigin_World[3] = { 0.0, 0.0, 0.0 };
    objectToWorldTransform->TransformPoint(newOrigin_ROI, newOrigin_World);
    markupsNode->SetCenterWorld(newOrigin_World);
    markupsNode->SetSize(newSize);

    bool flipLRHandle = bounds_ROI[1] < bounds_ROI[0];
    bool flipPAHandle = bounds_ROI[3] < bounds_ROI[2];
    bool flipISHandle = bounds_ROI[5] < bounds_ROI[4];
    if (flipLRHandle || flipPAHandle || flipISHandle)
      {
      this->FlipROIHandles(flipLRHandle, flipPAHandle, flipISHandle);
      }
    }
}

//----------------------------------------------------------------------
void vtkSlicerROIWidget::FlipROIHandles(bool flipLRHandle, bool flipPAHandle, bool flipISHandle)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetMarkupsDisplayNode();
  vtkMRMLMarkupsROINode* markupsNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !displayNode)
    {
    return;
    }

  int index = displayNode->GetActiveComponentIndex();
  if (flipLRHandle)
    {
    switch (index)
      {
      case vtkMRMLMarkupsROIDisplayNode::HandleLFace:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRFace;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRFace:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLFace;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLAICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRAICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRPICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLASCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRASCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRAICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLAICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLPICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRASCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLASCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLAEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRAEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRPEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLIEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRIEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLSEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRSEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRAEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLAEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLPEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRIEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLIEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRSEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLSEdge;
        break;
      default:
        break;
      }
    }

  if (flipPAHandle)
    {
    switch (index)
      {
      case vtkMRMLMarkupsROIDisplayNode::HandleAFace:
        index = vtkMRMLMarkupsROIDisplayNode::HandlePFace;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandlePFace:
        index = vtkMRMLMarkupsROIDisplayNode::HandleAFace;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLAICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLPICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLAICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLASCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLASCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRAICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRPICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRAICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRASCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRASCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLAEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRAEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandlePIEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleAIEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandlePSEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleASEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLAEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLPEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRAEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRPEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleAIEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandlePIEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleASEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandlePSEdge;
        break;
      default:
        break;
      }
    }

  if (flipISHandle)
    {
    switch (index)
      {
      case vtkMRMLMarkupsROIDisplayNode::HandleIFace:
        index = vtkMRMLMarkupsROIDisplayNode::HandleSFace;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleSFace:
        index = vtkMRMLMarkupsROIDisplayNode::HandleIFace;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLAICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLASCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLASCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLAICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLPSCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLPICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRAICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRASCorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPICorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRPICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRASCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRAICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRPSCorner:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRPICorner;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLIEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLSEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRIEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRSEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleAIEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleASEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandlePIEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandlePSEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleLSEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleLIEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleRSEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleRIEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandleASEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandleAIEdge;
        break;
      case vtkMRMLMarkupsROIDisplayNode::HandlePSEdge:
        index = vtkMRMLMarkupsROIDisplayNode::HandlePIEdge;
        break;
      default:
        break;
      }
    }

  displayNode->SetActiveComponent(displayNode->GetActiveComponentType(), index);
}
