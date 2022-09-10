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
#include "vtkMRMLViewDisplayableManager.h"
#include "vtkMRMLCameraDisplayableManager.h"
#include "vtkMRMLDisplayableManagerGroup.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLSliceLogic.h>

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkFollower.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOutlineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVectorText.h>

// STD includes

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLViewDisplayableManager );

//---------------------------------------------------------------------------
class vtkMRMLViewDisplayableManager::vtkInternal
{
public:
  vtkInternal(vtkMRMLViewDisplayableManager * external);
  ~vtkInternal();

  vtkMRMLCameraNode* CameraNode;

  void CreateAxis();
  void AddAxis(vtkRenderer * renderer);
  void UpdateAxis(vtkRenderer * renderer, vtkMRMLViewNode * viewNode);
  void UpdateRASBounds(double bounds[6]);

  void UpdateAxisVisibility();
  void UpdateAxisColor();
  void UpdateAxisLabelVisibility();
  void UpdateAxisLabelText();
  void SetAxisLabelColor(double newAxisLabelColor[3]);

  void UpdateRenderMode();

  void UpdateStereoType();

  void UpdateBackgroundColor();

  std::vector<vtkSmartPointer<vtkFollower> > AxisLabelActors;
  std::vector<vtkSmartPointer<vtkVectorText> > AxisLabelTexts;
  std::vector<vtkSmartPointer<vtkTransformPolyDataFilter> > CenterAxisLabelTexts;
  vtkSmartPointer<vtkActor>                  BoxAxisActor;
  vtkBoundingBox*                            BoxAxisBoundingBox;
  vtkMRMLViewDisplayableManager*             External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLViewDisplayableManager::vtkInternal::vtkInternal(vtkMRMLViewDisplayableManager * external)
{
  this->External = external;
  this->BoxAxisBoundingBox = new vtkBoundingBox();
  this->CreateAxis();
  this->CameraNode = nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLViewDisplayableManager::vtkInternal::~vtkInternal()
{
  delete this->BoxAxisBoundingBox;
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::CreateAxis()
{
  // Create the default bounding box
  vtkNew<vtkOutlineSource> boxSource;
  vtkNew<vtkPolyDataMapper> boxMapper;
  boxMapper->SetInputConnection(boxSource->GetOutputPort());

  this->BoxAxisActor = vtkSmartPointer<vtkActor>::New();
  this->BoxAxisActor->SetMapper(boxMapper.GetPointer());
  this->BoxAxisActor->SetScale(1.0, 1.0, 1.0);

  double boxColor[3];
  if (this->External->GetMRMLViewNode())
    {
    this->External->GetMRMLViewNode()->GetBoxColor(boxColor);
    }
  else
    {
    vtkMRMLViewNode::GetDefaultBoxColor(boxColor);
    }
  this->BoxAxisActor->GetProperty()->SetColor(boxColor);
  this->BoxAxisActor->SetPickable(0);

  this->AxisLabelActors.clear();
  this->AxisLabelTexts.clear();
  this->CenterAxisLabelTexts.clear();

  // default labels, will be overridden by view node AxisLabels
  const char* labels[6] = {"R", "A", "S", "L", "P", "I"};

  for(int i = 0; i < 6; ++i)
    {
    vtkNew<vtkVectorText> axisText;
    axisText->SetText(labels[i]);
    this->AxisLabelTexts.emplace_back(axisText.GetPointer());

    vtkNew<vtkTransformPolyDataFilter> centerAxisText;
    this->CenterAxisLabelTexts.emplace_back(centerAxisText.GetPointer());
    centerAxisText->SetInputConnection(axisText->GetOutputPort());

    vtkNew<vtkPolyDataMapper> axisMapper;
    axisMapper->SetInputConnection(centerAxisText->GetOutputPort());

    vtkNew<vtkFollower> axisActor;
    axisActor->SetMapper(axisMapper.GetPointer());
    axisActor->SetPickable(0);
    this->AxisLabelActors.emplace_back(axisActor.GetPointer());

    axisActor->GetProperty()->SetColor(1, 1, 1); // White
    axisActor->GetProperty()->SetDiffuse(0.0);
    axisActor->GetProperty()->SetAmbient(1.0);
    axisActor->GetProperty()->SetSpecular(0.0);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::AddAxis(vtkRenderer * renderer)
{
  assert(renderer);

  renderer->AddViewProp(this->BoxAxisActor);

  for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
    {
    vtkFollower* actor = this->AxisLabelActors[i];
    renderer->AddViewProp(actor);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateRASBounds(double bounds[6])
{
  //Bounds is x-min, x-max, y-min, y-max, z-min, z-max
  vtkMath::UninitializeBounds(bounds);

  if (this->External->GetMRMLViewNode() == nullptr)
    {
    return;
    }
  vtkMRMLScene *scene = this->External->GetMRMLViewNode()->GetScene();
  if (scene == nullptr)
    {
    return;
    }

  std::vector<vtkMRMLNode *> nodes;
  int nnodes = scene->GetNodesByClass("vtkMRMLDisplayableNode", nodes);
  for (int n=0; n < nnodes; n++)
    {
    vtkMRMLDisplayableNode* displayableNode =
      vtkMRMLDisplayableNode::SafeDownCast(nodes[n]);
    if (!displayableNode || vtkMRMLSliceLogic::IsSliceModelNode(displayableNode))
      {
      continue;
      }
    bool isDisplayableNodeVisibleInView = false;
    for (int i = 0; i < displayableNode->GetNumberOfDisplayNodes(); ++i)
      {
      vtkMRMLDisplayNode* displayNode = displayableNode->GetNthDisplayNode(i);
      if (displayNode && displayNode->IsDisplayableInView(
                           this->External->GetMRMLViewNode()->GetID()))
        {
        isDisplayableNodeVisibleInView = true;
        break;
        }
      }
    if (!isDisplayableNodeVisibleInView)
      {
      continue;
      }
    double nodeBounds[6];
    displayableNode->GetRASBounds(nodeBounds);
    if (vtkMath::AreBoundsInitialized(nodeBounds))
      {
      if (!vtkMath::AreBoundsInitialized(bounds))
        {
        for (int i=0; i<6; i++)
          {
          bounds[i] = nodeBounds[i];
          }
        }
      else
        {
        for (int i=0; i<3; i++)
          {
          if (bounds[2*i] > nodeBounds[2*i])
            {
            bounds[2*i] = nodeBounds[2*i];
            }
          if (bounds[2*i+1] < nodeBounds[2*i+1])
            {
            bounds[2*i+1] = nodeBounds[2*i+1];
            }
          }
        }
      }
    }

}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateAxis(vtkRenderer * renderer,
                                                            vtkMRMLViewNode * viewNode)
{
  if (!renderer || !renderer->IsActiveCameraCreated() || !viewNode)
    {
    return;
    }

  // Update box color
  this->BoxAxisActor->GetProperty()->SetColor(this->External->GetMRMLViewNode()->GetBoxColor());

  // Turn off box and axis labels to compute bounds
  int boxVisibility = this->BoxAxisActor->GetVisibility();
  this->BoxAxisActor->VisibilityOff();

  int axisLabelVisibility = 0;
  for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
    {
    vtkFollower* actor = this->AxisLabelActors[i];
    axisLabelVisibility = actor->GetVisibility();
    actor->VisibilityOff();
    }

  // Compute bounds
  double bounds[6];
  this->UpdateRASBounds(bounds);

  //renderer->ComputeVisiblePropBounds(bounds);

  // If there are no visible props, create a default set of bounds
  vtkBoundingBox newBBox;
  if (!vtkMath::AreBoundsInitialized(bounds))
    {
    newBBox.SetBounds(-100.0, 100.0,
                      -100.0, 100.0,
                      -100.0, 100.0);
    }
  else
    {
    newBBox.SetBounds(bounds);

    // Check for degenerate bounds
    double maxLength = newBBox.GetMaxLength();
    double minPoint[3], maxPoint[3];
    newBBox.GetMinPoint(minPoint[0], minPoint[1], minPoint[2]);
    newBBox.GetMaxPoint(maxPoint[0], maxPoint[1], maxPoint[2]);

    for (unsigned int i = 0; i < 3; i++)
      {
      if (newBBox.GetLength(i) == 0.0)
        {
        minPoint[i] = minPoint[i] - maxLength * .05;
        maxPoint[i] = maxPoint[i] + maxLength * .05;
        }
      }
    newBBox.SetMinPoint(minPoint);
    newBBox.SetMaxPoint(maxPoint);
    }

  // See if bounding box has changed. If not, no need to change the axis actors.
  if (newBBox != *(this->BoxAxisBoundingBox))
    {
    *(this->BoxAxisBoundingBox) = newBBox;

    double bounds[6];
    this->BoxAxisBoundingBox->GetBounds(bounds);

    vtkNew<vtkOutlineSource> boxSource;
    boxSource->SetBounds(bounds);

    vtkNew<vtkPolyDataMapper> boxMapper;
    boxMapper->SetInputConnection(boxSource->GetOutputPort());

    this->BoxAxisActor->SetMapper(boxMapper.GetPointer());
    this->BoxAxisActor->SetScale(1.0, 1.0, 1.0);

    // Letter size as fraction of bounding box size
    double letterSize = viewNode->GetLetterSize();
    // Letter size in world coordinate system
    double letterSizeWorld = this->BoxAxisBoundingBox->GetMaxLength() * letterSize;

    for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
      {
      vtkFollower* actor = this->AxisLabelActors[i];
      actor->SetScale(letterSizeWorld, letterSizeWorld, letterSizeWorld);
      // Apply a transform so that center of the letter is in (0,0,0) position.
      // This is needed because the actor's origin is only used for center of
      // actor scaling and rotation. vtkFollower always places the actor's
      // (0,0,0) position to the followed position.
      // Small errors may lead to huge misalignments when the view needs to be
      // zoomed in a lot (for example, for displaying a volume with 0.001 spacing).
      double offsetToCenter[3] = { 0.0, 0.0, 0.0 };
      this->AxisLabelTexts[i]->Update();
      vtkPolyData* textPoly = this->AxisLabelTexts[i]->GetOutput();
      if (textPoly)
        {
        double bounds[6] = { 0.0, 1.0, 0.0, 1.0, 0.0, 1.0 };
        textPoly->GetBounds(bounds);
        offsetToCenter[0] = -(bounds[0] + bounds[1]) / 2.0;
        offsetToCenter[1] = -(bounds[2] + bounds[3]) / 2.0;
        offsetToCenter[2] = -(bounds[4] + bounds[5]) / 2.0;
        }
      vtkNew<vtkTransform> transform;
      transform->Translate(offsetToCenter);
      this->CenterAxisLabelTexts[i]->SetTransform(transform);
      }

    // Position the axis labels
    double center[3];
    this->BoxAxisBoundingBox->GetCenter(center);

    // Offset the center of the label by 1.5-letter distance
    // (to have an approximately one-letter space between the label and the box).
    double offset = letterSizeWorld * 1.5;
    this->AxisLabelActors[0]->SetPosition(               // R
      bounds[1] + offset,
      center[1],
      center[2]);
    this->AxisLabelActors[1]->SetPosition(               // A
      center[0],
      bounds[3] + offset,
      center[2]);
    this->AxisLabelActors[2]->SetPosition(               // S
      center[0],
      center[1],
      bounds[5] + offset);

    this->AxisLabelActors[3]->SetPosition(               // L
      bounds[0] - offset,
      center[1],
      center[2]);
    this->AxisLabelActors[4]->SetPosition(               // P
      center[0],
      bounds[2] - offset,
      center[2]);
    this->AxisLabelActors[5]->SetPosition(               // I
      center[0],
      center[1],
      bounds[4] - offset);
    }

  // Update camera and make the axis visible again
  this->BoxAxisActor->SetVisibility(boxVisibility);
  for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
    {
    vtkFollower* actor = this->AxisLabelActors[i];
    actor->SetCamera(renderer->GetActiveCamera());
    actor->SetVisibility(axisLabelVisibility);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateAxisVisibility()
{
  int visible = this->External->GetMRMLViewNode()->GetBoxVisible();
  vtkDebugWithObjectMacro(this->External, << "UpdateAxisVisibility:" << visible);
  this->BoxAxisActor->SetVisibility(visible);
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateAxisColor()
{
  double boxColor[3] = {1.0, 0.0, 1.0};
  vtkMRMLViewNode::GetDefaultBoxColor(boxColor);
  this->External->GetMRMLViewNode()->GetBoxColor(boxColor);
  vtkDebugWithObjectMacro(this->External, << "UpdateAxisColor:" << boxColor[0] << "\t" << boxColor[1] << "\t" << boxColor[2]);
  this->BoxAxisActor->GetProperty()->SetColor(boxColor);
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateAxisLabelVisibility()
{
  vtkCamera *camera = this->External->GetRenderer() ?
    this->External->GetRenderer()->GetActiveCamera() : nullptr;
  if (!camera ||
      !this->External->GetMRMLViewNode())
    {
    return;
    }

  double orient[] = {-1,1};
  double dir[4];
  camera->GetDirectionOfProjection(dir);
  vtkMath::Normalize(dir);

  int visible = this->External->GetMRMLViewNode()->GetAxisLabelsVisible();
  int cameraBasedVilibility = this->External->GetMRMLViewNode()->GetAxisLabelsCameraDependent();

  double s2 = 0.5*sqrt(2.0);
  for  (int j=0; j<2; j++)
    {
    for  (int i=0; i<3; i++)
      {
      vtkFollower* actor = this->AxisLabelActors[i+3*j];
      if (cameraBasedVilibility)
        {
        double axis[] = {0,0,0};
        axis[i] = orient[j];
        double dot = vtkMath::Dot(axis, dir);
        if (dot > s2)
          {
          actor->SetVisibility(false);
          }
        else
          {
          actor->SetVisibility(visible);
          }
        }
      else
        {
        actor->SetVisibility(visible);
        }
      }
    }
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateAxisLabelText()
{
  vtkMRMLViewNode* viewNode = this->External->GetMRMLViewNode();
  if (!viewNode || !viewNode->GetAxisLabelsVisible())
    {
    return;
    }

  bool updateNeeded = false;
  // In this displayable manager class axis labels are ordered as +X,+Y,+Z,-X,-Y-,-Z.
  // In the view node axis labels are ordered as -X,+X,-Y,+Y,-Z,+Z.
  // viewAxisToDmAxis converts from view to displayable manager axis order.
  const int viewAxisToDmAxis[6]={3,0,4,1,5,2};
  for (int labelIndexView=0; labelIndexView<6; labelIndexView++)
    {
    if (strcmp(this->AxisLabelTexts[viewAxisToDmAxis[labelIndexView]]->GetText(),viewNode->GetAxisLabel(labelIndexView))!=0)
      {
      this->AxisLabelTexts[viewAxisToDmAxis[labelIndexView]]->SetText(viewNode->GetAxisLabel(labelIndexView));
      updateNeeded = true;
      }
    }
  if (updateNeeded)
    {
    this->External->RequestRender();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::SetAxisLabelColor(double newAxisLabelColor[3])
{
  for(std::size_t i = 0; i < this->AxisLabelActors.size(); ++i)
    {
    vtkFollower* actor = this->AxisLabelActors[i];
    actor->GetProperty()->SetColor(newAxisLabelColor);
    }
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateRenderMode()
{
  vtkDebugWithObjectMacro(this->External, << "UpdateRenderMode:" <<
                this->External->GetMRMLViewNode()->GetRenderMode());

  if (!this->External->GetRenderer()->IsActiveCameraCreated())
    {
    return;
    }

  vtkCamera *cam = this->External->GetRenderer()->GetActiveCamera();
  if (this->External->GetMRMLViewNode()->GetRenderMode() == vtkMRMLViewNode::Perspective)
    {
    cam->ParallelProjectionOff();
    }
  else if (this->External->GetMRMLViewNode()->GetRenderMode() == vtkMRMLViewNode::Orthographic)
    {
    cam->ParallelProjectionOn();
    cam->SetParallelScale(this->External->GetMRMLViewNode()->GetFieldOfView());
    }
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateStereoType()
{
  vtkDebugWithObjectMacro(this->External, << "UpdateStereoType:" <<
                this->External->GetMRMLViewNode()->GetStereoType());

  vtkRenderWindow * renderWindow = this->External->GetRenderer()->GetRenderWindow();
  int stereoType = this->External->GetMRMLViewNode()->GetStereoType();
  if (stereoType == vtkMRMLViewNode::RedBlue)
    {
    renderWindow->SetStereoTypeToRedBlue();
    }
  else if (stereoType == vtkMRMLViewNode::Anaglyph)
    {
    renderWindow->SetStereoTypeToAnaglyph();
    //renderWindow->SetAnaglyphColorSaturation(0.1);
    }
  else if (stereoType == vtkMRMLViewNode::QuadBuffer)
    {
    renderWindow->SetStereoTypeToCrystalEyes();
    }
  else if (stereoType == vtkMRMLViewNode::Interlaced)
    {
    renderWindow->SetStereoTypeToInterlaced();
    }
  else if (stereoType == vtkMRMLViewNode::UserDefined_1)
    {
    renderWindow->SetStereoType(101);
    }
  else if (stereoType == vtkMRMLViewNode::UserDefined_2)
    {
    renderWindow->SetStereoType(102);
    }
  else if (stereoType == vtkMRMLViewNode::UserDefined_3)
    {
    renderWindow->SetStereoType(103);
    }

  renderWindow->SetStereoRender(stereoType != vtkMRMLViewNode::NoStereo);
  this->External->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::vtkInternal::UpdateBackgroundColor()
{
  double backgroundColor[3] = {0.0, 0.0, 0.0};
  this->External->GetMRMLViewNode()->GetBackgroundColor(backgroundColor);
  double backgroundColor2[3] = {0.0, 0.0, 0.0};
  this->External->GetMRMLViewNode()->GetBackgroundColor2(backgroundColor2);
  vtkDebugWithObjectMacro(this->External, << "UpdateBackgroundColor (" <<
                backgroundColor[0] << ", " << backgroundColor[1] << ", "
                << backgroundColor[2] << ")");
  this->External->GetRenderer()->SetBackground(backgroundColor);
  this->External->GetRenderer()->SetBackground2(backgroundColor2);
  bool gradient = backgroundColor[0] != backgroundColor2[0] ||
                  backgroundColor[1] != backgroundColor2[1] ||
                  backgroundColor[2] != backgroundColor2[2];
  this->External->GetRenderer()->SetGradientBackground(gradient);

  // If new background color is White, switch axis color label to black
  if (backgroundColor[0] == 1.0 && backgroundColor[1] == 1.0 && backgroundColor[2] == 1.0)
    {
    double black[3] = {0.0, 0.0, 0.0};
    this->SetAxisLabelColor(black);
    }
  else
    {
    double white[3] = {1.0, 1.0, 1.0};
    this->SetAxisLabelColor(white);
    }

  this->External->RequestRender();
}

//---------------------------------------------------------------------------
// vtkMRMLViewDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLViewDisplayableManager::vtkMRMLViewDisplayableManager()
{
  this->Internal = new vtkInternal(this);
}

//---------------------------------------------------------------------------
vtkMRMLViewDisplayableManager::~vtkMRMLViewDisplayableManager()
{
  this->SetAndObserveCameraNode(nullptr);
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::AdditionalInitializeStep()
{
  // TODO: Listen to ModifiedEvent and update the box coords if needed
  this->AddMRMLDisplayableManagerEvent(vtkMRMLViewNode::ResetFocalPointRequestedEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::Create()
{
  this->Superclass::Create();

  assert(this->GetRenderer());
  assert(this->GetMRMLViewNode());

  this->Internal->AddAxis(this->GetRenderer());

  // CameraNodeDisplayableManager is expected to be instantiated !
  vtkMRMLCameraDisplayableManager * cameraDisplayableManager =
      vtkMRMLCameraDisplayableManager::SafeDownCast(
          this->GetMRMLDisplayableManagerGroup()->GetDisplayableManagerByClassName(
              "vtkMRMLCameraDisplayableManager"));
  assert(cameraDisplayableManager);

  // Listen for ActiveCameraChangedEvent
  // \tbd active camera should be set to view node instead and only observing
  //  view node should be necessary.
  cameraDisplayableManager->AddObserver(vtkMRMLCameraDisplayableManager::ActiveCameraChangedEvent,
                                        this->GetWidgetsCallbackCommand());

  this->SetAndObserveCameraNode(cameraDisplayableManager->GetCameraNode());

  this->UpdateFromViewNode();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager
::OnMRMLDisplayableNodeModifiedEvent(vtkObject* vtkNotUsed(caller))
{
  this->UpdateFromViewNode();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::UpdateFromViewNode()
{
  this->Internal->UpdateRenderMode();
  this->Internal->UpdateAxisLabelVisibility();
  this->Internal->UpdateAxisVisibility();
  this->Internal->UpdateAxisColor();
  this->Internal->UpdateAxisLabelText();
  this->Internal->UpdateStereoType();
  this->Internal->UpdateBackgroundColor();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager
::SetAndObserveCameraNode(vtkMRMLCameraNode* cameraNode)
{
  vtkSetAndObserveMRMLNodeMacro(this->Internal->CameraNode, cameraNode);
  this->Internal->UpdateAxis(this->GetRenderer(), this->GetMRMLViewNode());
  this->UpdateFromCameraNode();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager::UpdateFromCameraNode()
{
  this->Internal->UpdateAxisLabelVisibility();
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager
::ProcessMRMLNodesEvents(vtkObject * caller,
                         unsigned long event,
                         void * callData)
{
  if (vtkMRMLViewNode::SafeDownCast(caller))
    {
    if (event == vtkMRMLViewNode::ResetFocalPointRequestedEvent)
      {
      vtkDebugMacro(<< "ProcessMRMLNodesEvents - ResetFocalPointEvent");
      this->Internal->UpdateAxis(this->GetRenderer(), this->GetMRMLViewNode());
      this->Internal->UpdateAxisLabelVisibility();
      }
    // Note: event == ModifiedEvent is handled by superclass
    }
  else if(vtkMRMLCameraNode::SafeDownCast(caller))
    {
    if (event == vtkCommand::ModifiedEvent)
      {
      this->UpdateFromCameraNode();
      }
    }
  this->Superclass::ProcessMRMLNodesEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkMRMLViewDisplayableManager
::ProcessWidgetsEvents(vtkObject * caller,
                       unsigned long event,
                       void *callData)
{
  if (vtkMRMLCameraDisplayableManager::SafeDownCast(caller))
    {
    if (event == vtkMRMLCameraDisplayableManager::ActiveCameraChangedEvent)
      {
      vtkMRMLCameraDisplayableManager* cameraDisplayableManager =
        vtkMRMLCameraDisplayableManager::SafeDownCast(caller);
      this->SetAndObserveCameraNode(cameraDisplayableManager->GetCameraNode());
      }
    }
  this->Superclass::ProcessWidgetsEvents(caller, event, callData);
}
