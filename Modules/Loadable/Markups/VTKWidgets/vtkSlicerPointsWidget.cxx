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

#include "vtkSlicerPointsWidget.h"

#include "vtkMRMLInteractionEventData.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerPointsRepresentation2D.h"
#include "vtkSlicerPointsRepresentation3D.h"

vtkStandardNewMacro(vtkSlicerPointsWidget);

//----------------------------------------------------------------------
vtkSlicerPointsWidget::vtkSlicerPointsWidget() = default;

//----------------------------------------------------------------------
vtkSlicerPointsWidget::~vtkSlicerPointsWidget() = default;

//----------------------------------------------------------------------
void vtkSlicerPointsWidget::CreateDefaultRepresentation(
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode, vtkMRMLAbstractViewNode* viewNode, vtkRenderer* renderer)
{
  vtkSmartPointer<vtkSlicerMarkupsWidgetRepresentation> rep = nullptr;
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
    {
    rep = vtkSmartPointer<vtkSlicerPointsRepresentation2D>::New();
    }
  else
    {
    rep = vtkSmartPointer<vtkSlicerPointsRepresentation3D>::New();
    }
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetMarkupsDisplayNode(markupsDisplayNode);
  rep->UpdateFromMRML(nullptr, 0); // full update
}
