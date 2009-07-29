/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkProstateNavGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkProstateNavStep.h"
#include "vtkProstateNavStepSetUp.h"
#include "vtkProstateNavStepVerification.h"
#include "vtkProstateNavCalibrationStep.h"
#include "vtkProstateNavTargetingStep.h"
#include "vtkProstateNavManualControlStep.h"

#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerFiducialsLogic.h"

#include "vtkKWRenderWidget.h"
#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonSet.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWMenu.h"
#include "vtkKWLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWEvent.h"
#include "vtkKWOptions.h"

#include "vtkKWTkUtilities.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkCornerAnnotation.h"

// for Realtime Image
#include "vtkImageChangeInformation.h"
#include "vtkSlicerColorLogic.h"
//#include "vtkSlicerVolumesGUI.h"

#include "vtkCylinderSource.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkOpenIGTLinkIFGUI.h"
#include "vtkOpenIGTLinkIFLogic.h"
#include "vtkIGTLToMRMLCoordinate.h"
#include "vtkIGTLToMRMLBrpRobotCommand.h"

#include "vtkMRMLProstateNavManagerNode.h"

#include <vector>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkProstateNavGUI );
vtkCxxRevisionMacro ( vtkProstateNavGUI, "$Revision: 1.0 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkProstateNavGUI::vtkProstateNavGUI ( )
{
  
  //----------------------------------------------------------------
  // Logic values
  
  this->Logic = NULL;
  
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkProstateNavGUI::DataCallback);
  
  
  //----------------------------------------------------------------
  // Workphase Frame
  
  this->WorkPhaseButtonSet = NULL;

  // these need to be set to null no matter what
  this->ScannerStatusLabelDisp  = NULL;
  this->SoftwareStatusLabelDisp = NULL;
  this->RobotStatusLabelDisp = NULL;

  //----------------------------------------------------------------  
  // Wizard Frame
  
  this->WizardWidget = vtkKWWizardWidget::New();

  //----------------------------------------------------------------
  // Target Fiducials List (MRML)

  this->FiducialListNodeID = NULL;
  this->FiducialListNode   = NULL;

  this->Entered = 0;
  this->CoordinateConverter = NULL;
  this->CommandConverter = NULL;
 
  // -----------------------------------------
  // Set up manager

  this->ProstateNavManager =  vtkMRMLProstateNavManagerNode::New();

  vtkProstateNavStepSetUp* setupStep = vtkProstateNavStepSetUp::New();
  setupStep->SetTitleBackgroundColor(205.0/255.0, 200.0/255.0, 177.0/255.0);
  this->ProstateNavManager->AddNewStep("Set Up", setupStep);

  //vtkProstateNavScanControlStep* scanControlStep = vtkProstateNavScanControlStep::New();
  //scanControlStep->SetTitleBackgroundColor(179.0/255.0, 145.0/255.0, 105.0/255.0);
  //this->ProstateNavManager->AddNewStep("Planning", scanControlStep);

  vtkProstateNavCalibrationStep* calibrationStep = vtkProstateNavCalibrationStep::New();
  calibrationStep->SetTitleBackgroundColor(193.0/255.0, 115.0/255.0, 80.0/255.0);
  this->ProstateNavManager->AddNewStep("Calibration", calibrationStep);

  vtkProstateNavTargetingStep* targetingStep = vtkProstateNavTargetingStep::New();
  targetingStep->SetTitleBackgroundColor(138.0/255.0, 165.0/255.0, 111.0/255.0);
  this->ProstateNavManager->AddNewStep("Targeting", targetingStep);

  vtkProstateNavManualControlStep* manualStep = vtkProstateNavManualControlStep::New();
  manualStep->SetTitleBackgroundColor(179.0/255.0, 179.0/255.0, 230.0/255.0);
  this->ProstateNavManager->AddNewStep("Manual", manualStep);

  vtkProstateNavStepVerification* verificationStep = vtkProstateNavStepVerification::New();
  verificationStep->SetTitleBackgroundColor(179.0/255.0, 145.0/255.0, 105.0/255.0);
  this->ProstateNavManager->AddNewStep("Verification", verificationStep);

  this->ProstateNavManager->AllowAllTransitions();

}



