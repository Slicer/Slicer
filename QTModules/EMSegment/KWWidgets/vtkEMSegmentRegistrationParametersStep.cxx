#include "vtkEMSegmentRegistrationParametersStep.h"

#include "vtkEMSegmentGUI.h"
#include "vtkEMSegmentMRMLManager.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWWizardStep.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"
#include "vtkMRMLEMSGlobalParametersNode.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkEMSegmentRegistrationParametersStep);
vtkCxxRevisionMacro(vtkEMSegmentRegistrationParametersStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkEMSegmentRegistrationParametersStep::vtkEMSegmentRegistrationParametersStep()
{
  this->SetName("5/9. Edit Registration Parameters");
  this->RegistrationParametersAtlasImageMenuButton.clear();
  this->SetDescription("Specify atlas-to-input scans registration parameters.");

  this->RegistrationParametersFrame                   = NULL;
  this->RegistrationParametersAffineMenuButton        = NULL;
  this->RegistrationParametersDeformableMenuButton    = NULL;
  this->RegistrationParametersInterpolationMenuButton = NULL;
  this->RegistrationParametersPackageMenuButton       = NULL;
  this->RegistrationAtlasInputFrame = NULL;
}

//----------------------------------------------------------------------------
vtkEMSegmentRegistrationParametersStep::~vtkEMSegmentRegistrationParametersStep()
{
  for (int i = 0 ; i < (int) this->RegistrationParametersAtlasImageMenuButton.size(); i++)
    {      
      if (this->RegistrationParametersAtlasImageMenuButton[i])
    {
      this->RegistrationParametersAtlasImageMenuButton[i]->Delete();
      this->RegistrationParametersAtlasImageMenuButton[i] = NULL;
    }
    }
  this->RegistrationParametersAtlasImageMenuButton.clear();

  if (this->RegistrationParametersAffineMenuButton)
    {
    this->RegistrationParametersAffineMenuButton->SetParent(NULL);
    this->RegistrationParametersAffineMenuButton->Delete();
    this->RegistrationParametersAffineMenuButton = NULL;
    }

  if (this->RegistrationParametersDeformableMenuButton)
    {
    this->RegistrationParametersDeformableMenuButton->SetParent(NULL);
    this->RegistrationParametersDeformableMenuButton->Delete();
    this->RegistrationParametersDeformableMenuButton = NULL;
    }

  if (this->RegistrationParametersInterpolationMenuButton)
    {
    this->RegistrationParametersInterpolationMenuButton->SetParent(NULL);
    this->RegistrationParametersInterpolationMenuButton->Delete();
    this->RegistrationParametersInterpolationMenuButton = NULL;
    }

  if (this->RegistrationParametersPackageMenuButton)
    {
    this->RegistrationParametersPackageMenuButton->SetParent(NULL);
    this->RegistrationParametersPackageMenuButton->Delete();
    this->RegistrationParametersPackageMenuButton = NULL;
    }

  if (this->RegistrationParametersFrame)
    {
    this->RegistrationParametersFrame->SetParent(NULL);
    this->RegistrationParametersFrame->Delete();
    this->RegistrationParametersFrame = NULL;
    }

  if (this->RegistrationAtlasInputFrame)
    {
      this->RegistrationAtlasInputFrame->SetParent(NULL);
      this->RegistrationAtlasInputFrame->Delete();
      this->RegistrationAtlasInputFrame = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();
  if (!mrmlManager || !wizard_widget)
    {
    return;
    }
  wizard_widget->GetCancelButton()->SetEnabled(0);

  vtkKWWidget *parent = wizard_widget->GetClientArea();

  // Create the frame
  if (!this->RegistrationParametersFrame)
    {
    this->RegistrationParametersFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->RegistrationParametersFrame->IsCreated())
    {
    this->RegistrationParametersFrame->SetParent(parent);
    this->RegistrationParametersFrame->Create();
    this->RegistrationParametersFrame->SetLabelText(
      "Atlas-to-Input Registration Parameters");
    }
  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
    this->RegistrationParametersFrame->GetWidgetName());

  char buffer[256];


 if (!this->RegistrationAtlasInputFrame)
    {
    this->RegistrationAtlasInputFrame = vtkKWFrame::New();
    }
  if (!this->RegistrationAtlasInputFrame->IsCreated())
    {
      this->RegistrationAtlasInputFrame->SetParent(this->RegistrationParametersFrame->GetFrame());
    this->RegistrationAtlasInputFrame->Create();
    }
  this->Script(
    "pack %s -side top -anchor nw -fill x -padx 0 -pady 2", 
    this->RegistrationAtlasInputFrame->GetWidgetName());

  int enabled = parent->GetEnabled();
  // Create the atlas image volume selector
  this->AssignAtlasScansToInputChannels( enabled);

  // Create the affine registration menu button

  if (!this->RegistrationParametersAffineMenuButton)
    {
    this->RegistrationParametersAffineMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->RegistrationParametersAffineMenuButton->IsCreated())
    {
    this->RegistrationParametersAffineMenuButton->SetParent(
      this->RegistrationParametersFrame->GetFrame());
    this->RegistrationParametersAffineMenuButton->Create();
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->RegistrationParametersAffineMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RegistrationParametersAffineMenuButton->
      SetLabelText("Affine Registration:");

    sprintf(buffer, "RegistrationAffineCallback %d",
            vtkEMSegmentMRMLManager::RegistrationOff);
    this->RegistrationParametersAffineMenuButton->GetWidget()->
      GetMenu()->AddRadioButton("None", this, buffer);

    sprintf(buffer, "RegistrationAffineCallback %d", vtkEMSegmentMRMLManager::RegistrationFast);
    this->RegistrationParametersAffineMenuButton->GetWidget()->GetMenu()->AddRadioButton("Fast", this, buffer);
 
    sprintf(buffer, "RegistrationAffineCallback %d", vtkEMSegmentMRMLManager::RegistrationSlow);
    this->RegistrationParametersAffineMenuButton->GetWidget()->GetMenu()->AddRadioButton("Accurate", this, buffer);

    this->RegistrationParametersAffineMenuButton->SetBalloonHelpString(
      "Select affine registration.");
    }

  sprintf(buffer, "%d", mrmlManager->GetRegistrationAffineType());
  this->RegistrationParametersAffineMenuButton->GetWidget()->SetValue(buffer);
  this->RegistrationParametersAffineMenuButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);
  
  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersAffineMenuButton->GetWidgetName());

  // Create the deformable registration menu button

  if (!this->RegistrationParametersDeformableMenuButton)
    {
    this->RegistrationParametersDeformableMenuButton = 
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->RegistrationParametersDeformableMenuButton->IsCreated())
    {
    this->RegistrationParametersDeformableMenuButton->SetParent(
      this->RegistrationParametersFrame->GetFrame());
    this->RegistrationParametersDeformableMenuButton->Create();
    this->RegistrationParametersDeformableMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->RegistrationParametersDeformableMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RegistrationParametersDeformableMenuButton->SetLabelText("Deformable Registration:");

    sprintf(buffer, "RegistrationDeformableCallback %d", vtkEMSegmentMRMLManager::RegistrationOff);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->GetMenu()->AddRadioButton("None", this, buffer);
    sprintf(buffer, "RegistrationDeformableCallback %d", vtkEMSegmentMRMLManager::RegistrationFast);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->GetMenu()->AddRadioButton("Fast", this, buffer);
    sprintf(buffer, "RegistrationDeformableCallback %d",vtkEMSegmentMRMLManager::RegistrationSlow);
    this->RegistrationParametersDeformableMenuButton->GetWidget()->GetMenu()->AddRadioButton("Accurate", this, buffer);

    this->RegistrationParametersDeformableMenuButton->SetBalloonHelpString("Select deformable registration.");

    }

  sprintf(buffer, "%d", mrmlManager->GetRegistrationDeformableType());
  this->RegistrationParametersDeformableMenuButton->GetWidget()->SetValue(
    buffer);
  this->RegistrationParametersDeformableMenuButton->SetEnabled(
    mrmlManager->HasGlobalParametersNode() ? enabled : 0);

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersDeformableMenuButton->GetWidgetName());
  
  // Create the interpolation menu button

  if (!this->RegistrationParametersInterpolationMenuButton)
    {
    this->RegistrationParametersInterpolationMenuButton =
      vtkKWMenuButtonWithLabel::New();
    }
  if (!this->RegistrationParametersInterpolationMenuButton->IsCreated())
    {
    this->RegistrationParametersInterpolationMenuButton->SetParent(
      this->RegistrationParametersFrame->GetFrame());
    this->RegistrationParametersInterpolationMenuButton->Create();
    this->RegistrationParametersInterpolationMenuButton->GetWidget()->
      SetWidth(EMSEG_MENU_BUTTON_WIDTH);
    this->RegistrationParametersInterpolationMenuButton->GetLabel()->
      SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
    this->RegistrationParametersInterpolationMenuButton->
      SetLabelText("Interpolation:");
    this->RegistrationParametersInterpolationMenuButton->SetBalloonHelpString(
      "Select interpolation type.");

    sprintf(buffer, "RegistrationInterpolationCallback %d", 
            vtkEMSegmentMRMLManager::InterpolationNearestNeighbor);
    this->RegistrationParametersInterpolationMenuButton->
      GetWidget()->GetMenu()->AddRadioButton("Nearest Neighbor", this, buffer);
    sprintf(buffer, "RegistrationInterpolationCallback %d", 
            vtkEMSegmentMRMLManager::InterpolationLinear);
    this->RegistrationParametersInterpolationMenuButton->
      GetWidget()->GetMenu()->AddRadioButton("Linear", this, buffer);
    // no cubic for now
    //sprintf(buffer, "RegistrationInterpolationCallback %d", 
    //        vtkEMSegmentMRMLManager::InterpolationCubic);
    //this->RegistrationParametersInterpolationMenuButton->
    //  GetWidget()->GetMenu()->AddRadioButton("Cubic", this, buffer);
    }

  this->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 2", 
    this->RegistrationParametersInterpolationMenuButton->GetWidgetName());

    {
    vtksys_stl::string value;
    int v = mrmlManager->GetRegistrationInterpolationType();
    if (v == vtkEMSegmentMRMLManager::InterpolationNearestNeighbor)
      {
      value = "Nearest Neighbor";
      }
    else if (v == vtkEMSegmentMRMLManager::InterpolationLinear)
      {
      value = "Linear";
      }
    else if (v == vtkEMSegmentMRMLManager::InterpolationCubic)
      {
      value = "Cubic";
      }
    this->RegistrationParametersInterpolationMenuButton->
      GetWidget()->SetValue(value.c_str());
    this->RegistrationParametersInterpolationMenuButton->
      SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
    }


    // Create the package menu button

    if (!this->RegistrationParametersPackageMenuButton)
      {
      this->RegistrationParametersPackageMenuButton =
        vtkKWMenuButtonWithLabel::New();
      }
    if (!this->RegistrationParametersPackageMenuButton->IsCreated())
      {
      this->RegistrationParametersPackageMenuButton->SetParent(
        this->RegistrationParametersFrame->GetFrame());
      this->RegistrationParametersPackageMenuButton->Create();
      this->RegistrationParametersPackageMenuButton->GetWidget()->
        SetWidth(EMSEG_MENU_BUTTON_WIDTH);
      this->RegistrationParametersPackageMenuButton->GetLabel()->
        SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
      this->RegistrationParametersPackageMenuButton->
        SetLabelText("Package:");
      this->RegistrationParametersPackageMenuButton->SetBalloonHelpString(
        "Select interpolation package.");

      sprintf(buffer, "RegistrationPackageCallback %d",
              vtkEMSegmentMRMLManager::CMTK);
      this->RegistrationParametersPackageMenuButton->
        GetWidget()->GetMenu()->AddRadioButton("CMTK", this, buffer);

      sprintf(buffer, "RegistrationPackageCallback %d",
              vtkEMSegmentMRMLManager::BRAINS);
      this->RegistrationParametersPackageMenuButton->
        GetWidget()->GetMenu()->AddRadioButton("BRAINS", this, buffer);

      sprintf(buffer, "RegistrationPackageCallback %d",
              vtkEMSegmentMRMLManager::PLASTIMATCH);
      this->RegistrationParametersPackageMenuButton->
        GetWidget()->GetMenu()->AddRadioButton("PLASTIMATCH", this, buffer);

      sprintf(buffer, "RegistrationPackageCallback %d",
              vtkEMSegmentMRMLManager::DEMONS);
      this->RegistrationParametersPackageMenuButton->
        GetWidget()->GetMenu()->AddRadioButton("DEMONS", this, buffer);

      sprintf(buffer, "RegistrationPackageCallback %d",
              vtkEMSegmentMRMLManager::DRAMMS);
      this->RegistrationParametersPackageMenuButton->
        GetWidget()->GetMenu()->AddRadioButton("DRAMMS", this, buffer);

      sprintf(buffer, "RegistrationPackageCallback %d",
              vtkEMSegmentMRMLManager::ANTS);
      this->RegistrationParametersPackageMenuButton->
        GetWidget()->GetMenu()->AddRadioButton("ANTS", this, buffer);

      }


    this->Script(
      "pack %s -side top -anchor nw -padx 2 -pady 2",
      this->RegistrationParametersPackageMenuButton->GetWidgetName());

      {
      vtksys_stl::string value;
      int v = mrmlManager->GetRegistrationPackageType();
      switch(v)
      {
      case vtkEMSegmentMRMLManager::CMTK:
        {
        value = "CMTK";
        break;
        }
      case vtkEMSegmentMRMLManager::BRAINS:
        {
        value = "BRAINS";
        break;
        }
      case vtkEMSegmentMRMLManager::PLASTIMATCH:
        {
        value = "PLASTIMATCH";
        break;
        }
      case vtkEMSegmentMRMLManager::DRAMMS:
        {
        value = "DRAMMS";
        break;
        }
      case vtkEMSegmentMRMLManager::ANTS:
        {
        value = "ANTS";
        break;
        }
      default:
        value = "BRAINS";
      }
      this->RegistrationParametersPackageMenuButton->
        GetWidget()->SetValue(value.c_str());
      this->RegistrationParametersPackageMenuButton->
        SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
      }


    {
      int v = mrmlManager->GetRegistrationAffineType();
      vtksys_stl::string value = RegistrationTypeValueToString(v);
      this->RegistrationParametersAffineMenuButton->GetWidget()->SetValue(value.c_str());
      this->RegistrationParametersAffineMenuButton->SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
    }

    {
      int v = mrmlManager->GetRegistrationDeformableType();
      vtksys_stl::string value = RegistrationTypeValueToString(v);
      this->RegistrationParametersDeformableMenuButton->GetWidget()->SetValue(value.c_str());
      this->RegistrationParametersDeformableMenuButton->SetEnabled(mrmlManager->HasGlobalParametersNode() ? enabled : 0);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationAtlasImageCallback(
  vtkIdType volume_id)
{
  cout << "What RegistrationAtlasImageCallback " << endl;
  // The atlas image has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetRegistrationAtlasVolumeID(volume_id);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationAffineCallback(
  int type)
{
  // The affine registration type has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetRegistrationAffineType(type);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationDeformableCallback(
  int type)
{
  // The deformable registration type has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetRegistrationDeformableType(type);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationInterpolationCallback(
  int type)
{
  // The interpolation type has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetRegistrationInterpolationType(type);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationPackageCallback(
  int type)
{
  // The package type has changed because of user interaction

  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager)
    {
    mrmlManager->SetRegistrationPackageType(type);
    }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::AssignAtlasScansToInputChannels(int enabled) 
{
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  vtkMRMLEMSVolumeCollectionNode *inputNodes = mrmlManager->GetTargetInputNode();
  vtkMRMLEMSGlobalParametersNode* globalNode = mrmlManager->GetGlobalParametersNode();

  if (!inputNodes || !globalNode) 
    {
      return;
    }

  int oldSize = this->RegistrationParametersAtlasImageMenuButton.size();
  int newSize = mrmlManager->GetTargetNumberOfSelectedVolumes();

  if (oldSize > newSize)
    {
      for (int i = newSize ; i < oldSize; i++) 
    {
      if (this->RegistrationParametersAtlasImageMenuButton[i])
        {
      this->RegistrationParametersAtlasImageMenuButton[i]->Unpack();
          this->RegistrationParametersAtlasImageMenuButton[i]->Delete();
      this->RegistrationParametersAtlasImageMenuButton[i] = NULL;
        }
    }
    }

  this->RegistrationParametersAtlasImageMenuButton.resize(newSize);
  
  if (oldSize < newSize)
    {
      for (int i = oldSize ; i < newSize; i++) 
    {
      this->RegistrationParametersAtlasImageMenuButton[i] = NULL;
    }
    }
  for (int i = 0 ; i < newSize; i++)
    {      
      if (!this->RegistrationParametersAtlasImageMenuButton[i])
    {
      this->RegistrationParametersAtlasImageMenuButton[i] =  vtkKWMenuButtonWithLabel::New();
    }
      if (!this->RegistrationParametersAtlasImageMenuButton[i]->IsCreated())
    {
      this->RegistrationParametersAtlasImageMenuButton[i]->SetParent(this->RegistrationAtlasInputFrame);
      this->RegistrationParametersAtlasImageMenuButton[i]->Create();
      this->RegistrationParametersAtlasImageMenuButton[i]->GetWidget()->SetWidth(EMSEG_MENU_BUTTON_WIDTH);
      this->RegistrationParametersAtlasImageMenuButton[i]->GetLabel()->SetWidth(EMSEG_WIDGETS_LABEL_WIDTH);
      this->RegistrationParametersAtlasImageMenuButton[i]->SetLabelText(globalNode->GetNthTargetInputChannelName(i));
      this->RegistrationParametersAtlasImageMenuButton[i]->SetBalloonHelpString("Select atlas volume representing the --- channel.");
    }
      this->Script( "pack %s -side top -anchor nw -padx 2 -pady 2", this->RegistrationParametersAtlasImageMenuButton[i]->GetWidgetName());
      
      std::stringstream setCmd;
      setCmd << "RegistrationAtlasImageCallback " << i;
      vtkEMSegmentGUI::PopulateMenuWithLoadedVolumes(mrmlManager, this->RegistrationParametersAtlasImageMenuButton[i]->GetWidget()->GetMenu(), this,setCmd.str().c_str());
      
      if(!mrmlManager->GetVolumeNumberOfChoices() ||  !mrmlManager->ExistRegistrationAtlasVolumeKey(i) || !this->SetMenuButtonSelectedItem(this->RegistrationParametersAtlasImageMenuButton[i]->GetWidget()->GetMenu(), mrmlManager->GetRegistrationAtlasVolumeID(i)))
    {
      this->RegistrationParametersAtlasImageMenuButton[i]->GetWidget()->SetValue("");
    }
      this->RegistrationParametersAtlasImageMenuButton[i]->SetEnabled(mrmlManager->GetVolumeNumberOfChoices() ? enabled : 0);      
   }
}

//----------------------------------------------------------------------------
void vtkEMSegmentRegistrationParametersStep::RegistrationAtlasImageCallback(vtkIdType input_id,  vtkIdType volume_id)
{
  // The atlas image has changed because of user interaction
  vtkEMSegmentMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (mrmlManager && mrmlManager->IDMapContainsVTKNodeID(volume_id) )
    {
      mrmlManager->SetRegistrationAtlasVolumeID(input_id, volume_id);
    } 
}

//----------------------------------------------------------------------------
vtkstd::string vtkEMSegmentRegistrationParametersStep::RegistrationTypeValueToString(int v)
{
    vtkstd::string value = "None";
    if (v == vtkEMSegmentMRMLManager::RegistrationFast)
      {
      value = "Fast";
      }
    else if (v == vtkEMSegmentMRMLManager::RegistrationSlow)
      {
      value = "Accurate";
      }
    return value;
} 

