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
#include "BRPTPRInterface.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkProstateNavStep.h"
#include "vtkProstateNavConfigurationStep.h"
#include "vtkProstateNavScanControlStep.h"
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
#include "vtkSlicerVolumesGUI.h"

#include "vtkIGTDataStream.h"
#include "vtkCylinderSource.h"
#include "vtkMRMLLinearTransformNode.h"


#include <vector>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkProstateNavGUI );
vtkCxxRevisionMacro ( vtkProstateNavGUI, "$Revision: 1.0 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Button Colors and Labels for Work Phase Control
const double vtkProstateNavGUI::WorkPhaseColor[vtkProstateNavLogic::NumPhases][3] =
  {
  /* St */ { 1.0, 0.6, 1.0 },
  /* Pl */ { 0.6, 1.0, 0.6 },
  /* Cl */ { 1.0, 1.0, 0.6 },
  /* Tg */ { 0.6, 0.6, 1.0 },
  /* Mn */ { 0.6, 1.0, 1.0 },
  /* Em */ { 1.0, 0.0, 0.0 },
  };

const double vtkProstateNavGUI::WorkPhaseColorActive[vtkProstateNavLogic::NumPhases][3] =
  {
  /* St */ { 1.0, 0.4, 1.0 },
  /* Pl */ { 0.4, 1.0, 0.4 },
  /* Cl */ { 1.0, 1.0, 0.4 },
  /* Tg */ { 0.4, 0.4, 1.0 },
  /* Mn */ { 0.4, 1.0, 1.0 },
  /* Em */ { 1.0, 0.0, 0.0 },
  };

const double vtkProstateNavGUI::WorkPhaseColorDisabled[vtkProstateNavLogic::NumPhases][3] =
  {
  /* St */ { 1.0, 0.95, 1.0 },
  /* Pl */ { 0.95, 1.0, 0.95 },
  /* Cl */ { 1.0, 1.0, 0.95 },
  /* Tg */ { 0.95, 0.95, 1.0 },
  /* Mn */ { 0.95, 1.0, 1.0 },
  /* Em */ { 1.0, 0.0, 0.0 },
  };

const char *vtkProstateNavGUI::WorkPhaseStr[vtkProstateNavLogic::NumPhases] =
  {
  /* Su */ "Start Up",
  /* Pl */ "Planning",
  /* Cl */ "Calibration",
  /* Tg */ "Targeting",
  /* Mn */ "Manual",
  /* Em */ "Emergency",
  };

//---------------------------------------------------------------------------
vtkProstateNavGUI::vtkProstateNavGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  
  this->Logic = NULL;
  this->DataManager = vtkIGTDataManager::New();
  this->Pat2ImgReg = vtkIGTPat2ImgRegistration::New();
  
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkProstateNavGUI::DataCallback);
  
  /*
  this->Logic0 = NULL; 
  this->Logic1 = NULL; 
  this->Logic2 = NULL; 
  */
  this->SliceNode0 = NULL; 
  this->SliceNode1 = NULL; 
  this->SliceNode2 = NULL; 
  /*
  this->Control0 = NULL; 
  this->Control1 = NULL; 
  this->Control2 = NULL; 
  */
  
  this->NeedOrientationUpdate0 = 0;
  this->NeedOrientationUpdate1 = 0;
  this->NeedOrientationUpdate2 = 0;
  
  //this->NeedRealtimeImageUpdate = 0;
  this->FreezeOrientationUpdate = 0;
  
  
  //----------------------------------------------------------------
  // Workphase Frame
  
  this->WorkPhaseButtonSet = NULL;

#ifdef USE_NAVITRACK
  this->ScannerStatusLabelDisp  = NULL;
  this->SoftwareStatusLabelDisp = NULL;
#endif

  //----------------------------------------------------------------  
  // Wizard Frame
  
  this->WizardWidget = vtkKWWizardWidget::New();
  this->WizardSteps = new vtkProstateNavStep*[vtkProstateNavLogic::NumPhases];
  for (int i = 0; i < vtkProstateNavLogic::NumPhases; i ++)
    {
    this->WizardSteps[i] = NULL;
    }
  

  //----------------------------------------------------------------
  // Visualization Control Frame
  
  this->FreezeImageCheckButton = NULL;
  this->SetLocatorModeButton   = NULL;
  this->SetUserModeButton      = NULL;
  this->RedSliceMenu           = NULL;
  this->YellowSliceMenu        = NULL;
  this->GreenSliceMenu         = NULL;

  this->StartScanButton        = NULL;
  this->StopScanButton         = NULL;

  this->FreezeImageCheckButton = NULL;
  this->LocatorCheckButton     = NULL;

  this->ImagingMenu            = NULL;

  /*
  this->SliceDriver0 = 0;
  this->SliceDriver1 = 0;
  this->SliceDriver2 = 0;
  */
  

  //----------------------------------------------------------------
  // Target Fiducials List (MRML)

  this->FiducialListNodeID = NULL;
  this->FiducialListNode   = NULL;
  
  
}