//---------------------------------------------------------------------------
vtkProstateNavGUI::~vtkProstateNavGUI ( )
{
  this->RemoveGUIObservers();

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }


  //----------------------------------------------------------------
  // Workphase Frame

  if (this->WorkPhaseButtonSet)
    {
    this->WorkPhaseButtonSet->SetParent(NULL);
    this->WorkPhaseButtonSet->Delete();
    }
  this->SetModuleLogic ( NULL );


  //----------------------------------------------------------------
  // Wizard Frame

  if (this->WizardWidget)
    {
    this->WizardWidget->SetParent(NULL);
    this->WizardWidget->Delete(); 
    this->WizardWidget = NULL;
    }

  // -----------------------------------------
  // Work Phase Display Frame

  if (this->SoftwareStatusLabelDisp)
    {
    this->SoftwareStatusLabelDisp->SetParent(NULL);
    this->SoftwareStatusLabelDisp->Delete(); 
    this->SoftwareStatusLabelDisp = NULL;
    }
  if (this->ScannerStatusLabelDisp)
    {
    this->ScannerStatusLabelDisp->SetParent(NULL);
    this->ScannerStatusLabelDisp->Delete(); 
    this->ScannerStatusLabelDisp = NULL;
    }
  if (this->RobotStatusLabelDisp)
    {
    this->RobotStatusLabelDisp->SetParent(NULL);
    this->RobotStatusLabelDisp->Delete(); 
    this->RobotStatusLabelDisp = NULL;
    }
  if (this->CoordinateConverter)
    { 
    this->CoordinateConverter->Delete();
    this->CoordinateConverter = NULL;
    }
  if (this->CommandConverter)
    {
    this->CommandConverter->Delete();
    this->CommandConverter = NULL;
    }

}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    



    os << indent << "ProstateNavGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
   
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::RemoveGUIObservers ( )
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  if (appGUI) 
    {
    appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  //----------------------------------------------------------------
  // Workphase Frame

  if (this->WorkPhaseButtonSet)
    {
    for (int i = 0; i < this->WorkPhaseButtonSet->GetNumberOfWidgets(); i ++)
      {
      this->WorkPhaseButtonSet->GetWidget(i)->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
      }
    }
    

  //----------------------------------------------------------------
  // Wizard Frame

  this->WizardWidget->GetWizardWorkflow()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);


  this->RemoveLogicObservers();
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::RemoveLogicObservers ( )
{
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  // make a user interactor style to process our events
  // look at the InteractorStyle to get our events
  
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  //
  //appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
  //  ->GetRenderWindowInteractor()->GetInteractorStyle()
  //  ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  //appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
  //  ->GetRenderWindowInteractor()->GetInteractorStyle()
  //  ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  //appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
  //  ->GetRenderWindowInteractor()->GetInteractorStyle()
  //  ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  
  //----------------------------------------------------------------
  // Workphase Frame

  for (int i = 0; i < this->WorkPhaseButtonSet->GetNumberOfWidgets(); i ++)
    {
    this->WorkPhaseButtonSet->GetWidget(i)
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  
  
  //----------------------------------------------------------------
  // Wizard Frame

  this->WizardWidget->GetWizardWorkflow()->AddObserver(vtkKWWizardWorkflow::CurrentStateChangedEvent,
                                                       (vtkCommand *)this->GUICallbackCommand);


  //----------------------------------------------------------------
  // Etc Frame

  // observer load volume button

  this->AddLogicObservers();
  
  
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    this->GetLogic()->AddObserver(vtkProstateNavLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkProstateNavGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{

  /*
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  vtkSlicerInteractorStyle *istyle0 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Red")->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle1 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle2 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Green")->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());

  vtkCornerAnnotation *anno = NULL;
  if (style == istyle0)
    {
    anno = appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  else if (style == istyle1)
    {
    anno = appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  else if (style == istyle2)
    {
    anno = appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  if (anno)
    {
    const char *rasText = anno->GetText(1);
    if ( rasText != NULL )
      {
      std::string ras = std::string(rasText);
        
      // remove "R:," "A:," and "S:" from the string
      int loc = ras.find("R:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }
      loc = ras.find("A:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }
      loc = ras.find("S:", 0);
      if ( loc != std::string::npos ) 
        {
        ras = ras.replace(loc, 2, "");
        }
      
      // remove "\n" from the string
      int found = ras.find("\n", 0);
      while ( found != std::string::npos )
        {
        ras = ras.replace(found, 1, " ");
        found = ras.find("\n", 0);
        }
      
      }
    }
  */
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    return;
    }

  //----------------------------------------------------------------
  // Check Work Phase Transition Buttons

  if ( event == vtkKWPushButton::InvokedEvent)
    {
    int phase;
    for (phase = 0; phase < this->WorkPhaseButtonSet->GetNumberOfWidgets(); phase ++)
      {
      if (this->WorkPhaseButtonSet->GetWidget(phase) == vtkKWPushButton::SafeDownCast(caller))
        {
        break;
        }
      }
    if (phase < this->ProstateNavManager->GetNumberOfSteps()) // if pressed one of them
      {
      ChangeWorkPhase(phase, 1);
      }
    }


  //----------------------------------------------------------------
  // Wizard Frame

  else if (this->WizardWidget->GetWizardWorkflow() == vtkKWWizardWorkflow::SafeDownCast(caller) &&
      event == vtkKWWizardWorkflow::CurrentStateChangedEvent)
    {
          
    int phase = vtkProstateNavLogic::Emergency;
    vtkKWWizardStep* step =  this->WizardWidget->GetWizardWorkflow()->GetCurrentStep();

    int numSteps = this->ProstateNavManager->GetNumberOfSteps();
    for (int i = 0; i < numSteps; i ++)
      {
      if (step == vtkKWWizardStep::SafeDownCast(this->ProstateNavManager->GetStepPage(i)))
        {
        phase = i;
        }
      }
    
    ChangeWorkPhase(phase);
    }


  //----------------------------------------------------------------
  // Etc Frame

  // Process Wizard GUI (Active step only)
  else
    {
    int phase = this->ProstateNavManager->GetCurrentStep();
    this->ProstateNavManager->GetStepPage(phase)->ProcessGUIEvents(caller, event, callData);
    }

} 


void vtkProstateNavGUI::Init()
{
  
  this->GetMRMLScene()->AddNode(this->ProstateNavManager);

  int numSteps = this->ProstateNavManager->GetNumberOfSteps();
  for (int i = 0; i < numSteps; i ++)
    {
    this->ProstateNavManager->GetStepPage(i)->SetAndObserveMRMLScene(this->GetMRMLScene());
    }
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::DataCallback(vtkObject *caller, 
        unsigned long eid, void *clientData, void *callData)
{
  vtkProstateNavGUI *self = reinterpret_cast<vtkProstateNavGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkProstateNavGUI DataCallback");
  
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::ProcessLogicEvents ( vtkObject *caller,
    unsigned long event, void *callData )
{

  if (this->GetLogic() == vtkProstateNavLogic::SafeDownCast(caller))
    {
    if (event == vtkProstateNavLogic::StatusUpdateEvent)
      {
      this->UpdateDeviceStatus();
      }
    }
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::ProcessMRMLEvents ( vtkObject *caller,
    unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::Enter()
{

  // Fill in
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if (this->Entered == 0)
    {
    this->GetLogic()->SetGUI(this);

    ChangeWorkPhase(vtkProstateNavLogic::StartUp, 1);
  
    //----------------------------------------------------------------
    // Following code should be in the logic class, but GetApplication()
    // is not available there.
    
    vtkOpenIGTLinkIFGUI* igtlGUI = 
      vtkOpenIGTLinkIFGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())
                                        ->GetModuleGUIByName("OpenIGTLink IF"));
    if (igtlGUI)
      {
      if (!this->CoordinateConverter)
        {
        this->CoordinateConverter = vtkIGTLToMRMLCoordinate::New();
        }
      if (!this->CommandConverter)
        {
        this->CommandConverter = vtkIGTLToMRMLBrpRobotCommand::New();
        }
      igtlGUI->GetLogic()->RegisterMessageConverter(this->CoordinateConverter);
      igtlGUI->GetLogic()->RegisterMessageConverter(this->CommandConverter);
      }

    this->GetLogic()->Enter();

    this->Entered = 1;
    }

  
  //----------------------------------------------------------------
  // Target Fiducials
  // Junichi Tokuda 11/27/2007: Should it be in the Logic class ?

  if (!this->FiducialListNodeID)
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // Get a pointer to the Fiducials module
    vtkSlicerFiducialsGUI* fidGUI = vtkSlicerFiducialsGUI::SafeDownCast ( app->GetModuleGUIByName ("Fiducials"));
    if (fidGUI)
      {
      // Create New Fiducial list for Prostate Module
      vtkSlicerFiducialsLogic *fidLogic = fidGUI->GetLogic();
      if (fidLogic)
        {
        vtkMRMLFiducialListNode *newList = fidLogic->AddFiducialList();
        if (newList)
          {
          // Change the name of the list
          newList->SetName(this->GetMRMLScene()->GetUniqueNameByString("PM"));
          //fidGUI->SetFiducialListNodeID(id);
          }
        else
          {
          vtkErrorMacro("Unable to add a new fid list via the logic\n");
          }
          // now get the newly active node 
          this->FiducialListNodeID = newList->GetID(); 
          this->FiducialListNode = (vtkMRMLFiducialListNode *)this->GetMRMLScene()->GetNodeByID(newList->GetID());
 
          if (this->FiducialListNode == NULL)
            {
            vtkErrorMacro ("ERROR adding a new fiducial list for the point...\n");
            return;
            }
          }
       }
    }

}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::BuildGUI ( )
{

    // ---
    // MODULE GUI FRAME 
    // create a page
    this->UIPanel->AddPage ( "ProstateNav", "ProstateNav", NULL );

    BuildGUIForHelpFrame();
    BuildGUIForWorkPhaseFrame ();
    BuildGUIForWizardFrame();

}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::TearDownGUI ( )
{


  // disconnect circular references so destructor can be called
  
  this->GetLogic()->SetGUI(NULL);
  int numSteps = this->ProstateNavManager->GetNumberOfSteps();
  for (int i = 0; i < numSteps; i ++)
    {
    this->ProstateNavManager->GetStepPage(i)->SetGUI(NULL);
    this->ProstateNavManager->GetStepPage(i)->SetLogic(NULL);
    }

}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::BuildGUIForWizardFrame()
{
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "ProstateNav" );
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    

    // ----------------------------------------------------------------
    // WIZARD FRAME         
    // ----------------------------------------------------------------

    vtkSlicerModuleCollapsibleFrame *wizardFrame = 
      vtkSlicerModuleCollapsibleFrame::New();
    wizardFrame->SetParent(page);
    wizardFrame->Create();
    wizardFrame->SetLabelText("Wizard");
    wizardFrame->ExpandFrame();

    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                wizardFrame->GetWidgetName(), 
                page->GetWidgetName());
   
    this->WizardWidget->SetParent(wizardFrame->GetFrame());
    this->WizardWidget->Create();
    this->WizardWidget->GetSubTitleLabel()->SetHeight(1);
    this->WizardWidget->SetClientAreaMinimumHeight(200);
    //this->WizardWidget->SetButtonsPositionToTop();
    this->WizardWidget->NextButtonVisibilityOn();
    this->WizardWidget->BackButtonVisibilityOn();
    this->WizardWidget->OKButtonVisibilityOff();
    this->WizardWidget->CancelButtonVisibilityOff();
    this->WizardWidget->FinishButtonVisibilityOff();
    this->WizardWidget->HelpButtonVisibilityOn();

    app->Script("pack %s -side top -anchor nw -fill both -expand y",
                this->WizardWidget->GetWidgetName());
    wizardFrame->Delete();

    // -----------------------------------------------------------------
    // Add the steps to the workflow

    vtkKWWizardWorkflow *wizard_workflow = 
      this->WizardWidget->GetWizardWorkflow();

    // -----------------------------------------------------------------
    // Set GUI/Logic to each step and add to workflow

    int numSteps = this->ProstateNavManager->GetNumberOfSteps();

    for (int i = 0; i < numSteps; i ++)
      {
      this->ProstateNavManager->GetStepPage(i)->SetGUI(this);
      this->ProstateNavManager->GetStepPage(i)->SetLogic(this->Logic);
      wizard_workflow->AddNextStep(this->ProstateNavManager->GetStepPage(i));
      }


    // -----------------------------------------------------------------
    // Initial and finish step

    wizard_workflow->SetFinishStep(this->ProstateNavManager->GetStepPage(numSteps-1));
    wizard_workflow->CreateGoToTransitionsToFinishStep();
    wizard_workflow->SetInitialStep(this->ProstateNavManager->GetStepPage(0));

    // -----------------------------------------------------------------
    // Show the user interface

    if (wizard_workflow->GetCurrentStep())
      {
      wizard_workflow->GetCurrentStep()->ShowUserInterface();
      }

}


void vtkProstateNavGUI::BuildGUIForHelpFrame ()
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "ProstateNav" );

    // Define your help text here.
    const char *help = 
      "The **ProstateNav Module** helps you to do prostate Biopsy and Treatment by:"
      " getting Realtime Images from MR-Scanner into Slicer3, control Scanner with Slicer 3,"
      " determin fiducial detection and control the Robot."
      " Module and Logic mainly coded by Junichi Tokuda, David Gobbi and Philip Mewes"; 

    // ----------------------------------------------------------------
    // HELP FRAME         
    // ----------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *ProstateNavHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    ProstateNavHelpFrame->SetParent ( page );
    ProstateNavHelpFrame->Create ( );
    ProstateNavHelpFrame->CollapseFrame ( );
    ProstateNavHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        ProstateNavHelpFrame->GetWidgetName(), page->GetWidgetName());

    // configure the parent classes help text widget
    this->HelpText->SetParent ( ProstateNavHelpFrame->GetFrame() );
    this->HelpText->Create ( );
    this->HelpText->SetHorizontalScrollbarVisibility ( 0 );
    this->HelpText->SetVerticalScrollbarVisibility ( 1 );
    this->HelpText->GetWidget()->SetText ( help );
    this->HelpText->GetWidget()->SetReliefToFlat ( );
    this->HelpText->GetWidget()->SetWrapToWord ( );
    this->HelpText->GetWidget()->ReadOnlyOn ( );
    this->HelpText->GetWidget()->QuickFormattingOn ( );
    this->HelpText->GetWidget()->SetBalloonHelpString ( "" );
    app->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 4",
        this->HelpText->GetWidgetName ( ) );

    ProstateNavHelpFrame->Delete();

}

