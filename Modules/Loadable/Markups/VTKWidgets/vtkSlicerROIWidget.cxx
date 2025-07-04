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
vtkSlicerROIWidget::vtkSlicerROIWidget() = default;

//----------------------------------------------------------------------
vtkSlicerROIWidget::~vtkSlicerROIWidget() = default;

//----------------------------------------------------------------------
void vtkSlicerROIWidget::CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode,
                                                     vtkMRMLAbstractViewNode* viewNode,
                                                     vtkRenderer* renderer)
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
