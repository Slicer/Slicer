/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MarkupsModule/MRML includes
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>

// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsDisplayableManagerHelper.h"

// VTK includes
#include <vtkAbstractWidget.h>
#include <vtkCollection.h>
#include <vtkHandleWidget.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkPickingManager.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkSphereHandleRepresentation.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>

// STD includes
#include <algorithm>
#include <map>
#include <vector>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLMarkupsDisplayableManagerHelper);

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MarkupsNodeList:" << std::endl;

  for (unsigned int nodesIt = 0;
       nodesIt < this->MarkupsNodeList.size();
       ++nodesIt)
    {
    os << indent.GetNextIndent() << this->MarkupsNodeList[nodesIt]->GetID() << std::endl;
    }

  os << indent << "Widgets map:" << std::endl;
  WidgetsIt widgetIterator = this->Widgets.begin();
  for (widgetIterator =  this->Widgets.begin();
       widgetIterator != this->Widgets.end();
       ++widgetIterator)
    {
    os << indent.GetNextIndent() << widgetIterator->first->GetID() << " : widget is " << (widgetIterator->second ? "not null" : "null") << std::endl;
    if (widgetIterator->second &&
        widgetIterator->second->GetRepresentation())
      {
      vtkSeedRepresentation * seedRepresentation = vtkSeedRepresentation::SafeDownCast(widgetIterator->second->GetRepresentation());
      int numberOfSeeds = 0;
      if (seedRepresentation)
        {
        numberOfSeeds = seedRepresentation->GetNumberOfSeeds();
        }
      else
        {
        vtkWarningMacro("PrintSelf: no seed representation for widget assoc with markups node " << widgetIterator->first->GetID());
        }
      os << indent.GetNextIndent().GetNextIndent() << "enabled = "
        << widgetIterator->second->GetEnabled()
        << ", number of seeds = " << numberOfSeeds << std::endl;
      }
    }

  os << indent << "Widget Intersections:" << std::endl;
  for (WidgetIntersectionsIt intersectionsIt = this->WidgetIntersections.begin();
       intersectionsIt != this->WidgetIntersections.end();
       ++intersectionsIt)
    {
    os << indent.GetNextIndent() << intersectionsIt->first->GetID() << " : intersection is " << (intersectionsIt->second ? "not null" : "null") << std::endl;
    }

  os << indent << "Widget projections:" << std::endl;
  for (WidgetPointProjectionsIt it = this->WidgetPointProjections.begin();
       it != this->WidgetPointProjections.end();
       ++it)
    {
    os << indent.GetNextIndent() << it->first.c_str() << " : projection is "
       << (it->second ? "not null" : "null") << std::endl;
    }
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManagerHelper::vtkMRMLMarkupsDisplayableManagerHelper()
{
  this->SeedWidget = 0;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsDisplayableManagerHelper::~vtkMRMLMarkupsDisplayableManagerHelper()
{
  if(this->SeedWidget)
    {
    this->RemoveSeeds();
    }
  this->RemoveAllWidgetsAndNodes();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::UpdateLockedAllWidgetsFromNodes()
{
  // iterate through the node list
  for (unsigned int i = 0; i < this->MarkupsNodeList.size(); i++)
    {
    vtkMRMLMarkupsNode *markupsNode = this->MarkupsNodeList[i];
    this->UpdateLocked(markupsNode);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::UpdateLockedAllWidgetsFromInteractionNode(vtkMRMLInteractionNode *interactionNode)
{
  if (!interactionNode)
    {
    return;
    }

  int currentInteractionMode = interactionNode->GetCurrentInteractionMode();
  vtkDebugMacro("Markups DisplayableManager Helper: updateLockedAllWidgetsFromInteractionNode, currentInteractionMode = " << currentInteractionMode);
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
void vtkMRMLMarkupsDisplayableManagerHelper::UpdateLockedAllWidgets(bool locked)
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
void vtkMRMLMarkupsDisplayableManagerHelper::UpdateLocked(vtkMRMLMarkupsNode *node,
                                                          vtkMRMLInteractionNode *interactionNode)
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

  bool isLockedOnNode = (node->GetLocked() != 0 ? true : false);
  bool isLockedOnWidget = (widget->GetProcessEvents() != 0 ? false : true);
  bool isLockedOnInteraction = false;
  if (interactionNode &&
      interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
    {
    // place mode will over ride node locking
    isLockedOnInteraction = true;
    }

  vtkDebugMacro("UpdateLocked: isLockedOnNode = " << isLockedOnNode
            << ", isLockedOnWidget = " << isLockedOnWidget
            << ", isLockedOnInteraction = " << isLockedOnInteraction);
  // only update the processEvents state of the widget if it is different than
  // what the markups node or the interaction node says it needs to be
  if ( (isLockedOnNode && !isLockedOnWidget) ||
       (isLockedOnInteraction && !isLockedOnWidget) )
    {
    widget->ProcessEventsOff();
    }
  else if (!isLockedOnInteraction && !isLockedOnNode && isLockedOnWidget)
    {
    widget->ProcessEventsOn();
    // is it a seed widget that can support individually locked seeds?
    vtkSeedWidget *seedWidget = vtkSeedWidget::SafeDownCast(widget);
    if (seedWidget)
      {
      vtkDebugMacro("UpdateLocked: have a seed widget, list unlocked, checking seeds");
      int numMarkups = node->GetNumberOfMarkups();
      for (int i = 0; i < numMarkups; i++)
        {
        if (seedWidget->GetSeed(i) == NULL)
          {
          vtkErrorMacro("UpdateLocked: missing seed at index " << i);
          continue;
          }
        bool isLockedOnNthMarkup = node->GetNthMarkupLocked(i);
        bool isLockedOnNthSeed = seedWidget->GetSeed(i)->GetProcessEvents() == 0;
        if (isLockedOnNthMarkup && !isLockedOnNthSeed)
          {
          // lock it
          seedWidget->GetSeed(i)->ProcessEventsOff();
          }
        else if (!isLockedOnNthMarkup && isLockedOnNthSeed)
          {
          // unlock it
          seedWidget->GetSeed(i)->ProcessEventsOn();
          }
        }
      }
    }
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManagerHelper::GetWidget(vtkMRMLMarkupsNode * node)
{
  if (!node)
    {
    return 0;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetsIt it = this->Widgets.find(node);
  if (it == this->Widgets.end())
    {
    // std::cout << "GetWidget: no widget for node " << node->GetID() << std::endl;
    return 0;
    }

  return it->second;
}

//---------------------------------------------------------------------------
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManagerHelper::GetIntersectionWidget(
    vtkMRMLMarkupsNode * node)
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
vtkAbstractWidget * vtkMRMLMarkupsDisplayableManagerHelper::GetPointProjectionWidget(std::string uniqueFiducialID)
{
  if (!uniqueFiducialID.compare(""))
    {
    return 0;
    }

  // Make sure the map contains a vtkWidget associated with this uniqueFiducialID
  WidgetPointProjectionsIt it = this->WidgetPointProjections.find(uniqueFiducialID);
  if (it == this->WidgetPointProjections.end())
    {
    return 0;
    }

  return it->second;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::RemoveAllWidgetsAndNodes()
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

  WidgetPointProjectionsIt pointProjIt;
  for (pointProjIt = this->WidgetPointProjections.begin();
       pointProjIt != this->WidgetPointProjections.end();
       ++pointProjIt)
    {
    pointProjIt->second->Off();
    pointProjIt->second->Delete();
    }
  this->WidgetPointProjections.clear();

  this->MarkupsNodeList.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::RemoveWidgetAndNode(
    vtkMRMLMarkupsNode *node)
{
  if (!node)
    {
    return;
    }

  // Make sure the map contains a vtkWidget associated with this node
  WidgetsIt widgetIterator = this->Widgets.find(node);
  if (widgetIterator != this->Widgets.end())
    {
    // Delete and Remove vtkWidget from the map
    if (this->Widgets[node])
      {
      this->Widgets[node]->Off();
      this->Widgets[node]->Delete();
      }
    this->Widgets.erase(node);
    }

  WidgetIntersectionsIt widgetIntersectionIterator = this->WidgetIntersections.find(node);
  if (widgetIntersectionIterator != this->WidgetIntersections.end())
    {
    // we have a vtkAbstractWidget to represent the slice intersections for this node
    // now delete it!
    if (this->WidgetIntersections[node])
      {
      this->WidgetIntersections[node]->Off();
      this->WidgetIntersections[node]->Delete();
      }
    this->WidgetIntersections.erase(node);
    }

  // go through the list and remove the projection points for it
  // this can get called after a markup has been removed from the list,
  // so turn it around and iterate through all the markups in all the lists,
  // then delete the widget point projections for those that don't match up to
  // points in the scene
  vtkCollection *col = NULL;
  if (node->GetScene() != NULL)
    {
    col = node->GetScene()->GetNodesByClass("vtkMRMLMarkupsNode");
    }
  else
    {
    // without a scene to get the other nodes from, default to just using this node
    col = vtkCollection::New();
    col->AddItem(node);
    }

 unsigned int numberOfMarkupsNodes = col->GetNumberOfItems();
 // use a map so can use find, it's markup id: list node id
 std::map<std::string, std::string> currentMarkupIDs;
 for (unsigned int i = 0; i < numberOfMarkupsNodes; ++i)
   {
   vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(col->GetItemAsObject(i));
   if (!markupsNode)
     {
     continue;
     }
   for (int n = 0; n < markupsNode->GetNumberOfMarkups(); ++n)
    {
    std::string pointID = markupsNode->GetNthMarkupID(n);
    currentMarkupIDs[pointID] = std::string(markupsNode->GetID());
    }
   }
 col->RemoveAllItems();
 col->Delete();

 WidgetPointProjectionsIt widgetPointProjectionIterator = this->WidgetPointProjections.begin();
 // build up a vector of points to erase from the point projection map
 std::vector<std::string> projectionsToErase;
 for (; widgetPointProjectionIterator != this->WidgetPointProjections.end(); widgetPointProjectionIterator++)
   {
   // does this point projection correspond to a valid markup id?
   std::string pointID = widgetPointProjectionIterator->first;
   std::map<std::string,std::string>::iterator it;
   it = currentMarkupIDs.find(pointID);
   if (it == currentMarkupIDs.end())
      {
      // add it to a list of point to erase
      projectionsToErase.push_back(pointID);
      }
   }
 for (unsigned int p = 0; p < projectionsToErase.size(); ++p)
   {
   std::string pointID = projectionsToErase[p];
   if (this->WidgetPointProjections[pointID])
     {
     this->WidgetPointProjections[pointID]->Off();
     this->WidgetPointProjections[pointID]->Delete();
     }
   this->WidgetPointProjections.erase(pointID);
   }

  vtkMRMLMarkupsDisplayableManagerHelper::MarkupsNodeListIt nodeIterator = std::find(
      this->MarkupsNodeList.begin(),
      this->MarkupsNodeList.end(),
      node);

  // Make sure the map contains the markupsNode
  if (nodeIterator != this->MarkupsNodeList.end())
    {
    //vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(*nodeIterator);
    //if (markupsNode)
     // {
      //markupsNode->Delete();
     // }
    this->MarkupsNodeList.erase(nodeIterator);
    }

  // remove the entry for the node glyph types
  this->RemoveNodeGlyphType(node->GetDisplayNode());

}

//---------------------------------------------------------------------------
// Seeds for widget placement
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::PlaceSeed(double x, double y, vtkRenderWindowInteractor * interactor, vtkRenderer * renderer)
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

    vtkNew<vtkSphereHandleRepresentation> handle;
    handle->GetProperty()->SetColor(1,0,0);
    handle->SetHandleSize(5);

    vtkNew<vtkSeedRepresentation> rep;
    rep->SetHandleRepresentation(handle.GetPointer());

    //seed widget
    vtkSeedWidget * seedWidget = vtkSeedWidget::New();
    seedWidget->SetRepresentation(rep.GetPointer());

    if (interactor->GetPickingManager())
      {
      if (!(interactor->GetPickingManager()->GetEnabled()))
        {
        // if the picking manager is not already turned on for this
        // interactor, enable it
        interactor->GetPickingManager()->EnabledOn();
        }
      }

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

  this->HandleWidgetList.push_back(newhandle);

  this->SeedWidget->On();
  this->SeedWidget->CompleteInteraction();
  this->SeedWidget->ManagesCursorOff();
  this->SeedWidget->ProcessEventsOff();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::RemoveSeeds()
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
vtkHandleWidget * vtkMRMLMarkupsDisplayableManagerHelper::GetSeed(int index)
{
  if (this->HandleWidgetList.empty())
    {
    return 0;
    }

  return this->HandleWidgetList[index];
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsNode * vtkMRMLMarkupsDisplayableManagerHelper::GetMarkupsNodeFromDisplayNode(vtkMRMLMarkupsDisplayNode *displayNode)
{
  if (!displayNode ||
      !displayNode->GetID())
    {
    vtkErrorMacro("GetMarkupsNodeFromDisplayNode: display node or it's id is null");
    return NULL;
    }
  // iterate through the node list
  for (unsigned int i = 0; i < this->MarkupsNodeList.size(); i++)
    {
    vtkMRMLMarkupsNode *markupsNode = this->MarkupsNodeList[i];
    int numNodes = markupsNode->GetNumberOfDisplayNodes();
    for (int n = 0; n < numNodes; n++)
      {
      vtkMRMLDisplayNode *thisDisplayNode = markupsNode->GetNthDisplayNode(n);
      if (thisDisplayNode && thisDisplayNode->GetID() &&
          displayNode->GetID())
        {
        if (strcmp(thisDisplayNode->GetID(),displayNode->GetID()) == 0)
          {
          return markupsNode;
          }
        }
      }
    }
  vtkDebugMacro("GetMarkupsNodeFromDisplayNode: unable to find markups node that has display node " << (displayNode->GetID() ? displayNode->GetID() : "null"));
  return NULL;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::RecordWidgetForNode(vtkAbstractWidget* widget, vtkMRMLMarkupsNode *node)
{
  if (!widget)
    {
    vtkErrorMacro("RecordWidgetForNode: no widget!");
    return;
    }
  if (!node)
    {
    vtkErrorMacro("RecordWidgetForNode: no node!");
    return;
    }
  // save the widget to the map indexed by the node
  this->Widgets[node] = widget;
  // save the node
  this->MarkupsNodeList.push_back(node);
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsDisplayableManagerHelper::GetNodeGlyphType(vtkMRMLNode *displayNode, int index)
{
  std::map<vtkMRMLNode*, std::vector<int> >::iterator iter  = this->NodeGlyphTypes.find(displayNode);
  if (iter == this->NodeGlyphTypes.end())
    {
    // no record for this node
    return -1;
    }
  if (iter->second.size() - 1 < (unsigned int)index)
    {
    // no entry for this index
    return -1;
    }
  return iter->second[index];
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::SetNodeGlyphType(vtkMRMLNode *displayNode, int glyphType, int index)
{
  if (!displayNode)
    {
    return;
    }
  // is there already an entry for this node?
  std::map<vtkMRMLNode*, std::vector<int> >::iterator iter  = this->NodeGlyphTypes.find(displayNode);
  if (iter == this->NodeGlyphTypes.end())
    {
    // no? add one
    std::vector<int> newEntry;
    newEntry.resize(index + 1);
    newEntry.at(index) = glyphType;
    this->NodeGlyphTypes[displayNode] = newEntry;
    return;
    }

  // is there already an entry at this index?
  if (iter->second.size() <= (unsigned int)index)
    {
    // no? add space
    this->NodeGlyphTypes[displayNode].resize(index + 1);
    }
  // set it
  this->NodeGlyphTypes[displayNode].at(index) = glyphType;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::RemoveNodeGlyphType(vtkMRMLNode *displayNode)
{
  if (!displayNode)
    {
    return;
    }
  // is there an entry for this node?
  std::map<vtkMRMLNode*, std::vector<int> >::iterator iter  = this->NodeGlyphTypes.find(displayNode);
  if (iter == this->NodeGlyphTypes.end())
    {
    return;
    }
  // erase it
  this->NodeGlyphTypes.erase(iter);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::ClearNodeGlyphTypes()
{
  this->NodeGlyphTypes.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsDisplayableManagerHelper::PrintNodeGlyphTypes()
{
  std::cout << "Node Glyph Types:" << std::endl;
  for (std::map<vtkMRMLNode*, std::vector<int> >::iterator iter  = this->NodeGlyphTypes.begin();
       iter !=  this->NodeGlyphTypes.end();
       iter++)
    {
    std::cout << "\tDisplay node " << iter->first->GetID() << ": " << iter->first->GetName() << std::endl;
    for (unsigned int i = 0; i < iter->second.size(); i++)
      {
      std::cout << "\t\t" << i << " glyph type = " << iter->second[i] << std::endl;
      }
    }
}