//---------------------------------------------------------------------------
vtkProstateNavGUI::~vtkProstateNavGUI ( )
{

  if (this->DataManager)
    {
    // If we don't set the scence to NULL for DataManager,
    // Slicer will report a lot leak when it is closed.
    this->DataManager->SetMRMLScene(NULL);
    this->DataManager->Delete();
    }
  if (this->Pat2ImgReg)
    {
    this->Pat2ImgReg->Delete();
    }
  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  this->RemoveGUIObservers();


  //----------------------------------------------------------------
  // Workphase Frame

  if (this->WorkPhaseButtonSet)
    {
    this->WorkPhaseButtonSet->SetParent(NULL);
    this->WorkPhaseButtonSet->Delete();
    }


  //----------------------------------------------------------------
  // Wizard Frame

  if (this->WizardWidget)
    {
    this->WizardWidget->Delete();
    this->WizardWidget = NULL;
    }

  this->SetModuleLogic ( NULL );


  //----------------------------------------------------------------
  // Visualization Control Frame

  if (this->FreezeImageCheckButton)
    {
    this->FreezeImageCheckButton->SetParent(NULL );
    this->FreezeImageCheckButton->Delete ( );
    }

  if (this->SetLocatorModeButton)
    {
    this->SetLocatorModeButton->SetParent(NULL);
    this->SetLocatorModeButton->Delete();
    }
  
  if (this->SetUserModeButton)
    {
    this->SetUserModeButton->SetParent(NULL);
    this->SetUserModeButton->Delete();
    }

  if (this->RedSliceMenu)
    {
    this->RedSliceMenu->SetParent(NULL );
    this->RedSliceMenu->Delete ( );
    }

  if (this->YellowSliceMenu)
    {
    this->YellowSliceMenu->SetParent(NULL );
    this->YellowSliceMenu->Delete ( );
    }

  if (this->GreenSliceMenu)
    {
    this->GreenSliceMenu->SetParent(NULL );
    this->GreenSliceMenu->Delete ( );
    }

  if (this->StartScanButton)
    {
    this->StartScanButton->SetParent(NULL);
    this->StartScanButton->Delete();
    }

  if (this->StopScanButton)
    {
    this->StopScanButton->SetParent(NULL);
    this->StartScanButton->Delete();
    }

  if ( this->ImagingMenu )
    {
    this->ImagingMenu->SetParent(NULL);
    this->ImagingMenu->Delete();
    }


  //----------------------------------------------------------------
  // Etc Frame


  if (this->LocatorCheckButton)
  {
  this->LocatorCheckButton->SetParent(NULL );
  this->LocatorCheckButton->Delete ( );
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
  
  appGUI->GetMainSliceGUI0()->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI1()->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI2()->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  
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



  //----------------------------------------------------------------
  // Visualization Control Frame

  if (this->FreezeImageCheckButton)
    {
    this->FreezeImageCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->SetLocatorModeButton)
    {
    this->SetLocatorModeButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->SetUserModeButton)
    {
    this->SetUserModeButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->LocatorCheckButton)
    {
    this->LocatorCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    }

  if (this->RedSliceMenu)
    {
    this->RedSliceMenu->GetMenu()
      ->RemoveObserver((vtkCommand*)this->GUICallbackCommand);
    }
  if (this->YellowSliceMenu)
    {
    this->YellowSliceMenu->GetMenu()
      ->RemoveObserver((vtkCommand*)this->GUICallbackCommand);
    }
  if (this->GreenSliceMenu)
    {
    this->GreenSliceMenu->GetMenu()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ImagingMenu)
    {
    this->ImagingMenu->GetMenu()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  
  this->RemoveLogicObservers();
}


//---------------------------------------------------------------------------
void vtkProstateNavGUI::RemoveLogicObservers ( )
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
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
  
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  appGUI->GetMainSliceGUI0()->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI1()->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI2()->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  
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
  // Visualization Control Frame

  this->FreezeImageCheckButton
    ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->SetLocatorModeButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SetUserModeButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->StartScanButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->StopScanButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LocatorCheckButton
    ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->RedSliceMenu->GetMenu()
    ->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->YellowSliceMenu->GetMenu()
    ->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand*)this->GUICallbackCommand);
  this->GreenSliceMenu->GetMenu()
    ->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ImagingMenu->GetMenu()
    ->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand);


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
    /*
    this->GetLogic()->AddObserver(vtkProstateNavLogic::LocatorUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    */
    /*
    this->GetLogic()->AddObserver(vtkProstateNavLogic::SliceUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    */
    this->GetLogic()->AddObserver(vtkProstateNavLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);

    }
}