//---------------------------------------------------------------------------
void vtkProstateNavGUI::BuildGUIForWorkPhaseFrame ()
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "ProstateNav" );
  
  vtkSlicerModuleCollapsibleFrame *workphaseFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  workphaseFrame->SetParent(page);
  workphaseFrame->Create();
  workphaseFrame->SetLabelText("Workphase Frame");
  workphaseFrame->ExpandFrame();
  app->Script("pack %s -side top -anchor center -fill x -padx 2 -pady 2 -in %s",
              workphaseFrame->GetWidgetName(), page->GetWidgetName());
  

  // -----------------------------------------
  // Frames

  vtkKWFrame *workphaseStatusFrame = vtkKWFrame::New ( );
  workphaseStatusFrame->SetParent ( workphaseFrame->GetFrame() );
  workphaseStatusFrame->Create ( );
  workphaseStatusFrame->SetReliefToRaised();
  workphaseStatusFrame->SetBackgroundColor(0.9, 0.9, 0.9);
  
  vtkKWFrame *buttonFrame = vtkKWFrame::New();
  buttonFrame->SetParent( workphaseFrame->GetFrame());
  buttonFrame->Create();

  app->Script ( "pack %s %s -side top -anchor center -fill x -padx 2 -pady 1",
                workphaseStatusFrame->GetWidgetName(),
                buttonFrame->GetWidgetName());
  

  // -----------------------------------------
  // Work Phase Display Frame

  this->SoftwareStatusLabelDisp = vtkKWEntry::New();
  this->SoftwareStatusLabelDisp->SetParent(workphaseStatusFrame);
  this->SoftwareStatusLabelDisp->Create();
  this->SoftwareStatusLabelDisp->SetWidth(18);
  this->SoftwareStatusLabelDisp->SetReliefToFlat();
  this->SoftwareStatusLabelDisp->SetBackgroundColor(0.9, 0.9, 0.9);
  this->SoftwareStatusLabelDisp->SetValue (" NETWORK: OFF ");
  
  this->RobotStatusLabelDisp = vtkKWEntry::New();
  this->RobotStatusLabelDisp->SetParent(workphaseStatusFrame);
  this->RobotStatusLabelDisp->Create();
  this->RobotStatusLabelDisp->SetWidth(18);
  this->RobotStatusLabelDisp->SetReliefToFlat();
  this->RobotStatusLabelDisp->SetBackgroundColor(0.9, 0.9, 0.9);
  this->RobotStatusLabelDisp->SetValue (" ROBOT: OFF ");
  
  this->ScannerStatusLabelDisp = vtkKWEntry::New();
  this->ScannerStatusLabelDisp->SetParent(workphaseStatusFrame);
  this->ScannerStatusLabelDisp->Create();
  this->ScannerStatusLabelDisp->SetWidth(18);
  this->ScannerStatusLabelDisp->SetReliefToFlat();
  this->ScannerStatusLabelDisp->SetBackgroundColor(0.9, 0.9, 0.9);
  this->ScannerStatusLabelDisp->SetValue (" SCANNER: OFF ");
  
  this->Script("pack %s %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
               SoftwareStatusLabelDisp->GetWidgetName(),
               ScannerStatusLabelDisp->GetWidgetName(),
               RobotStatusLabelDisp->GetWidgetName()
               );
  
  // -----------------------------------------
  // Work Phase Transition Buttons Frame

  this->WorkPhaseButtonSet = vtkKWPushButtonSet::New();
  this->WorkPhaseButtonSet->SetParent(buttonFrame);
  this->WorkPhaseButtonSet->Create();
  this->WorkPhaseButtonSet->PackHorizontallyOn();
  this->WorkPhaseButtonSet->SetMaximumNumberOfWidgetsInPackingDirection(3);
  this->WorkPhaseButtonSet->SetWidgetsPadX(2);
  this->WorkPhaseButtonSet->SetWidgetsPadY(2);
  this->WorkPhaseButtonSet->UniformColumnsOn();
  this->WorkPhaseButtonSet->UniformRowsOn();

  int numSteps = this->ProstateNavManager->GetNumberOfSteps();
  for (int i = 0; i < numSteps; i ++)  
  //for (int i = 0; i < vtkProstateNavLogic::NumPhases; i ++)
    {
    double r;
    double g;
    double b;
    this->ProstateNavManager->GetStepPage(i)->GetTitleBackgroundColor(&r, &g, &b);
    const char* name = this->ProstateNavManager->GetStepName(i);

    this->WorkPhaseButtonSet->AddWidget(i);
    this->WorkPhaseButtonSet->GetWidget(i)->SetWidth(16);
    this->WorkPhaseButtonSet->GetWidget(i)->SetText(name);
    this->WorkPhaseButtonSet->GetWidget(i)->SetBackgroundColor(r, g, b);
    this->WorkPhaseButtonSet->GetWidget(i)->SetActiveBackgroundColor(r, g, b);
    /*
    this->WorkPhaseButtonSet->GetWidget(i)
      ->SetDisabledBackgroundColor(WorkPhaseColor[i][0], WorkPhaseColor[i][1], WorkPhaseColor[i][2]);
    */
    }
  
  this->Script("pack %s -side left -anchor w -fill x -padx 2 -pady 2", 
               this->WorkPhaseButtonSet->GetWidgetName());
  
  workphaseFrame->Delete ();
  buttonFrame->Delete ();
  workphaseStatusFrame->Delete ();
  
}

