
// Qt includes
#include <QStringList>
#include <QDebug>

// CTK includes
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLThreeDRenderView.h"
#include "qMRMLThreeDRenderView_p.h"
#include "vtkMRMLThreeDRenderViewInteractorStyle.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLInteractionNode.h>

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkFollower.h>
#include <vtkMath.h>
#include <vtkOutlineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkVectorText.h>
#include <vtkRenderWindowInteractor.h>

// STD includes
#include <vector>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLThreeDRenderView");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLThreeDRenderViewPrivate methods

//---------------------------------------------------------------------------
qMRMLThreeDRenderViewPrivate::qMRMLThreeDRenderViewPrivate()
{
  this->BoxAxisBoundingBox = new vtkBoundingBox();
  this->MRMLScene = 0;
  this->MRMLViewNode = 0;
  logger.setTrace();
}

//---------------------------------------------------------------------------
qMRMLThreeDRenderViewPrivate::~qMRMLThreeDRenderViewPrivate()
{
  delete this->BoxAxisBoundingBox; 
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::setMRMLScene(vtkMRMLScene* scene)
{
  if (scene == this->MRMLScene)
    {
    return; 
    }

//   if (scene)
//     {
//     // Add InteractionNode if needed
//     int interactionNodeCount = scene->GetNumberOfNodesByClass("vtkMRMLInteractionNode");
//     if (interactionNodeCount == 0)
//       {
//       VTK_CREATE(vtkMRMLInteractionNode, interactionNode);
//       scene->AddNode(interactionNode);
//       }
//     }

  // Clear associated view node
  this->setMRMLViewNode(0);
    
  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneClosingEvent, this, SLOT(onSceneClosingEvent()));

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneCloseEvent, this, SLOT(onSceneCloseEvent()));

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneLoadStartEvent, this, SLOT(onSceneLoadStartEvent()));

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneLoadEndEvent, this, SLOT(onSceneLoadEndEvent()));
    
  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeAddedEvent, this,
    SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
                      
  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::NodeRemovedEvent, this,
    SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));

  this->qvtkReconnect(
    this->MRMLScene, scene,
    vtkMRMLScene::SceneRestoredEvent, this, SLOT(onSceneRestoredEvent()));

  this->MRMLScene = scene;

  // Scan the given scene
  // TODO
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::setMRMLViewNode(vtkMRMLViewNode* viewNode)
{
  if (viewNode == this->MRMLViewNode)
    {
    return; 
    }
    
  this->MRMLViewNode = viewNode;
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::initialize(vtkMRMLViewNode* viewNode)
{
  CTK_P(qMRMLThreeDRenderView);

  if (viewNode == 0)
    {
    p->renderer()->SetActiveCamera(0);
    // TODO Clear box axis
    return;
    }
  
  // Create a cameraNode if required
  vtkMRMLCameraNode* cameraNode = this->lookUpMRMLCameraNode(viewNode);
  if (cameraNode == 0)
    {    
    VTK_CREATE(vtkMRMLCameraNode, newCameraNode);
    newCameraNode->SetName(this->MRMLScene->GetUniqueNameByString(newCameraNode->GetNodeTagName()));
    newCameraNode->SetActiveTag(viewNode->GetID());
    this->MRMLScene->AddNode(newCameraNode);
    cameraNode = newCameraNode;
    }

  // Assign camera to renderer
  vtkCamera* camera = cameraNode->GetCamera();
  Q_ASSERT(camera);
  p->renderer()->SetActiveCamera(camera);
  // Do not call if no camera otherwise it will create a new one without a node !
  p->renderer()->ResetCameraClippingRange();

  // Create Box axis
  this->createAxis();
  this->addAxis();
  
  if (this->InteractorStyle)
    {
    this->InteractorStyle->SetCameraNode(cameraNode);
    }
}

//---------------------------------------------------------------------------
vtkMRMLCameraNode* qMRMLThreeDRenderViewPrivate::lookUpMRMLCameraNode(vtkMRMLViewNode* viewNode)
{
  Q_ASSERT(viewNode);
    
  QString viewNodeID(viewNode->GetID());
  Q_ASSERT(!viewNodeID.isEmpty());
  
  std::vector<vtkMRMLNode*> cameraNodes;
  int cameraNodeCount = this->MRMLScene->GetNodesByClass("vtkMRMLCameraNode", cameraNodes);
  for (int n=0; n < cameraNodeCount; n++)
    {
    vtkMRMLCameraNode* cameraNode = vtkMRMLCameraNode::SafeDownCast(cameraNodes[n]);
    Q_ASSERT(cameraNode);
    QString activeTag(cameraNode->GetActiveTag());
    if (activeTag == viewNodeID)
      {
      Q_ASSERT(this->MRMLScene->GetNodeByID(cameraNode->GetID()));
      return cameraNode;
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::createAxis()
{
  // Create the default bounding box
  VTK_CREATE(vtkOutlineSource, boxSource);
  VTK_CREATE(vtkPolyDataMapper, boxMapper);
  boxMapper->SetInput(boxSource->GetOutput());
   
  this->BoxAxisActor = vtkSmartPointer<vtkActor>::New();
  this->BoxAxisActor->SetMapper(boxMapper);
  this->BoxAxisActor->SetScale(1.0, 1.0, 1.0);
  this->BoxAxisActor->GetProperty()->SetColor(1.0, 0.0, 1.0);
  this->BoxAxisActor->SetPickable(0);

  this->AxisLabelActors.clear();

  QStringList labels;
  labels << "R" << "A" << "S" << "L" << "P" << "I";

  foreach(const QString& label, labels)
    {
    VTK_CREATE(vtkVectorText, axisText);
    axisText->SetText(label.toLatin1());

    VTK_CREATE(vtkPolyDataMapper, axisMapper);
    axisMapper->SetInput(axisText->GetOutput());

    VTK_CREATE(vtkFollower, axisActor);
    axisActor->SetMapper(axisMapper);
    axisActor->SetPickable(0);
    this->AxisLabelActors << axisActor;

    axisActor->GetProperty()->SetColor(1, 1, 1);
    axisActor->GetProperty()->SetDiffuse(0.0);
    axisActor->GetProperty()->SetAmbient(1.0);
    axisActor->GetProperty()->SetSpecular(0.0);
    }

  this->updateAxis();
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::updateAxis()
{
  CTK_P(qMRMLThreeDRenderView);
  Q_ASSERT(this->MRMLViewNode);
  
  // Turn off box and axis labels to compute bounds
  this->BoxAxisActor->VisibilityOff();
  foreach(vtkFollower* actor, this->AxisLabelActors)
    {
    actor->VisibilityOff();
    }

  // Compute bounds
  double bounds[6];
  p->renderer()->ComputeVisiblePropBounds(bounds);

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

  // See if bounding box has changed. If not, no need to change the
  // axis actors.
  bool bBoxChanged = false;
  if (newBBox != *(this->BoxAxisBoundingBox))
    {
    bBoxChanged = true;
    *(this->BoxAxisBoundingBox) = newBBox;

    double bounds[6];
    this->BoxAxisBoundingBox->GetBounds(bounds);

    VTK_CREATE(vtkOutlineSource, boxSource);
    boxSource->SetBounds(bounds);

    VTK_CREATE(vtkPolyDataMapper, boxMapper);
    boxMapper->SetInput(boxSource->GetOutput());

    this->BoxAxisActor->SetMapper(boxMapper);
    this->BoxAxisActor->SetScale(1.0, 1.0, 1.0);

    double letterSize = this->MRMLViewNode->GetLetterSize();

    foreach(vtkFollower* actor, this->AxisLabelActors)
      {
      actor->SetScale(
        this->BoxAxisBoundingBox->GetMaxLength() * letterSize,
        this->BoxAxisBoundingBox->GetMaxLength() * letterSize,
        this->BoxAxisBoundingBox->GetMaxLength() * letterSize);
      actor->SetOrigin(.5, .5,.5);
      actor->SetCamera(p->activeCamera());
      }

    // Position the axis labels
    double center[3];
    this->BoxAxisBoundingBox->GetCenter(center);

    double offset = this->BoxAxisBoundingBox->GetMaxLength() * letterSize * 1.5;
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

  // Make the axis visible again
  this->BoxAxisActor->VisibilityOn();
  foreach(vtkFollower* actor, this->AxisLabelActors)
    {
    actor->VisibilityOn();
    }
    
  // Until we come up with a solution for all use cases, the resetting
  // of the camera is disabled
#if 0
  if (bBoxChanged)
    {
    p->resetCamera();
    p->renderer()->GetActiveCamera()->Dolly(1.5);
    p->renderer()->ResetCameraClippingRange();
    }
#endif
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::addAxis()
{
  CTK_P(qMRMLThreeDRenderView);
  Q_ASSERT(this->BoxAxisActor);
  p->renderer()->AddViewProp(this->BoxAxisActor);

  foreach(vtkFollower* actor, this->AxisLabelActors)
    {
    actor->SetCamera(p->activeCamera());
    p->renderer()->AddViewProp(actor);
    }
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onSceneClosingEvent()
{
  logger.trace("onSceneClosingEvent");
  CTK_P(qMRMLThreeDRenderView);
  p->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onSceneCloseEvent()
{
  logger.trace("onSceneCloseEvent");
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onSceneLoadStartEvent()
{
  CTK_P(qMRMLThreeDRenderView);
  p->setRenderEnabled(false);
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onSceneLoadEndEvent()
{
  CTK_P(qMRMLThreeDRenderView);
  p->setRenderEnabled(true);
  p->scheduleRender();
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onSceneRestoredEvent()
{
  logger.trace("onSceneRestoredEvent");
}
  
// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_ASSERT(scene);
  
  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(mrmlNode);
  if (!mrmlNode)
    {
    return;
    }

  logger.trace(QString("onNodeAddedEvent: %1").arg(mrmlNode->GetClassName()));
  
  if (mrmlNode->IsA("vtkMRMLDisplayableNode"))
    {
    }
  else if (mrmlNode->IsA("vtkMRMLDisplayNode"))
    {
    }
  else if (mrmlNode->IsA("vtkMRMLModelHierarchyNode"))
    {
    }
  else if (mrmlNode->IsA("vtkMRMLClipModelsNode"))
    {
    }
  else if (mrmlNode->IsA("vtkMRMLCameraNode"))
    {
    this->qvtkConnect(mrmlNode, vtkMRMLCameraNode::ActiveTagModifiedEvent, this,
      SLOT(onCameraNodeActiveTagModifiedEvent(vtkObject*)));
    }
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_ASSERT(scene);
  
  vtkMRMLNode * mrmlNode = vtkMRMLNode::SafeDownCast(node);
  Q_ASSERT(mrmlNode);
  if (!mrmlNode)
    {
    return;
    }

  qDebug() << "qMRMLThreeDRenderViewPrivate::onNodeRemovedEvent=>" << mrmlNode->GetClassName();
  
  if (mrmlNode->IsA("vtkMRMLDisplayableNode"))
    {
    }
  else if (mrmlNode->IsA("vtkMRMLDisplayNode"))
    {
    }
  else if (mrmlNode->IsA("vtkMRMLModelHierarchyNode"))
    {
    }
  else if (mrmlNode->IsA("vtkMRMLClipModelsNode"))
    {
    }
  else if (mrmlNode->IsA("vtkMRMLCameraNode"))
    {
    this->qvtkDisconnect(mrmlNode, vtkMRMLCameraNode::ActiveTagModifiedEvent, this,
      SLOT(onCameraNodeActiveTagModifiedEvent(vtkObject*)));
    }
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onCameraNodeActiveTagModifiedEvent(vtkObject* cameraNode)
{
  vtkMRMLCameraNode* node = vtkMRMLCameraNode::SafeDownCast(cameraNode);
  Q_ASSERT(node);  
  Q_UNUSED(node);
  qDebug() << "qMRMLThreeDRenderViewPrivate::onCameraNodeActiveTagModifiedEvent";
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onViewNodeModifiedEvent(vtkObject* viewNode)
{
  Q_UNUSED(viewNode);
  qDebug() << "qMRMLThreeDRenderViewPrivate::onViewNodeModifiedEvent";
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onDisplayableNodeModifiedEvent(vtkObject* displayableNode)
{
  Q_UNUSED(displayableNode);
  qDebug() << "qMRMLThreeDRenderViewPrivate::onDisplayableNodeModifiedEvent";
}

// --------------------------------------------------------------------------
void qMRMLThreeDRenderViewPrivate::onClipModelsNodeModifiedEvent(vtkObject* clipModelsNode)
{
  Q_UNUSED(clipModelsNode);
  qDebug() << "qMRMLThreeDRenderViewPrivate::onClipModelsNodeModifiedEvent";
}

// --------------------------------------------------------------------------
// qMRMLThreeDRenderView methods

// --------------------------------------------------------------------------
qMRMLThreeDRenderView::qMRMLThreeDRenderView(QWidget* _parent) : Superclass(_parent)
{
  CTK_INIT_PRIVATE(qMRMLThreeDRenderView);
  CTK_D(qMRMLThreeDRenderView);
  VTK_CREATE(vtkMRMLThreeDRenderViewInteractorStyle, interactorStyle);
  this->interactor()->SetInteractorStyle(interactorStyle);
  d->InteractorStyle = interactorStyle;
}

//------------------------------------------------------------------------------
void qMRMLThreeDRenderView::setMRMLScene(vtkMRMLScene* scene)
{
  CTK_D(qMRMLThreeDRenderView);
  d->setMRMLScene(scene);
}

//---------------------------------------------------------------------------
void qMRMLThreeDRenderView::setMRMLViewNode(vtkMRMLViewNode* viewNode)
{
  CTK_D(qMRMLThreeDRenderView);
  d->setMRMLViewNode(viewNode);
  d->initialize(viewNode);
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLThreeDRenderView, vtkMRMLViewNode*, mrmlViewNode, MRMLViewNode);

//---------------------------------------------------------------------------
void qMRMLThreeDRenderView::setAxisLabelColor( double r, double g, double b)
{
  CTK_D(qMRMLThreeDRenderView);
  foreach(vtkFollower* actor, d->AxisLabelActors)
    {
    actor->GetProperty()->SetColor ( r, g, b );
    }
}
