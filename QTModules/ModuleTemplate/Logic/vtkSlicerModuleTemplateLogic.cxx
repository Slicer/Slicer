/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// ModuleTemplate includes
#include "vtkSlicerModuleTemplateLogic.h"

// MRML includes

// VTK includes
#include <vtkNew.h>


//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerModuleTemplateLogic, "$Revision: 1.0$")
vtkStandardNewMacro(vtkSlicerModuleTemplateLogic);

//----------------------------------------------------------------------------
vtkSlicerModuleTemplateLogic::vtkSlicerModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerModuleTemplateLogic::~vtkSlicerModuleTemplateLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerModuleTemplateLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerModuleTemplateLogic::InitializeEventListeners()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(this->GetMRMLScene(), events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerModuleTemplateLogic::ProcessMRMLEvents(
  vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(event), void * vtkNotUsed(callData))
{
}

//-----------------------------------------------------------------------------
void vtkSlicerModuleTemplateLogic::RegisterNodes()
{
  if(!this->GetMRMLScene())
    {
    return;
    }

}


