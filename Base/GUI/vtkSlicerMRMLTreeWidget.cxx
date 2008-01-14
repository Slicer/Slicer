#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerMRMLTreeWidget.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWTree.h"
#include "vtkKWMenuButton.h"
#include "vtkKWLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWLabelWithLabel.h"

#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkKWTreeWithScrollbars.h"


#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMatrix4x4.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMRMLTreeWidget );
vtkCxxRevisionMacro ( vtkSlicerMRMLTreeWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerMRMLTreeWidget::vtkSlicerMRMLTreeWidget ( )
{
  this->TreeWidget = NULL;
  this->ContextMenu = NULL;
  this->NodeID = NULL;
  this->NodeName = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerMRMLTreeWidget::~vtkSlicerMRMLTreeWidget ( )
{
  if (this->TreeWidget)
    {
    this->TreeWidget->SetParent(NULL);
    this->TreeWidget->Delete();
    this->TreeWidget = NULL;
    }

  if (this->ContextMenu)
    {
    this->ContextMenu->SetParent(NULL);
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }

  if (this->NodeID)
    {
    this->NodeID->Delete();
    this->NodeID = NULL;
    }

  if (this->NodeName)
    {
    this->NodeName->Delete();
    this->NodeName = NULL;
    }
  this->ClearCutNodes();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerMRMLTreeWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
  vtkKWTree *tree = this->TreeWidget->GetWidget();
  vtkKWEntry *entry = this->NodeName->GetWidget();
  if (caller == tree)
    {
    // Selection changed

    vtksys_stl::string selected(
    this->TreeWidget->GetWidget()->GetSelection());

    //std::cout << "Selected = " << selected << "\n";

    if (event == vtkKWTree::SelectionChangedEvent)
      {
      // For example, one could populate the node inspector
      this->SetSelectesLeaves();
      this->UpdateNodeInspector(this->GetSelectedNodeInTree());
      }

    // Right click: context menu

    else if (event == vtkKWTree::RightClickOnNodeEvent)
      {
      // This code above should be in something like TriggerContextMenu

      if (!this->ContextMenu)
        {
        this->ContextMenu = vtkKWMenu::New();
        }
      if (!this->ContextMenu->IsCreated())
        {
        this->ContextMenu->SetParent(this);
        this->ContextMenu->Create();
        }
      this->ContextMenu->DeleteAllItems();

      int px, py;
      vtkKWTkUtilities::GetMousePointerCoordinates(tree, &px, &py);

      char command[125];
      
      vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID((const char *)callData);

      if (!this->IsLeafSelected((const char *)callData))
        {
        this->TreeWidget->GetWidget()->ClearSelection();
        this->TreeWidget->GetWidget()->SelectNode((const char *)callData);
        this->SelectedLeaves.clear();
        this->SelectedLeaves.push_back((const char *)callData);
        }

      if (this->SelectedLeaves.size() > 1)
        {
        // multiple nodes selected
        sprintf(command, "CutNodeCallback {%s}", (const char *)callData);
        this->ContextMenu->AddCommand("Cut Node", this, command);

        sprintf(command, "DeleteNodeCallback {%s}", (const char *)callData);
        this->ContextMenu->AddCommand("Delete Node", this, command);
        }
      else {
        // single node selected
        if ( node == NULL || (node != NULL && node->IsA("vtkMRMLTransformNode")) )
          {
          // scene or transform
          sprintf(command, "InsertTransformNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Insert Transform Node", this, command);
          }
        if ((node == NULL || (node != NULL && node->IsA("vtkMRMLTransformNode"))) &&
          this->CutNodes.size() > 0)
          {
          // scene or transform and cut node exists
          sprintf(command, "PasteNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Paste Node", this, command);
          }
        if (node != NULL && node->IsA("vtkMRMLTransformableNode") )
          {
          sprintf(command, "CutNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Cut Node", this, command);
    
          sprintf(command, "SelectNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Go To Editor...", this, command);
          }

        if (node != NULL && (node->IsA("vtkMRMLTransformableNode")))
          {
          vtkMRMLTransformableNode* tbnode = vtkMRMLTransformableNode::SafeDownCast(node);
          vtkMRMLTransformNode* tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(tbnode->GetTransformNodeID()));
          if (tbnode->GetParentTransformNode())
            {
            // avoid hardening nonlinear transforms onto volume nodes or linear transform nodes
            if (!(!tnode->CanApplyNonLinearTransforms() && !tnode->IsTransformToWorldLinear()))
              {
              sprintf(command, "HardenTransformCallback {%s}", (const char *)callData);
              this->ContextMenu->AddCommand("Harden Transforms", this, command);
              }
            }
          }
        if (node != NULL && node->IsA("vtkMRMLDisplayableNode") && !node->IsA("vtkMRMLVolumeNode") && !node->IsA("vtkMRMLTransformNode"))
          {
          sprintf(command, "ToggleVisibilityCallback {%s}", (const char *)callData);
          int index = this->ContextMenu->AddCheckButton("Toggle Visibility", this, command);
          this->ContextMenu->SetItemSelectedState(index, vtkMRMLDisplayableNode::SafeDownCast(node)->GetDisplayNode()->GetVisibility());
          }
        if (node != NULL)
          {
          sprintf(command, "DeleteNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Delete Node", this, command);
          }
        }
      this->ContextMenu->PopUp(px, py);
      }
    }
  else if (caller == entry) 
    {
    if (event == vtkKWEntry::EntryValueChangedEvent) 
      {
      vtkMRMLNode *node = this->GetSelectedNodeInTree();
      if (node)
        {
        node->SetName(entry->GetValue());
        this->UpdateTreeFromMRML();
        this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent);
        }
      }

    }
} 

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::DeleteNodeCallback(const char *id)
{
  // cout << "I want to delete MRML node " << id << endl;
  // delete node, then repopulate tree
  for (unsigned int i=0; i<this->SelectedLeaves.size(); i++)
    {
    vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(this->SelectedLeaves[i].c_str());
    if (node != NULL)
      {
      this->GetMRMLScene()->RemoveNode(node);
      }
    }
  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::PasteNodeCallback(const char *id)
{
  vtkMRMLTransformNode *tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  for (unsigned int i=0; i< this->CutNodes.size(); i++)
    {
    vtkMRMLTransformableNode *node = this->CutNodes[i];
    if (node != NULL)
      {
      if (tnode != NULL)
        {
        node->SetAndObserveTransformNodeID(tnode->GetID());
        }
      this->GetMRMLScene()->AddNode(node);
      }
    }
  this->ClearCutNodes();
  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::InsertTransformNodeCallback(const char *id)
{
  vtkMRMLTransformNode *tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  vtkMRMLLinearTransformNode *node = vtkMRMLLinearTransformNode::New();
  this->GetMRMLScene()->AddNodeNoNotify(node);
  if (tnode != NULL)
    {
    node->SetAndObserveTransformNodeID(tnode->GetID());
    }
  node->SetName(node->GetID());
  this->UpdateTreeFromMRML();
  this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, node);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::CutNodeCallback(const char *id)
{
  //cout << "I want to delete MRML node " << id << endl;
  this->ClearCutNodes();

  for (unsigned int i=0; i<this->SelectedLeaves.size(); i++)
    {
    vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID((const char *)this->SelectedLeaves[i].c_str());
    vtkMRMLTransformableNode *tnode = vtkMRMLTransformableNode::SafeDownCast(node);
    tnode->Register(this);
    this->CutNodes.push_back(tnode );
    this->GetMRMLScene()->RemoveNode(node);
    tnode->SetAndObserveTransformNodeID(NULL);
    }
  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::SelectNodeCallback(const char *id)
{
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(id);
  if (node != NULL)
    {
    this->InvokeEvent(vtkSlicerMRMLTreeWidget::SelectedEvent, node);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::NodeParentChangedCallback(
  const char *nodeID, const char *parentID, const char*)
{
  if (!strcmp(parentID, "Scene"))
    {
    vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));
    if (node != NULL)
      {
      vtkMRMLTransformNode *tnode = node->GetParentTransformNode();
      if (tnode != NULL)
        {
        node->SetAndObserveTransformNodeID(NULL);
        node->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
        }
      }
    }
  else 
    {
    vtkMRMLTransformNode *tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(parentID));
    if (tnode != NULL)
      {
      vtkMRMLTransformableNode *node = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));
      if (node != NULL)
        {
        if (tnode != NULL)
          {
          node->SetAndObserveTransformNodeID(tnode->GetID());
          node->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
          }
        }
      }
    }
  this->UpdateTreeFromMRML();
}
 