//----------------------------------------------------------------------------
int vtkProstateNavGUI::ChangeWorkPhase(int phase, int fChangeWizard)
{


  if (!this->ProstateNavManager->SwitchStep(phase)) // Set next phase
    {
    cerr << "ChangeWorkPhase: Cannot transition!" << endl;
    return 0;
    }
  
  int numSteps = this->ProstateNavManager->GetNumberOfSteps();
  
  for (int i = 0; i < numSteps; i ++)
    {
    vtkKWPushButton *pb = this->WorkPhaseButtonSet->GetWidget(i);
    if (i == this->ProstateNavManager->GetCurrentStep())
      {
      pb->SetReliefToSunken();
      }
    else if (this->ProstateNavManager->IsTransitionable(i))
      {
      double r;
      double g;
      double b;
      this->ProstateNavManager->GetStepPage(i)->GetTitleBackgroundColor(&r, &g, &b);
      
      pb->SetReliefToGroove();
      pb->SetStateToNormal();
      pb->SetBackgroundColor(r, g, b);
      }
    else
      {
      double r;
      double g;
      double b;
      this->ProstateNavManager->GetStepPage(i)->GetTitleBackgroundColor(&r, &g, &b);
      r = r * 1.5; r = (r > 1.0) ? 1.0 : r;
      g = g * 1.5; g = (r > 1.0) ? 1.0 : g;
      b = b * 1.5; b = (r > 1.0) ? 1.0 : b;
      
      pb->SetReliefToGroove();
      pb->SetStateToDisabled();
      pb->SetBackgroundColor(r, g, b);
      }
    }
  
  // Switch Wizard Frame
  if (fChangeWizard)
    {
    vtkKWWizardWorkflow *wizard = 
      this->WizardWidget->GetWizardWorkflow();
    
    int step_from;
    int step_to;
    
    //step_to = this->Logic->GetCurrentPhase();
    step_to = this->ProstateNavManager->GetCurrentStep();
    //step_from = this->Logic->GetPrevPhase();
    step_from = this->ProstateNavManager->GetPreviousStep();
    
    int steps =  step_to - step_from;
    if (steps > 0)
      {
      for (int i = 0; i < steps; i ++) 
        {
        wizard->AttemptToGoToNextStep();
        }
      }
    else
      {
      steps = -steps;
      for (int i = 0; i < steps; i ++)
        {
        wizard->AttemptToGoToPreviousStep();
        }
      }
    wizard->GetCurrentStep()->ShowUserInterface();
    }
  
  return 1;
}


