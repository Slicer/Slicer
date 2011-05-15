#include "MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManagerHelper.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationNode.h"

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkAbstractWidget.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedWidget.h>
#include <vtkHandleWidget.h>
#include <vtkSphereHandleRepresentation.h>
#include <vtkSeedRepresentation.h>
#include <vtkProperty.h>

// MRML includes
#include <vtkMRMLSliceNode.h>

// STD includes
#include <vector>
#include <map>
#include <algorithm>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationDisplayableManagerHelper);
vtkCxxRevisionMacro (vtkMRMLAnnotationDisplayableManagerHelper, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
void vtkMRMLAnnotationDisplayableManagerHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManagerHelper::vtkMRMLAnnotationDisplayableManagerHelper()
{
  this->SeedWidget = 0;
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationDisplayableManagerHelper::~vtkMRMLAnnotationDisplayableManagerHelper()
{
  if(this->SeedWidget)
    {
    this->RemoveSeeds();
    }
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
void vtkMRMLAnnotationDisplayableManagerHelper::UpdateVisible(vtkMRMLAnnotationNode* node)
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
    vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
    if (seedWidget)
      {
      seedWidget->CompleteInteraction();
      }
    }
  else
    {
    widget->EnabledOff();
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
  if (widget == 0)
    {
    return;
    }

  this->UpdateLocked(node);
  this->UpdateVisible(node);

}


//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetWidget(
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
vtkAbstractWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetIntersectionWidget(
    vtkMRMLAnnotationNode * node)
{
  if (!node)
    {
    return 0;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetIntersectionsIt it = this->WidgetIntersections.find(node);
  if (it == this->WidgetIntersections.end())
    {
    return 0;
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
  vtkDebugMacro("PlaceSeed: " << x << ":" << y)

  if (!interactor)
    {
    vtkErrorMacro("PlaceSeed: No renderInteractor found.")
    }

  if (!renderer)
    {
    vtkErrorMacro("PlaceSeed: No renderer found.")
    }

  if (!this->SeedWidget)
    {

    VTK_CREATE(vtkSphereHandleRepresentation, handle);
    handle->GetProperty()->SetColor(1,0,0);
    handle->SetHandleSize(5);

    VTK_CREATE(vtkSeedRepresentation, rep);
    rep->SetHandleRepresentation(handle);

    //seed widget
    vtkSeedWidget * seedWidget = vtkSeedWidget::New();
    seedWidget->SetRepresentation(rep);

    seedWidget->SetInteractor(interactor);
    seedWidget->SetCurrentRenderer(renderer);

    seedWidget->CompleteInteraction();
    seedWidget->ProcessEventsOff();

    this->SeedWidget = seedWidget;

    }

  // Seed widget exists here, just add a new handle at the position x,y

  double p[3] = {0,0,0};
  p[0]=x;
  p[1]=y;
  p[2]=0;

  //VTK_CREATE(vtkHandleWidget, newhandle);
  vtkHandleWidget * newhandle = this->SeedWidget->CreateNewHandle();
  vtkHandleRepresentation::SafeDownCast(newhandle->GetRepresentation())->SetDisplayPosition(p);

  this->HandleWidgetList.push_back(newhandle);

  this->SeedWidget->On();
  this->SeedWidget->CompleteInteraction();
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
    this->SeedWidget = 0;
    }
}

//---------------------------------------------------------------------------
vtkHandleWidget * vtkMRMLAnnotationDisplayableManagerHelper::GetSeed(int index)
{
  if (this->HandleWidgetList.empty())
    {
    return 0;
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
    return NULL;
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
  vtkDebugMacro("GetAnnotationNodeFromDisplayNode: unable to find annotation node that has display node " << (displayNode->GetID() ? displayNode->GetID() : "null"));
  return NULL;
}
