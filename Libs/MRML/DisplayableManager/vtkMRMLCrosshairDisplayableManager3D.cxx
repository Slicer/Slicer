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

  This file was originally developed by Andras Lasso (PerkLab, Queen's University).

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLCrosshairDisplayableManager3D.h"
#include "vtkMRMLCrosshairDisplayableManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkHandleWidget.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// STD includes
#include <algorithm>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLCrosshairDisplayableManager3D );

//---------------------------------------------------------------------------
class vtkMRMLCrosshairDisplayableManager3D::vtkInternal
{
public:
  vtkInternal(vtkMRMLCrosshairDisplayableManager3D * external);
  ~vtkInternal();

  vtkObserverManager* GetMRMLNodesObserverManager();
  void Modified();

  // Crosshair
  void SetCrosshairNode(vtkMRMLCrosshairNode* crosshairNode);

  // Build the crosshair representation
  void BuildCrosshair();

  vtkMRMLCrosshairDisplayableManager3D* External;

  vtkWeakPointer<vtkRenderWindowInteractor> RenderWindowInteractor;
  vtkSmartPointer<vtkPointHandleRepresentation3D> CrosshairRepresentation;
  vtkSmartPointer<vtkHandleWidget> CrosshairWidget;

  vtkWeakPointer<vtkMRMLCrosshairNode> CrosshairNode;
  int CrosshairMode;
  int CrosshairThickness;
  double CrosshairPosition[3];
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager3D::vtkInternal
::vtkInternal(vtkMRMLCrosshairDisplayableManager3D * external)
{
  this->External = external;
  this->CrosshairMode = -1;
  this->CrosshairThickness = -1;
  this->CrosshairPosition[0] = 0.0;
  this->CrosshairPosition[1] = 0.0;
  this->CrosshairPosition[2] = 0.0;

  this->CrosshairRepresentation = vtkSmartPointer<vtkPointHandleRepresentation3D>::New();
  this->CrosshairRepresentation->SetPlaceFactor(2.5);
  this->CrosshairRepresentation->SetHandleSize(30);
  this->CrosshairRepresentation->GetProperty()->SetColor(1.0, 0.8, 0.1);

  this->CrosshairWidget = vtkSmartPointer<vtkHandleWidget>::New();
  this->CrosshairWidget->SetRepresentation(this->CrosshairRepresentation);
  this->CrosshairWidget->EnabledOff();
  this->CrosshairWidget->ProcessEventsOff();
}

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager3D::vtkInternal::~vtkInternal()
{
  this->SetCrosshairNode(nullptr);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkMRMLCrosshairDisplayableManager3D::vtkInternal::GetMRMLNodesObserverManager()
{
  return this->External->GetMRMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::vtkInternal
::SetCrosshairNode(vtkMRMLCrosshairNode* crosshairNode)
{
  if (this->CrosshairNode == crosshairNode)
    {
    return;
    }
  vtkSetAndObserveMRMLNodeMacro(this->CrosshairNode, crosshairNode);
  this->BuildCrosshair();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::vtkInternal::BuildCrosshair()
{
  vtkRenderWindowInteractor* interactor = this->External->GetInteractor();
  if (!this->CrosshairNode.GetPointer() || !interactor)
    {
    this->CrosshairWidget->SetInteractor(nullptr);
    return;
    }

  this->CrosshairMode = this->CrosshairNode->GetCrosshairMode();
  if (this->CrosshairNode->GetCrosshairMode() == vtkMRMLCrosshairNode::NoCrosshair)
    {
    this->CrosshairWidget->EnabledOff();
    return;
    }
  this->CrosshairWidget->SetInteractor(interactor);
  this->CrosshairWidget->EnabledOn();

  int *screenSize = interactor->GetRenderWindow()->GetScreenSize();

  // Handle size is defined a percentage of screen size to accommodate high-DPI screens
  double handleSizeInScreenSizePercent = 5;
  if (this->CrosshairNode->GetCrosshairMode() == vtkMRMLCrosshairNode::ShowSmallBasic
    || this->CrosshairNode->GetCrosshairMode() == vtkMRMLCrosshairNode::ShowSmallIntersection)
    {
    handleSizeInScreenSizePercent = 2.5;
    }
  double handleSizeInPixels = double(screenSize[1])*(0.01*handleSizeInScreenSizePercent);
  this->CrosshairRepresentation->SetHandleSize(handleSizeInPixels);

  // Line Width
  // Base width is 1 on a full HD display.
  double baseWidth = 1 + int(screenSize[1] / 1000);
  switch (this->CrosshairNode->GetCrosshairThickness())
    {
    case vtkMRMLCrosshairNode::Medium:
      this->CrosshairRepresentation->GetProperty()->SetLineWidth(baseWidth * 2);
      break;
    case vtkMRMLCrosshairNode::Thick:
      this->CrosshairRepresentation->GetProperty()->SetLineWidth(baseWidth * 3);
      break;
    case vtkMRMLCrosshairNode::Fine:
    default:
      this->CrosshairRepresentation->GetProperty()->SetLineWidth(baseWidth);
      break;
    }
  this->CrosshairThickness = this->CrosshairNode->GetCrosshairThickness();
}

//---------------------------------------------------------------------------
// vtkMRMLCrosshairDisplayableManager3D methods

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager3D::vtkMRMLCrosshairDisplayableManager3D()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager3D::~vtkMRMLCrosshairDisplayableManager3D()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::ObserveMRMLScene()
{
  this->Internal->BuildCrosshair();
  this->Superclass::ObserveMRMLScene();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::UpdateFromMRMLScene()
{
  // search for the Crosshair node
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(this->GetMRMLScene());
  this->Internal->SetCrosshairNode(crosshairNode);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::UnobserveMRMLScene()
{
  this->Internal->SetCrosshairNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::OnMRMLNodeModified(vtkMRMLNode* node)
{
  if (!vtkMRMLCrosshairNode::SafeDownCast(node))
    {
    return;
    }

  // update the properties and style of the crosshair
  if (this->Internal->CrosshairMode != this->Internal->CrosshairNode->GetCrosshairMode()
    || (this->Internal->CrosshairMode != vtkMRMLCrosshairNode::NoCrosshair
      && this->Internal->CrosshairThickness != this->Internal->CrosshairNode->GetCrosshairThickness()))
    {
    this->Internal->BuildCrosshair();
    this->RequestRender();
    }

  // update the position of the actor
  double *ras = this->Internal->CrosshairNode->GetCrosshairRAS();
  double *lastRas = this->Internal->CrosshairPosition;
  double eps = 1.0e-12;
  if (fabs(lastRas[0] - ras[0]) > eps
    || fabs(lastRas[1] - ras[1]) > eps
    || fabs(lastRas[2] - ras[2]) > eps)
    {
    this->Internal->CrosshairRepresentation->SetWorldPosition(ras);
    lastRas[0] = ras[0];
    lastRas[1] = ras[1];
    lastRas[2] = ras[2];
    if (this->Internal->CrosshairMode != vtkMRMLCrosshairNode::NoCrosshair)
      {
      this->RequestRender();
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::Create()
{
  this->UpdateFromMRMLScene();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager3D::AdditionalInitializeStep()
{
  // Build the initial crosshair representation
  this->Internal->BuildCrosshair();
}
