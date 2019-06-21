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
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#include "vtkSlicerTextsLogic.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTextNode.h>
#include <vtkMRMLTextStorageNode.h>

vtkStandardNewMacro(vtkSlicerTextsLogic);

//----------------------------------------------------------------------------
vtkSlicerTextsLogic::vtkSlicerTextsLogic() = default;

//----------------------------------------------------------------------------
vtkSlicerTextsLogic::~vtkSlicerTextsLogic() = default;
