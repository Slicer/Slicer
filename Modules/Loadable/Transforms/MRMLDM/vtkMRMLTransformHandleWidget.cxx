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

// VTK includes
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkGeneralTransform.h>
#include <vtkLine.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointPlacer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

// MRML includes
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>

// MRMLDM includes
#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLTransformHandleWidgetRepresentation.h"
#include "vtkMRMLTransformHandleWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLTransformHandleWidget);

//----------------------------------------------------------------------
vtkMRMLTransformHandleWidget::vtkMRMLTransformHandleWidget()
{
  // Handle interactions
  this->SetEventTranslationClickAndDrag(WidgetStateOnTranslationHandle,
                                        vtkCommand::LeftButtonPressEvent,
                                        vtkEvent::AltModifier,
                                        WidgetStateTranslateTransformCenter,
                                        WidgetEventTranslateTransformCenterStart,
                                        WidgetEventTranslateTransformCenterEnd);
}

//----------------------------------------------------------------------
vtkMRMLTransformHandleWidget::~vtkMRMLTransformHandleWidget() = default;

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
bool vtkMRMLTransformHandleWidget::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& distance2)
{
  bool canProcess = Superclass::CanProcessInteractionEvent(eventData, distance2);
  if (canProcess)
  {
    return true;
  }

  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);
  if (widgetEvent == WidgetEventNone)
  {
    return false;
  }

  vtkMRMLInteractionWidgetRepresentation* rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep)
  {
    return false;
  }

  // Currently interacting
  if (this->WidgetState == WidgetStateTranslateTransformCenter)
  {
    distance2 = 0.0;
    return true;
  }

  return false;
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidget::ApplyTransform(vtkTransform* transformToApply)
{
  if (!this->GetTransformNode())
  {
    return;
  }

  MRMLNodeModifyBlocker blocker(this->GetTransformNode());

  vtkNew<vtkGeneralTransform> newTransformToParent;
  newTransformToParent->PostMultiply();

  vtkNew<vtkMatrix4x4> transformToParent;
  this->GetTransformNode()->GetMatrixTransformToParent(transformToParent);
  newTransformToParent->Concatenate(transformToParent);

  vtkMRMLTransformNode* parentTransformNode = this->GetTransformNode()->GetParentTransformNode();
  if (parentTransformNode)
  {
    // Perform the transformation in world coordinates
    vtkNew<vtkGeneralTransform> transformToWorld;
    parentTransformNode->GetTransformToWorld(transformToWorld);
    newTransformToParent->Concatenate(transformToWorld);
  }

  // Apply the transform
  newTransformToParent->Concatenate(transformToApply);

  if (parentTransformNode)
  {
    // Transform back to local coordinates
    vtkNew<vtkGeneralTransform> transformFromWorld;
    parentTransformNode->GetTransformFromWorld(transformFromWorld);
    newTransformToParent->Concatenate(transformFromWorld);
  }

  vtkNew<vtkMatrix4x4> newTransformToParentMatrix;
  double origin[3] = { 0.0, 0.0, 0.0 };
  double x[3] = { 1.0, 0.0, 0.0 };
  newTransformToParent->TransformVectorAtPoint(origin, x, x);
  double y[3] = { 0.0, 1.0, 0.0 };
  newTransformToParent->TransformVectorAtPoint(origin, y, y);
  double z[3] = { 0.0, 0.0, 1.0 };
  newTransformToParent->TransformVectorAtPoint(origin, z, z);
  newTransformToParent->TransformPoint(origin, origin);

  for (int i = 0; i < 3; ++i)
  {
    newTransformToParentMatrix->SetElement(i, 0, x[i]);
    newTransformToParentMatrix->SetElement(i, 1, y[i]);
    newTransformToParentMatrix->SetElement(i, 2, z[i]);
    newTransformToParentMatrix->SetElement(i, 3, origin[i]);
  }
  this->GetTransformNode()->SetMatrixTransformToParent(newTransformToParentMatrix);
}

