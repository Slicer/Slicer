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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLDisplayableManager includes
#include "vtkMRMLCrosshairDisplayableManager.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLLightBoxRendererManagerProxy.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceIntersectionWidget.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPickingManager.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProp.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkVersion.h>

// STD includes
#include <algorithm>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLCrosshairDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLCrosshairDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLCrosshairDisplayableManager * external);
  ~vtkInternal();

  vtkObserverManager* GetMRMLNodesObserverManager();
  void Modified();

  // Slice
  vtkMRMLSliceNode* GetSliceNode();
  void UpdateSliceNode();
  void UpdateIntersectingSliceNodes();
  // Crosshair
  void SetCrosshairNode(vtkMRMLCrosshairNode* crosshairNode);

  // Actors
  void SetActor(vtkActor2D* prop) {Actor = prop;};

  // Build the crosshair representation
  void BuildCrosshair();

  // Add a line to the crosshair in display coordinates (needs to be
  // passed the points and cellArray to manipulate).
  void AddCrosshairLine(vtkPoints *pts, vtkCellArray *cellArray,
                        int p1x, int p1y, int p2x, int p2y);

  // Has crosshair position changed?
  bool HasCrosshairPositionChanged();

  // Has crosshair property changed?
  bool HasCrosshairPropertyChanged();

  vtkMRMLCrosshairDisplayableManager*        External;
  int                                        PickState;
  int                                        ActionState;
  vtkSmartPointer<vtkActor2D>                Actor;
  vtkWeakPointer<vtkRenderer>                LightBoxRenderer;

  vtkWeakPointer<vtkMRMLCrosshairNode>       CrosshairNode;
  int CrosshairMode;
  int CrosshairThickness;
  double CrosshairPosition[3];

  vtkSmartPointer<vtkMRMLSliceIntersectionWidget> SliceIntersectionWidget;
};


//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager::vtkInternal
::vtkInternal(vtkMRMLCrosshairDisplayableManager * external)
{
  this->External = external;
  this->CrosshairNode = nullptr;
  this->Actor = nullptr;
  this->LightBoxRenderer = nullptr;
  this->CrosshairMode = -1;
  this->CrosshairThickness = -1;
  this->CrosshairPosition[0] = 0.0;
  this->CrosshairPosition[1] = 0.0;
  this->CrosshairPosition[2] = 0.0;
  this->SliceIntersectionWidget = vtkSmartPointer<vtkMRMLSliceIntersectionWidget>::New();
}

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetCrosshairNode(nullptr);
  this->LightBoxRenderer = nullptr;

  if (this->SliceIntersectionWidget)
    {
    this->SliceIntersectionWidget->SetMRMLApplicationLogic(nullptr);
    this->SliceIntersectionWidget->SetRenderer(nullptr);
    this->SliceIntersectionWidget->SetSliceNode(nullptr);
    }

  // everything should be empty
  assert(this->CrosshairNode == nullptr);
}

