#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"

#include "vtkSlicerModelHierarchyWidget.h"

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
#include "vtkKWSimpleEntryDialog.h"
#include "vtkKWEntry.h"

#include "vtkKWTreeWithScrollbars.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModelHierarchyWidget );
vtkCxxRevisionMacro ( vtkSlicerModelHierarchyWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerModelHierarchyWidget::vtkSlicerModelHierarchyWidget ( )
{
  this->TreeWidget = NULL;
  this->ContextMenu = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerModelHierarchyWidget::~vtkSlicerModelHierarchyWidget ( )
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
  if (this->MRMLScene)
    {
    this->SetMRMLScene(NULL);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerModelHierarchyWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
  vtkKWTree *tree = this->TreeWidget->GetWidget();
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
        }
      else if (strcmp(selected.c_str(), "Scene")) 
        {
        // single node selected
        if (node != NULL && node->IsA("vtkMRMLModelHierarchyNode") )
          {
          // hierarchy
          sprintf(command, "InsertHierarchyNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Insert Model Hierarchy Node", this, command);

          sprintf(command, "DeleteNodeCallback {%s}", (const char *)callData);
          this->ContextMenu->AddCommand("Delete Model Hierarchy Node", this, command);
          }
        }
      else if ( node == NULL )
        {
        // Scene selected
        sprintf(command, "InsertHierarchyNodeCallback {%s}", (const char *)callData);
        this->ContextMenu->AddCommand("Insert Model Hierarchy Node", this, command);
        }
      this->ContextMenu->PopUp(px, py);
      }
    }
} 

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::DeleteNodeCallback(const char *id)
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
void vtkSlicerModelHierarchyWidget::InsertHierarchyNodeCallback(const char *id)
{

  vtkMRMLModelHierarchyNode *parentNode = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  vtkMRMLModelHierarchyNode *node = vtkMRMLModelHierarchyNode::New();

  this->GetMRMLScene()->AddNodeNoNotify(node);
  vtkKWEntryWithLabel *entry = this->NameDialog->GetEntry();
  entry->GetWidget()->SetValue(node->GetName());
  int result = this->NameDialog->Invoke();
  if (!result) 
    {
    this->MRMLScene->RemoveNode(node);
    node = NULL;
    }
  else 
    {

    node->SetName(entry->GetWidget()->GetValue());
    if (parentNode != NULL)
      {
      node->SetParentNodeID(parentNode->GetID());
      }
    vtkMRMLModelDisplayNode *dnode = vtkMRMLModelDisplayNode::New();
    this->GetMRMLScene()->AddNodeNoNotify(dnode);
    node->SetAndObserveDisplayNodeID(dnode->GetID());
    this->UpdateTreeFromMRML();
    this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, node);
    }
}


//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::OpenHierarchyCommand(const char *id)
{
  vtkMRMLModelHierarchyNode *node = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  if (node != NULL) 
  {
    node->SetExpanded(1);
  }

}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::CloseHierarchyCommand(const char *id)
{
  vtkMRMLModelHierarchyNode *node = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  if (node != NULL) 
  {
    node->SetExpanded(0);
  }
}

