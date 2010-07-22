// AnnotationModule includes
#include "vtkMRMLAnnotationTextDisplayableManager.h"

// AnnotationModule/MRML includes
#include "vtkMRMLAnnotationTextNode.h"

// MRML includes
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkTextRepresentation.h>
#include <vtkTextWidget.h>
#include <vtkLineWidget2.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkLineRepresentation.h>
#include <vtkPolygonalSurfacePointPlacer.h>
#include <vtkMath.h>

// STD includes
#include <vector>
#include <map>
#include <algorithm>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLAnnotationTextDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLAnnotationTextDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
class vtkMRMLAnnotationTextDisplayableManager::vtkInternal
{
public:
  vtkInternal();

  void UpdateLockUnlock(vtkMRMLAnnotationTextNode* textNode);
  void UpdateWidget(vtkMRMLAnnotationTextNode *textNode);

  /// Get a vtkTextWidget* given \a textNode
  vtkTextWidget * GetTextWidget(vtkMRMLAnnotationTextNode * textNode);
  void RemoveTextWidget(vtkMRMLAnnotationTextNode *node);

  /// List of Nodes managed by the DisplayableManager
  std::vector<vtkMRMLAnnotationTextNode*> AnnotationTextNodeList;

  /// .. and its associated convenient typedef
  typedef std::vector<vtkMRMLAnnotationTextNode*>::iterator AnnotationTextNodeListIt;

  /// Map of vtkTextWidget indexed using associated node ID
  std::map<vtkMRMLAnnotationTextNode*, vtkTextWidget *> TextWidgets;

