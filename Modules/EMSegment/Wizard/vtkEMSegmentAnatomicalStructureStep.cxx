#include "vtkEMSegmentAnatomicalStructureStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWChangeColorButton.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWPushButtonSet.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentAnatomicalStructureStep);
vtkCxxRevisionMacro(vtkEMSegmentAnatomicalStructureStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentAnatomicalStructureStep::vtkEMSegmentAnatomicalStructureStep()
{
  this->SetName("2/9. Define Anatomical Tree");
  this->SetDescription("Define a hierarchy of structures.");

  this->ContextMenu  = NULL;

  this->AnatomicalStructureTree          = NULL;
  this->AnatomicalStructureFrame         = NULL;
  this->AnatomicalStructureTreeButtonSet = NULL;

  this->AnatomicalNodeAttributesFrame        = NULL;
  this->AnatomicalNodeAttributeNameEntry     = NULL;
  this->AnatomicalNodeIntensityLabelEntry    = NULL;
  this->AnatomicalNodeAttributeColorButton   = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentAnatomicalStructureStep::~vtkEMSegmentAnatomicalStructureStep()
{
  if (this->ContextMenu)
    {
    this->ContextMenu->Delete();
    this->ContextMenu = NULL;
    }

  if (this->AnatomicalStructureTree)
    {
    this->AnatomicalStructureTree->Delete();
    this->AnatomicalStructureTree = NULL;
    }

  if (this->AnatomicalStructureFrame)
    {
    this->AnatomicalStructureFrame->Delete();
    this->AnatomicalStructureFrame = NULL;
    }

  if (this->AnatomicalStructureTreeButtonSet)
    {
    this->AnatomicalStructureTreeButtonSet->Delete();
    this->AnatomicalStructureTreeButtonSet = NULL;
    }

  if (this->AnatomicalNodeAttributesFrame)
    {
    this->AnatomicalNodeAttributesFrame->Delete();
    this->AnatomicalNodeAttributesFrame = NULL;
    }

  if (this->AnatomicalNodeAttributeNameEntry)
    {
    this->AnatomicalNodeAttributeNameEntry->Delete();
    this->AnatomicalNodeAttributeNameEntry = NULL;
    }

  if (this->AnatomicalNodeIntensityLabelEntry)
    {
    this->AnatomicalNodeIntensityLabelEntry->Delete();
    this->AnatomicalNodeIntensityLabelEntry = NULL;
    }

  if (this->AnatomicalNodeAttributeColorButton)
    {
    this->AnatomicalNodeAttributeColorButton->Delete();
    this->AnatomicalNodeAttributeColorButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::ShowAnatomicalStructureTree()
{
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();

  // Create the frame

  if (!this->AnatomicalStructureFrame)
    {
    this->AnatomicalStructureFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->AnatomicalStructureFrame->IsCreated())
    {
    this->AnatomicalStructureFrame->SetParent(
      wizard_widget->GetClientArea());
    this->AnatomicalStructureFrame->Create();
    this->AnatomicalStructureFrame->SetLabelText("Anatomical Tree");
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->AnatomicalStructureFrame->GetWidgetName());

  // Create the tree

  if (!this->AnatomicalStructureTree)
    {
    this->AnatomicalStructureTree = vtkKWTreeWithScrollbars::New();
    }
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  if (!this->AnatomicalStructureTree->IsCreated())
    {
    this->AnatomicalStructureTree->SetParent(
      this->AnatomicalStructureFrame->GetFrame());
    this->AnatomicalStructureTree->Create();
    this->AnatomicalStructureTree->SetPadX(0);
    this->AnatomicalStructureTree->SetPadY(0);
    this->AnatomicalStructureTree->SetBorderWidth(2);
    this->AnatomicalStructureTree->SetReliefToSunken();
    this->AnatomicalStructureTree->SetHorizontalScrollbarVisibility(0);

    tree->SetHighlightThickness(0);
    tree->RedrawOnIdleOn();
    tree->SetHeight(7);
    
    this->Script("pack %s -side left -expand y -fill both -padx 0 -pady 0", 
                 this->AnatomicalStructureTree->GetWidgetName());
    }

  // Create the tree buttons (expand/collapse, for convenience)

  if (!this->AnatomicalStructureTreeButtonSet)
    {
    this->AnatomicalStructureTreeButtonSet = vtkKWPushButtonSet::New();
    }

  if (!this->AnatomicalStructureTreeButtonSet->IsCreated())
    {
    this->AnatomicalStructureTreeButtonSet->SetParent(
      this->AnatomicalStructureFrame->GetFrame());
    this->AnatomicalStructureTreeButtonSet->PackHorizontallyOff();
    this->AnatomicalStructureTreeButtonSet->Create();
    this->AnatomicalStructureTreeButtonSet->SetWidgetsPadX(0);
    this->AnatomicalStructureTreeButtonSet->SetWidgetsPadY(2);

    vtkKWPushButton *button;
    int button_id = 0;

    button = this->AnatomicalStructureTreeButtonSet->AddWidget(++button_id);
    if (button)
      {
      button->SetCommand(this, "OpenTreeCallback");
      button->SetBalloonHelpString("Open all nodes");
      button->SetImageToPredefinedIcon(vtkKWIcon::IconTreeOpen);
      }
    
    button = this->AnatomicalStructureTreeButtonSet->AddWidget(++button_id);
    if (button)
      {
      button->SetCommand(this, "CloseTreeCallback");
      button->SetBalloonHelpString("Close all nodes");
      button->SetImageToPredefinedIcon(vtkKWIcon::IconTreeClose);
      }
    
    this->Script("pack %s -anchor nw -fill none -expand n -padx 2", 
                 this->AnatomicalStructureTreeButtonSet->GetWidgetName());
    }
  
  tree->SetSelectionChangedCommand(NULL, NULL);
  tree->SetRightClickOnNodeCommand(NULL, NULL);
  tree->SetNodeParentChangedCommand(NULL, NULL);
  tree->EnableReparentingOff();

  this->SetAnatomicalTreeNodesSelectableOn();

  // Populate the tree

  vtkIdType root_id = mrmlManager->GetTreeRootNodeID();
  if (root_id)
    {
    this->PopulateAnatomicalStructureTree(NULL, root_id);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  this->ShowAnatomicalStructureTree();

  // Override the tree callbacks for that specific step

  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  tree->SetSelectionChangedCommand(
    this, "DisplaySelectedNodeAnatomicalAttributesCallback");
  tree->SetRightClickOnNodeCommand(this, "PopupNodeContextMenuCallback");
  tree->SetNodeParentChangedCommand(this, "NodeParentChangedCallback");
  tree->EnableReparentingOn();

  // Create the frame

  if (!this->AnatomicalNodeAttributesFrame)
    {
    this->AnatomicalNodeAttributesFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->AnatomicalNodeAttributesFrame->IsCreated())
    {
    this->AnatomicalNodeAttributesFrame->SetParent(
      wizard_widget->GetClientArea());
    this->AnatomicalNodeAttributesFrame->Create();
    this->AnatomicalNodeAttributesFrame->SetLabelText("Node Attributes");
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->AnatomicalNodeAttributesFrame->GetWidgetName());

  // Create the node name

  if (!this->AnatomicalNodeAttributeNameEntry)
    {
    this->AnatomicalNodeAttributeNameEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->AnatomicalNodeAttributeNameEntry->IsCreated())
    {
    this->AnatomicalNodeAttributeNameEntry->SetParent(
      this->AnatomicalNodeAttributesFrame->GetFrame());
    this->AnatomicalNodeAttributeNameEntry->Create();
    this->AnatomicalNodeAttributeNameEntry->SetLabelText("Name: ");
    this->AnatomicalNodeAttributeNameEntry->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 12);

    vtkKWEntry *entry = this->AnatomicalNodeAttributeNameEntry->GetWidget();
    entry->SetWidth(30);
    entry->SetCommandTriggerToAnyChange();
    }

  this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", 
               this->AnatomicalNodeAttributeNameEntry->GetWidgetName());

  // Create the node label

  if (!this->AnatomicalNodeIntensityLabelEntry)
    {
    this->AnatomicalNodeIntensityLabelEntry = vtkKWEntryWithLabel::New();
    }
  if (!this->AnatomicalNodeIntensityLabelEntry->IsCreated())
    {
    this->AnatomicalNodeIntensityLabelEntry->SetParent(
      this->AnatomicalNodeAttributesFrame->GetFrame());
    this->AnatomicalNodeIntensityLabelEntry->Create();
    this->AnatomicalNodeIntensityLabelEntry->SetLabelText("Label: ");
    this->AnatomicalNodeIntensityLabelEntry->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 12);
    this->AnatomicalNodeIntensityLabelEntry->GetWidget()->SetWidth(6);
    this->AnatomicalNodeIntensityLabelEntry->GetWidget()
      ->SetRestrictValueToInteger();
    this->AnatomicalNodeIntensityLabelEntry->GetWidget()
      ->SetCommandTriggerToAnyChange();
    }

  // Create the node color button

  if (!this->AnatomicalNodeAttributeColorButton)
    {
    this->AnatomicalNodeAttributeColorButton = vtkKWChangeColorButton::New();
    }
  if (!this->AnatomicalNodeAttributeColorButton->IsCreated())
    {
    this->AnatomicalNodeAttributeColorButton->SetParent(
      this->AnatomicalNodeAttributesFrame->GetFrame());
    this->AnatomicalNodeAttributeColorButton->LabelOutsideButtonOn();
    this->AnatomicalNodeAttributeColorButton->Create();
    this->AnatomicalNodeAttributeColorButton->SetLabelText("Color:");
    this->AnatomicalNodeAttributeColorButton->SetLabelWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 12);
    }

  // Update the UI with the proper value, if there is a selection

  this->DisplaySelectedNodeAnatomicalAttributesCallback();
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::DisplaySelectedNodeAnatomicalAttributesCallback()
{
  // Update the UI with the proper value, if there is a selection

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType sel_vol_id = 0;
  int sel_is_leaf_node = 0;
  int has_valid_selection = tree->HasSelection();
  if (has_valid_selection)
    {
    sel_node = tree->GetSelection();
    sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());
    sel_is_leaf_node = mrmlManager->GetTreeNodeIsLeaf(sel_vol_id);
    }
  int enabled = tree->GetEnabled();
  char buffer[256];

  // Update the node name

  if (this->AnatomicalNodeAttributeNameEntry)
    {
    vtkKWEntry *entry = this->AnatomicalNodeAttributeNameEntry->GetWidget();
    if (has_valid_selection)
      {
      vtksys_stl::string name(tree->GetNodeText(sel_node.c_str()));
      this->AnatomicalNodeAttributeNameEntry->SetEnabled(enabled);
      sprintf(buffer, "SelectedNodeNameChangedCallback %d", 
              static_cast<int>(sel_vol_id));
      entry->SetCommand(this, buffer);
      entry->SetValue(name.c_str());
      }
    else
      {
      this->AnatomicalNodeAttributeNameEntry->SetEnabled(0);
      entry->SetCommand(NULL, NULL);
      entry->SetValue(NULL);
      }
    }

  // Update the node intensity label

  if (this->AnatomicalNodeIntensityLabelEntry)
    {
    vtkKWEntry *entry = this->AnatomicalNodeIntensityLabelEntry->GetWidget();
    if (has_valid_selection && sel_is_leaf_node)
      {
      int intLabel = mrmlManager->GetTreeNodeIntensityLabel(sel_vol_id);
      this->AnatomicalNodeIntensityLabelEntry->SetEnabled(enabled);
      sprintf(buffer, 
              "SelectedNodeIntensityLabelChangedCallback %d", 
              static_cast<int>(sel_vol_id));
      entry->SetCommand(this, buffer);
      entry->SetValueAsInt(intLabel);
      this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", 
                   this->AnatomicalNodeIntensityLabelEntry->GetWidgetName());
      }
    else
      {
      this->AnatomicalNodeIntensityLabelEntry->SetEnabled(0);
      entry->SetCommand(NULL, NULL);
      entry->SetValue(NULL);
      this->Script("pack forget %s", 
                   this->AnatomicalNodeIntensityLabelEntry->GetWidgetName());
      }
    }

  // Update the node color

  if (this->AnatomicalNodeAttributeColorButton)
    {
    if (has_valid_selection && sel_is_leaf_node)
      {
      this->AnatomicalNodeAttributeColorButton->SetEnabled(enabled);
      sprintf(buffer, "SelectedNodeColorChangedCallback %d", 
              static_cast<int>(sel_vol_id));
      this->AnatomicalNodeAttributeColorButton->SetCommand(this, buffer);
      double rgb[3] = { 0.5, 0.5, 0.5 };
      mrmlManager->GetTreeNodeColor(sel_vol_id, rgb);
      this->AnatomicalNodeAttributeColorButton->SetColor(rgb);

      //
      // We hide the color button for now because the color that is
      // selected here is not the color that is displayed for the
      // segmentation later.  Colors are dealt with using the colormap
      // mechanism and that functionality should be integrated here at
      // some point.
      //this->Script("pack %s -side top -anchor nw -padx 2 -pady 2", 
      //             this->AnatomicalNodeAttributeColorButton->GetWidgetName());
      }
    else
      {
      this->AnatomicalNodeAttributeColorButton->SetEnabled(0);
      this->AnatomicalNodeAttributeColorButton->SetCommand(NULL, NULL);
      // see comment just above
      //this->Script("pack forget %s", 
      //             this->AnatomicalNodeAttributeColorButton->GetWidgetName());
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::SetAnatomicalTreeNodesSelectableOn()
{
  if (!this->AnatomicalStructureTree ||
      !this->AnatomicalStructureTree->IsCreated())
    {
    return;
    }
  
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  vtkIdType vol_id = mrmlManager->GetTreeRootNodeID();
  const char *root_node = tree->FindNodeWithUserDataAsInt(NULL, vol_id);
  if (root_node && *root_node)
    {
    vtksys_stl::string rootnode = root_node;
    this->SetAnatomicalTreeLeafNodeSelectableState(rootnode.c_str(), 1);
    this->SetAnatomicalTreeParentNodeSelectableState(rootnode.c_str(), 1);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::SetAnatomicalTreeLeafNodeSelectableState(
  const char* parent,  int state)
{
  if (!this->AnatomicalStructureTree ||
     !this->AnatomicalStructureTree->IsCreated())
    {
    return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtksys_stl::string parent_node = parent;
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  vtkIdType vol_id = tree->GetNodeUserDataAsInt(parent_node.c_str());
  vtkIdType child_id;

  int nb_children = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);
  for (int i = 0; i < nb_children; i++)
    {
    child_id = mrmlManager->GetTreeNodeChildNodeID(vol_id, i);
    const char* node = 
      tree->FindNodeWithUserDataAsInt(parent_node.c_str(), child_id);
    if (node)
      {
      if (mrmlManager->GetTreeNodeIsLeaf(child_id))
        {
        tree->SetNodeSelectableFlag(node, state);
        }
      else
        {
        this->SetAnatomicalTreeLeafNodeSelectableState(node, state);
        }
      }
    }

  if (nb_children == 0)
    {
    tree->SetNodeSelectableFlag(parent_node.c_str(), state);
    }

  if (!state && tree->HasSelection())
    {
    const char* sel_node = tree->GetSelection();
    if (sel_node && !tree->GetNodeSelectableFlag(sel_node))
      {
      tree->ClearSelection();
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::SetAnatomicalTreeParentNodeSelectableState(
  const char* parent, int state)
{
  if (!this->AnatomicalStructureTree ||
    !this->AnatomicalStructureTree->IsCreated())
    {
    return;
    }

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtksys_stl::string parent_node = parent;
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  vtkIdType vol_id = tree->GetNodeUserDataAsInt(parent_node.c_str());
  vtkIdType child_id;

  int nb_children = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);
  for (int i = 0; i < nb_children; i++)
    {
    child_id = mrmlManager->GetTreeNodeChildNodeID(vol_id, i);
    const char* node = 
      tree->FindNodeWithUserDataAsInt(parent_node.c_str(), child_id);
    if (node)
      {
      if (!mrmlManager->GetTreeNodeIsLeaf(child_id))
        {
        this->SetAnatomicalTreeParentNodeSelectableState(node, state);
        }
      }
    }

  if (nb_children > 0)
    {
    tree->SetNodeSelectableFlag(parent_node.c_str(), state);
    }

  if (!state && tree->HasSelection())
    {
    const char* sel_node = tree->GetSelection();
    if (sel_node && !tree->GetNodeSelectableFlag(sel_node))
      {
      tree->ClearSelection();
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::PopulateAnatomicalStructureTree(
  const char *parent, vtkIdType vol_id)
{
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();

  // Insert that volume if not found, or just update its label
  // Our aim here is to update the tree without removing all the nodes
  // first (and lose the selection as well as the open/close state of
  // all the nodes). 
  // Right now, if a node is found, it will updated. However, if the
  // node position had changed since last call (i.e. the node was already
  // there but was moved), this will not be updated accordingly (todo)

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  const char *found = tree->FindNodeWithUserDataAsInt(parent, vol_id);
  vtksys_stl::stringstream node;
  vtksys_stl::string nodename(mrmlManager->GetTreeNodeName(vol_id));
  if (!found)
    {
    node << tree->GetTclName() << vol_id;
    tree->AddNode(parent, node.str().c_str(), nodename.c_str());
    }
  else
    {
    node << found;
    tree->SetNodeText(node.str().c_str(), nodename.c_str());
    }
  tree->SetNodeUserDataAsInt(node.str().c_str(), vol_id); 

  // Insert its children

  int nb_children = mrmlManager->GetTreeNodeNumberOfChildren(vol_id);
  for (int i = 0; i < nb_children; i++)
    {
    this->PopulateAnatomicalStructureTree(
      node.str().c_str(), mrmlManager->GetTreeNodeChildNodeID(vol_id, i));
    }
}
   
//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::OpenTreeCallback()
{
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  tree->OpenFirstNode();
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::CloseTreeCallback()
{
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  tree->CloseFirstNode();
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::PopupNodeContextMenuCallback(
  const char *node)
{
  if (!node || !*node)
    {
    return;
    }

  // If we have a node selection, offer to delete the corresponding node, or
  // create a child underneath.

  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  tree->SelectNode(node);
  vtkIdType vol_id = tree->GetNodeUserDataAsInt(node);

  char buffer[256];

  if (!this->ContextMenu)
    {
    this->ContextMenu = vtkKWMenu::New();
    }
  if (!this->ContextMenu->IsCreated())
    {
    this->ContextMenu->SetParent(tree);
    this->ContextMenu->Create();
    }

  this->ContextMenu->DeleteAllItems();
  sprintf(buffer, "AddChildNodeCallback %d", static_cast<int>(vol_id));
  this->ContextMenu->AddCommand("Add sub-class", this, buffer);
  if (strcmp(node, "root_node"))
    {
    sprintf(buffer, "DeleteNodeCallback %d", static_cast<int>(vol_id));
    this->ContextMenu->AddCommand("Delete sub-class", this, buffer);
    }

  int px, py;
  vtkKWTkUtilities::GetMousePointerCoordinates(tree, &px, &py);
  this->ContextMenu->PopUp(px, py);
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::DeleteNodeCallback(vtkIdType sel_vol_id)
{
  if (vtkKWMessageDialog::PopupYesNo( 
        this->GetApplication(), 
        NULL, 
        "Delete node?",
        "Are you sure you want to delete this sub-class and its children?",
        vtkKWMessageDialog::WarningIcon | vtkKWMessageDialog::InvokeAtPointer))
    {
    vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
    vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
    vtksys_stl::string sel_node(
      tree->FindNodeWithUserDataAsInt(NULL, sel_vol_id));
    vtksys_stl::string parent_node(tree->GetNodeParent(sel_node.c_str()));
    tree->DeleteNode(sel_node.c_str());
    tree->SelectSingleNode(parent_node.c_str());
    mrmlManager->RemoveTreeNode(sel_vol_id);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::AddChildNodeCallback(vtkIdType sel_vol_id)
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  vtksys_stl::string sel_node(
    tree->FindNodeWithUserDataAsInt(NULL, sel_vol_id));
  char child_node[256];
  vtkIdType child_id = mrmlManager->AddTreeNode(sel_vol_id);
  sprintf(child_node, "node_%d", static_cast<int>(child_id));
  tree->AddNode(sel_node.c_str(), child_node, child_node);
  tree->SetNodeUserDataAsInt(child_node, child_id); 
  tree->OpenNode(sel_node.c_str());
  tree->SelectNode(child_node);
  this->DisplaySelectedNodeAnatomicalAttributesCallback();
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::NodeParentChangedCallback(
  const char *node, const char *new_parent, const char*)
{
  // Reparent a node (by drag and dropping)

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  vtkIdType vol_id = tree->GetNodeUserDataAsInt(node);
  vtkIdType new_parent_vol_id = tree->GetNodeUserDataAsInt(new_parent);
  mrmlManager->SetTreeNodeParentNodeID(vol_id, new_parent_vol_id);
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::SelectedNodeNameChangedCallback(
  vtkIdType sel_vol_id, const char *value)
{
  // The node name has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  const char *found_node = tree->FindNodeWithUserDataAsInt(NULL, sel_vol_id);
  if (found_node)
    {
    vtksys_stl::string node(found_node);
    tree->SetNodeText(node.c_str(), value);
    mrmlManager->SetTreeNodeName(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::SelectedNodeIntensityLabelChangedCallback(
  vtkIdType sel_vol_id, int value)
{
  // The node label has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWTree *tree = this->AnatomicalStructureTree->GetWidget();
  const char *found_node = tree->FindNodeWithUserDataAsInt(NULL, sel_vol_id);
  if (found_node)
    {
    vtksys_stl::string node(found_node);
    mrmlManager->SetTreeNodeIntensityLabel(sel_vol_id, value);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::SelectedNodeColorChangedCallback(
  vtkIdType sel_vol_id, double r, double g, double b)
{
  // The node color has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  double rgb[3]; rgb[0] = r; rgb[1] = g; rgb[2] = b;
  mrmlManager->SetTreeNodeColor(sel_vol_id, rgb);
}

//----------------------------------------------------------------------------
void vtkEMSegmentAnatomicalStructureStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