//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::ToggleVisibilityCallback(const char *id)
{
  vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!dnode)
    {
    return;
    }

  int newVisibilityState = this->ContextMenu->GetItemSelectedState("Toggle Visibility");
  for (int i=0; i<dnode->GetNumberOfDisplayNodes(); i++)
    {
    dnode->GetNthDisplayNode(i)->SetVisibility(newVisibilityState);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::HardenTransformCallback(const char *id)
{
  vtkMRMLTransformableNode* tbnode = vtkMRMLTransformableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!tbnode)
    {
    return;
    }

  vtkMRMLTransformNode* tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(tbnode->GetTransformNodeID()));

  if (!tnode)
    {
    return;
    }

  if (!tnode->IsTransformToWorldLinear() && !tnode->CanApplyNonLinearTransforms())
    {
    vtkErrorMacro(<<"Can't apply non linear transform to this node.");
    return;
    }

  this->GetMRMLScene()->SaveStateForUndo();

  if (tnode->IsTransformToWorldLinear())
    {
    vtkMatrix4x4* hardeningMatrix = vtkMatrix4x4::New();
    tnode->GetMatrixTransformToWorld(hardeningMatrix);
    tbnode->ApplyTransform(hardeningMatrix);
    hardeningMatrix->Delete();
    }
  else
    {
    vtkGeneralTransform* hardeningTransform = vtkGeneralTransform::New();
    tnode->GetTransformToWorld(hardeningTransform);
    tnode->ApplyTransform(hardeningTransform);
    hardeningTransform->Delete();
    }

  tbnode->SetAndObserveTransformNodeID(NULL);
  tbnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
  this->UpdateTreeFromMRML();
}
 
