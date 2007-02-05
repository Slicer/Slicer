#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWMessage.h"

#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"

#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"

#include "vtkSlicerColorDisplayWidget.h"

#include "vtkKWMessageDialog.h"

#include "vtkSlicerColorGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerColorGUI );
vtkCxxRevisionMacro ( vtkSlicerColorGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerColorGUI::vtkSlicerColorGUI ( )
{
  this->Logic = NULL;
  NACLabel = NULL;
  NAMICLabel = NULL;
  NCIGTLabel = NULL;
  BIRNLabel = NULL;
  this->ColorDisplayWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerColorGUI::~vtkSlicerColorGUI ( )
{

  this->SetModuleLogic ( NULL );
  
  if (this->ColorDisplayWidget ) 
    {
    this->ColorDisplayWidget->SetParent(NULL);
    this->ColorDisplayWidget->Delete ( );
    }
  if ( this->NACLabel )
    {
    this->NACLabel->SetParent ( NULL );
    this->NACLabel->Delete();
    this->NACLabel = NULL;
    }
  if ( this->NAMICLabel )
    {
    this->NAMICLabel->SetParent ( NULL );
    this->NAMICLabel->Delete();
    this->NAMICLabel = NULL;
    }
  if ( this->NCIGTLabel )
    {
    this->NCIGTLabel->SetParent ( NULL );
    this->NCIGTLabel->Delete();
    this->NCIGTLabel = NULL;
    }
  if ( this->BIRNLabel )
    {
    this->BIRNLabel->SetParent ( NULL );
    this->BIRNLabel->Delete();
    this->BIRNLabel = NULL;
    }

}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "SlicerColorGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
  // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::RemoveGUIObservers ( )
{
  vtkDebugMacro("vtkSlicerColorGUI: RemoveGUIObservers\n");
}


//---------------------------------------------------------------------------
void vtkSlicerColorGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkSlicerColorGUI: AddGUIObservers\n");
}



//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{    
  vtkDebugMacro("vtkSlicerColorGUI::ProcessMRMLEvents: event = " << event << ".\n");
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::Enter ( )
{
  vtkDebugMacro("vtkSlicerColorGUI: Enter\n");
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::Exit ( )
{
  vtkDebugMacro("vtkSlicerColorGUI: Exit\n");
}

//---------------------------------------------------------------------------
void vtkSlicerColorGUI::BuildGUI ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  // ---
  // MODULE GUI FRAME 
  // configure a page for a model loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "Color", "Color", NULL );
  
  // Define your help text and build the help frame here.
  const char *help = "The Color Module manages color look up tables.\nTables are used by mappers to translate between an integer and a colour value for display of models and volumes.\nSlicer supports three kinds of tables:\n1. Continuous scales, like the greyscale table.\n2. Parametric tables, defined by an equation, such as the FMRIPA table.\n3. Discreet tables, such as those read in from a file.\nUsers are only allowed to edit User type tables. TODO: allow copy from a standard one.";
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Color" );
  this->BuildHelpAndAboutFrame ( page, help, about );
  
  this->NACLabel = vtkKWLabel::New();
  this->NACLabel->SetParent ( this->GetLogoFrame() );
  this->NACLabel->Create();
  this->NACLabel->SetImageToIcon ( vtkSlicerModuleGUI::AcknowledgementIcons->GetNACLogo() );

  this->NAMICLabel = vtkKWLabel::New();
  this->NAMICLabel->SetParent ( this->GetLogoFrame() );
  this->NAMICLabel->Create();
  this->NAMICLabel->SetImageToIcon ( vtkSlicerModuleGUI::AcknowledgementIcons->GetNAMICLogo() );    

  this->NCIGTLabel = vtkKWLabel::New();
  this->NCIGTLabel->SetParent ( this->GetLogoFrame() );
  this->NCIGTLabel->Create();
  this->NCIGTLabel->SetImageToIcon ( vtkSlicerModuleGUI::AcknowledgementIcons->GetNCIGTLogo() );
    
  this->BIRNLabel = vtkKWLabel::New();
  this->BIRNLabel->SetParent ( this->GetLogoFrame() );
  this->BIRNLabel->Create();
  this->BIRNLabel->SetImageToIcon ( vtkSlicerModuleGUI::AcknowledgementIcons->GetBIRNLogo() );
  app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w", this->NAMICLabel->GetWidgetName());
  app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w",  this->NACLabel->GetWidgetName());
  app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  this->BIRNLabel->GetWidgetName());
  app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  this->NCIGTLabel->GetWidgetName());                  

  // ---
  // DISPLAY FRAME            
  vtkSlicerModuleCollapsibleFrame *displayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  displayFrame->SetParent ( page );
  displayFrame->Create ( );
  displayFrame->SetLabelText ("Display");
  displayFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                displayFrame->GetWidgetName(),
                this->UIPanel->GetPageWidget("Color")->GetWidgetName());
  
  // color display widget
  this->ColorDisplayWidget = vtkSlicerColorDisplayWidget::New ( );
  this->ColorDisplayWidget->SetMRMLScene(this->GetMRMLScene() );
  this->ColorDisplayWidget->SetParent ( displayFrame->GetFrame() );
  this->ColorDisplayWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ColorDisplayWidget->GetWidgetName(), 
                displayFrame->GetFrame()->GetWidgetName());
  
  displayFrame->Delete ( );
}
