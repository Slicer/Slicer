/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"

// MRML includes
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractSliceViewDisplayableManager);
vtkCxxRevisionMacro(vtkMRMLAbstractSliceViewDisplayableManager, "$Revision: 13525 $");

//----------------------------------------------------------------------------
// vtkMRMLAbstractSliceViewDisplayableManager methods

//----------------------------------------------------------------------------
vtkMRMLAbstractSliceViewDisplayableManager::vtkMRMLAbstractSliceViewDisplayableManager()
{
}

//----------------------------------------------------------------------------
vtkMRMLAbstractSliceViewDisplayableManager::~vtkMRMLAbstractSliceViewDisplayableManager()
{
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::OnMRMLDisplayableNodeModifiedEvent(
    vtkObject* caller)
{
  assert(vtkMRMLSliceNode::SafeDownCast(caller));
#ifndef _DEBUG
  (void)caller;
#endif
  this->OnMRMLSliceNodeModifiedEvent();
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode * vtkMRMLAbstractSliceViewDisplayableManager::GetMRMLSliceNode()
{
  return vtkMRMLSliceNode::SafeDownCast(this->GetMRMLDisplayableNode());
}


//---------------------------------------------------------------------------
/// Convert display to viewport coordinates
void vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(double x, double y, double * xyz)
{
  if (xyz == NULL || this->GetInteractor() == NULL || this->GetMRMLSliceNode() == NULL)
    {
    return;
    }

  double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
  double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];
  
  int numberOfColumns = this->GetMRMLSliceNode()->GetLayoutGridColumns();
  int numberOfRows = this->GetMRMLSliceNode()->GetLayoutGridRows();

  float tempX = x / windowWidth;
  float tempY = (windowHeight - y) / windowHeight;

  float z = floor(tempY*numberOfRows)*numberOfColumns + floor(tempX*numberOfColumns);

  vtkRenderer* pokedRenderer = this->GetInteractor()->FindPokedRenderer(x,y);

  xyz[0] = x - pokedRenderer->GetOrigin()[0];
  xyz[1] = y - pokedRenderer->GetOrigin()[1];
  xyz[2] = z;
  xyz[3] = 1;
}

