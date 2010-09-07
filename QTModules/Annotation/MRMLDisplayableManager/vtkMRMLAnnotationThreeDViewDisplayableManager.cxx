// AnnotationModule includes
#include "vtkMRMLAnnotationThreeDViewDisplayableManager.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationNode.h"

// MRML includes
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkMath.h>

// STD includes
#include <vector>
#include <map>
#include <algorithm>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

typedef void (*fp)(void);

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationThreeDViewDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationThreeDViewDisplayableManager, "$Revision: 1.2 $");

//---------------------------------------------------------------------------
vtkMRMLAnnotationThreeDViewDisplayableManager::vtkMRMLAnnotationThreeDViewDisplayableManager()
{
  this->Helper = vtkMRMLAnnotationDisplayableManagerHelper::New();
  this->m_ClickCounter = vtkMRMLAnnotationClickCounter::New();
  this->m_DisableInteractorStyleEventsProcessing = 0;
  this->m_Updating = 0;

  this->m_Focus = "vtkMRMLAnnotationNode";
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationThreeDViewDisplayableManager::~vtkMRMLAnnotationThreeDViewDisplayableManager()
{

  this->m_DisableInteractorStyleEventsProcessing = 0;
  this->m_Updating = 0;
  this->m_Focus = 0;
  this->Helper->Delete();
  this->m_ClickCounter->Delete();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::SetAndObserveNodes()
{
  VTK_CREATE(vtkIntArray, nodeEvents);
  nodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLAnnotationNode::LockModifiedEvent);
  nodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);


  // run through all associated nodes
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it;
  it = this->Helper->AnnotationNodeList.begin();
  while(it != this->Helper->AnnotationNodeList.end())
    {
    vtkSetAndObserveMRMLNodeEventsMacro(*it, *it, nodeEvents);
    ++it;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::Create()
{

  // hack to force initialization of the renderview
  this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
  this->GetInteractor()->InvokeEvent(vtkCommand::MouseWheelForwardEvent);


}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::ProcessMRMLEvents(vtkObject *caller,
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
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLSceneAboutToBeClosedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLSceneClosedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLSceneAboutToBeImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLSceneImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{

  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");

  if (!this->IsCorrectDisplayableManager())
    {
    // jump out
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Not the correct displayableManager, jumping out!")
    return;
    }

  vtkMRMLAnnotationNode * annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    return;
    }

  // Node added should not be already managed
  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt it = std::find(
      this->Helper->AnnotationNodeList.begin(),
      this->Helper->AnnotationNodeList.end(),
      annotationNode);
  if (it != this->Helper->AnnotationNodeList.end())
    {
      vtkErrorMacro("OnMRMLSceneNodeAddedEvent: This node is already associated to the displayable manager!")
      return;
    }

  // There should not be a widget for the new node
  if (this->Helper->GetWidget(annotationNode) != 0)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: A widget is already associated to this node!");
    return;
    }

  std::cout << "OnMRMLSceneNodeAddedEvent ThreeD -> CreateWidget" << std::endl;

  // Create the Widget and add it to the list.
  vtkAbstractWidget* newWidget = this->CreateWidget(annotationNode);
  if (!newWidget) {
    vtkDebugMacro("OnMRMLSceneNodeAddedEvent: Widget was not created!")
    // Exit here, if this is not the right displayableManager
    return;
  }
  this->Helper->Widgets[annotationNode] = newWidget;

  // Add the node to the list.
  this->Helper->AnnotationNodeList.push_back(annotationNode);

  // Refresh observers
  this->SetAndObserveNodes();

  this->RequestRender();

  // for the following, deactivate tracking of mouse clicks b/c it might be simulated

  this->m_DisableInteractorStyleEventsProcessing = 1;
  // tear down widget creation
  this->OnWidgetCreated(newWidget, annotationNode);
  this->m_DisableInteractorStyleEventsProcessing = 0;

  // Remove all placed seeds
  this->Helper->RemoveSeeds();

  // and render again after seeds were removed
  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node)
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
  this->Helper->RemoveWidget(annotationNode);

  // Refresh observers
  this->SetAndObserveNodes();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node)
{
  //this->DebugOn();

  vtkDebugMacro("OnMRMLAnnotationNodeModifiedEvent");

  if (this->m_Updating)
    {
    vtkDebugMacro("OnMRMLAnnotationNodeModifiedEvent: Updating in progress.. Exit now.")
    return;
    }

  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeModifiedEvent: Can not access node.")
    return;
    }

  std::cout << "OnMRMLAnnotationNodeModifiedEvent ThreeD->PropagateMRMLToWidget" << std::endl;

  // Update the standard settings of all widgets.
  this->Helper->UpdateWidget(annotationNode);

  vtkAbstractWidget * widget = this->Helper->GetWidget(annotationNode);

  // Propagate MRML changes to widget
  this->PropagateMRMLToWidget(annotationNode, widget);

  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLAnnotationNodeTransformModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeTransformModifiedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeTransformModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Helper->UpdateWidget(annotationNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnMRMLAnnotationNodeLockModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationNodeLockModifiedEvent");
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeLockModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Helper->UpdateWidget(annotationNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnInteractorStyleEvent(int eventid)
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
vtkAbstractWidget * vtkMRMLAnnotationThreeDViewDisplayableManager::GetWidget(vtkMRMLAnnotationNode * node)
{
  return this->Helper->GetWidget(node);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnClickInThreeDRenderWindowGetCoordinates()
{

  double x = this->GetInteractor()->GetEventPosition()[0];
  //int rawY = this->GetInteractor()->GetEventPosition()[1];
  //this->GetInteractor()->SetEventPositionFlipY(x, rawY);
  double y = this->GetInteractor()->GetEventPosition()[1];


  this->OnClickInThreeDRenderWindow(x, y);
}


//---------------------------------------------------------------------------
// Placement of widgets through seeds
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/// Place a seed for widgets
void vtkMRMLAnnotationThreeDViewDisplayableManager::PlaceSeed(double x, double y)
{

  // place the seed
  this->Helper->PlaceSeed(x,y,this->GetInteractor(),this->GetRenderer());

  this->RequestRender();

}

//---------------------------------------------------------------------------
/// Get the handle of a placed seed
vtkHandleWidget * vtkMRMLAnnotationThreeDViewDisplayableManager::GetSeed(int index)
{
  return this->Helper->GetSeed(index);
}

//---------------------------------------------------------------------------
// Coordinate conversions
//---------------------------------------------------------------------------
/// Convert display to world coordinates
double* vtkMRMLAnnotationThreeDViewDisplayableManager::GetDisplayToWorldCoordinates(double x, double y)
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
double* vtkMRMLAnnotationThreeDViewDisplayableManager::GetWorldToDisplayCoordinates(double r, double a, double s)
{

  double * displayCoordinates;

  this->GetRenderer()->SetWorldPoint(r,a,s,1);
  this->GetRenderer()->WorldToView();
  displayCoordinates = this->GetRenderer()->GetViewPoint();
  this->GetRenderer()->ViewToDisplay();

  return this->GetRenderer()->GetDisplayPoint();

}

//---------------------------------------------------------------------------
/// Convert world to display coordinates
double* vtkMRMLAnnotationThreeDViewDisplayableManager::GetWorldToDisplayCoordinates(double * worldPoints)
{

  vtkDebugMacro("GetWorldToDisplayCoordinates: RAS " << worldPoints[0] << ", " << worldPoints[1] << ", " << worldPoints[2])

  double * displayCoordinates;

  this->GetRenderer()->SetWorldPoint(worldPoints);
  this->GetRenderer()->WorldToView();
  displayCoordinates = this->GetRenderer()->GetViewPoint();
  this->GetRenderer()->ViewToDisplay();

  double * output = this->GetRenderer()->GetDisplayPoint();

  vtkDebugMacro("GetWorldToDisplayCoordinates: Display " << output[0] << ", " << output[1])

  return output;

}

//---------------------------------------------------------------------------
/// Check if it is the correct displayableManager
//---------------------------------------------------------------------------
bool vtkMRMLAnnotationThreeDViewDisplayableManager::IsCorrectDisplayableManager()
{

  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
        this->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLSelectionNode"));
  if ( selectionNode == NULL )
    {
    vtkErrorMacro ( "OnClickInThreeDRenderWindow: No selection node in the scene." );
    return false;
    }
  if ( selectionNode->GetActiveAnnotationID() == NULL)
    {
    return false;
    }
  // the purpose of the displayableManager is hardcoded
  return !strcmp(selectionNode->GetActiveAnnotationID(), this->m_Focus);

}

//---------------------------------------------------------------------------
// Functions to overload!
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnClickInThreeDRenderWindow(double x, double y)
{
  // The user clicked in the renderWindow
  vtkErrorMacro("OnClickInThreeDRenderWindow should be overloaded!");
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationThreeDViewDisplayableManager::CreateWidget(vtkMRMLAnnotationNode* node)
{
  // A widget should be created here.
  vtkErrorMacro("CreateWidget should be overloaded!");
  return 0;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node)
{
  // Actions after a widget was created should be executed here.
  vtkErrorMacro("OnWidgetCreated should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget)
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateMRMLToWidget should be overloaded!");
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationThreeDViewDisplayableManager::PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node)
{
  // The properties of a widget should be set here.
  vtkErrorMacro("PropagateWidgetToMRML should be overloaded!");
}
