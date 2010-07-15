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
#include <algorithm>
#include <cassert>

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

  ///
  vtkMRMLAnnotationTextNode* ActiveTextNode;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayableManager::vtkInternal::vtkInternal()
{
  this->ActiveTextNode = 0;
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
  assert(textWidget);

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
  assert(textNode);

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

#ifndef NDEBUG
  // Make sure the map contains a vtkWidgetText associated with this textNode
  TextWidgetsIt it = this->TextWidgets.find(textNode);
  assert(it != this->TextWidgets.end());
#endif

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
void vtkMRMLAnnotationTextDisplayableManager::SetAndObserveActiveTextNode(
    vtkMRMLAnnotationTextNode * textNode)
{
  VTK_CREATE(vtkIntArray, textNodeEvents);
  textNodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  textNodeEvents->InsertNextValue(vtkMRMLAnnotationNode::LockModifiedEvent);
  textNodeEvents->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);

  vtkSetAndObserveMRMLNodeEventsMacro(Internal->ActiveTextNode, textNode, textNodeEvents);
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
  std::cout << "vtkMRMLAnnotationTextDisplayableManager::ProcessMRMLEvents" <<std::endl;

  vtkMRMLAnnotationTextNode * annotationTextNode = vtkMRMLAnnotationTextNode::SafeDownCast(caller);
  if (annotationTextNode)
    {
    switch(event)
      {
      case vtkCommand::ModifiedEvent:
        this->OnMRMLAnnotationTextNodeModifiedEvent();
        break;
      case vtkMRMLTransformableNode::TransformModifiedEvent:
        this->OnMRMLAnnotationTextNodeTransformModifiedEvent();
        break;
      case vtkMRMLAnnotationNode::LockModifiedEvent:
        this->OnMRMLAnnotationTextNodeLockModifiedEvent();
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

#ifndef NDEBUG
  // Node added shouldn't be already managed
  vtkInternal::AnnotationTextNodeListIt it = std::find(
      this->Internal->AnnotationTextNodeList.begin(),
      this->Internal->AnnotationTextNodeList.end(),
      textNode);
  assert(it == this->Internal->AnnotationTextNodeList.end());
#endif

  // Create VTK Widget
  assert(this->Internal->GetTextWidget(textNode) == 0);
  vtkTextWidget* textWidget = vtkTextWidget::New();
  this->Internal->TextWidgets[textNode] = textWidget;

  // Associate TextRepresentation
  VTK_CREATE(vtkTextRepresentation, textRep);
  textRep->SetMoving(1);
  textRep->SetText(textNode->GetText(0));
  textWidget->SetRepresentation(textRep);
  textWidget->SetInteractor(this->GetInteractor());
  textWidget->On();

  // Add observers
  this->SetAndObserveActiveTextNode(textNode);

  this->Internal->AnnotationTextNodeList.push_back(textNode);

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

  vtkInternal::AnnotationTextNodeListIt it = std::find(
      this->Internal->AnnotationTextNodeList.begin(),
      this->Internal->AnnotationTextNodeList.end(),
      textNode);

  if (it == this->Internal->AnnotationTextNodeList.end())
    {
    return;
    }

  this->Internal->AnnotationTextNodeList.erase(it);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLAnnotationTextNodeModifiedEvent()
{
  std::cout << "OnMRMLAnnotationTextNodeModifiedEvent" << std::endl;
  assert(this->Internal->ActiveTextNode);
  this->Internal->UpdateWidget(this->Internal->ActiveTextNode);
  this->RequestRender();
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLAnnotationTextNodeTransformModifiedEvent()
{
  std::cout << "OnMRMLAnnotationTextNodeTransformModifiedEvent" << std::endl;
  assert(this->Internal->ActiveTextNode);
  this->Internal->UpdateWidget(this->Internal->ActiveTextNode);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationTextDisplayableManager::OnMRMLAnnotationTextNodeLockModifiedEvent()
{
  std::cout << "OnMRMLAnnotationTextNodeLockModifiedEvent" << std::endl;
  assert(this->Internal->ActiveTextNode);
  this->Internal->UpdateLockUnlock(this->Internal->ActiveTextNode);
}
