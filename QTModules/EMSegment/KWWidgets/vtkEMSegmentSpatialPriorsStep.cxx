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
  this->SetName("4/9. Define Atlas");
  this->SetDescription("Assign structure specific atlases to corresponding anatomy in the tree.");

  this->ImageFrame      = NULL;
  this->SpatialPriorsFrame      = NULL;
  this->SpatialPriorsVolumeFrame      = NULL;
  this->SpatialPriorsVolumeMenuButton = NULL;
  this->ParcellationVolumeMenuButton = NULL;
  this->NodeParametersLabel = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentSpatialPriorsStep::~vtkEMSegmentSpatialPriorsStep()
{
  if (this->ImageFrame) 
    {
      this->ImageFrame->Delete();
      this->ImageFrame = NULL;
    }
  if (this->SpatialPriorsVolumeMenuButton)
    {
    this->SpatialPriorsVolumeMenuButton->Delete();
    this->SpatialPriorsVolumeMenuButton = NULL;
    }

  if (this->NodeParametersLabel)
    {
    this->NodeParametersLabel->Delete();
    this->NodeParametersLabel = NULL;
    }

  if (this->SpatialPriorsVolumeFrame)
    {
    this->SpatialPriorsVolumeFrame->Delete();
    this->SpatialPriorsVolumeFrame = NULL;
    }
  if (this->SpatialPriorsFrame)
    {
    this->SpatialPriorsFrame->Delete();
    this->SpatialPriorsFrame = NULL;
    }

  if (this->ParcellationVolumeMenuButton)
    {
      this->ParcellationVolumeMenuButton->Delete();
      this->ParcellationVolumeMenuButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentSpatialPriorsStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkEMSegmentAnatomicalStructureStep *anat_step = this->GetGUI()->GetAnatomicalStructureStep();
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

  // Create the frame

  if (!this->SpatialPriorsVolumeFrame)
    {
    this->SpatialPriorsVolumeFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->SpatialPriorsVolumeFrame->IsCreated())
    {
    this->SpatialPriorsVolumeFrame->SetParent(wizard_widget->GetClientArea());
    this->SpatialPriorsVolumeFrame->Create();
    this->SpatialPriorsVolumeFrame->SetLabelText("Atlas Map");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2",
    this->SpatialPriorsVolumeFrame->GetWidgetName());

  // Create a label to display selected node

  if (!this->NodeParametersLabel)
    {
    this->NodeParametersLabel = vtkKWLabel::New();
    }
  if (!this->NodeParametersLabel->IsCreated())
    {
    this->NodeParametersLabel->SetParent(this->SpatialPriorsVolumeFrame->GetFrame());
    this->NodeParametersLabel->Create();
    this->Script(
      "pack %s -side top -anchor nw -padx 2 -pady 2",
      this->NodeParametersLabel->GetWidgetName());
    }

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
    this->SpatialPriorsVolumeMenuButton->SetLabelText("Select Probability Map:");
    this->SpatialPriorsVolumeMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->SpatialPriorsVolumeMenuButton->SetBalloonHelpString(
      "Select spatial prior volume for the selected node.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2",
    this->SpatialPriorsVolumeMenuButton->GetWidgetName());

 // Create the parcellation volume selector

  if (!this->ParcellationVolumeMenuButton)
    {
    this->ParcellationVolumeMenuButton = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->ParcellationVolumeMenuButton->IsCreated())
    {
    this->ParcellationVolumeMenuButton->SetParent(this->SpatialPriorsVolumeFrame->GetFrame());
    this->ParcellationVolumeMenuButton->Create();
    this->ParcellationVolumeMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->ParcellationVolumeMenuButton->SetLabelText("Select Parcellation Map:");
    this->ParcellationVolumeMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->ParcellationVolumeMenuButton->SetBalloonHelpString(
      "Select volume for further parcellating area the selected node.");
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2",
    this->ParcellationVolumeMenuButton->GetWidgetName());


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

  int enabled = tree->GetEnabled();
  char buffer[256];

  // Update the current tree node label

  if (this->NodeParametersLabel)
    {
    if (has_valid_selection)
      {
      this->NodeParametersLabel->SetEnabled(enabled);
      std::string nodetext("Class: " + std::string(tree->GetNodeText(sel_node.c_str())) );
      this->NodeParametersLabel->SetText( nodetext.c_str() );
      }
    else
      {
      this->NodeParametersLabel->SetEnabled(0);
      this->NodeParametersLabel->SetText("");
      }
    }

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
      vtkEMSegmentGUI::PopulateMenuWithLoadedVolumes(mrmlManager,menu, this, buffer);
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

   if (this->ParcellationVolumeMenuButton)
    {
      vtkKWMenu *menu =  this->ParcellationVolumeMenuButton->GetWidget()->GetMenu();
      menu->DeleteAllItems();
      if (has_valid_selection)
      {
        this->ParcellationVolumeMenuButton->SetEnabled(tree->GetEnabled());
        sprintf(buffer, "ParcellationVolumeCallback %d", 
              static_cast<int>(sel_vol_id));
        vtkEMSegmentGUI::PopulateMenuWithLoadedVolumes(mrmlManager,menu, this, buffer);
        vtkIdType vol_id  = mrmlManager->GetTreeNodeSubParcellationVolumeID(sel_vol_id);
        if(!this->SetMenuButtonSelectedItem(menu, vol_id))
         {
          this->ParcellationVolumeMenuButton->GetWidget()->SetValue("");
         }
      }
    else
      {
      this->ParcellationVolumeMenuButton->GetWidget()->SetValue("");
      this->ParcellationVolumeMenuButton->SetEnabled(0);
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
void vtkEMSegmentSpatialPriorsStep::ParcellationVolumeCallback(
  vtkIdType sel_vol_id, vtkIdType vol_id)
{
  // The parcellation volume has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }
  mrmlManager->SetTreeNodeSubParcellationVolumeID(sel_vol_id, vol_id);
}


//----------------------------------------------------------------------------
void vtkEMSegmentSpatialPriorsStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
