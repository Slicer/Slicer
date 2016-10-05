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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Terminologies includes
#include "vtkSlicerTerminologiesModuleLogic.h"

// MRMLLogic includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkIntArray.h>

// JSON includes
#include "vtkjsoncpp/json/json.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTerminologiesModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkSlicerTerminologiesModuleLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::~vtkSlicerTerminologiesModuleLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent);
  this->SetAndObserveMRMLSceneEvents(newScene, events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::OnMRMLSceneEndClose()
{
  if (!this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneEndClose: Invalid MRML scene!");
    return;
  }
}