  /// .. and its associated convenient typedef
  typedef std::map<vtkMRMLAnnotationTextNode*, vtkTextWidget *>::iterator TextWidgetsIt;

};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayableManager::vtkInternal::vtkInternal()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::vtkInternal::UpdateLockUnlock(
    vtkMRMLAnnotationTextNode* textNode)
{
  // Sanity checks
  if (textNode == 0)
    {
    return;
    }

  vtkTextWidget * textWidget = this->GetTextWidget(textNode);
  // A text widget is expected
  if(textWidget == 0)
    {
    return;
    }

  if (textNode->GetLocked())
    {
    textWidget->ProcessEventsOff();
    }
  else
    {
    textWidget->ProcessEventsOn();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::vtkInternal::UpdateWidget(
    vtkMRMLAnnotationTextNode *textNode)
{

  vtkTextWidget * textWidget = this->GetTextWidget(textNode);
  // textWidget is expected to be valid
  if (textNode == 0)
    {
    return;
    }

  // Obtain associated representation
  vtkTextRepresentation* textRepr =
      vtkTextRepresentation::SafeDownCast(textWidget->GetRepresentation());

  // Update Text
  textRepr->SetText(textNode->GetText(0));

  // Update visibility
  if (textNode->GetVisible())
    {
    textWidget->EnabledOn();
    }
  else
    {
    textWidget->EnabledOff();
    }
}


//---------------------------------------------------------------------------
vtkTextWidget * vtkMRMLAnnotationTextDisplayableManager::vtkInternal::GetTextWidget(
    vtkMRMLAnnotationTextNode * textNode)
{
  if (!textNode)
    {
    return 0;
    }

  // Make sure the map contains a vtkWidgetText associated with this textNode
  TextWidgetsIt it = this->TextWidgets.find(textNode);
  if (it == this->TextWidgets.end())
    {
    return 0;
    }

  return it->second;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::vtkInternal::RemoveTextWidget(
    vtkMRMLAnnotationTextNode *textNode)
{
  if (!textNode)
  {
    return;
  }

  // Make sure the map contains a vtkWidgetText associated with this textNode
  TextWidgetsIt it = this->TextWidgets.find(textNode);
  if (it == this->TextWidgets.end()) {
    return;
  }

  // Delete and Remove textWidget from the map
  this->TextWidgets[textNode]->Delete();
  this->TextWidgets.erase(textNode);

  vtkInternal::AnnotationTextNodeListIt it2 = std::find(
      this->AnnotationTextNodeList.begin(),
      this->AnnotationTextNodeList.end(),
      textNode);


  this->AnnotationTextNodeList.erase(it2);
}

//---------------------------------------------------------------------------
// vtkMRMLAnnotationTextDisplayableManager methods

//---------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayableManager::vtkMRMLAnnotationTextDisplayableManager()
{
  this->Internal = new vtkInternal;
}

//---------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayableManager::~vtkMRMLAnnotationTextDisplayableManager()
{
  delete this->Internal;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::SetAndObserveTextNodes()
{
  VTK_CREATE(vtkIntArray, textNodeEvents);
  textNodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  textNodeEvents->InsertNextValue(vtkMRMLAnnotationNode::LockModifiedEvent);
  textNodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);


  // run through all associated textNodes
  vtkInternal::AnnotationTextNodeListIt it;
  it = Internal->AnnotationTextNodeList.begin();
  while(it != Internal->AnnotationTextNodeList.end())
    {
    vtkSetAndObserveMRMLNodeEventsMacro(*it, *it, textNodeEvents);
    ++it;
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::Create()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::ProcessMRMLEvents(vtkObject *caller,
                                                                unsigned long event,
                                                                void *callData)
{
  vtkMRMLAnnotationTextNode * annotationTextNode = vtkMRMLAnnotationTextNode::SafeDownCast(caller);
  if (annotationTextNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLAnnotationTextNodeModifiedEvent(annotationTextNode);
        break;
      case vtkMRMLTransformableNode::TransformModifiedEvent:
        this->OnMRMLAnnotationTextNodeTransformModifiedEvent(annotationTextNode);
        break;
      case vtkMRMLAnnotationNode::LockModifiedEvent:
        this->OnMRMLAnnotationTextNodeLockModifiedEvent(annotationTextNode);
        break;
      }
    }
  else
    {
    this->Superclass::ProcessMRMLEvents(caller, event, callData);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLSceneAboutToBeClosedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLSceneClosedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLSceneAboutToBeImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLSceneImportedEvent()
{
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node)
{
  vtkMRMLAnnotationTextNode * textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);
  if (!textNode)
    {
    return;
    }

  // Node added should not be already managed
  vtkInternal::AnnotationTextNodeListIt it = std::find(
      this->Internal->AnnotationTextNodeList.begin(),
      this->Internal->AnnotationTextNodeList.end(),
      textNode);
  if (it != this->Internal->AnnotationTextNodeList.end())
    {
      vtkErrorMacro("OnMRMLSceneNodeAddedEvent - This node is already associated to the displayable manager!")
      return;
    }

  // There should not be a widget for the new node
  if (this->Internal->GetTextWidget(textNode) != 0)
    {
    vtkErrorMacro("OnMRMLSceneNodeAddedEvent - A widget is already associated to this node!");
    return;
    }

  // Create VTK Widget
  vtkTextWidget* textWidget = vtkTextWidget::New();
  this->Internal->TextWidgets[textNode] = textWidget;

  // Associate TextRepresentation
  VTK_CREATE(vtkTextRepresentation, textRep);
  textRep->SetMoving(1);
  textRep->SetText(textNode->GetText(0));
  textWidget->SetRepresentation(textRep);
  textWidget->SetInteractor(this->GetInteractor());
  textWidget->On();

  this->Internal->AnnotationTextNodeList.push_back(textNode);

  // Refresh observers
  this->SetAndObserveTextNodes();

  this->RequestRender();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node)
{
  vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);
  if (!textNode)
    {
    return;
    }
/*
  vtkInternal::AnnotationTextNodeListIt it = std::find(
      this->Internal->AnnotationTextNodeList.begin(),
      this->Internal->AnnotationTextNodeList.end(),
      textNode);

  if (it == this->Internal->AnnotationTextNodeList.end())
    {
    return;
    }

  this->Internal->AnnotationTextNodeList.erase(it);
*/

  this->Internal->RemoveTextWidget(textNode);

  // Refresh observers
  this->SetAndObserveTextNodes();

}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLAnnotationTextNodeModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationTextNodeModifiedEvent");
  vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);
  if (!textNode)
    {
    vtkErrorMacro("OnMRMLAnnotationTextNodeModifiedEvent - Can not access node.")
    return;
    }
  this->Internal->UpdateWidget(textNode);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLAnnotationTextNodeTransformModifiedEvent(vtkMRMLNode* node)
{
  vtkDebugMacro("OnMRMLAnnotationTextNodeTransformModifiedEvent");
  vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);
  if (!textNode)
    {
    vtkErrorMacro("OnMRMLAnnotationTextNodeTransformModifiedEvent - Can not access node.")
    return;
    }
  this->Internal->UpdateWidget(textNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLAnnotationTextNodeLockModifiedEvent(vtkMRMLNode* node)
{

  vtkMRMLAnnotationTextNode *textNode = vtkMRMLAnnotationTextNode::SafeDownCast(node);
  if (!textNode)
    {
    vtkErrorMacro("OnMRMLAnnotationTextNodeLockModifiedEvent - Can not access node.")
    return;
    }
  std::cout << "OnMRMLAnnotationTextNodeLockModifiedEvent (Id: " << textNode->GetID() << ")" << std::endl;
  this->Internal->UpdateLockUnlock(textNode);
}