//----------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::NodeParentChangedCallback(
  const char *nodeID, const char *parentID, const char*)
{
  vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));
  vtkMRMLModelHierarchyNode *hnode = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));
  if (!strcmp(parentID, "Scene"))
    {
    if (hnode != NULL)
      {
      vtkMRMLModelHierarchyNode *parentNode = vtkMRMLModelHierarchyNode::SafeDownCast(hnode->GetParentNode());
      if (parentNode != NULL)
        {
        hnode->SetParentNodeID(NULL);
        //node->InvokeEvent(vtkMRMLModelNode::HierarchyModifiedEvent);
        }
      }
    else if (mnode != NULL)
      {
      vtkMRMLModelHierarchyNode *parentNode = vtkMRMLModelHierarchyNode::GetModelHierarchyNode(this->GetMRMLScene(), mnode->GetID());
      if (parentNode != NULL)
        {
        parentNode->SetModelNodeID(NULL);
        //node->InvokeEvent(vtkMRMLModelNode::HierarchyModifiedEvent);
        }
      }
    }
  else 
    {
    // reparent to Hierarchy node
    vtkMRMLModelHierarchyNode *parentNode = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(parentID));
    if (parentNode != NULL)
      {
      if (hnode != NULL)
        {
        hnode->SetParentNodeID(parentNode->GetID());
        //node->InvokeEvent(vtkMRMLModelNode::HierarchyModifiedEvent);
        }
      else if (mnode != NULL)
        {
        vtkMRMLModelHierarchyNode *oldParentNode = vtkMRMLModelHierarchyNode::GetModelHierarchyNode(this->GetMRMLScene(), mnode->GetID());
        if (oldParentNode)
          {
          oldParentNode->SetModelNodeID(NULL);
          }
        parentNode->SetModelNodeID(mnode->GetID());
        //node->InvokeEvent(vtkMRMLModelNode::HierarchyModifiedEvent);
        }
      }
    }
  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  if (event == vtkCommand::ModifiedEvent ||
      event == vtkMRMLScene::NodeAddedEvent || 
      event == vtkMRMLScene::NodeRemovedEvent || 
      event == vtkMRMLScene::NewSceneEvent)
    {
    this->UpdateTreeFromMRML();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::RemoveWidgetObservers ( ) 
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
}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::CreateWidget ( )
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
  frame->SetLabelText ("Model Hierarchy Tree");
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
  //tree->SetSelectionModeToMultiple ();
  tree->SetSelectionModeToSingle();
  tree->SetNodeParentChangedCommand(this, "NodeParentChangedCallback");
  tree->SetOpenCommand(this, "OpenHierarchyCommand");
  tree->SetCloseCommand(this, "CloseHierarchyCommand");
  tree->EnableReparentingOn();
  tree->SetHeight(12);

  tree->AddObserver(
    vtkKWTree::SelectionChangedEvent, 
    (vtkCommand *)this->GUICallbackCommand );

  tree->AddObserver(
    vtkKWTree::RightClickOnNodeEvent, 
    (vtkCommand *)this->GUICallbackCommand);

  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
  events->Delete();

  this->NameDialog  = vtkKWSimpleEntryDialog::New();
  this->NameDialog->SetParent ( this->GetParent());
  this->NameDialog->SetTitle("Model Hierarchy Name");
  this->NameDialog->SetSize(400, 200);
  this->NameDialog->SetStyleToOkCancel();
  vtkKWEntryWithLabel *entry = this->NameDialog->GetEntry();
  entry->SetLabelText("Hierarchy Name");
  entry->GetWidget()->SetValue("");
  this->NameDialog->Create ( );

  this->UpdateTreeFromMRML();

  frame->Delete();

}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::UpdateTreeFromMRML()
  
{
  vtksys_stl::string selected_node(
    this->TreeWidget->GetWidget()->GetSelection());
  this->TreeWidget->GetWidget()->DeleteAllNodes();

  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;

  // create Root node
  this->TreeWidget->GetWidget()->AddNode(NULL, "Scene", "Scene");
  scene->InitTraversal();
  while (node=scene->GetNextNode())
    {
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

}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::AddNodeToTree(vtkMRMLNode *node)
{
  if (!node)
    {
    return;
    }

  if (!node->IsA("vtkMRMLModelNode") && !node->IsA("vtkMRMLModelHierarchyNode") || node->GetHideFromEditors())
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

  vtkMRMLModelHierarchyNode* parentNode = NULL;

  if ( node->IsA("vtkMRMLModelNode"))
    {
    parentNode = vtkMRMLModelHierarchyNode::GetModelHierarchyNode(this->GetMRMLScene(),
                                                                  node->GetID());
    }
  else if ( node->IsA("vtkMRMLModelHierarchyNode") )
    {
    vtkMRMLModelHierarchyNode *mhnode = vtkMRMLModelHierarchyNode::SafeDownCast(node);
    parentNode = vtkMRMLModelHierarchyNode::SafeDownCast(mhnode->GetParentNode());
    }

  if (parentNode)
    {
    parent_node = parentNode->GetID();
    this->AddNodeToTree(parentNode);
    }
  tree->AddNode(parent_node, ID, node_text.c_str());

}

//---------------------------------------------------------------------------
vtkMRMLNode* vtkSlicerModelHierarchyWidget::GetSelectedNodeInTree()
{
  vtksys_stl::string selected_node(
    this->TreeWidget->GetWidget()->GetSelection());

  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL, *first_selected_node = NULL;
  scene->InitTraversal();
  while (node = scene->GetNextNode())
    {
    if (!strcmp(node->GetID(), selected_node.c_str()))
      {
      first_selected_node = node;
      break;
      }
    }
  return first_selected_node;
}

//---------------------------------------------------------------------------
void vtkSlicerModelHierarchyWidget::SetSelectesLeaves()
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
int vtkSlicerModelHierarchyWidget::IsLeafSelected(const char *leaf)
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
