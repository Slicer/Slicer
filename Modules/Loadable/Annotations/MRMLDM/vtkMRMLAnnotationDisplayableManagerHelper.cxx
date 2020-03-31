
// Annotation MRML includes
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationDisplayNode.h>

// Annotation MRMLDisplayableManager includes
#include "vtkMRMLAnnotationDisplayableManagerHelper.h"

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkHandleWidget.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkSphereHandleRepresentation.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationDisplayableManagerHelper);

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManagerHelper::vtkMRMLAnnotationDisplayableManagerHelper()
{
  this->SeedWidget = nullptr;
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManagerHelper::~vtkMRMLAnnotationDisplayableManagerHelper()
{
  if(this->SeedWidget)
    {
    this->RemoveSeeds();
    }
  this->RemoveAllWidgetsAndNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::UpdateLockedAllWidgetsFromNodes()
{
  // iterate through the node list
  for (unsigned int i = 0; i < this->AnnotationNodeList.size(); i++)
    {
    vtkMRMLAnnotationNode *annotationNode = this->AnnotationNodeList[i];
    this->UpdateLocked(annotationNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::UpdateLockedAllWidgetsFromInteractionNode(vtkMRMLInteractionNode *interactionNode)
{
  if (!interactionNode)
    {
    return;
    }

  int currentInteractionMode = interactionNode->GetCurrentInteractionMode();
  vtkDebugMacro("Annotation DisplayableManager Helper: updateLockedAllWidgetsFromInteractionNode, currentInteractionMode = " << currentInteractionMode);
  if (currentInteractionMode == vtkMRMLInteractionNode::Place)
    {
    // turn off processing events on the 3d widgets
    this->UpdateLockedAllWidgets(true);
    }
  else if (currentInteractionMode == vtkMRMLInteractionNode::ViewTransform)
    {
    // reset the processing of events on the 3d widgets from the mrml nodes
    this->UpdateLockedAllWidgetsFromNodes();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::UpdateLockedAllWidgets(bool locked)
{
  // loop through all widgets and update lock status
  vtkDebugMacro("UpdateLockedAllWidgets: locked = " << locked);
  for (WidgetsIt it = this->Widgets.begin();
       it !=  this->Widgets.end();
       ++it)
    {
    if (it->second)
      {
      if (locked)
        {
        it->second->ProcessEventsOff();
        }
      else
        {
        it->second->ProcessEventsOn();
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::UpdateLocked(vtkMRMLAnnotationNode* node)
{
  // Sanity checks
  if (node == nullptr)
    {
    return;
    }

  vtkAbstractWidget * widget = this->GetWidget(node);
  // A widget is expected
  if(widget == nullptr)
    {
    return;
    }

  bool isLockedOnNode = (node->GetLocked() != 0 ? true : false);
  bool isLockedOnWidget = (widget->GetProcessEvents() != 0 ? false : true);

  // only update the processEvents state of the widget if it is different than on the node
  if (isLockedOnNode && !isLockedOnWidget)
    {
    widget->ProcessEventsOff();
    }
  else if (!isLockedOnNode && isLockedOnWidget)
    {
    widget->ProcessEventsOn();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::UpdateVisible(vtkMRMLAnnotationNode* node, bool displayableInViewer)
{
  // Sanity checks
  if (node == nullptr)
    {
    return;
    }

  vtkAbstractWidget * widget = this->GetWidget(node);
  // A widget is expected
  if(widget == nullptr)
    {
    return;
    }

  bool isVisibleOnNode = (node->GetDisplayVisibility() != 0 ? true : false);
  bool isVisibleOnWidget = (widget->GetEnabled() != 0 ? true : false);

  vtkDebugMacro("UpdateVisible: isVisibleOnNode = " << isVisibleOnNode
                << ", isVisibleOnWidget = " << isVisibleOnWidget
                << ", displayableInViewer = " << displayableInViewer);
  if (!displayableInViewer)
    {
    if (isVisibleOnWidget)
      {
      // this viewer can't display it, but the widget is currently on, so turn
      // it off
      widget->EnabledOff();
      }
    }
  else
    {
    // the viewer can display it, but only update the visibility on the widget
    // if it is different than on the node
    if (isVisibleOnNode && !isVisibleOnWidget)
      {
      widget->EnabledOn();
      vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
      if (seedWidget)
        {
        seedWidget->CompleteInteraction();
        }
      }
    else if (!isVisibleOnNode && isVisibleOnWidget)
      {
      widget->EnabledOff();
      }
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::UpdateWidget(
    vtkMRMLAnnotationNode *node)
{
  if (!node)
    {
      return;
    }

  vtkAbstractWidget * widget = this->GetWidget(node);
  // Widget is expected to be valid
  if (widget == nullptr)
    {
    return;
    }

  //this->UpdateLocked(node);
  this->UpdateVisible(node);

}


//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetWidget(
    vtkMRMLAnnotationNode * node)
{
  if (!node)
    {
    return nullptr;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetsIt it = this->Widgets.find(node);
  if (it == this->Widgets.end())
    {
    return nullptr;
    }

  return it->second;
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetIntersectionWidget(
    vtkMRMLAnnotationNode * node)
{
  if (!node)
    {
    return nullptr;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetIntersectionsIt it = this->WidgetIntersections.find(node);
  if (it == this->WidgetIntersections.end())
    {
    return nullptr;
    }

  return it->second;
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetOverLineProjectionWidget(
    vtkMRMLAnnotationNode * node)
{
  if (!node)
    {
    return nullptr;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetOverLineProjectionsIt it = this->WidgetOverLineProjections.find(node);
  if (it == this->WidgetOverLineProjections.end())
    {
    return nullptr;
    }

  return it->second;
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetUnderLineProjectionWidget(
    vtkMRMLAnnotationNode * node)
{
  if (!node)
    {
    return nullptr;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetUnderLineProjectionsIt it = this->WidgetUnderLineProjections.find(node);
  if (it == this->WidgetUnderLineProjections.end())
    {
    return nullptr;
    }

  return it->second;
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetPointProjectionWidget(
    vtkMRMLAnnotationNode * node)
{
  if (!node)
    {
    return nullptr;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetPointProjectionsIt it = this->WidgetPointProjections.find(node);
  if (it == this->WidgetPointProjections.end())
    {
    return nullptr;
    }

  return it->second;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::RemoveAllWidgetsAndNodes()
{
  WidgetsIt widgetIterator = this->Widgets.begin();
  for (widgetIterator =  this->Widgets.begin();
       widgetIterator != this->Widgets.end();
       ++widgetIterator)
    {
    widgetIterator->second->Off();
    widgetIterator->second->Delete();
    }
  this->Widgets.clear();

  WidgetIntersectionsIt intIt;
  for (intIt = this->WidgetIntersections.begin();
       intIt != this->WidgetIntersections.end();
       ++intIt)
    {
    intIt->second->Off();
    intIt->second->Delete();
    }
  this->WidgetIntersections.clear();

  WidgetOverLineProjectionsIt projOverIt;
  for (projOverIt = this->WidgetOverLineProjections.begin();
       projOverIt != this->WidgetOverLineProjections.end();
       ++projOverIt)
    {
    projOverIt->second->Off();
    projOverIt->second->Delete();
    }
  this->WidgetOverLineProjections.clear();

  WidgetUnderLineProjectionsIt projUnderIt;
  for (projUnderIt = this->WidgetUnderLineProjections.begin();
       projUnderIt != this->WidgetUnderLineProjections.end();
       ++projUnderIt)
    {
    projUnderIt->second->Off();
    projUnderIt->second->Delete();
    }
  this->WidgetUnderLineProjections.clear();

  WidgetPointProjectionsIt pointProjIt;
  for (pointProjIt = this->WidgetPointProjections.begin();
       pointProjIt != this->WidgetPointProjections.end();
       ++pointProjIt)
    {
    pointProjIt->second->Off();
    pointProjIt->second->Delete();
    }
  this->WidgetPointProjections.clear();

  this->AnnotationNodeList.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::RemoveWidgetAndNode(
    vtkMRMLAnnotationNode *node)
{
  if (!node)
  {
    return;
  }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetsIt widgetIterator = this->Widgets.find(node);
  if (widgetIterator != this->Widgets.end()) {
    // Delete and Remove vtkWidget from the map
    if (this->Widgets[node])
      {
      this->Widgets[node]->Off();
      this->Widgets[node]->Delete();
      }
    this->Widgets.erase(node);
  }

  WidgetIntersectionsIt widgetIntersectionIterator = this->WidgetIntersections.find(node);
  if (widgetIntersectionIterator != this->WidgetIntersections.end()) {
    // we have a vtkAbstractWidget to represent the slice intersections for this node
    // now delete it!
    if (this->WidgetIntersections[node])
      {
      this->WidgetIntersections[node]->Off();
      this->WidgetIntersections[node]->Delete();
      }
    this->WidgetIntersections.erase(node);
  }

  WidgetOverLineProjectionsIt widgetOverLineProjectionIterator = this->WidgetOverLineProjections.find(node);
  if (widgetOverLineProjectionIterator != this->WidgetOverLineProjections.end()) {

  if (this->WidgetOverLineProjections[node])
    {
    this->WidgetOverLineProjections[node]->Off();
    this->WidgetOverLineProjections[node]->Delete();
    }
  this->WidgetOverLineProjections.erase(node);
  }

  WidgetUnderLineProjectionsIt widgetUnderLineProjectionIterator = this->WidgetUnderLineProjections.find(node);
  if (widgetUnderLineProjectionIterator != this->WidgetUnderLineProjections.end()) {

  if (this->WidgetUnderLineProjections[node])
    {
    this->WidgetUnderLineProjections[node]->Off();
    this->WidgetUnderLineProjections[node]->Delete();
    }
  this->WidgetUnderLineProjections.erase(node);
  }

  WidgetPointProjectionsIt widgetPointProjectionIterator = this->WidgetPointProjections.find(node);
  if (widgetPointProjectionIterator != this->WidgetPointProjections.end()) {

  if (this->WidgetPointProjections[node])
    {
    this->WidgetPointProjections[node]->Off();
    this->WidgetPointProjections[node]->Delete();
    }
  this->WidgetPointProjections.erase(node);
  }

  vtkMRMLAnnotationDisplayableManagerHelper::AnnotationNodeListIt nodeIterator = std::find(
      this->AnnotationNodeList.begin(),
      this->AnnotationNodeList.end(),
      node);

  // Make sure the map contains the annotationNode
  if (nodeIterator != this->AnnotationNodeList.end())
    {
    //vtkMRMLAnnotationNode* annotationNode = vtkMRMLAnnotationNode::SafeDownCast(*nodeIterator);
    //if (annotationNode)
     // {
      //annotationNode->Delete();
     // }
    this->AnnotationNodeList.erase(nodeIterator);
    }

}

//---------------------------------------------------------------------------
// Seeds for widget placement
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::PlaceSeed(double x, double y, vtkRenderWindowInteractor * interactor, vtkRenderer * renderer)
{
  vtkDebugMacro("PlaceSeed: " << x << ":" << y);

  if (!interactor)
    {
    vtkErrorMacro("PlaceSeed: No renderInteractor found.");
    }

  if (!renderer)
    {
    vtkErrorMacro("PlaceSeed: No renderer found.");
    }

  if (!this->SeedWidget)
    {

    vtkNew<vtkSphereHandleRepresentation> handle;
    handle->GetProperty()->SetColor(1,0,0);
    handle->SetHandleSize(5);

    vtkNew<vtkSeedRepresentation> rep;
    rep->SetHandleRepresentation(handle.GetPointer());

    //seed widget
    vtkSeedWidget * seedWidget = vtkSeedWidget::New();
    seedWidget->SetRepresentation(rep.GetPointer());

    seedWidget->SetInteractor(interactor);
    seedWidget->SetCurrentRenderer(renderer);

    seedWidget->CompleteInteraction();
    seedWidget->ManagesCursorOff();
    seedWidget->ProcessEventsOff();


    this->SeedWidget = seedWidget;

    }

  // Seed widget exists here, just add a new handle at the position x,y

  double p[3] = {0,0,0};
  p[0]=x;
  p[1]=y;
  p[2]=0;

  //vtkNew<vtkHandleWidget, newhandle);
  vtkHandleWidget * newhandle = this->SeedWidget->CreateNewHandle();
  vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetDisplayPosition(p);

  this->HandleWidgetList.emplace_back(newhandle);

  this->SeedWidget->On();
  this->SeedWidget->CompleteInteraction();
  this->SeedWidget->ManagesCursorOff();
  this->SeedWidget->ProcessEventsOff();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::RemoveSeeds()
{
  while(!this->HandleWidgetList.empty())
    {
    this->HandleWidgetList.pop_back();
    }
  if (this->SeedWidget)
    {
    this->SeedWidget->Off();
    this->SeedWidget->Delete();
    this->SeedWidget = nullptr;
    }
}

//---------------------------------------------------------------------------
vtkHandleWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetSeed(int index)
{
  if (this->HandleWidgetList.empty())
    {
    return nullptr;
    }

  return this->HandleWidgetList[index];
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationNode * vtkMRMLAnnotationDisplayableManagerHelper::GetAnnotationNodeFromDisplayNode(vtkMRMLAnnotationDisplayNode *displayNode)
{
  if (!displayNode ||
      !displayNode->GetID())
    {
    vtkErrorMacro("GetAnnotationNodeFromDisplayNode: display node or it's id is null");
    return nullptr;
    }
  // iterate through the node list
  for (unsigned int i = 0; i < this->AnnotationNodeList.size(); i++)
    {
    vtkMRMLAnnotationNode *annotationNode = this->AnnotationNodeList[i];
    int numNodes = annotationNode->GetNumberOfDisplayNodes();
    for (int n = 0; n < numNodes; n++)
      {
      vtkMRMLDisplayNode *thisDisplayNode = annotationNode->GetNthDisplayNode(n);
      if (thisDisplayNode && thisDisplayNode->GetID() &&
          displayNode->GetID())
        {
        if (strcmp(thisDisplayNode->GetID(),displayNode->GetID()) == 0)
          {
          return annotationNode;
          }
        }
      }
    }
  vtkDebugMacro("GetAnnotationNodeFromDisplayNode: unable to find annotation node that has display node "
                << (displayNode->GetID() ? displayNode->GetID() : "null"));
  return nullptr;
}
