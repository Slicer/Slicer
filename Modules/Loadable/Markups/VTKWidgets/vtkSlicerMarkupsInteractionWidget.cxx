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

#include "vtkSlicerMarkupsInteractionWidget.h"
#include "vtkSlicerMarkupsInteractionWidgetRepresentation.h"

#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkGeneralTransform.h>
#include <vtkLine.h>
#include <vtkPlane.h>
#include <vtkPointPlacer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>

// Markups MRML includes
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsPlaneDisplayNode.h>
#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsROIDisplayNode.h>
#include <vtkMRMLMarkupsROINode.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerMarkupsInteractionWidget);

//----------------------------------------------------------------------
vtkSlicerMarkupsInteractionWidget::vtkSlicerMarkupsInteractionWidget() = default;

//----------------------------------------------------------------------
vtkSlicerMarkupsInteractionWidget::~vtkSlicerMarkupsInteractionWidget() = default;

//----------------------------------------------------------------------
vtkSlicerMarkupsInteractionWidget* vtkSlicerMarkupsInteractionWidget::CreateInstance() const
{
  vtkSlicerMarkupsInteractionWidgetCreateInstanceMacroBody(vtkSlicerMarkupsInteractionWidget);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsInteractionWidget::GetActiveComponentType()
{
  vtkSmartPointer<vtkSlicerMarkupsInteractionWidgetRepresentation> rep =
    vtkSlicerMarkupsInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  return rep->GetActiveComponentType();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::SetActiveComponentType(int type)
{
  vtkSmartPointer<vtkSlicerMarkupsInteractionWidgetRepresentation> rep =
    vtkSlicerMarkupsInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  rep->SetActiveComponentType(type);
}

//----------------------------------------------------------------------
int vtkSlicerMarkupsInteractionWidget::GetActiveComponentIndex()
{
  vtkSmartPointer<vtkSlicerMarkupsInteractionWidgetRepresentation> rep =
    vtkSlicerMarkupsInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  return rep->GetActiveComponentIndex();
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::SetActiveComponentIndex(int index)
{
  vtkSmartPointer<vtkSlicerMarkupsInteractionWidgetRepresentation> rep =
    vtkSlicerMarkupsInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  rep->SetActiveComponentIndex(index);
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsInteractionWidget::ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetDisplayNode();
  if (!displayNode)
  {
    return false;
  }

  return this->ProcessWidgetMenuDisplayNodeTypeAndIndex(
    eventData, displayNode, displayNode->GetActiveComponentType(), displayNode->GetActiveComponentIndex());
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::ApplyTransform(vtkTransform* transform)
{
  if (!this->GetMarkupsNode())
  {
    return;
  }

  MRMLNodeModifyBlocker blocker(this->GetMarkupsNode());

  vtkNew<vtkGeneralTransform> transformToWorld;
  vtkMRMLTransformNode::GetTransformBetweenNodes(
    this->GetMarkupsNode()->GetParentTransformNode(), nullptr, transformToWorld);

  vtkAbstractTransform* transformFromWorld = transformToWorld->GetInverse();

  vtkNew<vtkGeneralTransform> transformToApply;
  transformToApply->PostMultiply();
  transformToApply->Concatenate(transformToWorld);
  transformToApply->Concatenate(transform);
  transformToApply->Concatenate(transformFromWorld);

  this->GetMarkupsNode()->ApplyTransform(transformToApply, false);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* displayNode,
                                                                    vtkMRMLAbstractViewNode* viewNode,
                                                                    vtkRenderer* renderer)
{
  vtkSmartPointer<vtkSlicerMarkupsInteractionWidgetRepresentation> rep =
    vtkSmartPointer<vtkSlicerMarkupsInteractionWidgetRepresentation>::New();
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetDisplayNode(displayNode);
  rep->UpdateFromMRML(nullptr, 0); // full update
}

//----------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* vtkSlicerMarkupsInteractionWidget::GetDisplayNode()
{
  vtkSlicerMarkupsInteractionWidgetRepresentation* widgetRep =
    vtkSlicerMarkupsInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!widgetRep)
  {
    return nullptr;
  }
  return widgetRep->GetDisplayNode();
}

//----------------------------------------------------------------------
vtkMRMLMarkupsNode* vtkSlicerMarkupsInteractionWidget::GetMarkupsNode()
{
  vtkSlicerMarkupsInteractionWidgetRepresentation* widgetRep =
    vtkSlicerMarkupsInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!widgetRep)
  {
    return nullptr;
  }
  return widgetRep->GetMarkupsNode();
}

//-----------------------------------------------------------------------------
bool vtkSlicerMarkupsInteractionWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  bool processedEvent = false;
  switch (widgetEvent)
  {
    case WidgetEventJumpCursor:
      processedEvent = ProcessWidgetJumpCursor(eventData);
      break;
    default:
      break;
  }
  processedEvent = Superclass::ProcessInteractionEvent(eventData);
  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkSlicerMarkupsInteractionWidget::ProcessWidgetJumpCursor(vtkMRMLInteractionEventData* vtkNotUsed(eventData))
{
  vtkMRMLMarkupsNode* markupsNode = this->GetMarkupsNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
  {
    return false;
  }

  int componentIndex = markupsDisplayNode->GetActiveComponentIndex();
  int componentType = markupsDisplayNode->GetActiveComponentType();

  markupsNode->GetScene()->SaveStateForUndo();

  vtkNew<vtkMRMLInteractionEventData> jumpToPointEventData;
  jumpToPointEventData->SetType(vtkMRMLMarkupsDisplayNode::JumpToPointEvent);
  jumpToPointEventData->SetComponentType(componentType);
  jumpToPointEventData->SetComponentIndex(componentIndex);
  jumpToPointEventData->SetViewNode(this->WidgetRep->GetViewNode());

  // For interaction handle, send the position of the handle as well.
  // The position of the handle may be different in each view, so we need to get the position from the representation.
  vtkSlicerMarkupsInteractionWidgetRepresentation* rep =
    vtkSlicerMarkupsInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (rep)
  {
    double position_World[3] = { 0.0, 0.0, 0.0 };
    rep->GetInteractionHandlePositionWorld(
      rep->GetActiveComponentType(), rep->GetActiveComponentIndex(), position_World);
    jumpToPointEventData->SetWorldPosition(position_World);
  }

  markupsDisplayNode->InvokeEvent(vtkMRMLMarkupsDisplayNode::JumpToPointEvent, jumpToPointEventData);
  return true;
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::ScaleWidget(double eventPos[2], bool uniformScale /*=false*/)
{
  if (vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode()))
  {
    this->ScaleWidgetROI(eventPos, uniformScale);
  }
  else if (vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode()))
  {
    this->ScaleWidgetPlane(eventPos, uniformScale);
  }
  else
  {
    Superclass::ScaleWidget(eventPos, uniformScale);
  }
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::ScaleWidgetPlane(double eventPos[2], bool symmetricScale)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetDisplayNode();
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !displayNode)
  {
    return;
  }

  double lastEventPos_World[3] = { 0.0 };
  double eventPos_World[3] = { 0.0 };
  double orientation_World[9] = { 0.0 };

  vtkMRMLInteractionWidgetRepresentation* rep = vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
  {
    return;
  }

  if (rep->GetSliceNode())
  {
    // 2D view
    double eventPos_Slice[3] = { 0. };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);
  }
  else
  {
    // 3D view
    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, this->LastEventPosition, lastEventPos_World, orientation_World))
    {
      return;
    }

    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos, lastEventPos_World, eventPos_World, orientation_World))
    {
      return;
    }
  }

  vtkNew<vtkMatrix4x4> worldToObjectMatrix;
  markupsNode->GetObjectToWorldMatrix(worldToObjectMatrix);
  worldToObjectMatrix->Invert();
  vtkNew<vtkTransform> worldToObjectTransform;
  worldToObjectTransform->SetMatrix(worldToObjectMatrix);

  int index = displayNode->GetActiveComponentIndex();
  if (index <= vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge)
  {
    // We are interacting with one of the edges.
    // Restrict scaling to apply only in that direction.
    this->GetClosestPointOnInteractionAxis(InteractionScaleHandle, index, this->LastEventPosition, lastEventPos_World);
    this->GetClosestPointOnInteractionAxis(InteractionScaleHandle, index, eventPos, eventPos_World);
  }
  else
  {
    // We are interacting with one of the corners
    // Restrict scaling so that the projection of the interaction handle and the event position will be at the same
    // display position.
    double cameraDirectionEventPos_World[3] = { 0.0, 0.0, 0.0 };
    double cameraDirectionLastEventPos_World[3] = { 0.0, 0.0, 0.0 };
    if (rep->GetSliceNode())
    {
      vtkMRMLSliceNode* sliceNode = rep->GetSliceNode();
      if (sliceNode)
      {
        // Get the slice view normal
        double normal4_World[4] = { 0.0, 0.0, 1.0, 0.0 };
        sliceNode->GetSliceToRAS()->MultiplyPoint(normal4_World, normal4_World);
        cameraDirectionEventPos_World[0] = normal4_World[0];
        cameraDirectionEventPos_World[1] = normal4_World[1];
        cameraDirectionEventPos_World[2] = normal4_World[2];
        cameraDirectionLastEventPos_World[0] = normal4_World[0];
        cameraDirectionLastEventPos_World[1] = normal4_World[1];
        cameraDirectionLastEventPos_World[2] = normal4_World[2];
      }
    }
    else
    {
      vtkCamera* camera = this->Renderer->GetActiveCamera();
      if (camera && camera->GetParallelProjection())
      {
        camera->GetDirectionOfProjection(cameraDirectionEventPos_World);
        camera->GetDirectionOfProjection(cameraDirectionLastEventPos_World);
      }
      else if (camera)
      {
        // Camera position
        double cameraPosition_World[4] = { 0.0 };
        camera->GetPosition(cameraPosition_World);

        //  Compute the ray endpoints. The ray is along the line running from
        //  the camera position to the selection point, starting where this line
        //  intersects the front clipping plane, and terminating where this
        //  line intersects the back clipping plane.
        vtkMath::Subtract(cameraPosition_World, eventPos_World, cameraDirectionEventPos_World);
        vtkMath::Subtract(cameraPosition_World, lastEventPos_World, cameraDirectionLastEventPos_World);
      }
    }

    // Create a second point running along the line from the camera to the event position
    double eventPos2_World[3];
    vtkMath::Add(eventPos_World, cameraDirectionEventPos_World, eventPos2_World);
    double lastEventPos2_World[3];
    vtkMath::Add(lastEventPos_World, cameraDirectionLastEventPos_World, lastEventPos2_World);
    double t; // not used

    double normal_World[3] = { 0.0, 0.0, 0.0 };
    markupsNode->GetNormalWorld(normal_World);
    double origin_World[3] = { 0.0, 0.0, 0.0 };
    markupsNode->GetOriginWorld(origin_World);

    // Find the intersection of the vector from event positions, along the vector from the camera at that point
    vtkNew<vtkPlane> plane;
    plane->SetOrigin(origin_World);
    plane->SetNormal(normal_World);
    plane->IntersectWithLine(eventPos_World, eventPos2_World, t, eventPos_World);
    plane->IntersectWithLine(lastEventPos_World, lastEventPos2_World, t, lastEventPos_World);
  }

  double scaleVector_World[3] = { 0.0, 0.0, 0.0 };
  vtkMath::Subtract(eventPos_World, lastEventPos_World, scaleVector_World);

  double scaleVector_Object[3] = { 0.0, 0.0, 0.0 };
  worldToObjectTransform->TransformVector(scaleVector_World, scaleVector_Object);

  double bounds_Plane[4] = { 0.0, -1.0, 0.0, -1.0 };
  markupsNode->GetPlaneBounds(bounds_Plane);

  switch (index)
  {
    case vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
      bounds_Plane[0] += scaleVector_Object[0];
      if (symmetricScale)
      {
        bounds_Plane[1] -= scaleVector_Object[0];
      }
      break;
    case vtkMRMLMarkupsPlaneDisplayNode::HandleREdge:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
      bounds_Plane[1] += scaleVector_Object[0];
      if (symmetricScale)
      {
        bounds_Plane[0] -= scaleVector_Object[0];
      }
      break;
    default:
      break;
  }

  switch (index)
  {
    case vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
      bounds_Plane[2] += scaleVector_Object[1];
      if (symmetricScale)
      {
        bounds_Plane[3] -= scaleVector_Object[1];
      }
      break;
    case vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
    case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
      bounds_Plane[3] += scaleVector_Object[1];
      if (symmetricScale)
      {
        bounds_Plane[2] -= scaleVector_Object[1];
      }
      break;
    default:
      break;
  }

  // If we have crossed over the origin, we need to flip the selected handle across the Right and/or Anterior axis.
  bool flipLRHandle = bounds_Plane[1] < bounds_Plane[0];
  bool flipPAHandle = bounds_Plane[3] < bounds_Plane[2];
  if (flipLRHandle || flipPAHandle)
  {
    this->FlipPlaneHandles(flipLRHandle, flipPAHandle);
  }

  if (bounds_Plane[1] < bounds_Plane[0])
  {
    double tempBounds_Plane0 = bounds_Plane[0];
    bounds_Plane[0] = bounds_Plane[1];
    bounds_Plane[1] = tempBounds_Plane0;
  }

  if (bounds_Plane[3] < bounds_Plane[2])
  {
    double tempBounds_Plane2 = bounds_Plane[2];
    bounds_Plane[2] = bounds_Plane[3];
    bounds_Plane[3] = tempBounds_Plane2;
  }

  markupsNode->SetPlaneBounds(bounds_Plane);
  markupsNode->SetSizeMode(vtkMRMLMarkupsPlaneNode::SizeModeAbsolute);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::FlipPlaneHandles(bool flipLRHandle, bool flipPAHandle)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetDisplayNode();
  vtkMRMLMarkupsPlaneNode* markupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->GetMarkupsNode());
  if (!markupsNode || !displayNode)
  {
    return;
  }

  int index = displayNode->GetActiveComponentIndex();
  if (flipLRHandle)
  {
    switch (index)
    {
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleREdge;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleREdge:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLEdge;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner;
        break;
      default:
        break;
    }
  }

  if (flipPAHandle)
  {
    switch (index)
    {
      case vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandlePEdge:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleAEdge;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleLPCorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleLACorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner;
        break;
      case vtkMRMLMarkupsPlaneDisplayNode::HandleRPCorner:
        index = vtkMRMLMarkupsPlaneDisplayNode::HandleRACorner;
        break;
      default:
        break;
    }
  }

  displayNode->SetActiveComponent(displayNode->GetActiveComponentType(), index);
}