//---------------------------------------------------------------------------
vtkObserverManager* vtkMRMLCrosshairDisplayableManager::vtkInternal::GetMRMLNodesObserverManager()
{
  return this->External->GetMRMLNodesObserverManager();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::vtkInternal::Modified()
{
  return this->External->Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLCrosshairDisplayableManager::vtkInternal::HasCrosshairPositionChanged()
{
  if (this->CrosshairNode.GetPointer() == nullptr)
    {
    return false;
    }

  // update the position of the actor
  double *ras = this->CrosshairNode->GetCrosshairRAS();
  double *lastRas = this->CrosshairPosition;
  double eps = 1.0e-12;
  if (fabs(lastRas[0] - ras[0]) > eps
    || fabs(lastRas[1] - ras[1]) > eps
    || fabs(lastRas[2] - ras[2]) > eps)
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLCrosshairDisplayableManager::vtkInternal::HasCrosshairPropertyChanged()
{
  if (this->CrosshairNode.GetPointer() == nullptr)
    {
    return false;
    }

  if (this->CrosshairMode != this->CrosshairNode->GetCrosshairMode()
    || this->CrosshairThickness != this->CrosshairNode->GetCrosshairThickness())
    {
    return true;
    }
  else
    {
    return false;
    }
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLCrosshairDisplayableManager::vtkInternal::GetSliceNode()
{
  return this->External->GetMRMLSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::vtkInternal::UpdateSliceNode()
{
  assert(!this->GetSliceNode() || this->GetSliceNode()->GetLayoutName());

  // search for the Crosshair node
  vtkMRMLCrosshairNode* crosshairNode = vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(this->External->GetMRMLScene());
  this->SetCrosshairNode(crosshairNode);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::vtkInternal::UpdateIntersectingSliceNodes()
{
  if (this->External->GetMRMLScene() == nullptr)
    {
    this->SliceIntersectionWidget->SetSliceNode(nullptr);
    return;
    }

  vtkMRMLApplicationLogic *mrmlAppLogic = this->External->GetMRMLApplicationLogic();

  if (!this->SliceIntersectionWidget->GetRenderer())
    {
    this->SliceIntersectionWidget->SetMRMLApplicationLogic(mrmlAppLogic);
    this->SliceIntersectionWidget->CreateDefaultRepresentation();
    this->SliceIntersectionWidget->SetRenderer(this->External->GetRenderer());
    this->SliceIntersectionWidget->SetSliceNode(this->GetSliceNode());
    }
  else
    {
    this->SliceIntersectionWidget->SetSliceNode(this->GetSliceNode());
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::vtkInternal
::SetCrosshairNode(vtkMRMLCrosshairNode* crosshairNode)
{
  if (this->CrosshairNode == crosshairNode)
    {
    return;
    }
  vtkSetAndObserveMRMLNodeMacro(this->CrosshairNode, crosshairNode);
}

//---------------------------------------------------------------------------
vtkMRMLCrosshairNode* vtkMRMLCrosshairDisplayableManager::FindCrosshairNode(vtkMRMLScene* scene)
{
  if (scene == nullptr)
    {
    return nullptr;
    }

  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> crosshairs;
  crosshairs.TakeReference(scene->GetNodesByClass("vtkMRMLCrosshairNode"));
  for (crosshairs->InitTraversal(it);
       (node = (vtkMRMLNode*)crosshairs->GetNextItemAsObject(it)) ;)
    {
    vtkMRMLCrosshairNode* crosshairNode =
      vtkMRMLCrosshairNode::SafeDownCast(node);
    if (crosshairNode
        && crosshairNode->GetCrosshairName() == std::string("default"))
      {
      return crosshairNode;
      }
    }
  // no matching crosshair node is found
  //assert(0);
  return nullptr;
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::vtkInternal::BuildCrosshair()
{
  // Remove the old actor is any
  if (this->Actor.GetPointer())
    {
    if (this->LightBoxRenderer)
      {
      this->LightBoxRenderer->RemoveActor(this->Actor);
      }
    this->Actor = nullptr;
    }

  if (!this->CrosshairNode.GetPointer())
    {
    return;
    }

  // Get the size of the window
  const int *screenSize = this->External->GetInteractor()->GetRenderWindow()->GetScreenSize();

  // Constants in display coordinates to define the crosshair
  int negW = -1.0*screenSize[0];
  int negWminus = -5;
  int negWminus2 = -10;
  int posWplus = 5;
  int posWplus2 = 10;
  int posW = screenSize[0];

  int negH = -1.0*screenSize[1];
  int negHminus = -5;
  int negHminus2 = -10;
  int posHplus = 5;
  int posHplus2 = 10;
  int posH = screenSize[1];

  // Set up the VTK data structures
  vtkNew<vtkPolyData> polyData;
  vtkNew<vtkCellArray> cellArray;
  vtkNew<vtkPoints> points;
  polyData->SetLines(cellArray.GetPointer());
  polyData->SetPoints(points.GetPointer());

  vtkNew<vtkPolyDataMapper2D> mapper;
  vtkNew<vtkActor2D> actor;
  mapper->SetInputData(polyData.GetPointer());
  actor->SetMapper(mapper.GetPointer());

  if (this->LightBoxRenderer)
    {
    this->LightBoxRenderer->AddActor(actor.GetPointer());
    }

  // Cache the actor
  this->SetActor(actor.GetPointer());

  // Define the geometry
  switch (this->CrosshairNode->GetCrosshairMode())
    {
    case vtkMRMLCrosshairNode::NoCrosshair:
      break;
    case vtkMRMLCrosshairNode::ShowBasic:
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, negH, 0, negHminus);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, posHplus, 0, posH);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             negW, 0, negWminus, 0);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             posWplus, 0, posW, 0);
      break;
    case vtkMRMLCrosshairNode::ShowIntersection:
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             negW, 0, posW, 0);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, negH, 0, posH);
      break;
    case vtkMRMLCrosshairNode::ShowSmallBasic:
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, negHminus2, 0, negHminus);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, posHplus, 0, posHplus2);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             negWminus2, 0, negWminus, 0);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             posWplus, 0, posWplus2, 0);
      break;
    case vtkMRMLCrosshairNode::ShowSmallIntersection:
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             0, negHminus2, 0, posHplus2);
      this->AddCrosshairLine(points.GetPointer(), cellArray.GetPointer(),
                             negWminus2, 0, posWplus2, 0);
      break;
    default:
      break;
    }

  // Set the properties
  //

  // Line Width
  if (this->CrosshairNode->GetCrosshairThickness() == vtkMRMLCrosshairNode::Fine)
    {
    actor->GetProperty()->SetLineWidth(1);
    }
  else if (this->CrosshairNode->GetCrosshairThickness() == vtkMRMLCrosshairNode::Medium)
    {
    actor->GetProperty()->SetLineWidth(3);
    }
  else if (this->CrosshairNode->GetCrosshairThickness() == vtkMRMLCrosshairNode::Thick)
    {
    actor->GetProperty()->SetLineWidth(5);
    }

  // Color
  actor->GetProperty()->SetColor(1.0, 0.8, 0.1);
  actor->GetProperty()->SetOpacity(1.0);


  // Set the visibility
  if (this->CrosshairNode->GetCrosshairMode() == vtkMRMLCrosshairNode::NoCrosshair)
    {
    actor->VisibilityOff();
    }
  else
    {
    actor->VisibilityOn();
    }

  this->CrosshairMode = this->CrosshairNode->GetCrosshairMode();
  this->CrosshairThickness = this->CrosshairNode->GetCrosshairThickness();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::vtkInternal::AddCrosshairLine(vtkPoints *pts, vtkCellArray *cellArray, int p1x, int p1y, int p2x, int p2y)
{
  vtkIdType p1 = pts->InsertNextPoint(p1x, p1y, 0);
  vtkIdType p2 = pts->InsertNextPoint(p2x, p2y, 0);

  cellArray->InsertNextCell(2);
  cellArray->InsertCellPoint(p1);
  cellArray->InsertCellPoint(p2);
}

