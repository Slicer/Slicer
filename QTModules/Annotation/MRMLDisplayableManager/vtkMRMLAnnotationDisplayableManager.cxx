// AnnotationModule includes
#include "vtkMRMLAnnotationDisplayableManager.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationNode.h"

// MRML includes
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLInteractionNode.h>
#include "vtkMRMLSelectionNode.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkSphereHandleRepresentation.h>
#include <vtkLineRepresentation.h>
#include <vtkPolygonalSurfacePointPlacer.h>
#include <vtkMath.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkRenderWindow.h>
#include <vtkProperty2D.h>
#include <vtkHandleWidget.h>
#include <vtkSphereWidget.h>

// STD includes
#include <vector>
#include <map>
#include <algorithm>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

typedef void (*fp)(void);

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationDisplayableManager, "$Revision: 1.1 $");

//---------------------------------------------------------------------------
class vtkMRMLAnnotationDisplayableManager::vtkInternal
{
public:
  vtkInternal();

  void UpdateLocked(vtkMRMLAnnotationNode* node);
  void UpdateVisible(vtkMRMLAnnotationNode* node);
  void UpdateWidget(vtkMRMLAnnotationNode* node);

  /// Get a vtkAbstractWidget* given \a node
  vtkAbstractWidget * GetWidget(vtkMRMLAnnotationNode * node);
  void RemoveWidget(vtkMRMLAnnotationNode *node);

  /// List of Nodes managed by the DisplayableManager
  std::vector<vtkMRMLAnnotationNode*> AnnotationNodeList;

  /// .. and its associated convenient typedef
  typedef std::vector<vtkMRMLAnnotationNode*>::iterator AnnotationNodeListIt;