//----------------------------------------------------------------------
void vtkSlicerMarkupsInteractionWidget::ScaleWidgetROI(double eventPos[2], bool symmetricScale)
{
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->GetMarkupsNode());
  if (!roiNode)
  {
    Superclass::ScaleWidget(eventPos);
    return;
  }

  vtkMRMLMarkupsDisplayNode* displayNode = this->GetDisplayNode();
  if (!displayNode)
  {
    return;
  }

  MRMLNodeModifyBlocker blocker(roiNode);

  double lastEventPos_World[3] = { 0.0, 0.0, 0.0 };
  double eventPos_World[3] = { 0.0, 0.0, 0.0 };
  double orientation_World[9] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  vtkMRMLInteractionWidgetRepresentation* rep = vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->WidgetRep);
  if (!rep)
  {
    return;
  }

  if (rep->GetSliceNode())
  {
    // 2D view
    double eventPos_Slice[3] = { 0. };
    eventPos_Slice[0] = this->LastEventPosition[0];
    eventPos_Slice[1] = this->LastEventPosition[1];
    rep->GetSliceToWorldCoordinates(eventPos_Slice, lastEventPos_World);

    eventPos_Slice[0] = eventPos[0];
    eventPos_Slice[1] = eventPos[1];
    rep->GetSliceToWorldCoordinates(eventPos_Slice, eventPos_World);
  }
  else
  {
    // 3D view
    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, this->LastEventPosition, lastEventPos_World, orientation_World))
    {
      return;
    }

    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos, lastEventPos_World, eventPos_World, orientation_World))
    {
      return;
    }
  }

  if (this->GetActiveComponentType() == InteractionScaleHandle)
  {
    vtkNew<vtkMatrix4x4> worldToObjectMatrix;
    worldToObjectMatrix->DeepCopy(roiNode->GetObjectToWorldMatrix());
    worldToObjectMatrix->Invert();
    vtkNew<vtkTransform> worldToObjectTransform;
    worldToObjectTransform->SetMatrix(worldToObjectMatrix);

    int index = displayNode->GetActiveComponentIndex();
    if (index < 6 && !rep->GetSliceNode())
    {
      this->GetClosestPointOnInteractionAxis(
        InteractionScaleHandle, index, this->LastEventPosition, lastEventPos_World);
      this->GetClosestPointOnInteractionAxis(InteractionScaleHandle, index, eventPos, eventPos_World);
    }

    double scaleVector_World[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Subtract(eventPos_World, lastEventPos_World, scaleVector_World);

    double scaleVector_ROI[3] = { 0.0, 0.0, 0.0 };
    worldToObjectTransform->TransformVector(scaleVector_World, scaleVector_ROI);

    double radius_ROI[3] = { 0.0, 0.0, 0.0 };
    roiNode->GetSize(radius_ROI);
    vtkMath::MultiplyScalar(radius_ROI, 0.5);

    double bounds_ROI[6] = {
      -radius_ROI[0], radius_ROI[0], -radius_ROI[1], radius_ROI[1], -radius_ROI[2], radius_ROI[2]
    };
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
    objectToWorldTransform->SetMatrix(roiNode->GetObjectToWorldMatrix());
    double newOrigin_World[3] = { 0.0, 0.0, 0.0 };
    objectToWorldTransform->TransformPoint(newOrigin_ROI, newOrigin_World);
    roiNode->SetCenterWorld(newOrigin_World);
    roiNode->SetSize(newSize);

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
void vtkSlicerMarkupsInteractionWidget::FlipROIHandles(bool flipLRHandle, bool flipPAHandle, bool flipISHandle)
{
  vtkMRMLMarkupsDisplayNode* displayNode = this->GetDisplayNode();
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