//---------------------------------------------------------------------------
void vtkProstateNavGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{

  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  vtkSlicerInteractorStyle *istyle0 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI0()->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle1 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI1()->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle2 
    = vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI2()->GetSliceViewer()
                                             ->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle());
  
  vtkCornerAnnotation *anno = NULL;
  if (style == istyle0)
    {
    anno = appGUI->GetMainSliceGUI0()->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  else if (style == istyle1)
    {
    anno = appGUI->GetMainSliceGUI1()->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
    }
  else if (style == istyle2)
    {
    anno = appGUI->GetMainSliceGUI2()->GetSliceViewer()->GetRenderWidget()->GetCornerAnnotation();
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
    if (phase < vtkProstateNavLogic::NumPhases) // if pressed one of them
      {
      ChangeWorkPhase(phase, 1);
      }
    }


  //----------------------------------------------------------------
  // Wizard Frame

  if (this->WizardWidget->GetWizardWorkflow() == vtkKWWizardWorkflow::SafeDownCast(caller) &&
      event == vtkKWWizardWorkflow::CurrentStateChangedEvent)
    {
          
    int phase = vtkProstateNavLogic::Emergency;
    vtkKWWizardStep* step =  this->WizardWidget->GetWizardWorkflow()->GetCurrentStep();

    for (int i = 0; i < vtkProstateNavLogic::NumPhases-1; i ++)
      {
      if (step == vtkKWWizardStep::SafeDownCast(this->WizardSteps[i]))
        {
        phase = i;
        }
      }
    
    ChangeWorkPhase(phase);
    }


  //----------------------------------------------------------------
  // Visualization Control Frame
  
  else if (this->LocatorCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    int checked = this->LocatorCheckButton->GetSelectedState(); 
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID("vtkMRMLModelNode1")); 
    if (model != NULL)
      {
      vtkMRMLModelDisplayNode *disp = model->GetModelDisplayNode();
        
      if (disp != NULL)
        {
        vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
        vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
        disp->SetColor(color->SliceGUIGreen);
        disp->SetVisibility(checked);
        }
      }
    }
  else if (this->RedSliceMenu->GetMenu() == vtkKWMenu::SafeDownCast(caller)
            && event == vtkKWMenu::MenuItemInvokedEvent)
    {
    const char* selected = this->RedSliceMenu->GetValue();
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_RED, selected);
    }

  else if (this->YellowSliceMenu->GetMenu() == vtkKWMenu::SafeDownCast(caller)
            && event == vtkKWMenu::MenuItemInvokedEvent)
    {
    const char* selected = this->YellowSliceMenu->GetValue();
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_YELLOW, selected);
    }

  else if (this->GreenSliceMenu->GetMenu() == vtkKWMenu::SafeDownCast(caller)
          && event == vtkKWMenu::MenuItemInvokedEvent)
    {
    const char* selected = this->GreenSliceMenu->GetValue();
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_GREEN, selected);
    }

  // -- "Locator" button 
  else if (this->SetLocatorModeButton == vtkKWPushButton::SafeDownCast(caller) 
            && event == vtkKWPushButton::InvokedEvent)
    {
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_RED, "Locator");
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_YELLOW, "Locator");
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_GREEN, "Locator");
    }
  
  // -- "User" button 
  else if (this->SetUserModeButton == vtkKWPushButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent)
    {
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_RED, "User");
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_YELLOW, "User");
    ChangeSlicePlaneDriver(vtkProstateNavGUI::SLICE_PLANE_GREEN, "User");
    }
  
  // -- "Freeze Image Position" check button 
  else if (this->FreezeImageCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    if (this->FreezeImageCheckButton->GetSelectedState() == 1)
      {
      this->FreezeOrientationUpdate = 1;
      }
    else
      {
      this->FreezeOrientationUpdate = 0;
      }
    }
  else if (this->ImagingMenu->GetMenu() == vtkKWMenu::SafeDownCast(caller)
           && event == vtkKWMenu::MenuItemInvokedEvent )
    {
      
    const char* selected = this->ImagingMenu->GetValue();
    if (strcmp(selected, "None") == 0)
      {
      //this->RealtimeImageOrient = vtkProstateNavGUI::SLICE_RTIMAGE_NONE;
      this->GetLogic()->SetRealtimeImageOrient(vtkProstateNavLogic::SLICE_RTIMAGE_NONE);
      }
    else if (strcmp(selected, "Perpendicular") == 0)
      {
      //this->RealtimeImageOrient = vtkProstateNavGUI::SLICE_RTIMAGE_PERP;
      this->GetLogic()->SetRealtimeImageOrient(vtkProstateNavLogic::SLICE_RTIMAGE_PERP);
      }
    else if (strcmp(selected, "In-plane 90") == 0)
      {
      //this->RealtimeImageOrient = vtkProstateNavGUI::SLICE_RTIMAGE_INPLANE90;
      this->GetLogic()->SetRealtimeImageOrient(vtkProstateNavLogic::SLICE_RTIMAGE_INPLANE90);
      }
    else //if ( strcmp(selected, "In-plane") == 0 )
      {
      //this->RealtimeImageOrient = vtkProstateNavGUI::SLICE_RTIMAGE_INPLANE;
      this->GetLogic()->SetRealtimeImageOrient(vtkProstateNavLogic::SLICE_RTIMAGE_INPLANE);
      }
    
    std::cerr << "ImagingMenu =======> " << selected << "  :  " << this->RealtimeImageOrient << std::endl;
    
    }

  else if (this->StartScanButton == vtkKWPushButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent)
    {
    this->Logic->ScanStart();
    }
  else if (this->StopScanButton == vtkKWPushButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent)
    {
    this->Logic->ScanStop();
    }
  

  //----------------------------------------------------------------
  // Etc Frame

          

  // Process Wizard GUI (Active step only)
  else
    {
    int phase = this->Logic->GetCurrentPhase();
    this->WizardSteps[phase]->ProcessGUIEvents(caller, event, callData);
    }

} 