//----------------------------------------------------------------------
void vtkMRMLTransformHandleWidget::CreateDefaultRepresentation(vtkMRMLTransformDisplayNode* displayNode,
                                                               vtkMRMLAbstractViewNode* viewNode,
                                                               vtkRenderer* renderer)
{
  vtkSmartPointer<vtkMRMLTransformHandleWidgetRepresentation> rep =
    vtkSmartPointer<vtkMRMLTransformHandleWidgetRepresentation>::New();
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetDisplayNode(displayNode);
  rep->UpdateFromMRML(nullptr, 0); // full update
}

//----------------------------------------------------------------------
vtkMRMLTransformDisplayNode* vtkMRMLTransformHandleWidget::GetDisplayNode()
{
  vtkMRMLTransformHandleWidgetRepresentation* widgetRep =
    vtkMRMLTransformHandleWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!widgetRep)
  {
    return nullptr;
  }
  return widgetRep->GetDisplayNode();
}

//----------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLTransformHandleWidget::GetTransformNode()
{
  vtkMRMLTransformHandleWidgetRepresentation* widgetRep =
    vtkMRMLTransformHandleWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!widgetRep)
  {
    return nullptr;
  }
  return widgetRep->GetTransformNode();
}

//-----------------------------------------------------------------------------
bool vtkMRMLTransformHandleWidget::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  unsigned long widgetEvent = this->TranslateInteractionEventToWidgetEvent(eventData);

  bool processedEvent = false;
  switch (widgetEvent)
  {
    case WidgetEventTranslateTransformCenterStart:
      processedEvent = this->ProcessWidgetTranslateTransformCenterStart(eventData);
      break;
    case WidgetEventTranslateTransformCenterEnd:
      processedEvent = ProcessEndMouseDrag(eventData);
      break;
    case WidgetEventMenu:
      processedEvent = ProcessWidgetMenu(eventData);
      break;
    default:
      processedEvent = Superclass::ProcessInteractionEvent(eventData);
      break;
  }
  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkMRMLTransformHandleWidget::ProcessWidgetMenu(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLTransformNode* transformNode = this->GetTransformNode();
  vtkMRMLTransformDisplayNode* displayNode = this->GetDisplayNode();
  if (!transformNode || !displayNode)
  {
    return false;
  }

  return this->ProcessWidgetMenuDisplayNodeTypeAndIndex(
    eventData, displayNode, displayNode->GetActiveInteractionType(), displayNode->GetActiveInteractionIndex());
}

//-------------------------------------------------------------------------
bool vtkMRMLTransformHandleWidget::ProcessEndMouseDrag(vtkMRMLInteractionEventData* eventData)
{
  if (!this->WidgetRep)
  {
    return false;
  }

  if (this->WidgetState != vtkMRMLTransformHandleWidget::WidgetStateTranslateTransformCenter)
  {
    return Superclass::ProcessEndMouseDrag(eventData);
  }

  int activeComponentType = this->GetActiveComponentType();
  if (activeComponentType == InteractionTranslationHandle)
  {
    this->SetWidgetState(WidgetStateOnTranslationHandle);
  }

  this->EndWidgetInteraction();

  // only claim this as processed if the mouse was moved (this allows the event to be interpreted as button click)
  bool processedEvent = eventData->GetMouseMovedSinceButtonDown();
  return processedEvent;
}

//-------------------------------------------------------------------------
bool vtkMRMLTransformHandleWidget::ProcessWidgetTranslateTransformCenterStart(vtkMRMLInteractionEventData* eventData)
{
  if ((this->WidgetState != vtkMRMLInteractionWidget::WidgetStateOnWidget
       && this->WidgetState != vtkMRMLInteractionWidget::WidgetStateOnTranslationHandle))
  {
    return false;
  }

  this->SetWidgetState(WidgetStateTranslateTransformCenter);
  this->StartWidgetInteraction(eventData);
  return true;
}

