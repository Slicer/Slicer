
// MRMLLogic includes
#include "vtkMRMLApplicationLogic.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>

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
  this->SetSelectionNode(0);
  this->SetInteractionNode(0);
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
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->SelectionNode address "
                << this->Internal->SelectionNode);
  return this->Internal->SelectionNode;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetSelectionNode(vtkMRMLSelectionNode* newSelectionNode)
{
  vtkSetObjectBodyMacro(Internal->SelectionNode, vtkMRMLSelectionNode, newSelectionNode);
}

//----------------------------------------------------------------------------
vtkMRMLInteractionNode * vtkMRMLApplicationLogic::GetInteractionNode()
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): "
                << "returning Internal->InteractionNode address "
                << this->Internal->InteractionNode);
  return this->Internal->InteractionNode;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetInteractionNode(vtkMRMLInteractionNode* newInteractionNode)
{
  vtkSetObjectBodyMacro(Internal->InteractionNode, vtkMRMLInteractionNode, newInteractionNode);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::ProcessMRMLEvents(vtkObject * vtkNotUsed(caller),
                                                unsigned long vtkNotUsed(event),
                                                void * vtkNotUsed(callData))
{
  //
  // Look for a selection node in the scene:
  // - we always use the first one in the scene
  // - if it doesn't match the one we had, we switch
  // - if there isn't one, we create one
  // - we add it to the scene if needed
  //
  vtkMRMLSelectionNode *node =
      vtkMRMLSelectionNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

  // selection node
  if (node == 0)
    {
    node = vtkMRMLSelectionNode::New();
    this->SetSelectionNode(node);
    node->Delete();
    }
  if ( this->Internal->SelectionNode != node )
    {
      this->SetSelectionNode(node);
    }
  if (this->GetMRMLScene()->GetNodeByID(this->Internal->SelectionNode->GetID()) == NULL)
    {
    this->SetMRMLScene(this->GetMRMLScene());
    this->SetSelectionNode(vtkMRMLSelectionNode::SafeDownCast(
        this->GetMRMLScene()->AddNode(this->Internal->SelectionNode)));
    this->SetAndObserveMRMLScene(this->GetMRMLScene());
    }


  vtkMRMLInteractionNode *inode;
  inode = vtkMRMLInteractionNode::SafeDownCast (
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));

  // interaction node
  if (inode == 0)
    {
    inode = vtkMRMLInteractionNode::New();
    this->SetInteractionNode(inode);
    inode->Delete();
    }
  if (this->Internal->InteractionNode != inode)
    {
    this->SetInteractionNode(inode);
    }
  if (this->GetMRMLScene()->GetNodeByID(this->Internal->InteractionNode->GetID()) == 0)
    {
    this->SetMRMLScene(this->GetMRMLScene());
    this->SetInteractionNode(vtkMRMLInteractionNode::SafeDownCast(
        this->GetMRMLScene()->AddNode(this->Internal->InteractionNode)));
    this->SetAndObserveMRMLScene(this->GetMRMLScene());
    }
}