void vtkProstateNavGUI::Init()
{
    this->DataManager->SetMRMLScene(this->GetMRMLScene());
    //   this->LocatorModelID = std::string(this->DataManager->RegisterStream(0));
    //this->LocatorModelID_new = std::string(this->DataManager->RegisterStream_new(0));
    
}



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
  
  this->SliceNode0 = appGUI->GetMainSliceGUI0()->GetLogic()->GetSliceNode();
  this->SliceNode1 = appGUI->GetMainSliceGUI1()->GetLogic()->GetSliceNode();
  this->SliceNode2 = appGUI->GetMainSliceGUI2()->GetLogic()->GetSliceNode();
  /*
  this->Control0 = appGUI->GetMainSliceGUI0()->GetSliceController();
  this->Control1 = appGUI->GetMainSliceGUI1()->GetSliceController();
  this->Control2 = appGUI->GetMainSliceGUI2()->GetSliceController();
  */
  
  this->GetLogic()->AddRealtimeVolumeNode("Realtime");
  
  ChangeWorkPhase(vtkProstateNavLogic::StartUp, 1);
  
  // neccessary?
  //this->Logic0->GetForegroundLayer()->SetUseReslice(0);
  
  //----------------------------------------------------------------
  // Target Fiducials
  // Junichi Tokuda 11/27/2007: Should it be in the Logic class ?
  
  if (!this->FiducialListNodeID)
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // Get a pointer to the Fiducials module
    vtkSlicerFiducialsGUI *fidGUI
      = (vtkSlicerFiducialsGUI*)app->GetModuleGUIByName("Fiducials");
    fidGUI->Enter();

    // Create New Fiducial list for Prostate Module
    vtkSlicerFiducialsLogic *fidLogic = (vtkSlicerFiducialsLogic*)(fidGUI->GetLogic());
    vtkMRMLFiducialListNode *newList = fidLogic->AddFiducialList();

    if (newList != NULL)
      {
      // Change the name of the list
      newList->SetName(this->GetMRMLScene()->GetUniqueNameByString("PM"));

      fidGUI->SetFiducialListNodeID(newList->GetID());
      newList->Delete();
      }
    else
      {
        vtkErrorMacro("Unable to add a new fid list via the logic\n");
      }
    // now get the newly active node 
    this->FiducialListNodeID = fidGUI->GetFiducialListNodeID();
    this->FiducialListNode 
      = (vtkMRMLFiducialListNode *)this->GetMRMLScene()->GetNodeByID(this->FiducialListNodeID);
    if (this->FiducialListNode == NULL)
      {
      vtkErrorMacro ("ERROR adding a new fiducial list for the point...\n");
      return;
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
    BuildGUIForVisualizationControlFrame();
    //BuildGUIForDeviceFrame();

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
    // Config File step

    if (!this->WizardSteps[vtkProstateNavLogic::StartUp])
      {
      this->WizardSteps[vtkProstateNavLogic::StartUp] = vtkProstateNavConfigurationStep::New();
      }

    // -----------------------------------------------------------------
    // Scan Control step

    if (!this->WizardSteps[vtkProstateNavLogic::Planning])
      {
      this->WizardSteps[vtkProstateNavLogic::Planning] = vtkProstateNavScanControlStep::New();
      }

    // -----------------------------------------------------------------
    // Calibration step

    if (!this->WizardSteps[vtkProstateNavLogic::Calibration])
      {
      this->WizardSteps[vtkProstateNavLogic::Calibration] = vtkProstateNavCalibrationStep::New();
      }

    // -----------------------------------------------------------------
    // Targeting step

    if (!this->WizardSteps[vtkProstateNavLogic::Targeting])
      {
      this->WizardSteps[vtkProstateNavLogic::Targeting] = vtkProstateNavTargetingStep::New();
      }

    // -----------------------------------------------------------------
    // ManualControl step

    if (!this->WizardSteps[vtkProstateNavLogic::Manual])
      {
      this->WizardSteps[vtkProstateNavLogic::Manual] = vtkProstateNavManualControlStep::New();
      }


    // -----------------------------------------------------------------
    // Set GUI/Logic to each step and add to workflow

    for (int i = 0; i < vtkProstateNavLogic::NumPhases-1; i ++)
      {
      this->WizardSteps[i]->SetGUI(this);
      this->WizardSteps[i]->SetLogic(this->Logic);

      // Set color for the wizard title:

      this->WizardSteps[i]->SetTitleBackgroundColor(0.8, 0.8, 1.0);
      //this->WizardSteps[i]->SetTitleBackgroundColor(WorkPhaseColor[i][0],
      //                                              WorkPhaseColor[i][1],
      //                                              WorkPhaseColor[i][2]);
      wizard_workflow->AddNextStep(this->WizardSteps[i]);
      }


    // -----------------------------------------------------------------
    // Initial and finish step

    //wizard_workflow->SetFinishStep(this->ManualControlStep);
    wizard_workflow->SetFinishStep(this->WizardSteps[vtkProstateNavLogic::Manual]);
    wizard_workflow->CreateGoToTransitionsToFinishStep();
    //wizard_workflow->SetInitialStep(this->ConfigurationStep);
    wizard_workflow->SetInitialStep(this->WizardSteps[vtkProstateNavLogic::StartUp]);

    // -----------------------------------------------------------------
    // Show the user interface

    this->WizardWidget->GetWizardWorkflow()->
      GetCurrentStep()->ShowUserInterface();

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
  
  for (int i = 0; i < vtkProstateNavLogic::NumPhases; i ++)
    {
    this->WorkPhaseButtonSet->AddWidget(i);
    this->WorkPhaseButtonSet->GetWidget(i)->SetWidth(16);
    this->WorkPhaseButtonSet->GetWidget(i)->SetText(WorkPhaseStr[i]);
    this->WorkPhaseButtonSet->GetWidget(i)
      ->SetBackgroundColor(WorkPhaseColor[i][0], WorkPhaseColor[i][1], WorkPhaseColor[i][2]);
    this->WorkPhaseButtonSet->GetWidget(i)
      ->SetActiveBackgroundColor(WorkPhaseColor[i][0], WorkPhaseColor[i][1], WorkPhaseColor[i][2]);
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

//---------------------------------------------------------------------------
void vtkProstateNavGUI::BuildGUIForVisualizationControlFrame ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("ProstateNav");
  
  vtkSlicerModuleCollapsibleFrame *visCtrlFrame = vtkSlicerModuleCollapsibleFrame::New();
  visCtrlFrame->SetParent(page);
  visCtrlFrame->Create();
  visCtrlFrame->SetLabelText("Visualization / Scanner Control");
  visCtrlFrame->CollapseFrame();
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               visCtrlFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Locator Display frame: Options to locator display 

  vtkKWFrameWithLabel *displayFrame = vtkKWFrameWithLabel::New ( );
  displayFrame->SetParent(visCtrlFrame->GetFrame());
  displayFrame->Create();
  displayFrame->SetLabelText("Locator Display");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               displayFrame->GetWidgetName());
  
  this->LocatorCheckButton = vtkKWCheckButton::New();
  this->LocatorCheckButton->SetParent(displayFrame->GetFrame());
  this->LocatorCheckButton->Create();
  this->LocatorCheckButton->SelectedStateOff();
  this->LocatorCheckButton->SetText("Show Locator");
  
  this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
               this->LocatorCheckButton->GetWidgetName());
  
  
  // -----------------------------------------
  // Driver frame: Locator can drive slices 

  vtkKWFrameWithLabel *driverFrame = vtkKWFrameWithLabel::New();
  driverFrame->SetParent(visCtrlFrame->GetFrame());
  driverFrame->Create();
  driverFrame->SetLabelText ("Driver");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               driverFrame->GetWidgetName());
  
  // slice frame
  vtkKWFrame *sliceFrame = vtkKWFrame::New();
  sliceFrame->SetParent(driverFrame->GetFrame());
  sliceFrame->Create();
  app->Script("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
              sliceFrame->GetWidgetName(),
              driverFrame->GetFrame()->GetWidgetName());
  
  
  // Contents in slice frame 
  vtkSlicerColor *color = app->GetSlicerTheme()->GetSlicerColors ( );
  
  this->RedSliceMenu = vtkKWMenuButton::New();
  this->RedSliceMenu->SetParent(sliceFrame);
  this->RedSliceMenu->Create();
  this->RedSliceMenu->SetWidth(10);
  this->RedSliceMenu->SetBackgroundColor(color->SliceGUIRed);
  this->RedSliceMenu->SetActiveBackgroundColor(color->SliceGUIRed);
  this->RedSliceMenu->GetMenu()->AddRadioButton ("User");
  this->RedSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->RedSliceMenu->GetMenu()->AddRadioButton ("RT Image");
  this->RedSliceMenu->SetValue ("User");
  
  this->YellowSliceMenu = vtkKWMenuButton::New();
  this->YellowSliceMenu->SetParent(sliceFrame);
  this->YellowSliceMenu->Create();
  this->YellowSliceMenu->SetWidth(10);
  this->YellowSliceMenu->SetBackgroundColor(color->SliceGUIYellow);
  this->YellowSliceMenu->SetActiveBackgroundColor(color->SliceGUIYellow);
  this->YellowSliceMenu->GetMenu()->AddRadioButton ("User");
  this->YellowSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->YellowSliceMenu->GetMenu()->AddRadioButton ("RT Image");
  this->YellowSliceMenu->SetValue ("User");
  
  this->GreenSliceMenu = vtkKWMenuButton::New();
  this->GreenSliceMenu->SetParent(sliceFrame);
  this->GreenSliceMenu->Create();
  this->GreenSliceMenu->SetWidth(10);
  this->GreenSliceMenu->SetBackgroundColor(color->SliceGUIGreen);
  this->GreenSliceMenu->SetActiveBackgroundColor(color->SliceGUIGreen);
  this->GreenSliceMenu->GetMenu()->AddRadioButton ("User");
  this->GreenSliceMenu->GetMenu()->AddRadioButton ("Locator");
  this->GreenSliceMenu->GetMenu()->AddRadioButton ("RT Image");
  this->GreenSliceMenu->SetValue ("User");
  
  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
               this->RedSliceMenu->GetWidgetName(),
               this->YellowSliceMenu->GetWidgetName(),
               this->GreenSliceMenu->GetWidgetName());
  
  
  // Mode frame
  vtkKWFrame *modeFrame = vtkKWFrame::New();
  modeFrame->SetParent ( driverFrame->GetFrame() );
  modeFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
               modeFrame->GetWidgetName(),
               driverFrame->GetFrame()->GetWidgetName());
  
  // "Locator All" button
  this->SetLocatorModeButton = vtkKWPushButton::New ( );
  this->SetLocatorModeButton->SetParent ( modeFrame );
  this->SetLocatorModeButton->Create ( );
  this->SetLocatorModeButton->SetText ("Locator All");
  this->SetLocatorModeButton->SetWidth (12);
  
  // "User All" button
  this->SetUserModeButton = vtkKWPushButton::New ( );
  this->SetUserModeButton->SetParent ( modeFrame );
  this->SetUserModeButton->Create ( );
  this->SetUserModeButton->SetText ("User All");
  this->SetUserModeButton->SetWidth (12);
  
  
  // "Freeze" check button
  this->FreezeImageCheckButton = vtkKWCheckButton::New();
  this->FreezeImageCheckButton->SetParent(modeFrame);
  this->FreezeImageCheckButton->Create();
  this->FreezeImageCheckButton->SelectedStateOff();
  this->FreezeImageCheckButton->SetText("Freeze Image Position");
  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
               this->SetLocatorModeButton->GetWidgetName(),
               this->SetUserModeButton->GetWidgetName(),
               this->FreezeImageCheckButton->GetWidgetName());
  
  
  // -----------------------------------------
  // Real-time imaging: Scanner controled

  vtkKWFrameWithLabel *rtImageFrame = vtkKWFrameWithLabel::New ( );
  rtImageFrame->SetParent(visCtrlFrame->GetFrame());
  rtImageFrame->Create();
  rtImageFrame->SetLabelText("Real-time Imaging");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               rtImageFrame->GetWidgetName());

  // Scan start/stop frame
  vtkKWFrame *scanFrame = vtkKWFrame::New();
  scanFrame->SetParent (rtImageFrame->GetFrame());
  scanFrame->Create();
  app->Script("pack %s -side top -anchor nw -fill x -pady 1 -in %s",
              scanFrame->GetWidgetName(),
              rtImageFrame->GetFrame()->GetWidgetName());
  
  this->StartScanButton = vtkKWPushButton::New();
  this->StartScanButton->SetParent(scanFrame);
  this->StartScanButton->Create();
  this->StartScanButton->SetText("Start Scan");
  this->StartScanButton->SetWidth(12);
  
  this->StopScanButton = vtkKWPushButton::New();
  this->StopScanButton->SetParent(scanFrame);
  this->StopScanButton->Create();
  this->StopScanButton->SetText("Stop Scan");
  this->StopScanButton->SetWidth(12);

  this->ImagingMenu = vtkKWMenuButton::New();
  this->ImagingMenu->SetParent(scanFrame);
  this->ImagingMenu->Create();
  this->ImagingMenu->SetWidth(10);
  this->ImagingMenu->GetMenu()->AddRadioButton ("None");
  this->ImagingMenu->GetMenu()->AddRadioButton ("Perpendicular");
  this->ImagingMenu->GetMenu()->AddRadioButton ("In-plane 90");
  this->ImagingMenu->GetMenu()->AddRadioButton ("In-plane");
  this->ImagingMenu->SetValue("None");

  this->Script("pack %s %s %s -side left -anchor w -padx 2 -pady 2", 
               StartScanButton->GetWidgetName(),
               StopScanButton->GetWidgetName(),
               ImagingMenu->GetWidgetName());


  displayFrame->Delete();
  driverFrame->Delete();
  modeFrame->Delete();
  sliceFrame->Delete();
  visCtrlFrame->Delete();
}