//---------------------------------------------------------------------------
// vtkMRMLCrosshairDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager::vtkMRMLCrosshairDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager::~vtkMRMLCrosshairDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::ObserveMRMLScene()
{
  this->Internal->BuildCrosshair();
  this->Superclass::ObserveMRMLScene();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::UpdateFromMRMLScene()
{
  this->Internal->UpdateSliceNode();
  this->Internal->UpdateIntersectingSliceNodes();
  //this->Internal->SliceIntersectionWidget->SetSliceNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->SliceIntersectionWidget->SetSliceNode(nullptr);
  this->Internal->SetCrosshairNode(nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::OnMRMLNodeModified(
    vtkMRMLNode* vtkNotUsed(node))
{
  // update the properties and style of the crosshair
  bool builtCrosshair = false;
  if (this->Internal->HasCrosshairPropertyChanged())
    {
    this->Internal->BuildCrosshair();
    builtCrosshair = true;
    }

  // update the position of the actor
  if ((this->Internal->HasCrosshairPositionChanged() || builtCrosshair)
      && this->Internal->Actor)
    {
    double xyz[3];
    double *ras = this->Internal->CrosshairNode->GetCrosshairRAS();
    this->ConvertRASToXYZ(ras, xyz);

    this->Internal->Actor->SetPosition(xyz[0], xyz[1]);

    // put the actor in the right lightbox
    if (this->GetLightBoxRendererManagerProxy())
      {
      int id = (int) (floor(xyz[2] + 0.5)); // round to find the lightbox
      vtkRenderer *renderer
        = this->GetLightBoxRendererManagerProxy()->GetRenderer(id);
      if (renderer == nullptr)
        {
        // crosshair must not be displayed in this view
        this->Internal->Actor->SetVisibility(false);
        }
      else
        {
        // crosshair must be displayed in this view
        if (this->Internal->LightBoxRenderer == renderer)
          {
          this->Internal->Actor->SetVisibility(true);
          }
        else
          {
          if (this->Internal->LightBoxRenderer)
            {
            this->Internal->LightBoxRenderer->RemoveActor(this->Internal->Actor);
            }
          this->Internal->Actor->SetVisibility(true);
          renderer->AddActor(this->Internal->Actor);
          this->Internal->LightBoxRenderer = renderer;
          }
        }
      }

    double *lastRas = this->Internal->CrosshairPosition;
    lastRas[0] = ras[0];
    lastRas[1] = ras[1];
    lastRas[2] = ras[2];
    }

  // Request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::Create()
{
  // Setup the SliceNode, CrosshairNode
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::AdditionalInitializeStep()
{
  // Build the initial crosshair representation
  this->Internal->BuildCrosshair();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::OnMRMLSliceNodeModifiedEvent()
{
  if (this->Internal->CrosshairNode)
    {
    // slice position may have changed

    // change last crosshair position to force a position update
    this->Internal->CrosshairPosition[0] += 100;

    // update cursor RAS position from XYZ (normalized screen) position
    double xyz[3] = { 0.0 };
    vtkMRMLSliceNode *crosshairSliceNode = this->Internal->CrosshairNode->GetCursorPositionXYZ(xyz);
    if (crosshairSliceNode != nullptr && crosshairSliceNode == this->Internal->GetSliceNode())
      {
      this->Internal->CrosshairNode->SetCursorPositionXYZ(xyz, crosshairSliceNode);
      }

    this->OnMRMLNodeModified(this->Internal->CrosshairNode);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLCrosshairDisplayableManager::CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2)
{
  if (!this->Internal->SliceIntersectionWidget)
    {
    return false;
    }
  return this->Internal->SliceIntersectionWidget->CanProcessInteractionEvent(eventData, closestDistance2);
}

//---------------------------------------------------------------------------
bool vtkMRMLCrosshairDisplayableManager::ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData)
{
  if (!this->Internal->SliceIntersectionWidget)
    {
    return false;
    }
  return this->Internal->SliceIntersectionWidget->ProcessInteractionEvent(eventData);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::SetActionsEnabled(int actions)
{
  this->Internal->SliceIntersectionWidget->SetActionsEnabled(actions);
}

//---------------------------------------------------------------------------
int vtkMRMLCrosshairDisplayableManager::GetActionsEnabled()
{
  return this->Internal->SliceIntersectionWidget->GetActionsEnabled();
}

//---------------------------------------------------------------------------
vtkMRMLSliceIntersectionWidget* vtkMRMLCrosshairDisplayableManager::GetSliceIntersectionWidget()
{
  return this->Internal->SliceIntersectionWidget;
}

//---------------------------------------------------------------------------
int vtkMRMLCrosshairDisplayableManager::GetMouseCursor()
{
  if (!this->Internal->SliceIntersectionWidget)
    {
    return VTK_CURSOR_DEFAULT;
    }
  return this->Internal->SliceIntersectionWidget->GetMouseCursor();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::SetHasFocus(bool hasFocus)
{
  if (this->Internal->SliceIntersectionWidget)
    {
    this->Internal->SliceIntersectionWidget->Leave(nullptr);
    }
}