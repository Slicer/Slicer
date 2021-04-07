/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#include "vtkSlicerTestLineWidget.h"

// Liver Markups VTKWidgets include
#include "vtkSlicerTestLineRepresentation3D.h"
#include "vtkSlicerTestLineRepresentation2D.h"

// VTK includes
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLSliceNode.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTestLineWidget);

//------------------------------------------------------------------------------
vtkSlicerTestLineWidget::vtkSlicerTestLineWidget()
{

}

//------------------------------------------------------------------------------
vtkSlicerTestLineWidget::~vtkSlicerTestLineWidget() = default;

//------------------------------------------------------------------------------
void vtkSlicerTestLineWidget::CreateDefaultRepresentation(vtkMRMLMarkupsDisplayNode* markupsDisplayNode,
                                                          vtkMRMLAbstractViewNode* viewNode,
                                                          vtkRenderer* renderer)
{
  vtkSmartPointer<vtkSlicerMarkupsWidgetRepresentation> rep = nullptr;
  if (vtkMRMLSliceNode::SafeDownCast(viewNode))
    {
    rep = vtkSmartPointer<vtkSlicerTestLineRepresentation2D>::New();
    }
  else
    {
    rep = vtkSmartPointer<vtkSlicerTestLineRepresentation3D>::New();
    }
  this->SetRenderer(renderer);
  this->SetRepresentation(rep);
  rep->SetViewNode(viewNode);
  rep->SetMarkupsDisplayNode(markupsDisplayNode);
  rep->UpdateFromMRML(nullptr, 0); // full update
}

//------------------------------------------------------------------------------
vtkSlicerMarkupsWidget* vtkSlicerTestLineWidget::CreateInstance() const
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerTestLineWidget");
  if(ret)
    {
    return static_cast<vtkSlicerTestLineWidget*>(ret);
    }

  vtkSlicerTestLineWidget* result = new vtkSlicerTestLineWidget;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
  result->InitializeObjectBase();
#endif
  return result;
}