  /// Map of vtkWidget indexed using associated node ID
  std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget *> Widgets;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLAnnotationNode*, vtkAbstractWidget *>::iterator WidgetsIt;

};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::vtkInternal::vtkInternal()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::vtkInternal::UpdateLocked(
    vtkMRMLAnnotationNode* node)
{
  // Sanity checks
  if (node == 0)
    {
    return;
    }

  vtkAbstractWidget * widget = this->GetWidget(node);
  // A widget is expected
  if(widget == 0)
    {
    return;
    }

  if (node->GetLocked())
    {
    widget->ProcessEventsOff();
    }
  else
    {
    widget->ProcessEventsOn();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::vtkInternal::UpdateVisible(
    vtkMRMLAnnotationNode* node)
{
  // Sanity checks
  if (node == 0)
    {
    return;
    }

  vtkAbstractWidget * widget = this->GetWidget(node);
  // A widget is expected
  if(widget == 0)
    {
    return;
    }

  if (node->GetVisible())
    {
    widget->EnabledOn();
    }
  else
    {
    widget->EnabledOff();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::vtkInternal::UpdateWidget(
    vtkMRMLAnnotationNode *node)
{
  if (!node)
    {
      return;
    }

  vtkAbstractWidget * widget = this->GetWidget(node);
  // Widget is expected to be valid
  if (widget == 0)
    {
    return;
    }

  this->UpdateLocked(node);
  this->UpdateVisible(node);

}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManager::vtkInternal::GetWidget(
    vtkMRMLAnnotationNode * node)
{
  if (!node)
    {
    return 0;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetsIt it = this->Widgets.find(node);
  if (it == this->Widgets.end())
    {
    return 0;
    }

  return it->second;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::vtkInternal::RemoveWidget(
    vtkMRMLAnnotationNode *node)
{
  if (!node)
  {
    return;
  }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetsIt it = this->Widgets.find(node);
  if (it == this->Widgets.end()) {
    return;
  }

  // Delete and Remove vtkWidget from the map
  this->Widgets[node]->Delete();
  this->Widgets.erase(node);

  vtkInternal::AnnotationNodeListIt it2 = std::find(
      this->AnnotationNodeList.begin(),
      this->AnnotationNodeList.end(),
      node);

  this->AnnotationNodeList.erase(it2);
}

//---------------------------------------------------------------------------
// vtkMRMLAnnotationDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::vtkMRMLAnnotationDisplayableManager()
{
  this->Internal = new vtkInternal;
  this->m_ClickCounter = vtkMRMLAnnotationClickCounter::New();
  this->m_SeedWidget = 0;
  this->m_DisableInteractorStyleEventsProcessing = 0;
  this->m_Updating = 0;

  this->m_Focus = "vtkMRMLAnnotationNode";
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::~vtkMRMLAnnotationDisplayableManager()
{
  delete this->Internal;
  this->m_SeedWidget = 0;
  this->m_DisableInteractorStyleEventsProcessing = 0;
  this->m_Updating = 0;
  this->m_Focus = 0;
  this->m_ClickCounter->Delete();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::SetAndObserveNodes()
{
  VTK_CREATE(vtkIntArray, nodeEvents);
  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLAnnotationNode::LockModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);


  // run through all associated nodes
  vtkInternal::AnnotationNodeListIt it;
  it = Internal->AnnotationNodeList.begin();
  while(it != Internal->AnnotationNodeList.end())
    {
    vtkSetAndObserveMRMLNodeEventsMacro(*it, *it, nodeEvents);
    ++it;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::Create()
{

  // hack to force initialization of the renderview
  this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
  this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelForwardEvent);

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::ProcessMRMLEvents(vtkObject *caller,
                                                            unsigned long event,
                                                            void *callData)
{
  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(caller);
  if (annotationNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLAnnotationNodeModifiedEvent(annotationNode);
        break;
      case vtkMRMLTransformableNode::TransformModifiedEvent:
        this->OnMRMLAnnotationNodeTransformModifiedEvent(annotationNode);
        break;
      case vtkMRMLAnnotationNode::LockModifiedEvent:
        this->OnMRMLAnnotationNodeLockModifiedEvent(annotationNode);
        break;
      }
    }
  else
    {
    this->Superclass::ProcessMRMLEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneAboutToBeClosedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneClosedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneAboutToBeImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  this->DebugOn();

  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");
  std::cout << node->GetID() << std::endl;
  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }

  // Node added should not be already managed
  vtkInternal::AnnotationNodeListIt it = std::find(
      this->Internal->AnnotationNodeList.begin(),
      this->Internal->AnnotationNodeList.end(),
      annotationNode);
  if (it != this->Internal->AnnotationNodeList.end())
    {
      vtkErrorMacro("OnMRMLSceneNodeAddedEvent: This node is already associated to the displayable manager!")
      return;
    }

  // There should not be a widget for the new node
  if (this->Internal->GetWidget(annotationNode) != 0)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: A widget is already associated to this node!");
    return;
    }

  // Create the Widget and add it to the list.
  vtkAbstractWidget* newWidget = this->CreateWidget(annotationNode);
  if (!newWidget) {
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Widget was not created!")
    // Exit here, if this is not the right displayableManager
    return;
  }
  this->Internal->Widgets[annotationNode] = newWidget;

  // Add the node to the list.
  this->Internal->AnnotationNodeList.push_back(annotationNode);

  // Remove all placed seeds
  while(!this->m_HandleWidgetList.empty())
    {
    this->m_HandleWidgetList.pop_back();
    }
  if (this->m_SeedWidget)
    {
    this->m_SeedWidget->Off();
    this->m_SeedWidget = 0;
    }

  // Refresh observers
  this->SetAndObserveNodes();

  this->RequestRender();

  // for the following, deactivate tracking of mouse clicks b/c it might be simulated

  this->m_DisableInteractorStyleEventsProcessing = 1;
  // tear down widget creation
  this->OnWidgetCreated(newWidget, annotationNode);
  this->m_DisableInteractorStyleEventsProcessing = 0;

  // Remove all placed seeds
  while(!this->m_HandleWidgetList.empty())
    {
    this->m_HandleWidgetList.pop_back();
    }
  if (this->m_SeedWidget)
    {
    this->m_SeedWidget->Off();
    this->m_SeedWidget = 0;
    }

  // and render again after seeds were removed
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLSceneNodeRemovedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }
  /*
  // Remove the node from the list.
  vtkInternal::AnnotationNodeListIt it = std::find(
      this->Internal->AnnotationNodeList.begin(),
      this->Internal->AnnotationNodeList.end(),
      annotationNode);
  if (it == this->Internal->AnnotationNodeList.end())
    {
    return;
    }
  this->Internal->AnnotationNodeList.erase(it);
  */

  // Remove the widget from the list.
  this->Internal->RemoveWidget(annotationNode);

  // Refresh observers
  this->SetAndObserveNodes();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeModifiedEvent");

  if (this->m_Updating)
    {
    vtkDebugMacro("PropagateWidgetToMRML: Updating in progress.. Exit now.")
    return;
    }

  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeModifiedEvent: Can not access node.")
    return;
    }

  // Update the standard settings of all widgets.
  this->Internal->UpdateWidget(annotationNode);

  vtkAbstractWidget * widget = this->Internal->GetWidget(annotationNode);

  // Propagate MRML changes to widget
  this->PropagateMRMLToWidget(annotationNode, widget);

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationNodeTransformModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeTransformModifiedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeTransformModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Internal->UpdateWidget(annotationNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnMRMLAnnotationNodeLockModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeLockModifiedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeLockModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Internal->UpdateWidget(annotationNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  if (this->m_DisableInteractorStyleEventsProcessing == 1)
    {
    vtkDebugMacro("OnInteractorStyleEvent: Processing of events was disabled.")
    return;
    }
  if (eventid == vtkCommand::LeftButtonReleaseEvent)
    {
    if (this->GetInteractionNode()->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      {
      this->OnClickInThreeDRenderWindowGetCoordinates();
      }
    }
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManager::GetWidget(vtkMRMLAnnotationNode * node)
{
  return this->Internal->GetWidget(node);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnClickInThreeDRenderWindowGetCoordinates()
{

  double x = this->GetInteractor()->GetEventPosition()[0];
  //int rawY = this->GetInteractor()->GetEventPosition()[1];
  //this->GetInteractor()->SetEventPositionFlipY(x, rawY);
  double y = this->GetInteractor()->GetEventPosition()[1];


  this->OnClickInThreeDRenderWindow(x, y);
}

//---------------------------------------------------------------------------
/// Place a seed for widgets
void vtkMRMLAnnotationDisplayableManager::PlaceSeed(double x, double y)
{
  vtkDebugMacro("PlaceSeed: " << x << ":" << y)

  if (!this->m_SeedWidget)
    {

    VTK_CREATE(vtkSphereHandleRepresentation, handle);
    handle->GetProperty()->SetColor(1,0,0);
    handle->SetHandleSize(5);

    VTK_CREATE(vtkSeedRepresentation, rep);
    rep->SetHandleRepresentation(handle);

    //seed widget
    vtkSeedWidget * seedWidget = vtkSeedWidget::New();
    //seedWidget->CreateDefaultRepresentation();
    seedWidget->SetRepresentation(rep);

    seedWidget->SetInteractor(this->GetInteractor());
    seedWidget->SetCurrentRenderer(this->GetRenderer());

    seedWidget->ProcessEventsOff();
    seedWidget->CompleteInteraction();

    //seedWidget->On();

    this->m_SeedWidget = seedWidget;

    }

  // Seed widget exists here, just add a new handle at the position x,y

  double p[3];
  p[0]=x;
  p[1]=y;
  p[2]=0;

  //VTK_CREATE(vtkHandleWidget, newhandle);
  vtkHandleWidget * newhandle = this->m_SeedWidget->CreateNewHandle();
  vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetDisplayPosition(p);

  this->m_HandleWidgetList.push_back(newhandle);

  this->m_SeedWidget->On();

  this->RequestRender();

}

//---------------------------------------------------------------------------
/// Convert display to world coordinates
double* vtkMRMLAnnotationDisplayableManager::GetDisplayToWorldCoordinates(double x, double y)
{
  double coordinates[3];
  coordinates[0]=(double)x;
  coordinates[1]=(double)y;
  coordinates[2]=0;

  this->GetRenderer()->SetDisplayPoint(coordinates);
  this->GetRenderer()->DisplayToView();
  this->GetRenderer()->GetViewPoint(coordinates);
  this->GetRenderer()->ViewToWorld();
  return this->GetRenderer()->GetWorldPoint();

}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
double* vtkMRMLAnnotationDisplayableManager::GetWorldToDisplayCoordinates(double r, double a, double s)
{

  double * displayCoordinates;

  this->GetRenderer()->SetWorldPoint(r,a,s,1);
  this->GetRenderer()->WorldToView();
  displayCoordinates = this->GetRenderer()->GetViewPoint();
  this->GetRenderer()->ViewToDisplay();

  return this->GetRenderer()->GetDisplayPoint();

}

//---------------------------------------------------------------------------
/// Check if it is the correct displayableManager
bool vtkMRMLAnnotationDisplayableManager::IsCorrectDisplayableManager()
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLSelectionNode"));
  if ( selectionNode == NULL )
    {
    vtkErrorMacro ( "OnClickInThreeDRenderWindow: No selection node in the scene." );
    return false;
    }

  // the purpose of the displayableManager is hardcoded
  return !strcmp(selectionNode->GetActiveAnnotationID(), this->m_Focus);

}

//---------------------------------------------------------------------------
// Functions to overload!
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
{
  // The user clicked in the renderWindow
  vtkErrorMacro("OnClickInThreeDRenderWindow should be overloaded!");
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{
  // A widget should be created here.
  vtkErrorMacro("CreateWidget should be overloaded!");
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{
  // Actions after a widget was created should be executed here.
  vtkErrorMacro("OnWidgetCreated should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateMRMLToWidget should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateWidgetToMRML should be overloaded!");
}
