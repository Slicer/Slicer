#include "vtkEMSegmentSpatialPriorsStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWTree.h"
#include "vtkKWTreeWithScrollbars.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"

#include "vtkEMSegmentAnatomicalStructureStep.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentSpatialPriorsStep);
vtkCxxRevisionMacro(vtkEMSegmentSpatialPriorsStep, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkEMSegmentSpatialPriorsStep::vtkEMSegmentSpatialPriorsStep()
{
  this->SetName("3/9. Assign Atlas");
  this->SetDescription("Assign atlases for anatomical structures.");

  this->SpatialPriorsVolumeFrame      = NULL;
  this->SpatialPriorsVolumeMenuButton = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentSpatialPriorsStep::~vtkEMSegmentSpatialPriorsStep()
{
  if (this->SpatialPriorsVolumeMenuButton)
    {
    this->SpatialPriorsVolumeMenuButton->Delete();
    this->SpatialPriorsVolumeMenuButton = NULL;
    }

  if (this->SpatialPriorsVolumeFrame)
    {
    this->SpatialPriorsVolumeFrame->Delete();
    this->SpatialPriorsVolumeFrame = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentSpatialPriorsStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  anat_step->ShowAnatomicalStructureTree();

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkIdType vol_id = mrmlManager->GetTreeRootNodeID();
  const char *root_node = 
    anat_step->GetAnatomicalStructureTree()->GetWidget()->FindNodeWithUserDataAsInt(
      NULL, vol_id);
  if (root_node && *root_node)
    {
    anat_step->SetAnatomicalTreeParentNodeSelectableState(root_node, 0);
    }

  // Override the tree callbacks for that specific step

  anat_step->GetAnatomicalStructureTree()->GetWidget()->SetSelectionChangedCommand(
      this, "DisplaySelectedNodeSpatialPriorsCallback");

  vtkKWWidget *parent = wizard_widget->GetClientArea();
  
  // Create the frame

  if (!this->SpatialPriorsVolumeFrame)
    {
    this->SpatialPriorsVolumeFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->SpatialPriorsVolumeFrame->IsCreated())
    {
    this->SpatialPriorsVolumeFrame->SetParent(parent);
    this->SpatialPriorsVolumeFrame->Create();
    this->SpatialPriorsVolumeFrame->SetLabelText("Probabilistic Atlas");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
    this->SpatialPriorsVolumeFrame->GetWidgetName());

  // Create the spatial prior volume selector

  if (!this->SpatialPriorsVolumeMenuButton)
    {
    this->SpatialPriorsVolumeMenuButton = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->SpatialPriorsVolumeMenuButton->IsCreated())
    {
    this->SpatialPriorsVolumeMenuButton->SetParent(
      this->SpatialPriorsVolumeFrame->GetFrame());
    this->SpatialPriorsVolumeMenuButton->Create();
    this->SpatialPriorsVolumeMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->SpatialPriorsVolumeMenuButton->SetLabelText("Select Volume:");
    this->SpatialPriorsVolumeMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->SpatialPriorsVolumeMenuButton->SetBalloonHelpString(
      "Select volume for the selected node.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->SpatialPriorsVolumeMenuButton->GetWidgetName());

  // Update the UI with the proper value, if there is a selection

  this->DisplaySelectedNodeSpatialPriorsCallback();
}

//----------------------------------------------------------------------------
void vtkEMSegmentSpatialPriorsStep::DisplaySelectedNodeSpatialPriorsCallback()
{
  // Update the UI with the proper value, if there is a selection

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  vtkEMSegmentAnatomicalStructureStep *anat_step = 
    this->GetGUI()->GetAnatomicalStructureStep();
  vtkKWTree *tree = anat_step->GetAnatomicalStructureTree()->GetWidget();
  vtksys_stl::string sel_node;
  vtkIdType sel_vol_id = 0;
  int has_valid_selection = tree->HasSelection();
  if (has_valid_selection)
    {
    sel_node = tree->GetSelection();
    sel_vol_id = tree->GetNodeUserDataAsInt(sel_node.c_str());
    has_valid_selection = mrmlManager->GetTreeNodeIsLeaf(sel_vol_id);
    }
  char buffer[256];

  // Update the spatial prior volume selector

  if (this->SpatialPriorsVolumeMenuButton)
    {
    vtkKWMenu *menu = 
      this->SpatialPriorsVolumeMenuButton->GetWidget()->GetMenu();
    menu->DeleteAllItems();
    if (has_valid_selection)
      {
      this->SpatialPriorsVolumeMenuButton->SetEnabled(tree->GetEnabled());
      sprintf(buffer, "SpatialPriorsVolumeCallback %d", 
              static_cast<int>(sel_vol_id));
      this->PopulateMenuWithLoadedVolumes(menu, this, buffer);
      vtkIdType vol_id = mrmlManager->GetTreeNodeSpatialPriorVolumeID(sel_vol_id);
      if(!this->SetMenuButtonSelectedItem(menu, vol_id))
        {
        this->SpatialPriorsVolumeMenuButton->GetWidget()->SetValue("");
        }
      }
    else
      {
      this->SpatialPriorsVolumeMenuButton->GetWidget()->SetValue("");
      this->SpatialPriorsVolumeMenuButton->SetEnabled(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentSpatialPriorsStep::SpatialPriorsVolumeCallback(
  vtkIdType sel_vol_id, vtkIdType vol_id)
{
  // The spatial prior volume has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTreeNodeSpatialPriorVolumeID(sel_vol_id, vol_id);
}

//----------------------------------------------------------------------------
void vtkEMSegmentSpatialPriorsStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
