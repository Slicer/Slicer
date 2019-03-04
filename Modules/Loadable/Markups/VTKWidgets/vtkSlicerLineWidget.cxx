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

#include "vtkSlicerLineWidget.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerLineRepresentation2D.h"
#include "vtkSlicerLineRepresentation3D.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkSphereSource.h"
#include "vtkProperty.h"
#include "vtkProperty2D.h"
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkSlicerLineWidget);

//----------------------------------------------------------------------
vtkSlicerLineWidget::vtkSlicerLineWidget()
{
  this->SetEventTranslation(vtkCommand::LeftButtonPressEvent, vtkEvent::AltModifier, WidgetRotateStart);
  this->SetEventTranslation(vtkCommand::LeftButtonReleaseEvent, vtkEvent::AnyModifier, WidgetRotateEnd);

  this->SetEventTranslation(vtkCommand::RightButtonPressEvent, vtkEvent::AltModifier, WidgetScaleStart);
  this->SetEventTranslation(vtkCommand::RightButtonReleaseEvent, vtkEvent::AnyModifier, WidgetScaleEnd);

  this->SetEventTranslation(vtkCommand::RightButtonPressEvent, vtkEvent::NoModifier, WidgetPick);
}

//----------------------------------------------------------------------
vtkSlicerLineWidget::~vtkSlicerLineWidget()
{
}

//----------------------------------------------------------------------
void vtkSlicerLineWidget::CreateDefaultRepresentation(
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer)
{
  vtkSmartPointer<vtkSlicerAbstractWidgetRepresentation> rep = NULL;
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
  {
    rep = vtkSmartPointer<vtkSlicerLineRepresentation2D>::New();
  }
  else
  {
    rep = vtkSmartPointer<vtkSlicerLineRepresentation3D>::New();
  }
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetMarkupsDisplayNode(markupsDisplayNode);
  rep->UpdateFromMRML(nullptr, 0); // full update
}
