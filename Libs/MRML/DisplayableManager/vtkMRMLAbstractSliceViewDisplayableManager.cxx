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
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLAbstractSliceViewDisplayableManager);

//----------------------------------------------------------------------------
// vtkMRMLAbstractSliceViewDisplayableManager methods

//----------------------------------------------------------------------------
vtkMRMLAbstractSliceViewDisplayableManager::vtkMRMLAbstractSliceViewDisplayableManager() = default;

//----------------------------------------------------------------------------
vtkMRMLAbstractSliceViewDisplayableManager::~vtkMRMLAbstractSliceViewDisplayableManager() = default;

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
void vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(double x, double y, double xyz[3])
{
  Self::ConvertDeviceToXYZ(this->GetInteractor(), this->GetMRMLSliceNode(), x, y, xyz);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(
    vtkRenderWindowInteractor * interactor, vtkMRMLSliceNode * sliceNode,
    double x, double y, double xyz[3])
{
  if (xyz == nullptr || interactor == nullptr || sliceNode == nullptr)
    {
    return;
    }

  double windowWidth = interactor->GetRenderWindow()->GetSize()[0];
  double windowHeight = interactor->GetRenderWindow()->GetSize()[1];

  int numberOfColumns = sliceNode->GetLayoutGridColumns();
  int numberOfRows = sliceNode->GetLayoutGridRows();

  float tempX = x / windowWidth;
  float tempY = (windowHeight - 1 - y) / windowHeight;

  float z = floor(tempY*numberOfRows)*numberOfColumns + floor(tempX*numberOfColumns);

  vtkRenderer* pokedRenderer = interactor->FindPokedRenderer(x,y);

  xyz[0] = x - (pokedRenderer ? pokedRenderer->GetOrigin()[0] : 0.);
  xyz[1] = y - (pokedRenderer ? pokedRenderer->GetOrigin()[1] : 0.);
  xyz[2] = z;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::ConvertDeviceToXYZ(
    vtkRenderer * renderer, vtkMRMLSliceNode * sliceNode,
    double x, double y, double xyz[3])
{
  if (xyz == nullptr || renderer == nullptr || sliceNode == nullptr)
    {
    return;
    }

  double windowWidth = renderer->GetRenderWindow()->GetSize()[0];
  double windowHeight = renderer->GetRenderWindow()->GetSize()[1];

  int numberOfColumns = sliceNode->GetLayoutGridColumns();
  int numberOfRows = sliceNode->GetLayoutGridRows();

  float tempX = x / windowWidth;
  float tempY = (windowHeight - 1 - y) / windowHeight;

  float z = floor(tempY*numberOfRows)*numberOfColumns + floor(tempX*numberOfColumns);

  xyz[0] = x - renderer->GetOrigin()[0];
  xyz[1] = y - renderer->GetOrigin()[1];
  xyz[2] = z;
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::ConvertRASToXYZ(double ras[3], double xyz[3])
{
  Self::ConvertRASToXYZ(this->GetMRMLSliceNode(), ras, xyz);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::ConvertRASToXYZ(
    vtkMRMLSliceNode * sliceNode, double ras[3], double xyz[3])
{
  if (sliceNode == nullptr)
    {
    return;
    }
  vtkNew<vtkMatrix4x4> rasToXYZ;
  vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToXYZ.GetPointer());

  double rasw[4] = { ras[0], ras[1], ras[2], 1.0 };
  double xyzw[4];
  rasToXYZ->MultiplyPoint(rasw, xyzw);
  xyz[0] = xyzw[0]/xyzw[3]; xyz[1] = xyzw[1]/xyzw[3]; xyz[2] = xyzw[2]/xyzw[3];
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::ConvertXYZToRAS(double xyz[3], double ras[3])
{
  Self::ConvertXYZToRAS(this->GetMRMLSliceNode(), xyz, ras);
}

//---------------------------------------------------------------------------
void vtkMRMLAbstractSliceViewDisplayableManager::ConvertXYZToRAS(
    vtkMRMLSliceNode * sliceNode, double xyz[3], double ras[3])
{
  if (sliceNode == nullptr)
    {
    return;
    }
  double rasw[4], xyzw[4];
  xyzw[0] = xyz[0]; xyzw[1] = xyz[1]; xyzw[2] = xyz[2]; xyzw[3] = 1.0;

  sliceNode->GetXYToRAS()->MultiplyPoint(xyzw, rasw);
  ras[0] = rasw[0]/rasw[3]; ras[1] = rasw[1]/rasw[3]; ras[2] = rasw[2]/rasw[3];
}
