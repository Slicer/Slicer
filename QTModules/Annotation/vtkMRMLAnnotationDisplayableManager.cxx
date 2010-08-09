// AnnotationModule includes
#include "vtkMRMLAnnotationDisplayableManager.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationNode.h"

// MRML includes
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLInteractionNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkLineRepresentation.h>
#include <vtkPolygonalSurfacePointPlacer.h>
#include <vtkMath.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkProperty2D.h>

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

// vtkMRMLAnnotationDisplayableManager seedCallback
//---------------------------------------------------------------------------
class vtkSeedCallback : public vtkCommand
{
public:
  static vtkSeedCallback *New()
  { return new vtkSeedCallback; }
  vtkSeedCallback() {}

  virtual void Execute(vtkObject*, unsigned long event, void *calldata)
     {

        std::cout << "entering" << std::endl;
       if (event == vtkCommand::PlacePointEvent)
       {
         cout << "Point placed, total of: "
             << this->SeedRepresentation->GetNumberOfSeeds() << endl;
       }
       if (event == vtkCommand::InteractionEvent)
       {
         if (calldata)
         {
           cout << "Interacting with seed : "
               << *(static_cast< int * >(calldata)) << endl;
         }
       }


       cout << "List of seeds (Display coordinates):" << endl;
       for(vtkIdType i = 0; i < this->SeedRepresentation->GetNumberOfSeeds(); i++)
         {
         double pos[3];
         this->SeedRepresentation->GetSeedDisplayPosition(i, pos);
         cout << "(" << pos[0] << " " << pos[1] << " " << pos[2] << ")" << endl;
         }

     }

     void SetRepresentation(vtkSmartPointer<vtkSeedRepresentation> rep) {this->SeedRepresentation = rep;}
   private:
     vtkSmartPointer<vtkSeedRepresentation> SeedRepresentation;
};

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::vtkMRMLAnnotationDisplayableManager()
{
  this->Internal = new vtkInternal;
  this->m_ClickCounter = vtkMRMLAnnotationClickCounter::New();
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManager::~vtkMRMLAnnotationDisplayableManager()
{
  delete this->Internal;
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
  vtkDebugMacro("OnMRMLSceneNodeAddedEvent");
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
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent: Widget was not created!")
    return;
  }
  this->Internal->Widgets[annotationNode] = newWidget;

  // Add the node to the list.
  this->Internal->AnnotationNodeList.push_back(annotationNode);

  // Refresh observers
  this->SetAndObserveNodes();

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
  vtkMRMLAnnotationNode *annotationNode = vtkMRMLAnnotationNode::SafeDownCast(node);
  if (!annotationNode)
    {
    vtkErrorMacro("OnMRMLAnnotationNodeModifiedEvent - Can not access node.")
    return;
    }
  // Update the standard settings of all widgets.
  this->Internal->UpdateWidget(annotationNode);

  // Set individual properties of the widget.
  this->SetWidget(annotationNode);

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
  std::cout << "PlaceSeed" << std::endl;

  VTK_CREATE(vtkPointHandleRepresentation3D, handle);
  handle->GetProperty()->SetColor(1,0,0);
  handle->AllOn();
  VTK_CREATE(vtkSeedRepresentation, rep);
  rep->SetHandleRepresentation(handle);

  //seed widget
  VTK_CREATE(vtkSeedWidget, seedWidget);
  seedWidget->SetInteractor(this->GetInteractor());
  seedWidget->SetRepresentation(rep);

  vtkSmartPointer<vtkSeedCallback> seedCallback = vtkSmartPointer<vtkSeedCallback>::New();
  seedCallback->SetRepresentation(rep);
  seedWidget->AddObserver(vtkCommand::PlacePointEvent,seedCallback);
  seedWidget->AddObserver(vtkCommand::InteractionEvent,seedCallback);

  seedWidget->On();

  this->RequestRender();

}


//---------------------------------------------------------------------------
// Functions to overload!
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
void vtkMRMLAnnotationDisplayableManager::SetWidget(vtkMRMLAnnotationNode* node)
{
  // The properties of a widget should be set here.
  vtkErrorMacro("SetWidget should be overloaded!");
}