//----------------------------------------------------------------------------
int vtkProstateNavGUI::ChangeWorkPhase(int phase, int fChangeWizard)
{

  cerr << "ChangeWorkPhase: started" << endl;
    if (!this->Logic->SwitchWorkPhase(phase)) // Set next phase
    {
      cerr << "ChangeWorkPhase: Cannot make transition!" << endl;
        return 0;
    }
  
    for (int i = 0; i < vtkProstateNavLogic::NumPhases; i ++)
    {
        vtkKWPushButton *pb = this->WorkPhaseButtonSet->GetWidget(i);
        if (i == this->Logic->GetCurrentPhase())
        {
            pb->SetReliefToSunken();
        }
        else if (this->Logic->IsPhaseTransitable(i))
        {
            pb->SetReliefToGroove();
            pb->SetStateToNormal();
            pb->SetBackgroundColor(WorkPhaseColor[i][0],
                                   WorkPhaseColor[i][1],
                                   WorkPhaseColor[i][2]);
        }
        else
        {
            pb->SetReliefToGroove();
            pb->SetStateToDisabled();
            pb->SetBackgroundColor(WorkPhaseColorDisabled[i][0],
                                   WorkPhaseColorDisabled[i][1],
                                   WorkPhaseColorDisabled[i][2]);
        }
    }

    // Switch Wizard Frame
    // 11/09/2007 Junichi Tokuda -- This part looks ugly. Will be fixed later.
    if (fChangeWizard)
    {
        vtkKWWizardWorkflow *wizard = 
          this->WizardWidget->GetWizardWorkflow();
        
        int step_from;
        int step_to;
        
        step_to = this->Logic->GetCurrentPhase();
        step_from = this->Logic->GetPrevPhase();
        if (step_to == vtkProstateNavLogic::Emergency)
        {
            step_to = vtkProstateNavLogic::Manual;
        }
        if (step_from == vtkProstateNavLogic::Emergency)
        {
            step_from = vtkProstateNavLogic::Manual;
        }
        
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

  if (this->LocatorCheckButton->GetSelectedState())
    {
      //this->UpdateLocator();
    }
  if (!this->FreezeOrientationUpdate)
    {
      //this->UpdateSliceDisplay(nx, ny, nz, tx, ty, tz, px, py, pz);
    }

}


//----------------------------------------------------------------------------
void vtkProstateNavGUI::UpdateDeviceStatus()
{
  
  std::cerr << "Updating Device Status Display .........." << std::endl;

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
    sprintf(label, "RBT: %s", vtkProstateNavGUI::WorkPhaseStr[status]);
    this->RobotStatusLabelDisp->SetValue(label);
    this->RobotStatusLabelDisp->SetBackgroundColor(vtkProstateNavGUI::WorkPhaseColorActive[status][0],
                                                   vtkProstateNavGUI::WorkPhaseColorActive[status][1],
                                                   vtkProstateNavGUI::WorkPhaseColorActive[status][2]);
    }

  status = this->GetLogic()->GetScannerWorkPhase();
  if (status < 0)
    {
    this->ScannerStatusLabelDisp->SetValue(" SCANNER: OFF ");
    this->ScannerStatusLabelDisp->SetBackgroundColor(0.9, 0.9, 0.9);
    }
  else
    {
    sprintf(label, "SCNR: %s", vtkProstateNavGUI::WorkPhaseStr[status]);
    this->ScannerStatusLabelDisp->SetValue(label);
    this->ScannerStatusLabelDisp->SetBackgroundColor(vtkProstateNavGUI::WorkPhaseColorActive[status][0],
                                                     vtkProstateNavGUI::WorkPhaseColorActive[status][1],
                                                     vtkProstateNavGUI::WorkPhaseColorActive[status][2]);

    }

}


