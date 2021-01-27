/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Cameras Logic includes
#include "vtkSlicerCamerasModuleLogic.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerCamerasModuleLogic);

//----------------------------------------------------------------------------
vtkSlicerCamerasModuleLogic::vtkSlicerCamerasModuleLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerCamerasModuleLogic::~vtkSlicerCamerasModuleLogic() = default;

//----------------------------------------------------------------------------
void vtkSlicerCamerasModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLCameraNode* vtkSlicerCamerasModuleLogic
::GetViewActiveCameraNode(vtkMRMLViewNode* viewNode)
{
  if (!this->GetMRMLScene())
    {
    return nullptr;
    }
  vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(
    this->GetMRMLScene()->GetSingletonNode(viewNode->GetLayoutName(), "vtkMRMLCameraNode"));
  return cameraNode;
}
