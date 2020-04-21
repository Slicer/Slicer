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

#include "vtkSlicerCurveWidget.h"

#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerCurveRepresentation2D.h"
#include "vtkSlicerCurveRepresentation3D.h"

#include "vtkCommand.h"
#include "vtkEvent.h"

vtkStandardNewMacro(vtkSlicerCurveWidget);

//----------------------------------------------------------------------
vtkSlicerCurveWidget::vtkSlicerCurveWidget()
{
  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget, vtkCommand::LeftButtonPressEvent, vtkEvent::AltModifier,
    WidgetStateRotate, WidgetEventRotateStart, WidgetEventRotateEnd);
  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget, vtkCommand::RightButtonPressEvent, vtkEvent::AltModifier,
    WidgetStateScale, WidgetEventScaleStart, WidgetEventScaleEnd);

  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::LeftButtonPressEvent, vtkEvent::ControlModifier, WidgetEventControlPointInsert);
}

//----------------------------------------------------------------------
vtkSlicerCurveWidget::~vtkSlicerCurveWidget() = default;

//----------------------------------------------------------------------
void vtkSlicerCurveWidget::CreateDefaultRepresentation(
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer)
{
  vtkSmartPointer<vtkSlicerMarkupsWidgetRepresentation> rep = nullptr;
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
    {
    rep = vtkSmartPointer<vtkSlicerCurveRepresentation2D>::New();
    }
  else
    {
    rep = vtkSmartPointer<vtkSlicerCurveRepresentation3D>::New();
    }
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetMarkupsDisplayNode(markupsDisplayNode);
  rep->UpdateFromMRML(nullptr, 0); // full update
}

//----------------------------------------------------------------------
bool vtkSlicerCurveWidget::ProcessControlPointInsert(vtkMRMLInteractionEventData* eventData)
{
  vtkMRMLMarkupsCurveNode* markupsNode = this->GetMarkupsCurveNode();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode = this->GetMarkupsDisplayNode();
  if (!markupsNode || !markupsDisplayNode)
    {
    return false;
    }

  int foundComponentType = vtkMRMLMarkupsDisplayNode::ComponentNone;
  int foundComponentIndex = -1;
  double closestDistance2 = 0.0;

  // Force finding closest line (CanInteract() method would not tell near control points which side of the markup we are at)
  vtkSlicerCurveRepresentation2D* rep2d = vtkSlicerCurveRepresentation2D::SafeDownCast(this->WidgetRep);
  vtkSlicerCurveRepresentation3D* rep3d = vtkSlicerCurveRepresentation3D::SafeDownCast(this->WidgetRep);
  if (rep2d)
    {
    rep2d->CanInteractWithCurve(eventData, foundComponentType, foundComponentIndex, closestDistance2);
    }
  else if (rep3d)
    {
    rep3d->CanInteractWithCurve(eventData, foundComponentType, foundComponentIndex, closestDistance2);
    }
  else
    {
    return false;
    }
  if (foundComponentType != vtkMRMLMarkupsDisplayNode::ComponentLine)
    {
    return false;
    }

  // Determine point position in local coordinate system
  double worldPos[3] = { 0.0 };
  const int* displayPos = eventData->GetDisplayPosition();
  if (rep3d)
    {
    if (!eventData->IsWorldPositionValid())
      {
      return false;
      }
    vtkIdType lineIndex = markupsNode->GetClosestPointPositionAlongCurveWorld(eventData->GetWorldPosition(), worldPos);
    if (lineIndex < 0)
      {
      return false;
      }
    }
  else
    {
    double doubleDisplayPos[3] = { static_cast<double>(displayPos[0]), static_cast<double>(displayPos[1]), 0.0 };
    rep2d->GetSliceToWorldCoordinates(doubleDisplayPos, worldPos);
    }

  markupsNode->GetScene()->SaveStateForUndo();

  // Create new control point and insert
  vtkMRMLMarkupsNode::ControlPoint* controlPoint = new vtkMRMLMarkupsNode::ControlPoint;
  vtkMRMLMarkupsNode::ControlPoint* foundControlPoint = markupsNode->GetNthControlPoint(foundComponentIndex);
  if (foundControlPoint)
    {
    (*controlPoint) = (*foundControlPoint);
    }
  else
    {
    vtkWarningMacro("ProcessControlPointInsert: Found control point is out of bounds");
    }
  markupsNode->TransformPointFromWorld(worldPos, controlPoint->Position);
  if (!markupsNode->InsertControlPoint(controlPoint, foundComponentIndex + 1))
    {
    delete controlPoint;
    return false;
    }

  // Activate the control point that has just been inserted
  this->SetWidgetState(WidgetStateOnWidget);
  this->GetMarkupsDisplayNode()->SetActiveControlPoint(foundComponentIndex + 1);

  return true;
}

//----------------------------------------------------------------------
vtkMRMLMarkupsCurveNode* vtkSlicerCurveWidget::GetMarkupsCurveNode()
{
  vtkSlicerMarkupsWidgetRepresentation* rep = this->GetMarkupsRepresentation();
  if (!rep)
    {
    return nullptr;
    }
  return vtkMRMLMarkupsCurveNode::SafeDownCast(rep->GetMarkupsNode());
}