//-------------------------------------------------------------------------
bool vtkMRMLTransformHandleWidget::ProcessMouseMove(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLInteractionWidgetRepresentation* rep =
    vtkMRMLInteractionWidgetRepresentation::SafeDownCast(this->GetRepresentation());
  if (!rep || !eventData)
  {
    return false;
  }

  int state = this->WidgetState;

  if (state != WidgetStateTranslateTransformCenter)
  {
    return Superclass::ProcessMouseMove(eventData);
  }

  // Process the motion
  // Based on the displacement vector (computed in display coordinates) and
  // the cursor state (which corresponds to which part of the widget has been
  // selected), the widget points are modified.
  // First construct a local coordinate system based on the display coordinates
  // of the widget.
  double eventPos[2]{
    static_cast<double>(eventData->GetDisplayPosition()[0]),
    static_cast<double>(eventData->GetDisplayPosition()[1]),
  };

  if (state == WidgetStateTranslateTransformCenter)
  {
    this->TranslateTransformCenter(eventPos);
  }

  this->LastEventPosition[0] = eventPos[0];
  this->LastEventPosition[1] = eventPos[1];
  return true;
}

//-------------------------------------------------------------------------
void vtkMRMLTransformHandleWidget::TranslateTransformCenter(double eventPos[2])
{
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
    double eventPos_Display[2] = { 0. };

    eventPos_Display[0] = this->LastEventPosition[0];
    eventPos_Display[1] = this->LastEventPosition[1];

    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos_Display, lastEventPos_World, eventPos_World, orientation_World))
    {
      return;
    }
    lastEventPos_World[0] = eventPos_World[0];
    lastEventPos_World[1] = eventPos_World[1];
    lastEventPos_World[2] = eventPos_World[2];

    eventPos_Display[0] = eventPos[0];
    eventPos_Display[1] = eventPos[1];

    if (!rep->GetPointPlacer()->ComputeWorldPosition(
          this->Renderer, eventPos_Display, lastEventPos_World, eventPos_World, orientation_World))
    {
      return;
    }
  }

  double translationVector_World[3];
  translationVector_World[0] = eventPos_World[0] - lastEventPos_World[0];
  translationVector_World[1] = eventPos_World[1] - lastEventPos_World[1];
  translationVector_World[2] = eventPos_World[2] - lastEventPos_World[2];

  int index = this->GetActiveComponentIndex();

  double translationAxis_World[3] = { 0 };
  rep->GetInteractionHandleAxisWorld(InteractionTranslationHandle, index, translationAxis_World);

  // Only perform constrained translation if the length of the axis is non-zero.
  if (vtkMath::Norm(translationAxis_World) > 0)
  {
    double lastEventPositionOnAxis_World[3] = { 0.0, 0.0, 0.0 };
    this->GetClosestPointOnInteractionAxis(
      InteractionTranslationHandle, index, this->LastEventPosition, lastEventPositionOnAxis_World);

    double eventPositionOnAxis_World[3] = { 0.0, 0.0, 0.0 };
    this->GetClosestPointOnInteractionAxis(InteractionTranslationHandle, index, eventPos, eventPositionOnAxis_World);

    vtkMath::Subtract(eventPositionOnAxis_World, lastEventPositionOnAxis_World, translationVector_World);
    double distance = vtkMath::Norm(translationVector_World);
    if (vtkMath::Dot(translationVector_World, translationAxis_World) < 0)
    {
      distance *= -1.0;
    }
    translationVector_World[0] = distance * translationAxis_World[0];
    translationVector_World[1] = distance * translationAxis_World[1];
    translationVector_World[2] = distance * translationAxis_World[2];
  }

  double origin_World[3] = { 0.0, 0.0, 0.0 };
  rep->GetInteractionHandleOriginWorld(origin_World);

  vtkNew<vtkGeneralTransform> transform;
  this->GetTransformNode()->GetTransformFromWorld(transform);

  double centerOfTransformation[3] = { 0.0, 0.0, 0.0 };
  this->GetTransformNode()->GetCenterOfTransformation(centerOfTransformation);
  vtkMath::Add(transform->TransformVectorAtPoint(origin_World, translationVector_World),
               centerOfTransformation,
               centerOfTransformation);
  this->GetTransformNode()->SetCenterOfTransformation(centerOfTransformation);
}

//-------------------------------------------------------------------------
bool vtkMRMLTransformHandleWidget::ProcessJumpCursor(vtkMRMLInteractionEventData* eventData)
{
  int type = this->GetActiveComponentType();
  if (type != InteractionTranslationHandle)
  {
    return false;
  }

  int index = this->GetActiveComponentIndex();
  if (index != 3)
  {
    return false;
  }

  return Superclass::ProcessJumpCursor(eventData);
}
