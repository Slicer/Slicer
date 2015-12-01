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
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkActor2D.h>
#include <vtkCallbackCommand.h>
#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
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
  // Slice Composite
  vtkMRMLSliceCompositeNode* FindSliceCompositeNode();
  void SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode);
  // Crosshair
  vtkMRMLCrosshairNode* FindCrosshairNode();
  void SetCrosshairNode(vtkMRMLCrosshairNode* crosshairNode);

  // Actors
  void SetActor(vtkActor2D* prop) {Actor = prop;};
  void SetHighlightActor(vtkActor2D* prop) {HighlightActor = prop;};

  // Build the crosshair representation
  void BuildCrosshair();

  // Add a line to the crosshair in display coordinates (needs to be
  // passed the points and cellArray to manipulate).
  void AddCrosshairLine(vtkPoints *pts, vtkCellArray *cellArray,
                        int p1x, int p1y, int p2x, int p2y);

  // Did crosshair position change?
  bool DidCrosshairPositionChange();

  // Did crosshair property change?
  bool DidCrosshairPropertyChange();

  // PickStates
  enum
  {
    NoPick,
    Outside,
    Near,
    Over,
    Horizontal,
    Vertical
  };

  // ActionStates
  enum
  {
    NoAction,
    Dragging
  };

  vtkMRMLCrosshairDisplayableManager*        External;
  int                                        PickState;
  int                                        ActionState;
  vtkWeakPointer<vtkMRMLSliceCompositeNode>  SliceCompositeNode;
  vtkWeakPointer<vtkMRMLCrosshairNode>       CrosshairNode;
  vtkSmartPointer<vtkActor2D>                Actor;
  vtkSmartPointer<vtkActor2D>                HighlightActor;
  vtkSmartPointer<vtkMRMLCrosshairNode>      CrosshairNodeCache;
  vtkWeakPointer<vtkRenderer>                LightBoxRenderer;
};


//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager::vtkInternal
::vtkInternal(vtkMRMLCrosshairDisplayableManager * external)
{
  this->External = external;
  this->PickState = NoPick;
  this->ActionState = NoAction;
  this->SliceCompositeNode = 0;
  this->CrosshairNode = 0;
  this->Actor = 0;
  this->HighlightActor = 0;
  this->LightBoxRenderer = 0;
  this->CrosshairNodeCache = vtkSmartPointer<vtkMRMLCrosshairNode>::New();
}

