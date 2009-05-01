#include "vtkIGTNavigationRegistrationStep.h"

#include "vtkIGTNavigationGUI.h"
#include "vtkIGTNavigationMRMLManager.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWEntry.h"
#include "vtkMRMLFiducialListNode.h"



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkIGTNavigationRegistrationStep);
vtkCxxRevisionMacro(vtkIGTNavigationRegistrationStep, "$Revision: 1.9 $");

//----------------------------------------------------------------------------
vtkIGTNavigationRegistrationStep::vtkIGTNavigationRegistrationStep()
{
  this->SetName("4/5. Registration");
  this->SetDescription("Register coordinate systems");

  this->ImageFiducialFrame = NULL;
  this->ImageFiducialMenuButton1 = NULL;
  this->ImageFiducialMenuButton2 = NULL;
  this->ImageFiducialMenuButton3 = NULL;
  this->ImageFiducialMenuButton4 = NULL;

  this->ImageFiducialTextBox1 = NULL;
  this->ImageFiducialTextBox2 = NULL;
  this->ImageFiducialTextBox3 = NULL;
  this->ImageFiducialTextBox4 = NULL;
  
  this->TrackerFiducialFrame = NULL;
  this->TrackerFiducialPushButton1 = NULL;
  this->TrackerFiducialPushButton2 = NULL;
  this->TrackerFiducialPushButton3 = NULL;
  this->TrackerFiducialPushButton4 = NULL;

  this->TrackerFiducialTextBox1 = NULL;
  this->TrackerFiducialTextBox2 = NULL;
  this->TrackerFiducialTextBox3 = NULL;
  this->TrackerFiducialTextBox4 = NULL;

  this->RunRegistrationPushButton = NULL;

}

