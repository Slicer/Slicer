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

#include "vtkSlicerClosedCurveWidget.h"

#include "vtkMRMLSliceNode.h"
#include "vtkSlicerCurveRepresentation2D.h"
#include "vtkSlicerCurveRepresentation3D.h"
#include "vtkCommand.h"
#include "vtkEvent.h"

vtkStandardNewMacro(vtkSlicerClosedCurveWidget);

//----------------------------------------------------------------------
vtkSlicerClosedCurveWidget::vtkSlicerClosedCurveWidget()
{
  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget, vtkCommand::LeftButtonPressEvent, vtkEvent::AltModifier,
    WidgetStateRotate, WidgetEventRotateStart, WidgetEventRotateEnd);
  this->SetEventTranslationClickAndDrag(WidgetStateOnWidget, vtkCommand::RightButtonPressEvent, vtkEvent::AltModifier,
    WidgetStateScale, WidgetEventScaleStart, WidgetEventScaleEnd);

  this->SetEventTranslation(WidgetStateOnWidget, vtkCommand::LeftButtonPressEvent, vtkEvent::ControlModifier, WidgetEventControlPointInsert);
}

//----------------------------------------------------------------------
vtkSlicerClosedCurveWidget::~vtkSlicerClosedCurveWidget() = default;

//----------------------------------------------------------------------
void vtkSlicerClosedCurveWidget::CreateDefaultRepresentation(
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
void vtkSlicerClosedCurveWidget::AddPointOnCurveAction(vtkAbstractWidget *vtkNotUsed(w))
{
  /* TODO: implement this

  vtkSlicerClosedCurveWidget *self = reinterpret_cast<vtkSlicerClosedCurveWidget*>(w);
  if ( self->WidgetState != vtkSlicerClosedCurveWidget::Manipulate)
    {
    return false;
    }

  vtkSlicerMarkupsWidgetRepresentation *rep =
    reinterpret_cast<vtkSlicerMarkupsWidgetRepresentation*>(self->WidgetRep);

  int X = self->Interactor->GetEventPosition()[0];
  int Y = self->Interactor->GetEventPosition()[1];
  double pos[2];
  pos[0] = X;
  pos[1] = Y;

  int addedControlPointIndex = rep->AddNodeOnWidget( X, Y );
  if ( addedControlPointIndex < 0 )
    {
    return false;
    }

  this->GetMarkupsNode(addedControlPointIndex)
  self->GrabFocus(self->EventCallbackCommand);
  self->StartInteraction();
  rep->StartWidgetInteraction( pos );
  self->CurrentHandle = rep->GetActiveNode();
  rep->SetCurrentOperationToPick();
  return true;
  */
}
