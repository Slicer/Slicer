
// MRMLLogic includes
#include "vtkMRMLApplicationLogic.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMRMLApplicationLogic, "$Revision$");
vtkStandardNewMacro(vtkMRMLApplicationLogic);

//----------------------------------------------------------------------------
class vtkMRMLApplicationLogic::vtkInternal
{
public:
  vtkInternal();

  vtkMRMLSelectionNode *    SelectionNode;
  vtkMRMLInteractionNode *  InteractionNode;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkInternal::vtkInternal()
{
  this->SelectionNode = 0;
  this->InteractionNode = 0;
}

//----------------------------------------------------------------------------
// vtkMRMLApplicationLogic methods

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkMRMLApplicationLogic()
{
  this->Internal = new vtkInternal;
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::~vtkMRMLApplicationLogic()
{
  if (this->Internal->SelectionNode)
    {
    this->Internal->SelectionNode->UnRegister(this);
    }
  if (this->Internal->InteractionNode)
    {
    this->Internal->InteractionNode->UnRegister(this);
    }
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLSelectionNode * vtkMRMLApplicationLogic::GetSelectionNode()
{
  return this->Internal->SelectionNode;
}

//----------------------------------------------------------------------------
vtkMRMLInteractionNode * vtkMRMLApplicationLogic::GetInteractionNode()
{
  return this->Internal->InteractionNode;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetMRMLSceneInternal(vtkMRMLScene *newScene)
{
  vtkMRMLSelectionNode * selectionNode = 0;
  if (newScene)
    {
    // Selection Node
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
        newScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
    if (!selectionNode)
      {
      selectionNode =
          vtkMRMLSelectionNode::SafeDownCast(newScene->AddNode(vtkMRMLSelectionNode::New()));
      assert(selectionNode);
      selectionNode->Delete();
      }
    selectionNode->Register(this);
    }
  this->Internal->SelectionNode = selectionNode;

  vtkMRMLInteractionNode * interactionNode = 0;
  if (newScene)
    {
    // Interaction Node
    interactionNode = vtkMRMLInteractionNode::SafeDownCast (
        newScene->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
    if (!interactionNode)
      {
      interactionNode =
          vtkMRMLInteractionNode::SafeDownCast(newScene->AddNode(vtkMRMLInteractionNode::New()));
      assert(interactionNode);
      interactionNode->Delete();
      }
    interactionNode->Register(this);
    }
  this->Internal->InteractionNode = interactionNode;
}

////----------------------------------------------------------------------------
//void vtkMRMLApplicationLogic::ProcessMRMLEvents(vtkObject * vtkNotUsed(caller),
//                                                unsigned long vtkNotUsed(event),
//                                                void * vtkNotUsed(callData))
//{
//  //
//  // Look for a selection node in the scene:
//  // - we always use the first one in the scene
//  // - if it doesn't match the one we had, we switch
//  // - if there isn't one, we create one
//  // - we add it to the scene if needed
//  //
//  vtkMRMLSelectionNode *node =
//      vtkMRMLSelectionNode::SafeDownCast(
//          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

//  // selection node
//  if (node == 0)
//    {
//    node = vtkMRMLSelectionNode::New();
//    this->SetSelectionNode(node);
//    node->Delete();
//    }
//  if ( this->Internal->SelectionNode != node )
//    {
//      this->SetSelectionNode(node);
//    }
//  if (this->GetMRMLScene()->GetNodeByID(this->Internal->SelectionNode->GetID()) == NULL)
//    {
//    this->SetMRMLScene(this->GetMRMLScene());
//    this->SetSelectionNode(vtkMRMLSelectionNode::SafeDownCast(
//        this->GetMRMLScene()->AddNode(this->Internal->SelectionNode)));
//    this->SetAndObserveMRMLScene(this->GetMRMLScene());
//    }


//  vtkMRMLInteractionNode *inode;
//  inode = vtkMRMLInteractionNode::SafeDownCast (
//          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));

//  // interaction node
//  if (inode == 0)
//    {
//    inode = vtkMRMLInteractionNode::New();
//    this->SetInteractionNode(inode);
//    inode->Delete();
//    }
//  if (this->Internal->InteractionNode != inode)
//    {
//    this->SetInteractionNode(inode);
//    }
//  if (this->GetMRMLScene()->GetNodeByID(this->Internal->InteractionNode->GetID()) == 0)
//    {
//    this->SetMRMLScene(this->GetMRMLScene());
//    this->SetInteractionNode(vtkMRMLInteractionNode::SafeDownCast(
//        this->GetMRMLScene()->AddNode(this->Internal->InteractionNode)));
//    this->SetAndObserveMRMLScene(this->GetMRMLScene());
//    }
//}