//----------------------------------------------------------------------------
vtkIGTNavigationRegistrationStep::~vtkIGTNavigationRegistrationStep()
{
  if( this->ImageFiducialFrame)
    {
    this->ImageFiducialFrame->Delete();
    this->ImageFiducialFrame = NULL;
    }

  if( this->TrackerFiducialFrame)
    {
    this->TrackerFiducialFrame->Delete();
    this->TrackerFiducialFrame = NULL;
    }

  if( this->ImageFiducialMenuButton1)
    {
    this->ImageFiducialMenuButton1->Delete();
    this->ImageFiducialMenuButton1 = NULL;
    }

  if( this->ImageFiducialMenuButton2)
    {
    this->ImageFiducialMenuButton2->Delete();
    this->ImageFiducialMenuButton2 = NULL;
    }
  if( this->ImageFiducialMenuButton3)
    {
    this->ImageFiducialMenuButton3->Delete();
    this->ImageFiducialMenuButton3 = NULL;
    }

  if( this->ImageFiducialMenuButton4)
    {
    this->ImageFiducialMenuButton4->Delete();
    this->ImageFiducialMenuButton4 = NULL;
    }

  if( this->ImageFiducialTextBox1)
    {
    this->ImageFiducialTextBox1->Delete();
    this->ImageFiducialTextBox1 = NULL;
    }

  if( this->ImageFiducialTextBox2)
    {
    this->ImageFiducialTextBox2->Delete();
    this->ImageFiducialTextBox2 = NULL;
    }

   if( this->ImageFiducialTextBox3)
    {
    this->ImageFiducialTextBox3->Delete();
    this->ImageFiducialTextBox3 = NULL;
    }

   if( this->ImageFiducialTextBox4)
    {
    this->ImageFiducialTextBox4->Delete();
    this->ImageFiducialTextBox4 = NULL;
    }
 
  if( this->TrackerFiducialPushButton1)
    {
    this->TrackerFiducialPushButton1->Delete();
    this->TrackerFiducialPushButton1 = NULL;
    }
  if( this->TrackerFiducialPushButton2)
    {
    this->TrackerFiducialPushButton2->Delete();
    this->TrackerFiducialPushButton2 = NULL;
    }
  if( this->TrackerFiducialPushButton3)
    {
    this->TrackerFiducialPushButton3->Delete();
    this->TrackerFiducialPushButton3 = NULL;
    }
  if( this->TrackerFiducialPushButton4)
    {
    this->TrackerFiducialPushButton4->Delete();
    this->TrackerFiducialPushButton4 = NULL;
    }

  if( this->TrackerFiducialTextBox1)
    {
    this->TrackerFiducialTextBox1->Delete();
    this->TrackerFiducialTextBox1 = NULL;
    }

  if( this->TrackerFiducialTextBox2)
    {
    this->TrackerFiducialTextBox2->Delete();
    this->TrackerFiducialTextBox2 = NULL;
    }

  if( this->TrackerFiducialTextBox3)
    {
    this->TrackerFiducialTextBox3->Delete();
    this->TrackerFiducialTextBox3 = NULL;
    }

  if( this->TrackerFiducialTextBox4)
    {
    this->TrackerFiducialTextBox4->Delete();
    this->TrackerFiducialTextBox4 = NULL;
    }

  if( this->RunRegistrationPushButton)
    {
    this->RunRegistrationPushButton->Delete();
    this->RunRegistrationPushButton = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkIGTNavigationRegistrationStep::ShowUserInterface()
{
  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizard_widget = this->GetGUI()->GetWizardWidget();

  wizard_widget->GetCancelButton()->SetEnabled(0);

  if (!this->ImageFiducialFrame)
    {
    this->ImageFiducialFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->ImageFiducialFrame->IsCreated())
    {
    this->ImageFiducialFrame->SetParent(
      wizard_widget->GetClientArea());
    this->ImageFiducialFrame->Create();
    this->ImageFiducialFrame->SetLabelText("Image Fiducial Coordinates"); 
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->ImageFiducialFrame->GetWidgetName());

  // Create menus to select fiducials 
  // First fiducial
  if (!this->ImageFiducialMenuButton1)
    {
    this->ImageFiducialMenuButton1 = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->ImageFiducialMenuButton1->IsCreated())
    {
    this->ImageFiducialMenuButton1->SetParent(
      this->ImageFiducialFrame->GetFrame());
    this->ImageFiducialMenuButton1->Create();
    this->ImageFiducialMenuButton1->GetLabel()->SetWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    this->ImageFiducialMenuButton1->SetLabelText("Image Fiducial 1:");
    this->ImageFiducialMenuButton1->GetWidget()->SetWidth(
      EMSEG_MENU_BUTTON_WIDTH + 10);
    this->ImageFiducialMenuButton1->SetBalloonHelpString(
      "Select image fiducial 1 .");
    }

  this->Script("grid %s -column 0 -row 0 -sticky nw -padx 2 -pady 2", 
               this->ImageFiducialMenuButton1->GetWidgetName());
 
  // Entry text book for the first fiducial 
  if (!this->ImageFiducialTextBox1)
    {
    this->ImageFiducialTextBox1 = vtkKWEntry::New();
    }
  if (!this->ImageFiducialTextBox1->IsCreated())
    {
    this->ImageFiducialTextBox1->SetParent(
      this->ImageFiducialFrame->GetFrame());
    this->ImageFiducialTextBox1->Create();
    this->ImageFiducialTextBox1->ReadOnlyOn();
    }
  this->Script("grid %s -column 1 -row 0 -sticky nw -padx 2 -pady 2", 
               this->ImageFiducialTextBox1->GetWidgetName());

  // Second fiducial 
  if (!this->ImageFiducialMenuButton2)
    {
    this->ImageFiducialMenuButton2 = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->ImageFiducialMenuButton2->IsCreated())
    {
    this->ImageFiducialMenuButton2->SetParent(
      this->ImageFiducialFrame->GetFrame());
    this->ImageFiducialMenuButton2->Create();
    this->ImageFiducialMenuButton2->GetLabel()->SetWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    this->ImageFiducialMenuButton2->SetLabelText("Image Fiducial 2:");
    this->ImageFiducialMenuButton2->GetWidget()->SetWidth(
      EMSEG_MENU_BUTTON_WIDTH + 10);
    this->ImageFiducialMenuButton2->SetBalloonHelpString(
      "Select image fiducial 1 .");
    }

  this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 2", 
               this->ImageFiducialMenuButton2->GetWidgetName());
 
  // Entry text book for the second fiducial 
  if (!this->ImageFiducialTextBox2)
    {
    this->ImageFiducialTextBox2 = vtkKWEntry::New();
    }
  if (!this->ImageFiducialTextBox2->IsCreated())
    {
    this->ImageFiducialTextBox2->SetParent(
      this->ImageFiducialFrame->GetFrame());
    this->ImageFiducialTextBox2->Create();
    this->ImageFiducialTextBox2->ReadOnlyOn();
    }
  this->Script("grid %s -column 1 -row 1 -sticky nw -padx 2 -pady 2", 
               this->ImageFiducialTextBox2->GetWidgetName());


  // Third fiducial 
  if (!this->ImageFiducialMenuButton3)
    {
    this->ImageFiducialMenuButton3 = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->ImageFiducialMenuButton3->IsCreated())
    {
    this->ImageFiducialMenuButton3->SetParent(
      this->ImageFiducialFrame->GetFrame());
    this->ImageFiducialMenuButton3->Create();
    this->ImageFiducialMenuButton3->GetLabel()->SetWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    this->ImageFiducialMenuButton3->SetLabelText("Image Fiducial 3:");
    this->ImageFiducialMenuButton3->GetWidget()->SetWidth(
      EMSEG_MENU_BUTTON_WIDTH + 10);
    this->ImageFiducialMenuButton3->SetBalloonHelpString(
      "Select image fiducial 1 .");
    }

  this->Script("grid %s -column 0 -row 2 -sticky nw -padx 2 -pady 2", 
               this->ImageFiducialMenuButton3->GetWidgetName());
 
  // Entry text book for the third fiducial 
  if (!this->ImageFiducialTextBox3)
    {
    this->ImageFiducialTextBox3 = vtkKWEntry::New();
    }
  if (!this->ImageFiducialTextBox3->IsCreated())
    {
    this->ImageFiducialTextBox3->SetParent(
      this->ImageFiducialFrame->GetFrame());
    this->ImageFiducialTextBox3->Create();
    this->ImageFiducialTextBox3->ReadOnlyOn();
    }
  this->Script("grid %s -column 1 -row 2 -sticky nw -padx 2 -pady 2", 
               this->ImageFiducialTextBox3->GetWidgetName());

  // Fourth fiducial 
  if (!this->ImageFiducialMenuButton4)
    {
    this->ImageFiducialMenuButton4 = vtkKWMenuButtonWithLabel::New();
    }
  if (!this->ImageFiducialMenuButton4->IsCreated())
    {
    this->ImageFiducialMenuButton4->SetParent(
      this->ImageFiducialFrame->GetFrame());
    this->ImageFiducialMenuButton4->Create();
    this->ImageFiducialMenuButton4->GetLabel()->SetWidth(
      EMSEG_WIDGETS_LABEL_WIDTH - 10);
    this->ImageFiducialMenuButton4->SetLabelText("Image Fiducial 4:");
    this->ImageFiducialMenuButton4->GetWidget()->SetWidth(
      EMSEG_MENU_BUTTON_WIDTH + 10);
    this->ImageFiducialMenuButton4->SetBalloonHelpString(
      "Select image fiducial 1 .");
    }

  this->Script("grid %s -column 0 -row 3 -sticky nw -padx 2 -pady 2", 
               this->ImageFiducialMenuButton4->GetWidgetName());
 
  // Entry text book for the fourth fiducial 
  if (!this->ImageFiducialTextBox4)
    {
    this->ImageFiducialTextBox4 = vtkKWEntry::New();
    }
  if (!this->ImageFiducialTextBox4->IsCreated())
    {
    this->ImageFiducialTextBox4->SetParent(
      this->ImageFiducialFrame->GetFrame());
    this->ImageFiducialTextBox4->Create();
    this->ImageFiducialTextBox4->ReadOnlyOn();
    }
  this->Script("grid %s -column 1 -row 3 -sticky nw -padx 2 -pady 2", 
               this->ImageFiducialTextBox4->GetWidgetName());

  this->PopulateFiducialPointsSelector( ImageFiducialMenuButton1, "FiducialPointSelectorCallback1");
  this->PopulateFiducialPointsSelector( ImageFiducialMenuButton2, "FiducialPointSelectorCallback2");
  this->PopulateFiducialPointsSelector( ImageFiducialMenuButton3, "FiducialPointSelectorCallback3");
  this->PopulateFiducialPointsSelector( ImageFiducialMenuButton4, "FiducialPointSelectorCallback4");

  if (!this->TrackerFiducialFrame)
    {
    this->TrackerFiducialFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->TrackerFiducialFrame->IsCreated())
    {
    this->TrackerFiducialFrame->SetParent(
      wizard_widget->GetClientArea());
    this->TrackerFiducialFrame->Create();
    this->TrackerFiducialFrame->SetLabelText("Tracker Fiducial Coordinates"); 
    }

  this->Script("pack %s -side top -expand n -fill both -padx 0 -pady 2", 
               this->TrackerFiducialFrame->GetWidgetName());
 
  // Create push buttons to gather fiducial coordinates in tracker coordinate
  // system 
  //
  // Tracker coordinate for fiducial 1
  if (!this->TrackerFiducialPushButton1)
    {
    this->TrackerFiducialPushButton1 = vtkKWPushButton::New();
    }
  if (!this->TrackerFiducialPushButton1->IsCreated())
    {
    this->TrackerFiducialPushButton1->SetParent(
      this->TrackerFiducialFrame->GetFrame());
    this->TrackerFiducialPushButton1->Create();
    this->TrackerFiducialPushButton1->SetText("Collect Tracker Coordinate Fiducial 1");
    this->TrackerFiducialPushButton1->SetBalloonHelpString(
      "Capture Tracker coordinate of fiducial 1");
    }
  this->Script("grid %s -column 0 -row 0 -sticky nw -padx 2 -pady 2", 
               this->TrackerFiducialPushButton1->GetWidgetName());

  // Entry text book for the first fiducial 
  if (!this->TrackerFiducialTextBox1)
    {
    this->TrackerFiducialTextBox1 = vtkKWEntry::New();
    }
  if (!this->TrackerFiducialTextBox1->IsCreated())
    {
    this->TrackerFiducialTextBox1->SetParent(
      this->TrackerFiducialFrame->GetFrame());
    this->TrackerFiducialTextBox1->Create();
    this->TrackerFiducialTextBox1->ReadOnlyOn();
    }
  this->Script("grid %s -column 1 -row 0 -sticky nw -padx 2 -pady 2", 
               this->TrackerFiducialTextBox1->GetWidgetName());

  // Tracker coordinate for fiducial 2
  if (!this->TrackerFiducialPushButton2)
    {
    this->TrackerFiducialPushButton2 = vtkKWPushButton::New();
    }
  if (!this->TrackerFiducialPushButton2->IsCreated())
    {
    this->TrackerFiducialPushButton2->SetParent(
      this->TrackerFiducialFrame->GetFrame());
    this->TrackerFiducialPushButton2->Create();
    this->TrackerFiducialPushButton2->SetText("Collect Tracker Coordinate Fiducial 2");
    this->TrackerFiducialPushButton2->SetBalloonHelpString(
      "Capture Tracker coordinate of fiducial 2");
    }
  this->Script("grid %s -column 0 -row 1 -sticky nw -padx 2 -pady 2", 
               this->TrackerFiducialPushButton2->GetWidgetName());

  // Entry text book for the first fiducial 
  if (!this->TrackerFiducialTextBox2)
    {
    this->TrackerFiducialTextBox2 = vtkKWEntry::New();
    }
  if (!this->TrackerFiducialTextBox2->IsCreated())
    {
    this->TrackerFiducialTextBox2->SetParent(
      this->TrackerFiducialFrame->GetFrame());
    this->TrackerFiducialTextBox2->Create();
    this->TrackerFiducialTextBox2->ReadOnlyOn();
    }
  this->Script("grid %s -column 1 -row 1 -sticky nw -padx 2 -pady 2", 
               this->TrackerFiducialTextBox2->GetWidgetName());

  // Tracker coordinate for fiducial 3
  if (!this->TrackerFiducialPushButton3)
    {
    this->TrackerFiducialPushButton3 = vtkKWPushButton::New();
    }
  if (!this->TrackerFiducialPushButton3->IsCreated())
    {
    this->TrackerFiducialPushButton3->SetParent(
      this->TrackerFiducialFrame->GetFrame());
    this->TrackerFiducialPushButton3->Create();
    this->TrackerFiducialPushButton3->SetText("Collect Tracker Coordinate Fiducial 3");
    this->TrackerFiducialPushButton3->SetBalloonHelpString(
      "Capture Tracker coordinate of fiducial 1");
    }
  this->Script("grid %s -column 0 -row 2 -sticky nw -padx 2 -pady 2", 
               this->TrackerFiducialPushButton3->GetWidgetName());

 // Entry text book for the third fiducial 
  if (!this->TrackerFiducialTextBox3)
    {
    this->TrackerFiducialTextBox3 = vtkKWEntry::New();
    }
  if (!this->TrackerFiducialTextBox3->IsCreated())
    {
    this->TrackerFiducialTextBox3->SetParent(
      this->TrackerFiducialFrame->GetFrame());
    this->TrackerFiducialTextBox3->Create();
    this->TrackerFiducialTextBox3->ReadOnlyOn();
    }
  this->Script("grid %s -column 1 -row 2 -sticky nw -padx 2 -pady 2", 
               this->TrackerFiducialTextBox3->GetWidgetName());


  // Tracker coordinate for fiducial 4
  if (!this->TrackerFiducialPushButton4)
    {
    this->TrackerFiducialPushButton4 = vtkKWPushButton::New();
    }
  if (!this->TrackerFiducialPushButton4->IsCreated())
    {
    this->TrackerFiducialPushButton4->SetParent(
      this->TrackerFiducialFrame->GetFrame());
    this->TrackerFiducialPushButton4->Create();
    this->TrackerFiducialPushButton4->SetText("Collect Tracker Coordinate Fiducial 4");
    this->TrackerFiducialPushButton4->SetBalloonHelpString(
      "Capture Tracker coordinate of fiducial 1");
    }

  this->Script("grid %s -column 0 -row 3 -sticky nw -padx 2 -pady 2", 
               this->TrackerFiducialPushButton4->GetWidgetName());
 // Entry text book for the fourth fiducial 
  if (!this->TrackerFiducialTextBox4)
    {
    this->TrackerFiducialTextBox4 = vtkKWEntry::New();
    }
  if (!this->TrackerFiducialTextBox4->IsCreated())
    {
    this->TrackerFiducialTextBox4->SetParent(
      this->TrackerFiducialFrame->GetFrame());
    this->TrackerFiducialTextBox4->Create();
    this->TrackerFiducialTextBox4->ReadOnlyOn();
    }
  this->Script("grid %s -column 1 -row 3 -sticky nw -padx 2 -pady 2", 
               this->TrackerFiducialTextBox4->GetWidgetName());


  if (!this->RunRegistrationPushButton)
    {
    this->RunRegistrationPushButton = vtkKWPushButton::New();
    }

  if (!this->RunRegistrationPushButton->IsCreated())
    {
    this->RunRegistrationPushButton->SetParent(
      wizard_widget->GetClientArea());
    this->RunRegistrationPushButton->Create();
    this->RunRegistrationPushButton->SetText("Run Registration"); 
    this->RunRegistrationPushButton->SetCommand(this, "ComputeRegistration");
    }

  this->Script("pack %s -side left -anchor nw -padx 0 -pady 2", 
               this->RunRegistrationPushButton->GetWidgetName());

  //Add a help to the step
  vtkKWPushButton * helpButton =  wizard_widget->GetHelpButton();

  vtkKWMessageDialog *msg_dlg1 = vtkKWMessageDialog::New();
  msg_dlg1->SetParent(wizard_widget->GetClientArea());
  msg_dlg1->SetStyleToOkCancel();
  msg_dlg1->Create();
  msg_dlg1->SetTitle("Registration Step");
  msg_dlg1->SetText( "This is the fourth step in IGT applications. In this step, coordinate "
                     "transformation between preoperative data and surgical guidance system "
                     "is computed");

  helpButton->SetCommand(msg_dlg1, "Invoke");

  msg_dlg1->Delete();

}

//----------------------------------------------------------------------------
void vtkIGTNavigationRegistrationStep::PopulateFiducialPointsSelector(vtkKWMenuButtonWithLabel * menuLabelButton,
                                                            const char* callback)
{
  char buffer[256];

  vtkIGTNavigationMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();
  if (!mrmlManager)
    {
    return;
    }

  vtkKWMenu* menu = menuLabelButton->GetWidget()->GetMenu();
  menu->DeleteAllItems();

  int nb_of_fiducial_List = mrmlManager->GetFiducialNumberOfChoices();
  for(int index=0; index < nb_of_fiducial_List; index++)
    {
    vtkIdType fiducial_id_list = mrmlManager->GetFiducialNthID(index);
    sprintf(buffer, "%s %d", callback, static_cast<int>(fiducial_id_list));
    const char *name = mrmlManager->GetFiducialName(fiducial_id_list);
    if (name)
      {
      menu->AddRadioButton(name, this, buffer);
      }
    }
}

//----------------------------------------------------------------------------
void vtkIGTNavigationRegistrationStep::FiducialPointSelectorCallback1(
  vtkIdType fiducial_id)
{

  float fiducialPosition[3];

  if(this->GetFiducialXYZPosition( fiducialPosition, fiducial_id ))
    {
    char fiducialCoordinateString[256];
    
    if ( fiducialPosition != NULL )
      {
      sprintf(fiducialCoordinateString, "%4.2lf,%4.2lf,%4.2lf", fiducialPosition[0]
                                                ,  fiducialPosition[1]
                                                ,  fiducialPosition[2]);
      this->ImageFiducialTextBox1->SetValue( fiducialCoordinateString );
      }
    }
}

//----------------------------------------------------------------------------
void vtkIGTNavigationRegistrationStep::FiducialPointSelectorCallback2(
  vtkIdType fiducial_id)
{

  float fiducialPosition[3];

  if(this->GetFiducialXYZPosition( fiducialPosition, fiducial_id ))
    {
    char fiducialCoordinateString[256];
    
    if ( fiducialPosition != NULL )
      {
      sprintf(fiducialCoordinateString, "%4.2lf,%4.2lf,%4.2lf", fiducialPosition[0]
                                                ,  fiducialPosition[1]
                                                ,  fiducialPosition[2]);
      this->ImageFiducialTextBox2->SetValue( fiducialCoordinateString );
      }
    }
}

//----------------------------------------------------------------------------
void vtkIGTNavigationRegistrationStep::FiducialPointSelectorCallback3(
  vtkIdType fiducial_id)
{

  float fiducialPosition[3];

  if(this->GetFiducialXYZPosition( fiducialPosition, fiducial_id ))
    {
    char fiducialCoordinateString[256];
    
    if ( fiducialPosition != NULL )
      {
      sprintf(fiducialCoordinateString, "%4.2lf,%4.2lf,%4.2lf", fiducialPosition[0]
                                                ,  fiducialPosition[1]
                                                ,  fiducialPosition[2]);
      this->ImageFiducialTextBox3->SetValue( fiducialCoordinateString );
      }
    }
}

//----------------------------------------------------------------------------
void vtkIGTNavigationRegistrationStep::FiducialPointSelectorCallback4(
  vtkIdType fiducial_id)
{

  float fiducialPosition[3];

  if(this->GetFiducialXYZPosition( fiducialPosition, fiducial_id ))
    {
    char fiducialCoordinateString[256];
    
    if ( fiducialPosition != NULL )
      {
      sprintf(fiducialCoordinateString, "%4.2lf,%4.2lf,%4.2lf", fiducialPosition[0]
                                                ,  fiducialPosition[1]
                                                ,  fiducialPosition[2]);
      this->ImageFiducialTextBox4->SetValue( fiducialCoordinateString );
      }
    }
}


bool vtkIGTNavigationRegistrationStep::GetFiducialXYZPosition ( float * fiducialPosition, vtkIdType fiducial_id )
{
  vtkIGTNavigationMRMLManager *mrmlManager = this->GetGUI()->GetMRMLManager();


  if (mrmlManager)
    {
    vtkMRMLFiducialListNode * snode = mrmlManager->GetFiducialNode( fiducial_id );

    if ( snode )
      {

      //use the first element of the fiducial list
      float * position = snode->GetNthFiducialXYZ( 0 );
      fiducialPosition[0] = position[0];
      fiducialPosition[1] = position[1];
      fiducialPosition[2] = position[2];
      
      return true;
      }
    }

  return false;
}

//----------------------------------------------------------------------------
void vtkIGTNavigationRegistrationStep::ComputeRegistration()
{
} 

//----------------------------------------------------------------------------
void vtkIGTNavigationRegistrationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