//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdateTreeFromMRML();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::RemoveWidgetObservers ( ) 
{
  if (this->TreeWidget) 
    {
    this->TreeWidget->GetWidget()->RemoveObservers(
      vtkKWTree::SelectionChangedEvent, 
      (vtkCommand *)this->GUICallbackCommand);  
 
    this->TreeWidget->GetWidget()->RemoveObservers(
      vtkKWTree::RightClickOnNodeEvent, 
      (vtkCommand *)this->GUICallbackCommand);  
    }
  if (this->NodeName)
    {
    this->NodeName->GetWidget()->RemoveObservers(  vtkKWEntry::EntryValueChangedEvent,
                                                 (vtkCommand *)this->GUICallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // MRML Tree

  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  frame->SetLabelText ("MRML Tree");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );

  this->TreeWidget = vtkKWTreeWithScrollbars::New() ;
  this->TreeWidget->SetParent ( frame->GetFrame() );
  this->TreeWidget->VerticalScrollbarVisibilityOn();
  this->TreeWidget->HorizontalScrollbarVisibilityOff();
  
  this->TreeWidget->Create ( );
  this->TreeWidget->SetBalloonHelpString("MRML Tree");
  ///  this->TreeWidget->SetBorderWidth(2);
  //this->TreeWidget->SetReliefToGroove();
  this->Script ( "pack %s -side top -anchor nw -expand y -fill both -padx 2 -pady 2",
                 this->TreeWidget->GetWidgetName());

  vtkKWTree *tree = this->TreeWidget->GetWidget();
  tree->SelectionFillOn();
  tree->SetSelectionModeToMultiple ();
  tree->SetNodeParentChangedCommand(this, "NodeParentChangedCallback");
  tree->EnableReparentingOn();
  //tree->SetSelectionModeToSingle();
  tree->SetHeight(12);

  tree->AddObserver(
    vtkKWTree::SelectionChangedEvent, 
    (vtkCommand *)this->GUICallbackCommand );

  tree->AddObserver(
    vtkKWTree::RightClickOnNodeEvent, 
    (vtkCommand *)this->GUICallbackCommand);

  this->UpdateTreeFromMRML();

  frame->Delete();

  // MRML Node Inspector

  frame = vtkKWFrameWithLabel::New();
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  frame->SetLabelText ("MRML Node Inspector");
  //frame->CollapseFrame ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );

  int label_width = 10;

  this->NodeID = vtkKWLabelWithLabel::New() ;
  this->NodeID->SetParent(frame->GetFrame());
  this->NodeID->SetLabelText("ID:");
  this->NodeID->Create();
  this->NodeID->GetWidget()->SetAnchorToWest();
  this->NodeID->SetLabelWidth(label_width);
  this->NodeID->SetBalloonHelpString("MRML Node ID");
  this->Script(
    "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
    this->NodeID->GetWidgetName());

  this->NodeName = vtkKWEntryWithLabel::New() ;
  this->NodeName->SetParent(frame->GetFrame());
  this->NodeName->SetLabelText("Name:");
  this->NodeName->Create();
  this->NodeName->SetLabelWidth(label_width);
  this->NodeName->SetBalloonHelpString("MRML Node Name");
  this->Script(
    "pack %s -side top -anchor nw -expand y -fill x -padx 2 -pady 2",
    this->NodeName->GetWidgetName());

  this->NodeName->GetWidget()->AddObserver(  vtkKWEntry::EntryValueChangedEvent,
                                            (vtkCommand *)this->GUICallbackCommand );

  frame->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::UpdateTreeFromMRML()
  
{
  vtksys_stl::string selected_node(
    this->TreeWidget->GetWidget()->GetSelection());
  this->TreeWidget->GetWidget()->DeleteAllNodes();

  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;

  // create Root node
  this->TreeWidget->GetWidget()->AddNode(NULL, "Scene", "Scene");
  int nnodes = scene->GetNumberOfNodes();
  for (int n=0; n<nnodes; n++)
    {
    node = scene->GetNthNode(n);
    this->AddNodeToTree(node);
    }

  // check that the selected node is still in the tree
  if (this->TreeWidget->GetWidget()->HasNode(selected_node.c_str()))
    {
    this->TreeWidget->GetWidget()->SelectNode(selected_node.c_str());
    }
  else
    {
    if (selected_node != "")
      {
      vtkWarningMacro("Selected node no longer in tree: " << selected_node.c_str());
      }
    }
  // At this point you probably want to reset the MRML node inspector fields
  // in case nothing in the tree is selected anymore (here, we delete all nodes
  // each time, so nothing will be selected, but it's not a bad thing to 
  // try to save the old selection, or just update the tree in a smarter
  // way).

  this->TreeWidget->GetWidget()->OpenFirstNode ();

  this->UpdateNodeInspector(this->GetSelectedNodeInTree());
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::AddNodeToTree(vtkMRMLNode *node)
{
  if (!node)
    {
    return;
    }

  if (node->GetHideFromEditors())
   {
    return;
   }

  vtkKWTree *tree = this->TreeWidget->GetWidget();

  if (tree->HasNode(node->GetID()))
    {
    return;
    }

  char *ID = node->GetID();
    
  vtksys_stl::string node_text(node->GetName());
  if (node_text.size())
    {
    node_text += " (";
    node_text += ID;
    node_text += ")";
    }
  else
    {
    node_text = ID;
    }
  /*
  node_text += ": ";
  node_text += node->GetClassName();
  */
  
  const char *parent_node = "Scene";
  int isTransformable = node->IsA("vtkMRMLTransformableNode");
  if (isTransformable)
    {
    vtkMRMLTransformableNode *transformableNode = 
      vtkMRMLTransformableNode::SafeDownCast(node);
    vtkMRMLTransformableNode *parent = 
      transformableNode->GetParentTransformNode();
    if (parent)
      {
      parent_node = parent->GetID();
      this->AddNodeToTree(parent);

      }
    }
  tree->AddNode(parent_node, ID, node_text.c_str());

  // This is how you can set icons for each node. You can either use
  // predefined icons, or go through the usual resources framework, just
  // like the toolbar icons are constructed (see SetNodeImage...)
  if (isTransformable)
    {
    //tree->SetNodeImageToPredefinedIcon(ID, vtkKWIcon::IconWarningMini);
    //tree->SetNodePadX(ID, 20);
    }
}

//---------------------------------------------------------------------------
vtkMRMLNode* vtkSlicerMRMLTreeWidget::GetSelectedNodeInTree()
{
  vtksys_stl::string selected_node(
    this->TreeWidget->GetWidget()->GetSelection());

  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL, *first_selected_node = NULL;
  int nnodes = scene->GetNumberOfNodes();
  for (int n=0; n<nnodes; n++)
    {
    node = scene->GetNthNode(n);
    if (!strcmp(node->GetID(), selected_node.c_str()))
      {
      first_selected_node = node;
      break;
      }
    }
  return first_selected_node;
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::UpdateNodeInspector(vtkMRMLNode *node)
{
  if (this->NodeID)
    {
    this->NodeID->GetWidget()->SetText(
      node ? node->GetID() : "");
    }

  if (this->NodeName)
    {
    this->NodeName->GetWidget()->SetValue(
      node ? node->GetName() : "");
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::SetSelectesLeaves()
{
  this->SelectedLeaves.clear();

  vtksys_stl::string selectedLeaves(
    this->TreeWidget->GetWidget()->GetSelection());

  vtksys_stl::string::size_type locStart = 0;
  vtksys_stl::string::size_type locEnd = 0;
  do 
    {
    locEnd = selectedLeaves.find( " ", locStart );
    if (locEnd != vtksys_stl::string::npos)
      {
      vtksys_stl::string selectedLeaf = selectedLeaves.substr(locStart, locEnd-locStart);
      this->SelectedLeaves.push_back(selectedLeaf);
      locStart = locEnd+1;
      }
    }
  while (locEnd != vtksys_stl::string::npos) ;
  vtksys_stl::string selectedLeaf = selectedLeaves.substr(locStart);
  this->SelectedLeaves.push_back(selectedLeaf);
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLTreeWidget::IsLeafSelected(const char *leaf)
{
  vtksys_stl::string sleaf(leaf);

  for (unsigned int i=0; i<this->SelectedLeaves.size(); i++)
    {
    if (this->SelectedLeaves[i].compare(sleaf) == 0)
      {
      return 1;
      }
    }
  return 0;
}

void vtkSlicerMRMLTreeWidget::ClearCutNodes()
{
  for (unsigned int i=0; i<this->CutNodes.size(); i++)
    {
    this->CutNodes[i]->UnRegister(this);
    }
  this->CutNodes.clear();
}