void vtkProstateNavGUI::ChangeSlicePlaneDriver(int slice, const char* driver)
{
  std::cerr << "ChangeSlicePlaneDriver -- Slice: " << slice << ", Driver: " << driver << std::endl;
  
  if (slice == vtkProstateNavGUI::SLICE_PLANE_RED)
    {
    this->RedSliceMenu->SetValue(driver);
    if (strcmp(driver, "User") == 0)
      {
      this->SliceNode0->SetOrientationToAxial();
      //this->SliceDriver0 = vtkProstateNavGUI::SLICE_DRIVER_USER;
      this->GetLogic()->SetSliceDriver0(vtkProstateNavLogic::SLICE_DRIVER_USER);
      }
    else if (strcmp(driver, "Locator") == 0)
      {
      //this->SliceDriver0 = vtkProstateNavGUI::SLICE_DRIVER_LOCATOR;
      this->GetLogic()->SetSliceDriver0(vtkProstateNavLogic::SLICE_DRIVER_LOCATOR);
      }
    else if (strcmp(driver, "RT Image") == 0)
      {
      //this->SliceDriver0 = vtkProstateNavGUI::SLICE_DRIVER_RTIMAGE;
      this->GetLogic()->SetSliceDriver0(vtkProstateNavLogic::SLICE_DRIVER_RTIMAGE);
      }
    }
  else if (slice == vtkProstateNavGUI::SLICE_PLANE_YELLOW)
    {
    this->YellowSliceMenu->SetValue(driver);
    if (strcmp(driver, "User") == 0)
      {
      this->SliceNode1->SetOrientationToSagittal();
      //this->SliceDriver1 = vtkProstateNavGUI::SLICE_DRIVER_USER;
      this->GetLogic()->SetSliceDriver1(vtkProstateNavLogic::SLICE_DRIVER_USER);
      }
    else if (strcmp(driver, "Locator") == 0)
      {
      //this->SliceDriver1 = vtkProstateNavGUI::SLICE_DRIVER_LOCATOR;
      this->GetLogic()->SetSliceDriver1(vtkProstateNavLogic::SLICE_DRIVER_LOCATOR);
      }
    else if (strcmp(driver, "RT Image") == 0)
      {
      //this->SliceDriver1 = vtkProstateNavGUI::SLICE_DRIVER_RTIMAGE;
      this->GetLogic()->SetSliceDriver1(vtkProstateNavLogic::SLICE_DRIVER_RTIMAGE);
      }
    }
  else //if ( slice == vtkProstateNavGUI::SLICE_PLANE_GREEN )
    {
    this->GreenSliceMenu->SetValue(driver);
    if (strcmp(driver, "User") == 0)
      {
      this->SliceNode2->SetOrientationToCoronal();
      //this->SliceDriver2 = vtkProstateNavGUI::SLICE_DRIVER_USER;
      this->GetLogic()->SetSliceDriver2(vtkProstateNavLogic::SLICE_DRIVER_USER);
      }
    else if (strcmp(driver, "Locator") == 0)
      {
      //this->SliceDriver2 = vtkProstateNavGUI::SLICE_DRIVER_LOCATOR;
      this->GetLogic()->SetSliceDriver2(vtkProstateNavLogic::SLICE_DRIVER_LOCATOR);
      }
    else if (strcmp(driver, "RT Image") == 0)
      {
      //this->SliceDriver2 = vtkProstateNavGUI::SLICE_DRIVER_RTIMAGE;
      this->GetLogic()->SetSliceDriver2(vtkProstateNavLogic::SLICE_DRIVER_RTIMAGE);
      }
    }
}

