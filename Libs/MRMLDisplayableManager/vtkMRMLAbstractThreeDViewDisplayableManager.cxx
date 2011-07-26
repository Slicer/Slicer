/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"

// MRML includes
#include <vtkMRMLViewNode.h>

// VTK includes

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractThreeDViewDisplayableManager);
vtkCxxRevisionMacro(vtkMRMLAbstractThreeDViewDisplayableManager, "$Revision: 13525 $");

//----------------------------------------------------------------------------
// vtkMRMLAbstractThreeDViewDisplayableManager methods

//----------------------------------------------------------------------------
vtkMRMLAbstractThreeDViewDisplayableManager::vtkMRMLAbstractThreeDViewDisplayableManager()
{
}

//----------------------------------------------------------------------------
vtkMRMLAbstractThreeDViewDisplayableManager::~vtkMRMLAbstractThreeDViewDisplayableManager()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractThreeDViewDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(
    vtkObject* caller)
{
  assert(vtkMRMLViewNode::SafeDownCast(caller));
#ifndef _DEBUG
  (void)caller;
#endif
  this->OnMRMLViewNodeModifiedEvent();
}

//---------------------------------------------------------------------------
vtkMRMLViewNode * vtkMRMLAbstractThreeDViewDisplayableManager::GetMRMLViewNode()
{
  return vtkMRMLViewNode::SafeDownCast(this->GetMRMLDisplayableNode());
}