//----------------------------------------------------------------------------
void vtkProstateNavGUI::UpdateAll()
{

}


//----------------------------------------------------------------------------
void vtkProstateNavGUI::UpdateDeviceStatus()
{
  
  int status;
  char label[128];

  bool network = this->GetLogic()->GetConnection();
  if (!network)
    {
    this->SoftwareStatusLabelDisp->SetBackgroundColor(0.9, 0.9, 0.9);
    this->SoftwareStatusLabelDisp->SetValue(" NETWORK: OFF ");
    }
  else
    {
    this->SoftwareStatusLabelDisp->SetBackgroundColor(0.0, 0.5, 1.0);
    this->SoftwareStatusLabelDisp->SetValue(" NETWORK: ON ");
    }

  status = this->GetLogic()->GetRobotWorkPhase();
  if (status < 0)
    {
    this->RobotStatusLabelDisp->SetBackgroundColor(0.9, 0.9, 0.9);
    this->RobotStatusLabelDisp->SetValue(" ROBOT: OFF ");
    }
  else
    {
    double r;
    double g;
    double b;
    this->ProstateNavManager->GetStepPage(status)->GetTitleBackgroundColor(&r, &g, &b);
    const char* name = this->ProstateNavManager->GetStepName(status);

    sprintf(label, "RBT: %s", name);
    this->RobotStatusLabelDisp->SetValue(label);
    this->RobotStatusLabelDisp->SetBackgroundColor(r, g, b);
    }

  status = this->GetLogic()->GetScannerWorkPhase();
  if (status < 0)
    {
    this->ScannerStatusLabelDisp->SetValue(" SCANNER: OFF ");
    this->ScannerStatusLabelDisp->SetBackgroundColor(0.9, 0.9, 0.9);
    }
  else
    {
    double r;
    double g;
    double b;
    this->ProstateNavManager->GetStepPage(status)->GetTitleBackgroundColor(&r, &g, &b);
    const char* name = this->ProstateNavManager->GetStepName(status);

    sprintf(label, "SCNR: %s", name);
    this->ScannerStatusLabelDisp->SetValue(label);
    this->ScannerStatusLabelDisp->SetBackgroundColor(r, g, b);

    }

}


