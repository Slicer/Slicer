/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRMLLogic includes
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLSliceLogic.h"
#include <vtkMRMLSliceLinkLogic.h>

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkSmartPointer.h>

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
  ~vtkInternal();

  vtkMRMLSelectionNode *    SelectionNode;
  vtkMRMLInteractionNode *  InteractionNode;
  vtkSmartPointer<vtkCollection> SliceLogics;
  vtkSmartPointer<vtkMRMLSliceLinkLogic> SliceLinkLogic;
};

//----------------------------------------------------------------------------
// vtkInternal methods

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkInternal::vtkInternal()
{
  this->SelectionNode = 0;
  this->InteractionNode = 0;
  
  this->SliceLinkLogic = vtkSmartPointer<vtkMRMLSliceLinkLogic>::New();
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic::vtkInternal::~vtkInternal()
{
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
vtkMRMLSelectionNode * vtkMRMLApplicationLogic::GetSelectionNode()const
{
  return this->Internal->SelectionNode;
}

//----------------------------------------------------------------------------
vtkMRMLInteractionNode * vtkMRMLApplicationLogic::GetInteractionNode()const
{
  return this->Internal->InteractionNode;
}

//----------------------------------------------------------------------------
vtkCollection* vtkMRMLApplicationLogic::GetSliceLogics()const
{
  return this->Internal->SliceLogics;
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetSliceLogics(vtkCollection* sliceLogics)
{
  this->Internal->SliceLogics = sliceLogics;
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
    }
  this->SetSelectionNode(selectionNode);

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
    }
  this->SetInteractionNode(interactionNode);

  this->Superclass::SetMRMLSceneInternal(newScene);

  this->Internal->SliceLinkLogic->SetMRMLScene(newScene);
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetSelectionNode(vtkMRMLSelectionNode *selectionNode)
{
  if (selectionNode == this->Internal->SelectionNode)
    {
    return;
    }
  if (this->Internal->SelectionNode)
    {
    this->Internal->SelectionNode->UnRegister(this);
    }
  this->Internal->SelectionNode = selectionNode;
  if (this->Internal->SelectionNode)
    {
    this->Internal->SelectionNode->Register(this);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::SetInteractionNode(vtkMRMLInteractionNode *interactionNode)
{
  if (interactionNode == this->Internal->InteractionNode)
    {
    return;
    }
  if (this->Internal->InteractionNode)
    {
    this->Internal->InteractionNode->UnRegister(this);
    }
  this->Internal->InteractionNode = interactionNode;
  if (this->Internal->InteractionNode)
    {
    this->Internal->InteractionNode->Register(this);
    }
  this->Modified();
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

//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::PropagateVolumeSelection(int fit)
{
  if ( !this->Internal->SelectionNode || !this->GetMRMLScene() )
    {
    std::cout << this->Internal->SelectionNode << "    " << this->GetMRMLScene() << std::endl;
    return;
    }

  char *ID = this->Internal->SelectionNode->GetActiveVolumeID();
  char *secondID = this->Internal->SelectionNode->GetSecondaryVolumeID();
  char *labelID = this->Internal->SelectionNode->GetActiveLabelVolumeID();

  vtkMRMLSliceCompositeNode *cnode;
  const int nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i = 0; i < nnodes; i++)
    {
    cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
      this->GetMRMLScene()->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    if(!cnode->GetDoPropagateVolumeSelection())
      {
      continue;
      }
    cnode->SetBackgroundVolumeID( ID );
    cnode->SetForegroundVolumeID( secondID );
    cnode->SetLabelVolumeID( labelID );
    }

  if (fit) {
    this->FitSliceToAll();
  }
}


//----------------------------------------------------------------------------
void vtkMRMLApplicationLogic::FitSliceToAll()
{
  if (this->Internal->SliceLogics.GetPointer() == 0)
    {
    return;
    }
  vtkMRMLSliceLogic* sliceLogic = 0;
  vtkCollectionSimpleIterator it;
  for(this->Internal->SliceLogics->InitTraversal(it);
      (sliceLogic = vtkMRMLSliceLogic::SafeDownCast(
        this->Internal->SliceLogics->GetNextItemAsObject(it)));)
    {
    vtkMRMLSliceNode *sliceNode = sliceLogic->GetSliceNode();
    int *dims = sliceNode->GetDimensions();
    sliceLogic->FitSliceToAll(dims[0], dims[1]);
    }
}
