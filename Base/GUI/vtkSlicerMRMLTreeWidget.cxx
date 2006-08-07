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
#include "vtkMRMLTransformableNode.h"
#include "vtkKWTreeWithScrollbars.h"

#include <vtksys/stl/string>

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
  this->CutNode = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerMRMLTreeWidget::~vtkSlicerMRMLTreeWidget ( )
{
  if (this->TreeWidget)
    {
    this->TreeWidget->Delete();
    this->TreeWidget = NULL;
    }

  if (this->ContextMenu)
    {
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
  this->SetCutNode(NULL);
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
  if (caller == tree)
    {
    // Selection changed

    if (event == vtkKWTree::SelectionChangedEvent)
      {
      // For example, one could populate the node inspector

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

      if ((node == NULL || (node != NULL && node->IsA("vtkMRMLTransformNode"))) &&
        this->GetCutNode() != NULL)
        {
        // scene or transform
        sprintf(command, "PasteNodeCallback {%s}", (const char *)callData);
        this->ContextMenu->AddCommand("Paste Node", this, command);
        }
      if (node != NULL && node->IsA("vtkMRMLTransformableNode") )
        {
        sprintf(command, "CutNodeCallback {%s}", (const char *)callData);
        this->ContextMenu->AddCommand("Cut Node", this, command);
        }
      if (node != NULL)
        {
        sprintf(command, "DeleteNodeCallback {%s}", (const char *)callData);
        this->ContextMenu->AddCommand("Delete Node", this, command);
        }
      this->ContextMenu->PopUp(px, py);
      }
    }
} 

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::DeleteNodeCallback(const char *id)
{
  cout << "I want to delete MRML node " << id << endl;
  // delete node, then repopulate tree
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(id);
  if (node != NULL)
    {
    this->GetMRMLScene()->RemoveNode(node);
    }

  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::PasteNodeCallback(const char *id)
{
  vtkMRMLTransformNode *tnode = vtkMRMLTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));
  vtkMRMLTransformableNode *node = this->GetCutNode();
  if (node != NULL)
    {
    if (tnode != NULL)
      {
      node->SetAndObserveTransformNodeID(tnode->GetID());
      }
    this->GetMRMLScene()->AddNode(node);
    }

  this->SetCutNode(NULL);
  this->UpdateTreeFromMRML();
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLTreeWidget::CutNodeCallback(const char *id)
{
  cout << "I want to delete MRML node " << id << endl;
  vtkMRMLNode *node = this->GetMRMLScene()->GetNodeByID(id);
  vtkMRMLTransformableNode *tnode = vtkMRMLTransformableNode::SafeDownCast(node);
  this->SetCutNode(tnode );
  this->GetMRMLScene()->RemoveNode(node);
  tnode->SetAndObserveTransformNodeID(NULL);
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
 this->TreeWidget->GetWidget()->RemoveObservers(
   vtkKWTree::SelectionChangedEvent, 
   (vtkCommand *)this->GUICallbackCommand);  
 
 this->TreeWidget->GetWidget()->RemoveObservers(
   vtkKWTree::RightClickOnNodeEvent, 
   (vtkCommand *)this->GUICallbackCommand);  
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
  tree->SetSelectionModeToSingle();
  tree->SetHeight(12);

  tree->AddObserver(
    vtkKWTree::SelectionChangedEvent, 
    (vtkCommand *)this->GUICallbackCommand );

  tree->AddObserver(
    vtkKWTree::RightClickOnNodeEvent, 
    (vtkCommand *)this->GUICallbackCommand);

  this->UpdateTreeFromMRML();

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
  scene->InitTraversal();
  while (node=scene->GetNextNode())
    {
    this->AddNodeToTree(node);
    }

  this->TreeWidget->GetWidget()->SelectNode(selected_node.c_str());

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
  int node_is_transformable = node->IsA("vtkMRMLTransformableNode");
  if (node_is_transformable)
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
  if (node_is_transformable)
    {
    tree->SetNodeImageToPredefinedIcon(ID, vtkKWIcon::IconWarningMini);
    tree->SetNodePadX(ID, 20);
    }
}

//---------------------------------------------------------------------------
vtkMRMLNode* vtkSlicerMRMLTreeWidget::GetSelectedNodeInTree()
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
