// MRML includes
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationHierarchyNode);

//----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode::vtkMRMLAnnotationHierarchyNode() = default;

//----------------------------------------------------------------------------
vtkMRMLAnnotationHierarchyNode::~vtkMRMLAnnotationHierarchyNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);

}

//----------------------------------------------------------------------------
const char* vtkMRMLAnnotationHierarchyNode::GetNodeTagName()
{
  return "AnnotationHierarchyNode";
}
//----------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::ReadXMLAttributes( const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::WriteXML(ostream& of, int indent)
{
  Superclass::WriteXML(of,indent);
}


//---------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::DeleteDirectChildren()
{

  vtkMRMLScene *scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("GetChildrenDisplayableNodes: scene is null, cannot find children of this node");
    return;
    }

  vtkCollection* deleteList = vtkCollection::New();

  vtkMRMLAnnotationHierarchyNode *hnode = nullptr;
  int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationHierarchyNode");
  for (int n=0; n < numNodes; n++)
    {
    hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(scene->GetNthNodeByClass(n, "vtkMRMLAnnotationHierarchyNode"));
    vtkDebugMacro("GetChildrenHierarchyNodes: hierarchy node " << n << " has id " << hnode->GetID());

    // let's check if the found hnode is a direct child of this node
    if (hnode->GetID() && this->GetID() && hnode->GetParentNodeID() &&
        strcmp(hnode->GetID(), this->GetID()) &&
        !strcmp(hnode->GetParentNodeID(),this->GetID()))
      {
      // it is a direct child

      // now let's check if it is a user-created hierarchy
      if (!hnode->GetHideFromEditors())
        {
        // move it to the parent of the hierarchy to be deleted
        hnode->SetParentNodeID(this->GetParentNodeID());

        }
      else
        {
        // it must be a 1-1 hierarchy node coming directly with an annotation
        vtkMRMLAnnotationNode* anode = vtkMRMLAnnotationNode::SafeDownCast(scene->GetNodeByID(hnode->GetDisplayableNodeID()));
        if (anode)
          {
          //this->GetScene()->RemoveNode(hnode);
          //this->GetScene()->RemoveNode(anode);
          deleteList->AddItem(hnode);
          deleteList->AddItem(anode);
          }
        } // if user-created check

      } // check if it is a direct child of this

    } // loop through all nodes

  deleteList->InitTraversal();

  for (int j =0; j<deleteList->GetNumberOfItems();++j)
    {

    vtkMRMLNode* dNode = vtkMRMLNode::SafeDownCast(deleteList->GetItemAsObject(j));

    if (dNode)
      {
      this->GetScene()->RemoveNode(dNode);
      }

    }
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::GetDirectChildren(vtkCollection *children)
{
  this->GetChildren(children, 1);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::GetAllChildren(vtkCollection *children)
{
  this->GetChildren(children, -1);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::GetChildren(vtkCollection *children, int level)
{
  if (children == nullptr)
    {
    return;
    }

  if (level-- == 0)
    {
    return;
    }

  vtkMRMLScene *scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("GetChildrenDisplayableNodes: scene is null, cannot find children of this node");
    return;
    }

  vtkMRMLAnnotationHierarchyNode *hnode = nullptr;
  int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationHierarchyNode");
  for (int n=0; n < numNodes; n++)
    {
    hnode = vtkMRMLAnnotationHierarchyNode::SafeDownCast(scene->GetNthNodeByClass(n, "vtkMRMLAnnotationHierarchyNode"));
    vtkDebugMacro("GetChildrenHierarchyNodes: hierarchy node " << n << " has id " << hnode->GetID());

    // let's check if the found hnode is a direct child of this node
    if (hnode->GetID() && this->GetID() && hnode->GetParentNodeID() &&
        strcmp(hnode->GetID(), this->GetID()) &&
        !strcmp(hnode->GetParentNodeID(),this->GetID()))
      {
      // it is a direct child

      // now let's check if it is a user-created hierarchy
      if (!hnode->GetHideFromEditors())
        {
        // yes it is
        children->AddItem(hnode);
        }
      else
        {
        // it must be a 1-1 hierarchy node coming directly with an annotation
        vtkMRMLAnnotationNode* anode = vtkMRMLAnnotationNode::SafeDownCast(scene->GetNodeByID(hnode->GetDisplayableNodeID()));
        if (anode)
          {
          children->AddItem(anode);
          }
        } // if user-created check
      hnode->GetChildren(children, level);
      } // check if it is a direct child of this

    } // loop through all nodes
}

//-------------------------------------------------------------------------
bool vtkMRMLAnnotationHierarchyNode::CanApplyNonLinearTransforms()const
{
  return true;
}

//-------------------------------------------------------------------------
void vtkMRMLAnnotationHierarchyNode::ApplyTransform(vtkAbstractTransform* vtkNotUsed(transform))
{
  // iterate over children?
}
