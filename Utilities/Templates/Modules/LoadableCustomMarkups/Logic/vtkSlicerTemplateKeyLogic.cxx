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

#include "vtkSlicerTemplateKeyLogic.h"

// TemplateKey MRML includes
#include "vtkMRMLMarkupsTestLineNode.h"

// TemplateKey VTKWidgets includes
#include "vtkSlicerTestLineWidget.h"

// MRML includes
#include <vtkMRMLScene.h>

// Markups logic includes
#include <vtkSlicerMarkupsLogic.h>

// Markups MRML includes
#include <vtkMRMLMarkupsDisplayNode.h>

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTemplateKeyLogic);

//---------------------------------------------------------------------------
vtkSlicerTemplateKeyLogic::vtkSlicerTemplateKeyLogic() {}

//---------------------------------------------------------------------------
vtkSlicerTemplateKeyLogic::~vtkSlicerTemplateKeyLogic() = default;

//---------------------------------------------------------------------------
void vtkSlicerTemplateKeyLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkSlicerTemplateKeyLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes failed: invalid scene");
    return;
  }

  vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(this->GetModuleLogic("Markups"));
  if (!markupsLogic)
  {
    vtkErrorMacro("RegisterNodes failed: invalid markups module logic");
    return;
  }

  vtkNew<vtkMRMLMarkupsTestLineNode> markupsTestLineNode;
  vtkNew<vtkSlicerTestLineWidget> testLineWidget;
  markupsLogic->RegisterMarkupsNode(markupsTestLineNode, testLineWidget);
}