//---------------------------------------------------------------------------
vtkMRMLCrosshairDisplayableManager::vtkInternal::~vtkInternal()
{
  this->SetSliceCompositeNode(0);
  this->SetCrosshairNode(0);
  this->LightBoxRenderer = 0;
  this->CrosshairNodeCache = 0;
  // everything should be empty
  assert(this->SliceCompositeNode == 0);
  assert(this->CrosshairNode == 0);
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
bool vtkMRMLCrosshairDisplayableManager::vtkInternal::DidCrosshairPositionChange()
{
  if (this->CrosshairNode.GetPointer() == 0)
    {
    return false;
    }

  double *cacheRAS = this->CrosshairNodeCache->GetCrosshairRAS();
  double *ras = this->CrosshairNode->GetCrosshairRAS();
  double eps = 1.0e-12;

  if (fabs(cacheRAS[0] - ras[0]) < eps
      && fabs(cacheRAS[1] - ras[1]) < eps
      && fabs(cacheRAS[2] - ras[2]) < eps)
    {
    return false;
    }
  else
    {
    return true;
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLCrosshairDisplayableManager::vtkInternal::DidCrosshairPropertyChange()
{
  if (this->CrosshairNode.GetPointer() == 0)
    {
    return false;
    }

  bool change = false;

  if (this->CrosshairNodeCache->GetCrosshairMode() != this->CrosshairNode->GetCrosshairMode())
    {
    change = true;
    }

  if (this->CrosshairNodeCache->GetCrosshairThickness() != this->CrosshairNode->GetCrosshairThickness())
    {
    change = true;
    }

  return change;
}


//---------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLCrosshairDisplayableManager::vtkInternal
::GetSliceNode()
{
  return this->External->GetMRMLSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::vtkInternal::UpdateSliceNode()
{
  assert(!this->GetSliceNode() || this->GetSliceNode()->GetLayoutName());
  // search the scene for a matching slice composite node
  if (!this->SliceCompositeNode.GetPointer() || // the slice composite has been deleted
      !this->SliceCompositeNode->GetLayoutName() || // the slice composite points to a diff slice node
      strcmp(this->SliceCompositeNode->GetLayoutName(),
             this->GetSliceNode()->GetLayoutName()))
    {
    vtkMRMLSliceCompositeNode* sliceCompositeNode =
      this->FindSliceCompositeNode();
    this->SetSliceCompositeNode(sliceCompositeNode);
    }

  // search for the Crosshair node
  vtkMRMLCrosshairNode* crosshairNode = this->FindCrosshairNode();
  this->SetCrosshairNode(crosshairNode);
}

//---------------------------------------------------------------------------
vtkMRMLSliceCompositeNode* vtkMRMLCrosshairDisplayableManager::vtkInternal
::FindSliceCompositeNode()
{
  if (this->GetSliceNode() == 0 ||
      this->External->GetMRMLApplicationLogic() == 0)
    {
    return 0;
    }

  vtkMRMLSliceLogic *sliceLogic = NULL;
  vtkMRMLApplicationLogic *mrmlAppLogic = this->External->GetMRMLApplicationLogic();
  if (mrmlAppLogic)
    {
    sliceLogic = mrmlAppLogic->GetSliceLogic(this->GetSliceNode());
    }
  if (sliceLogic)
    {
    return sliceLogic->GetSliceCompositeNode(this->GetSliceNode());
    }
  // no matching slice composite node is found
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::vtkInternal
::SetSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode)
{
  if (this->SliceCompositeNode == compositeNode)
    {
    return;
    }
  vtkSetAndObserveMRMLNodeMacro(this->SliceCompositeNode, compositeNode);
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

  // Initialize the states
  if (this->CrosshairNode)
    {
    this->ActionState = NoAction;
    if (this->CrosshairNode->GetNavigation())
      {
      this->PickState = Outside;
      }
    else
      {
      this->PickState = NoPick;
      }
    }
}

//---------------------------------------------------------------------------
vtkMRMLCrosshairNode* vtkMRMLCrosshairDisplayableManager::vtkInternal
::FindCrosshairNode()
{
  if (this->External->GetMRMLScene() == 0)
    {
    return 0;
    }

  vtkMRMLNode* node;
  vtkCollectionSimpleIterator it;
  vtkSmartPointer<vtkCollection> crosshairs;
  crosshairs.TakeReference(this->External->GetMRMLScene()->GetNodesByClass("vtkMRMLCrosshairNode"));
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
  return 0;
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
    this->Actor = 0;
    }
  if (this->HighlightActor.GetPointer())
    {
    if (this->LightBoxRenderer)
      {
      this->LightBoxRenderer->RemoveActor(this->HighlightActor);
      }
    this->HighlightActor = 0;
    }

  if (!this->CrosshairNode.GetPointer())
    {
    return;
    }

  // Get the size of the window
  int *screenSize = this->External->GetInteractor()->GetRenderWindow()->GetScreenSize();

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

  // Highlight actor
  //
  //
  // Set up the VTK data structures
  vtkNew<vtkPolyData> hpolyData;
  vtkNew<vtkCellArray> hcellArray;
  vtkNew<vtkCellArray> hvcellArray;
  vtkNew<vtkPoints> hpoints;
  hpolyData->SetLines(hcellArray.GetPointer());
  hpolyData->SetVerts(hvcellArray.GetPointer());
  hpolyData->SetPoints(hpoints.GetPointer());

  vtkNew<vtkPolyDataMapper2D> hmapper;
  vtkNew<vtkActor2D> hactor;
  hmapper->SetInputData(hpolyData.GetPointer());
  hactor->SetMapper(hmapper.GetPointer());

  if (this->LightBoxRenderer)
    {
    this->LightBoxRenderer->AddActor(hactor.GetPointer());
    }

  // Cache the actor
  this->SetHighlightActor(hactor.GetPointer());

  // center dot (need to offset by half a pixel, why?
  int cindex = hpoints->InsertNextPoint(-0.5,-0.5, 0);

  // box for highlight
  int llindex = hpoints->InsertNextPoint(-5, -5, 0);
  int lrindex = hpoints->InsertNextPoint( 5, -5, 0);
  int urindex = hpoints->InsertNextPoint( 5,  5, 0);
  int ulindex = hpoints->InsertNextPoint(-5,  5, 0);

  hcellArray->InsertNextCell(5);
  hcellArray->InsertCellPoint(llindex);
  hcellArray->InsertCellPoint(lrindex);
  hcellArray->InsertCellPoint(urindex);
  hcellArray->InsertCellPoint(ulindex);
  hcellArray->InsertCellPoint(llindex);

  hvcellArray->InsertNextCell(1);
  hvcellArray->InsertCellPoint(cindex);

  hactor->GetProperty()->SetColor(1.0, 0.8, 0.1);
  hactor->GetProperty()->SetOpacity(1.0);

  hactor->VisibilityOff();
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
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::UnobserveMRMLScene()
{
  this->Internal->SetSliceCompositeNode(0);
  this->Internal->SetCrosshairNode(0);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::OnMRMLNodeModified(vtkMRMLNode* node)
{
  if (vtkMRMLCrosshairNode::SafeDownCast(node))
    {
    //std::cout << "Crosshair node modified: " << caller << ", " << this->Internal->CrosshairNode << std::endl;

    // update the properties and style of the crosshair
    bool builtCrosshair = false;
    if (this->Internal->DidCrosshairPropertyChange())
      {
      this->Internal->BuildCrosshair();
      builtCrosshair = true;
      }

    // update the position of the actor
    if ((this->Internal->DidCrosshairPositionChange() || builtCrosshair)
        && this->Internal->Actor && this->Internal->HighlightActor)
      {
      double xyz[3];
      double *ras = this->Internal->CrosshairNode->GetCrosshairRAS();
      this->ConvertRASToXYZ(ras, xyz);

      if (this->Internal->CrosshairNode->GetNavigation())
        {
        int id = this->Internal->CrosshairNode->GetLightBoxPane();
        this->Internal->GetSliceNode()->JumpSliceByOffsetting(id, ras[0], ras[1], ras[2]);
        this->ConvertRASToXYZ(ras, xyz); // recompute since matrices changed
        }

      this->Internal->Actor->SetPosition(xyz[0], xyz[1]);
      this->Internal->HighlightActor->SetPosition(xyz[0], xyz[1]);

      // put the actor in the right lightbox
      if (this->GetLightBoxRendererManagerProxy())
        {
        int id = (int) (xyz[2] + 0.5); // round to find the lightbox
        vtkRenderer *renderer
          = this->GetLightBoxRendererManagerProxy()->GetRenderer(id);
        if (renderer != this->Internal->LightBoxRenderer)
          {
          if (this->Internal->LightBoxRenderer)
            {
            this->Internal->LightBoxRenderer
              ->RemoveActor(this->Internal->Actor);
            this->Internal->LightBoxRenderer
              ->RemoveActor(this->Internal->HighlightActor);
            }
          if (renderer)
            {
            renderer->AddActor(this->Internal->Actor);
            renderer->AddActor(this->Internal->HighlightActor);
            }
          this->Internal->LightBoxRenderer = renderer;
          }
        }

      //std::cout << this->Internal->GetSliceNode()->GetSingletonTag() << " -- RAS: " << this->Internal->CrosshairNode->GetCrosshairRAS()[0] << ", " << this->Internal->CrosshairNode->GetCrosshairRAS()[1] << ", " << this->Internal->CrosshairNode->GetCrosshairRAS()[2] << ", XYZ = " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
      }

    // Update the cache of the crosshair
    this->Internal->CrosshairNodeCache->Copy(this->Internal->CrosshairNode);
    }

  // if (vtkMRMLSliceCompositeNode::SafeDownCast(caller))
  //   {
  // //std::cout << "SLICE COMPOSITE UPDATED" << std::endl;
  //   this->Internal->UpdateSliceCompositeNode(vtkMRMLSliceCompositeNode::SafeDownCast(caller));
  //   }
  // else if (vtkMRMLDisplayableNode::SafeDownCast(caller))
  //   {
  //   //std::cout << "VOLUME UPDATED" << std::endl;
  //   if (callData == 0)
  //     {
  //     this->Internal->UpdateVolume(vtkMRMLDisplayableNode::SafeDownCast(caller));
  //     }
  //   }
  // else if (vtkMRMLDisplayNode::SafeDownCast(caller))
  //   {
  //   //std::cout << "DISPLAY NODE UPDATED" << std::endl;
  //   this->Internal->UpdateVolumeDisplayNode(vtkMRMLDisplayNode::SafeDownCast(caller));
  //   }

  // Request a render
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::Create()
{
  // Setup the SliceNode, SliceCompositeNode, CrosshairNode
  this->Internal->UpdateSliceNode();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::OnInteractorStyleEvent(int eventid)
{
   // std::cout << "InteractorStyle event: " << eventid
   //           << ", eventname:" << vtkCommand::GetStringFromEventId(eventid)
   //           << std::endl;

  this->Superclass::OnInteractorStyleEvent(eventid);

  if (this->Internal->CrosshairNode)
    {
    switch (eventid)
      {
      case vtkCommand::LeaveEvent:
        this->Internal->CrosshairNode->SetCursorPositionInvalid();
        // If we are not Navigating, reposition the crosshair to the center
        if (this->Internal->CrosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair
            && this->Internal->CrosshairNode->GetNavigation() == 0)
          {
          double xyz[3], ras[3];
          vtkRenderer *renderer
            = this->GetLightBoxRendererManagerProxy()->GetRenderer(0);
          xyz[0] = renderer->GetSize()[0] / 2.0;
          xyz[1] = renderer->GetSize()[1] / 2.0;
          xyz[2] = 0;
          this->ConvertXYZToRAS(xyz, ras);

          this->Internal->CrosshairNode->SetCrosshairRAS(ras[0], ras[1],ras[2]);
          }
        break;
      // When we switch to the Slicer window and the mouse is already over a view
      // then only OnInteractorStyleEvent is called with vtkCommand::EnterEvent,
      // (OnInteractorEvent is not called) therefore we need to handle EnterEvent here
      case vtkCommand::EnterEvent:
          {
          // On a mouse move, determine which lightbox is under the mouse
          // and the RAS position of the mouse
          int *pos = this->GetInteractor()->GetEventPosition();
          double xyz[3];
          this->ConvertDeviceToXYZ(pos[0], pos[1], xyz);
          this->Internal->CrosshairNode->SetCursorPositionXYZ(xyz, this->GetMRMLSliceNode());
          }
        break;
      case vtkCommand::LeftButtonReleaseEvent:
        // Button release is only meaningful in navigation mode
        if (this->Internal->CrosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair
            && this->Internal->CrosshairNode->GetNavigation())
          {
          this->Internal->PickState = vtkInternal::NoPick;
          this->Internal->ActionState = vtkInternal::NoAction;
          }
        break;

      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::OnInteractorEvent(int eventid)
{
  // Here, we process events that are normally swallowed by the InteractorStyle
  //
  //

   // std::cout << "Interactor event: " << eventid
   //           << ", eventname:" << vtkCommand::GetStringFromEventId(eventid)
   //           << std::endl;

  this->Superclass::OnInteractorEvent(eventid);

  if (this->Internal->CrosshairNode)
    {
    bool renderNeeded = false;
    switch (eventid)
      {
      case vtkCommand::LeftButtonPressEvent:
        // Button press is only meaningful in navigation mode
        if (this->Internal->CrosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair
            && this->Internal->CrosshairNode->GetNavigation())
          {
          // check whether we are close enough to navigate
          if (this->Internal->PickState == vtkInternal::Over)
            {
            this->Internal->ActionState = vtkInternal::Dragging;

            // Set the abort flag so that no other callbacks respond
            // (may need to set a priority).
            this->InteractorAbortFlagOn();
            }
          }
        break;
      case vtkCommand::LeaveEvent:
        this->Internal->CrosshairNode->SetCursorPositionInvalid();
        break;
      case vtkCommand::MouseMoveEvent:
      case vtkCommand::EnterEvent:
        // On a mouse move, determine which lightbox is under the mouse
        // and the RAS position of the mouse
        int *pos = this->GetInteractor()->GetEventPosition();

        double xyz[3];
        this->ConvertDeviceToXYZ(pos[0], pos[1], xyz);

        this->Internal->CrosshairNode->SetCursorPositionXYZ(xyz, this->GetMRMLSliceNode());

        double ras[3];
        this->ConvertXYZToRAS(xyz, ras);

        // Navigation mode and Cross-referencing mode handle the
        // information differently
        if (this->Internal->CrosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair
            && this->Internal->CrosshairNode->GetNavigation())
          {
          // Navigation mode.
          switch (this->Internal->ActionState)
            {
            case vtkInternal::NoAction:
            {
              // Check whether we need to highlight, is the mouse
              // close to the crosshair?

              // Crosshair in XYZ
              double *c_ras = this->Internal->CrosshairNode->GetCrosshairRAS();
              double c_xyz[3];
              this->ConvertRASToXYZ(c_ras, c_xyz);

              // check tolerances
              double tol = 5;
              if ( (fabs(xyz[0] - c_xyz[0]) < tol)
                   && (fabs(xyz[1] - c_xyz[1]) < tol) )
                {
                this->Internal->PickState = vtkInternal::Over;
                if (this->Internal->HighlightActor && !this->Internal->HighlightActor->GetVisibility())
                  {
                  this->Internal->HighlightActor->VisibilityOn();
                  renderNeeded = true;
                  }
                }
              // else if ( fabs(xyz[0] - c_xyz[0]) < tol )
              //   {
              //   this->Internal->PickState = vtkInternal::Vertical;
              //   if (this->Internal->HighlightActor && !this->Internal->HighlightActor->GetVisibility())
              //     {
              //     this->Internal->HighlightActor->VisibilityOn();
              //     renderNeeed = true;
              //     }
              //   }
              // else if ( fabs(xyz[1] - c_xyz[1]) < tol && !this->Internal->HighlightActor->GetVisibility())
              //   {
              //   this->Internal->PickState = vtkInternal::Horizontal;
              //   if (this->Internal->HighlightActor)
              //     {
              //     this->Internal->HighlightActor->VisibilityOn();
              //     renderNeeded = true;
              //     }
              //   }
              else if ( (fabs(xyz[0] - c_xyz[0]) < 2.0*tol)
                        && (fabs(xyz[1] - c_xyz[1]) < 2.0*tol) )
                {
                this->Internal->PickState = vtkInternal::Near;
                if (this->Internal->HighlightActor && !this->Internal->HighlightActor->GetVisibility())
                  {
                  this->Internal->HighlightActor->VisibilityOn();
                  renderNeeded = true;
                  }
                }
              else
                {
                this->Internal->PickState = vtkInternal::Outside;
                if (this->Internal->HighlightActor && this->Internal->HighlightActor->GetVisibility())
                  {
                  this->Internal->HighlightActor->VisibilityOff();
                  renderNeeded = true;
                  }
                }
              break;
            }
            case vtkInternal::Dragging:
              // Set the new position on the crosshair and a suggested
              // lightbox pane
              int id = (int) (xyz[2] + 0.5); // round to find the lightbox
              this->Internal->CrosshairNode->SetCrosshairRAS(ras, id);
              // modifying the CrosshairRAS will trigger a render
              break;
            }
          }
        else if (this->Internal->CrosshairNode->GetCrosshairMode() != vtkMRMLCrosshairNode::NoCrosshair)
          {
          // Cross-referencing mode. Set the new position on the crosshair
          // modifying the CrosshairRAS will trigger a render
          this->Internal->CrosshairNode->SetCrosshairRAS(ras);
          }
        break;
      }

    if (renderNeeded)
      {
      this->RequestRender();
      }
    }

}

//---------------------------------------------------------------------------
int vtkMRMLCrosshairDisplayableManager::ActiveInteractionModes()
{
  // Crosshairs wants to get events all the time
  return vtkMRMLInteractionNode::Place | vtkMRMLInteractionNode::ViewTransform;
}


//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::AdditionalInitializeStep()
{
  // Add an observation directly on the interactor to capture
  // events which are normally swallowed by the InteractorStyle

  // MoveMoveEvent - priority = -1.0. Allows InteractorStyle to get
  // the move event before the Crosshair.  This is needed so that
  // Shift-MouseMove can jump the slices and reposition the SliceNodes
  // before the Crosshair tries to draw. This allows the Crosshair to
  // draw on the right slice (avoids a blinking crosshair where the
  // crosshair is turned off when it is drawn on one slice and the
  // SliceViewer is displaying a different slice).
  this->AddInteractorObservableEvent(vtkCommand::MouseMoveEvent, -1.0);

  // LeftButtonPress - priority = 1.0. Allows the Crosshair to get the
  // left button press before the InteractorStyle. This allows the
  // Crosshair to decide if the user is navigating and the mouse is
  // close to the cross center and therefore the crosshair will
  // swallow the event (giving precedence to navigating the Crosshair
  // over window level in the InteractorStyle).
  this->AddInteractorObservableEvent(vtkCommand::LeftButtonPressEvent, 1.0);

  // Build the initial crosshair representation
  this->Internal->BuildCrosshair();
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairDisplayableManager::OnMRMLSliceNodeModifiedEvent()
{
  vtkMRMLSliceNode* nd = this->GetMRMLSliceNode();

  if (nd)
    {
      //std::cout << "Slice Node changed" << std::endl;
      this->Internal->BuildCrosshair();
      if (this->Internal->Actor && this->Internal->HighlightActor)
      {
      double xyz[3];
      double *ras = this->Internal->CrosshairNode->GetCrosshairRAS();
      this->ConvertRASToXYZ(ras, xyz);

      this->Internal->Actor->SetPosition(xyz[0], xyz[1]);
      this->Internal->HighlightActor->SetPosition(xyz[0], xyz[1]);
      }

      this->RequestRender();
    }
}
