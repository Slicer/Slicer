#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkPatientToImageRegistrationGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWRenderWidget.h"
#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWPushButton.h"
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


#include "vtkKWTkUtilities.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkCornerAnnotation.h"

#include "vtkIGTDataStream.h"

#include "vtkCylinderSource.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerNodeSelectorWidget.h"


//---------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkPatientToImageRegistrationGUI, "$Revision: 1.0 $");


vtkPatientToImageRegistrationGUI* vtkPatientToImageRegistrationGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkPatientToImageRegistrationGUI");
  if (ret)
    {
    return (vtkPatientToImageRegistrationGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPatientToImageRegistrationGUI;
}



//---------------------------------------------------------------------------
vtkPatientToImageRegistrationGUI::vtkPatientToImageRegistrationGUI ( )
{
  this->Logic = NULL;

  this->TransformNodeNameEntry = NULL;
  this->FiducialListNodeNameEntry = NULL;

  this->PatCoordinatesEntry = NULL;
  this->SlicerCoordinatesEntry = NULL;
  this->GetPatCoordinatesPushButton = NULL;
  this->AddPointPairPushButton = NULL;

  this->PointPairMultiColumnList = NULL;

  // this->LoadPointPairPushButton = NULL;
  // this->SavePointPairPushButton = NULL;
  this->DeletePointPairPushButton = NULL;
  this->DeleteAllPointPairPushButton = NULL;
  this->RegisterPushButton = NULL;
  this->ResetPushButton = NULL;

  this->CloseScene = false;
  this->TimerFlag = 0;

  this->TranslationScale = NULL;
  this->TrackerSelector = NULL;
  this->PivotCalibrationErrorReport = NULL;
  this->OffsetReport = NULL;
  this->PivotCalibrationLabel = NULL;
  this->StartPivotCalibrationPushButton = NULL;
  this->FinishPivotCalibrationPushButton = NULL;
  this->PivotCalibrationCheckButton = NULL;
 
}



//---------------------------------------------------------------------------
vtkPatientToImageRegistrationGUI::~vtkPatientToImageRegistrationGUI ( )
{
  this->RemoveGUIObservers();

 if (this->TransformNodeNameEntry)
    {
    this->TransformNodeNameEntry->SetParent(NULL);
    this->TransformNodeNameEntry->Delete();
    }
  if (this->FiducialListNodeNameEntry)
    {
    this->FiducialListNodeNameEntry->SetParent(NULL);
    this->FiducialListNodeNameEntry->Delete();
    }
  if (this->PatCoordinatesEntry)
    {
    this->PatCoordinatesEntry->SetParent(NULL );
    this->PatCoordinatesEntry->Delete ( );
    }
  if (this->SlicerCoordinatesEntry)
    {
    this->SlicerCoordinatesEntry->SetParent(NULL );
    this->SlicerCoordinatesEntry->Delete ( );
    }
  if (this->GetPatCoordinatesPushButton)
    {
    this->GetPatCoordinatesPushButton->SetParent(NULL );
    this->GetPatCoordinatesPushButton->Delete ( );
    }
  if (this->AddPointPairPushButton)
    {
    this->AddPointPairPushButton->SetParent(NULL );
    this->AddPointPairPushButton->Delete ( );
    }

  if (this->PointPairMultiColumnList)
    {
    this->PointPairMultiColumnList->SetParent(NULL );
    this->PointPairMultiColumnList->Delete ( );
    }

  /*
     if (this->LoadPointPairPushButton)
     {
     this->LoadPointPairPushButton->SetParent(NULL );
     this->LoadPointPairPushButton->Delete ( );
     }
     if (this->SavePointPairPushButton)
     {
     this->SavePointPairPushButton->SetParent(NULL );
     this->SavePointPairPushButton->Delete ( );
     }
     */
  if (this->DeletePointPairPushButton)
    {
    this->DeletePointPairPushButton->SetParent(NULL );
    this->DeletePointPairPushButton->Delete ( );
    }
  if (this->DeleteAllPointPairPushButton)
    {
    this->DeleteAllPointPairPushButton->SetParent(NULL );
    this->DeleteAllPointPairPushButton->Delete ( );
    }
  if (this->RegisterPushButton)
    {
    this->RegisterPushButton->SetParent(NULL );
    this->RegisterPushButton->Delete ( );
    }
  if (this->ResetPushButton)
    {
    this->ResetPushButton->SetParent(NULL );
    this->ResetPushButton->Delete ( );
    }

  if (this->TranslationScale)
    {
    this->TranslationScale->SetParent(NULL);
    this->TranslationScale->Delete();
    this->TranslationScale = NULL;
    }

  if (this->TrackerSelector)
    {
    this->TrackerSelector->SetParent(NULL);
    this->TrackerSelector->Delete();
    }
  if (this->PivotCalibrationErrorReport)
    {
    this->PivotCalibrationErrorReport->SetParent(NULL);
    this->PivotCalibrationErrorReport->Delete();
    }
  if (this->OffsetReport)
    {
    this->OffsetReport->SetParent(NULL);
    this->OffsetReport->Delete();
    }
  if (this->PivotCalibrationLabel)
    {
    this->PivotCalibrationLabel->SetParent(NULL);
    this->PivotCalibrationLabel->Delete();
    }
  if (this->StartPivotCalibrationPushButton)
    {
    this->StartPivotCalibrationPushButton->SetParent(NULL);
    this->StartPivotCalibrationPushButton->Delete();
    }
  if (this->FinishPivotCalibrationPushButton)
    {
    this->FinishPivotCalibrationPushButton->SetParent(NULL);
    this->FinishPivotCalibrationPushButton->Delete();
    }
  if (this->PivotCalibrationCheckButton)
    {
    this->PivotCalibrationCheckButton->SetParent(NULL);
    this->PivotCalibrationCheckButton->Delete();
    }

  this->SetModuleLogic ( NULL );
}



//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "PatientToImageRegistrationGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
  // print widgets?
}


//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::RemoveGUIObservers ( )
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if ( appGUI )
    {
    appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
      ->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

//  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
//  appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
//  appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
//  appGUI->GetMainSliceGUI("Blue")->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle()->RemoveObserver((vtkCommand *)this->GUICallbackCommand);


  if (this->GetPatCoordinatesPushButton)
    {
    this->GetPatCoordinatesPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AddPointPairPushButton)
    {
    this->AddPointPairPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->DeletePointPairPushButton)
    {
    this->DeletePointPairPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->DeleteAllPointPairPushButton)
    {
    this->DeleteAllPointPairPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->RegisterPushButton)
    {
    this->RegisterPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ResetPushButton)
    {
    this->ResetPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->StartPivotCalibrationPushButton)
    {
    this->StartPivotCalibrationPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
   if (this->FinishPivotCalibrationPushButton)
    {
    this->FinishPivotCalibrationPushButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->PivotCalibrationCheckButton)
    {
    this->PivotCalibrationCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
 
  if (this->TrackerSelector)
    {
    this->TrackerSelector->
        RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
}



//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  // make a user interactor style to process our events
  // look at the InteractorStyle to get our events

  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  //events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  //events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  this->TrackerSelector
    ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  //----------------------------------------------------------------
  // Main Slice GUI

  appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
    ->GetRenderWindowInteractor()->GetInteractorStyle()
    ->AddObserver(vtkCommand::LeftButtonPressEvent, (vtkCommand *)this->GUICallbackCommand);
  

  // Fill in
  // observer load volume button
  this->GetPatCoordinatesPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddPointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeletePointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllPointPairPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->RegisterPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  //this->ResetPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StartPivotCalibrationPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FinishPivotCalibrationPushButton->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PivotCalibrationCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

}



void vtkPatientToImageRegistrationGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  vtkSlicerInteractorStyle *istyle0 = 
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Red")->GetSliceViewer()->GetRenderWidget()
                                           ->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle1 = 
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Yellow")->GetSliceViewer()->GetRenderWidget()
                                           ->GetRenderWindowInteractor()->GetInteractorStyle());
  vtkSlicerInteractorStyle *istyle2 = 
    vtkSlicerInteractorStyle::SafeDownCast(appGUI->GetMainSliceGUI("Green")->GetSliceViewer()->GetRenderWidget()
                                           ->GetRenderWindowInteractor()->GetInteractorStyle());

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
    anno->Print(cout);
    const char *rasText = anno->GetText(1);
    if ( rasText != NULL )
      {
      std::string ras = std::string(rasText);

      // remove "R:," "A:," and "S:" from the string
      size_t loc = ras.find("R:", 0);
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
      size_t found = ras.find("\n", 0);
      while ( found != std::string::npos )
        {
        ras = ras.replace(found, 1, " ");
        found = ras.find("\n", 0);
        }

      this->SlicerCoordinatesEntry->GetWidget()->SetValue(ras.c_str());
      }
    }
}


//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::ProcessGUIEvents ( vtkObject *caller,
                                        unsigned long event, void *callData )
{
  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    // Get RAS coordinates of mouse click from any 2D slice windows
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    }
  else
    {
    // Handle mouse click event on Slicer interface on the left
    //
    if (this->GetPatCoordinatesPushButton == vtkKWPushButton::SafeDownCast(caller) 
        && event == vtkKWPushButton::InvokedEvent)
      {
      double px, py, pz;
      char value[50];
      this->GetLogic()->GetCurrentPosition(&px, &py, &pz);
      sprintf(value, "%6.2f  %6.2f  %6.2f", px, py, pz);
      this->PatCoordinatesEntry->GetWidget()->SetValue(value);
      }
      else if (this->AddPointPairPushButton == vtkKWPushButton::SafeDownCast(caller) 
               && event == vtkKWPushButton::InvokedEvent)
        {
        int scSize = 0;
        int pcSize = 0;
        const char *pc = this->PatCoordinatesEntry->GetWidget()->GetValue();
        const char *sc = this->SlicerCoordinatesEntry->GetWidget()->GetValue();

        if (pc) 
          {
          const vtksys_stl::string pcCor(pc);
          pcSize = pcCor.size();
        }
        if (sc) 
          {
          const vtksys_stl::string scCor(sc);
          scSize = scCor.size();
          }

        if (pcSize < 5 || scSize < 5)
          {
          vtkSlicerApplication::GetInstance()->ErrorMessage("Patient or Slicer coordinates are invalid."); 
          }
        else 
          {
          int row = this->PointPairMultiColumnList->GetWidget()->GetNumberOfRows();
          this->PointPairMultiColumnList->GetWidget()->AddRow();
          this->PointPairMultiColumnList->GetWidget()->SetCellText(row, 0, pc);
          this->PointPairMultiColumnList->GetWidget()->SetCellText(row, 1, sc);
          }
      }
    else if (this->DeletePointPairPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      int numOfRows = this->PointPairMultiColumnList->GetWidget()->GetNumberOfSelectedRows();
      if (numOfRows == 1)
        {
        int index[2];
        this->PointPairMultiColumnList->GetWidget()->GetSelectedRows(index);
        this->PointPairMultiColumnList->GetWidget()->DeleteRow(index[0]);
        }
      }
    else if (this->DeleteAllPointPairPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      this->PointPairMultiColumnList->GetWidget()->DeleteAllRows();
      }
    else if (this->RegisterPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      int row = this->PointPairMultiColumnList->GetWidget()->GetNumberOfRows();
      if (row < 2)
        {
        vtkSlicerApplication::GetInstance()->ErrorMessage("At least 2 pairs of landmarks are needed for patient to image registration.");
        }
      else
        {
        this->GetLogic()->GetPat2ImgReg()->SetNumberOfPoints(row);
        float pc1 = 0.0, pc2 = 0.0, pc3 = 0.0, sc1 = 0.0, sc2 = 0.0, sc3 = 0.0;
        for (int r = 0; r < row; r++)
          {
          for (int c = 0; c < 2; c++)
            {
            const char *val = this->PointPairMultiColumnList->GetWidget()->GetCellText(r, c);
            if (c == 0)
              {
              sscanf(val, "%f %f %f", &pc1, &pc2, &pc3);
              }
            else
              {
              sscanf(val, "%f %f %f", &sc1, &sc2, &sc3);
              }
            }
          this->GetLogic()->GetPat2ImgReg()->AddPoint(r, sc1, sc2, sc3, pc1, pc2, pc3);
          }

        int error = this->GetLogic()->PerformPatientToImageRegistration();
        if (error)
          {
          vtkSlicerApplication::GetInstance()->ErrorMessage("Error registration between patient and image landmarks.");
          return;
          }
        }
      }
    else if (this->ResetPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      this->GetLogic()->SetUseRegistration(0);
      }
    else if (this->StartPivotCalibrationPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      // std::cerr << "DEBUG: StartPivotCalibrationPushButton is selected." << std::endl;
      if (this->TrackerSelector->GetSelected() == NULL)
        {
        //--- report an error to user if no tracker is available 
        std::string message = "Please make sure your tracker is available.";
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent(this->StartPivotCalibrationPushButton);
        dialog->SetStyleToCancel();
        dialog->SetText(message.c_str());
        dialog->Create( );
        dialog->SetMasterWindow(this->StartPivotCalibrationPushButton);
        dialog->ModalOn();
        dialog->Invoke();
        dialog->Delete();
        }
      else
        {
        this->GetLogic()->SetOriginalTrackerNode(vtkMRMLLinearTransformNode::SafeDownCast(this->TrackerSelector->GetSelected())); 
        this->GetLogic()->CollectDataForPivotCalibration(1);
        this->DoPivot = true;
        }
      }
    else if (this->FinishPivotCalibrationPushButton == vtkKWPushButton::SafeDownCast(caller) 
             && event == vtkKWPushButton::InvokedEvent)
      {
      //std::cerr << "DEBUG: FinishPivotCalibrationPushButton is selected." << std::endl;
      this->DoPivot = false;
      this->GetLogic()->ComputePivotCalibration();
      // report offset 
      double trans[3];
      this->GetLogic()->GetTranslation(trans);
      char str[10];
      sprintf(str, "%4.2f", trans[2]);
      this->OffsetReport->GetWidget()->SetValue(str);

      // report error
      double error = this->GetLogic()->GetRMSE();
      sprintf(str, "%4.2f", error);
      this->PivotCalibrationErrorReport->GetWidget()->SetValue(str);

      }
    else if (this->PivotCalibrationCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
             && event == vtkKWCheckButton::SelectedStateChangedEvent)
      {
      this->GetLogic()->SetUsePivotCalibration(this->PivotCalibrationCheckButton->GetSelectedState());
      }
    else if (this->TrackerSelector ==  vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
             && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
             this->TrackerSelector->GetSelected() != NULL)
      {
      //TODO:this->UpdateMRML();
      std::cerr << "DEBUG: TrackerSelector is selected." << std::endl;
      }
    }
} 



//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::ProcessLogicEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::ProcessMRMLEvents ( vtkObject *caller,
                                         unsigned long event, void *callData )
{
  if (event == vtkMRMLScene::SceneClosedEvent)
    {
    }
}



//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::Enter ( )
{
  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
    this->TimerInterval = 100;  // 100 ms
    ProcessTimerEvents();
    }

}



//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {

    // continue to collect tracking data for pivot calibration
    if (this->DoPivot)
      { 
      this->GetLogic()->CollectDataForPivotCalibration(0);
      }

    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}



//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::Exit ( )
{
  this->TimerFlag = 0;
}


//---------------------------------------------------------------------------
void vtkPatientToImageRegistrationGUI::BuildGUI ( )
{
  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "PatientToImageRegistration", "PatientToImageRegistration", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForTrackerFrame();
  BuildGUIForCalibrationFrame();
  BuildGUIForLandmarksFrame();
  // BuildGUIForICPFrame();
}



void vtkPatientToImageRegistrationGUI::BuildGUIForHelpFrame()
{

  // ----------------------------------------------------------------
  // HELP FRAME         
  // ----------------------------------------------------------------

  // Define your help text here.

  const char *help = "PatientToImageRegistration is a patient to image registration module for surgical navigation. \n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:PatientToImageRegistration-Documentation-3.4</a>";
  const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. The PatientToImageRegistration module was contributed by Haiying Liu and Noby Hata at SPL, BWH (Ron Kikinis).";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "PatientToImageRegistration" );
  this->BuildHelpAndAboutFrame (page, help, about);

}



void vtkPatientToImageRegistrationGUI::BuildGUIForTrackerFrame()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "PatientToImageRegistration" );

  // ----------------------------------------------------------------
  // Tracker frame
  // ----------------------------------------------------------------
  vtkSlicerModuleCollapsibleFrame *trackerFrame = vtkSlicerModuleCollapsibleFrame::New ( );    
  trackerFrame->SetParent ( page );
  trackerFrame->Create ( );
  trackerFrame->SetLabelText ("Tracker");
  trackerFrame->CollapseFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                trackerFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Tracker List Selector

  this->TrackerSelector = vtkSlicerNodeSelectorWidget::New();
  this->TrackerSelector->SetNodeClass("vtkMRMLLinearTransformNode", NULL, NULL, NULL);
  //this->TrackerSelector->SetNewNodeEnabled(1); // don't want to create this here. Create in OpenIGTLink.
  this->TrackerSelector->SetParent( trackerFrame->GetFrame() );
  this->TrackerSelector->Create();
  this->TrackerSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->TrackerSelector->UpdateMenu();

  this->TrackerSelector->SetBorderWidth(2);
  this->TrackerSelector->SetLabelText( "Probe Reading: ");
  this->TrackerSelector->SetBalloonHelpString("Tracking data from a probe stored in a transform.");

  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->TrackerSelector->GetWidgetName());

  trackerFrame->Delete();
}



void vtkPatientToImageRegistrationGUI::BuildGUIForCalibrationFrame()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "PatientToImageRegistration" );

  // ----------------------------------------------------------------
  // Calibration frame
  // ----------------------------------------------------------------
  vtkSlicerModuleCollapsibleFrame *calibrationFrame = vtkSlicerModuleCollapsibleFrame::New ( );    
  calibrationFrame->SetParent ( page );
  calibrationFrame->Create ( );
  calibrationFrame->SetLabelText ("Calibration");
  calibrationFrame->CollapseFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                calibrationFrame->GetWidgetName(), page->GetWidgetName());


  // add calibration processing frame 
  vtkKWFrame *processFrame = vtkKWFrame::New();
  processFrame->SetParent ( calibrationFrame->GetFrame() );
  processFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                processFrame->GetWidgetName());

  this->PivotCalibrationLabel = vtkKWLabel::New();
  this->PivotCalibrationLabel->SetParent(processFrame);
  this->PivotCalibrationLabel->Create();
  this->PivotCalibrationLabel->SetText("Pivot Calibration:");
  this->PivotCalibrationLabel->SetWidth ( 14 );

  this->StartPivotCalibrationPushButton = vtkKWPushButton::New();
  this->StartPivotCalibrationPushButton->SetParent(processFrame);
  this->StartPivotCalibrationPushButton->Create();
  this->StartPivotCalibrationPushButton->SetText("Start");
  this->StartPivotCalibrationPushButton->SetWidth ( 12 );

  this->FinishPivotCalibrationPushButton = vtkKWPushButton::New();
  this->FinishPivotCalibrationPushButton->SetParent(processFrame);
  this->FinishPivotCalibrationPushButton->Create();
  this->FinishPivotCalibrationPushButton->SetText( "Done" );
  this->FinishPivotCalibrationPushButton->SetWidth ( 12 );

  this->Script( "pack %s %s %s -side left -anchor nw -expand n -padx 2 -pady 2", 
                this->PivotCalibrationLabel->GetWidgetName(),
                this->StartPivotCalibrationPushButton->GetWidgetName(),
                this->FinishPivotCalibrationPushButton->GetWidgetName());

  // add the offset frame
  vtkKWFrame *offsetFrame = vtkKWFrame::New();
  offsetFrame->SetParent ( calibrationFrame->GetFrame() );
  offsetFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                offsetFrame->GetWidgetName());

  this->OffsetReport = vtkKWEntryWithLabel::New();
  this->OffsetReport->SetParent(offsetFrame);
  this->OffsetReport->Create();
  this->OffsetReport->SetWidth(40);
  this->OffsetReport->SetLabelWidth(20);
  this->OffsetReport->SetLabelText("Offset from tip to sensor:");
  this->OffsetReport->GetWidget()->SetValue ( "0.0" );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->OffsetReport->GetWidgetName());

  // add the error frame
  vtkKWFrame *errorFrame = vtkKWFrame::New();
  errorFrame->SetParent ( calibrationFrame->GetFrame() );
  errorFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                errorFrame->GetWidgetName());

  this->PivotCalibrationErrorReport = vtkKWEntryWithLabel::New();
  this->PivotCalibrationErrorReport->SetParent(errorFrame);
  this->PivotCalibrationErrorReport->Create();
  this->PivotCalibrationErrorReport->SetWidth(40);
  this->PivotCalibrationErrorReport->SetLabelWidth(20);
  this->PivotCalibrationErrorReport->SetLabelText("Root mean square error:");
  this->PivotCalibrationErrorReport->GetWidget()->SetValue ( "0.0" );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->PivotCalibrationErrorReport->GetWidgetName());

  // add the choice frame
  vtkKWFrame *choiceFrame = vtkKWFrame::New();
  choiceFrame->SetParent ( calibrationFrame->GetFrame() );
  choiceFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                choiceFrame->GetWidgetName());

  this->PivotCalibrationCheckButton = vtkKWCheckButton::New();
  this->PivotCalibrationCheckButton->SetParent(choiceFrame);
  this->PivotCalibrationCheckButton->Create();
  this->PivotCalibrationCheckButton->SelectedStateOn();
  this->PivotCalibrationCheckButton->SetText("Apply Calibration");

  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->PivotCalibrationCheckButton->GetWidgetName());


  calibrationFrame->Delete();
  processFrame->Delete();
  errorFrame->Delete();
  choiceFrame->Delete();
  offsetFrame->Delete();
}



void vtkPatientToImageRegistrationGUI::BuildGUIForLandmarksFrame ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "PatientToImageRegistration" );

  // ----------------------------------------------------------------
  // REGISTRATION FRAME            
  // ----------------------------------------------------------------
  vtkSlicerModuleCollapsibleFrame *regFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  regFrame->SetParent ( page );
  regFrame->Create();
  regFrame->SetLabelText ("Landmark Registration");
  regFrame->CollapseFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                regFrame->GetWidgetName(), page->GetWidgetName());


  // add a landmark 
  vtkKWFrameWithLabel *addFrame = vtkKWFrameWithLabel::New();
  addFrame->SetParent ( regFrame->GetFrame() );
  addFrame->Create ( );
  addFrame->SetLabelText ("Add a landmark");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                addFrame->GetWidgetName());

  vtkKWFrame *patFrame = vtkKWFrame::New();
  patFrame->SetParent ( addFrame->GetFrame() );
  patFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                patFrame->GetWidgetName());

  vtkKWFrame *imageFrame = vtkKWFrame::New();
  imageFrame->SetParent ( addFrame->GetFrame() );
  imageFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                imageFrame->GetWidgetName());

  vtkKWFrame *okFrame = vtkKWFrame::New();
  okFrame->SetParent ( addFrame->GetFrame() );
  okFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                okFrame->GetWidgetName());

  this->PatCoordinatesEntry = vtkKWEntryWithLabel::New();
  this->PatCoordinatesEntry->SetParent(patFrame);
  this->PatCoordinatesEntry->Create();
  this->PatCoordinatesEntry->SetWidth(35);
  this->PatCoordinatesEntry->SetLabelWidth(12);
  this->PatCoordinatesEntry->SetLabelText("Patient Side:");
  this->PatCoordinatesEntry->GetWidget()->SetValue ( "" );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->PatCoordinatesEntry->GetWidgetName());

  this->SlicerCoordinatesEntry = vtkKWEntryWithLabel::New();
  this->SlicerCoordinatesEntry->SetParent(imageFrame);
  this->SlicerCoordinatesEntry->Create();
  this->SlicerCoordinatesEntry->SetWidth(35);
  this->SlicerCoordinatesEntry->SetLabelWidth(12);
  this->SlicerCoordinatesEntry->SetLabelText("Image Side:");
  this->SlicerCoordinatesEntry->GetWidget()->SetValue ( "" );
  this->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
                this->SlicerCoordinatesEntry->GetWidgetName());


  this->GetPatCoordinatesPushButton = vtkKWPushButton::New();
  this->GetPatCoordinatesPushButton->SetParent(okFrame);
  this->GetPatCoordinatesPushButton->Create();
  this->GetPatCoordinatesPushButton->SetText("Get");
  this->GetPatCoordinatesPushButton->SetWidth ( 12 );
  this->AddPointPairPushButton = vtkKWPushButton::New();
  this->AddPointPairPushButton->SetParent(okFrame);
  this->AddPointPairPushButton->Create();
  this->AddPointPairPushButton->SetText( "Add" );
  this->AddPointPairPushButton->SetWidth ( 12 );

  this->Script( "pack %s %s -side left -anchor nw -expand n -padx 2 -pady 2", 
                this->GetPatCoordinatesPushButton->GetWidgetName(),
                this->AddPointPairPushButton->GetWidgetName());

  // list of defined point pairs 
  vtkKWFrameWithLabel *listFrame = vtkKWFrameWithLabel::New();
  listFrame->SetParent ( regFrame->GetFrame() );
  listFrame->Create ( );
  listFrame->SetLabelText ("Defined landmark(s)");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                listFrame->GetWidgetName());


  // add the multicolumn list to show the points
  this->PointPairMultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->PointPairMultiColumnList->SetParent ( listFrame->GetFrame() );
  this->PointPairMultiColumnList->Create ( );
  this->PointPairMultiColumnList->SetHeight(1);
  this->PointPairMultiColumnList->GetWidget()->SetSelectionTypeToRow();
  this->PointPairMultiColumnList->GetWidget()->MovableRowsOff();
  this->PointPairMultiColumnList->GetWidget()->MovableColumnsOff();
  // set up the columns of data for each point
  // refer to the header file for order
  this->PointPairMultiColumnList->GetWidget()->AddColumn("Patient Side");
  this->PointPairMultiColumnList->GetWidget()->AddColumn("Image Side");

  // make the selected column editable by checkbox
  //    this->PointPairMultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->SelectedColumn);

  // now set the attributes that are equal across the columns
  for (int col = 0; col < 2; col++)
    {
    this->PointPairMultiColumnList->GetWidget()->SetColumnWidth(col, 23);
    this->PointPairMultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
    this->PointPairMultiColumnList->GetWidget()->ColumnEditableOff(col);
    }
  /*
     if (col >= this->XColumn && col <= this->OrZColumn)
     {
     this->PointPairMultiColumnList->GetWidget()->SetColumnEditWindowToSpinBox(col);
     }
     */
  // set the name column width to be higher
  // this->PointPairMultiColumnList->GetWidget()->SetColumnWidth(this->NameColumn, 15);
  // set the selected column width a bit higher
  // this->PointPairMultiColumnList->GetWidget()->SetColumnWidth(this->SelectedColumn, 9);

  app->Script ( "pack %s -fill both -expand true",
                this->PointPairMultiColumnList->GetWidgetName());
  //                  listFrame->GetWidgetName());
  //    this->PointPairMultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");

  // button frame
  vtkKWFrame *buttonFrame = vtkKWFrame::New();
  buttonFrame->SetParent ( listFrame->GetFrame() );
  buttonFrame->Create ( );
  app->Script ("pack %s -side top -anchor nw -fill x -pady 0 -in %s",
               buttonFrame->GetWidgetName(),
               listFrame->GetFrame()->GetWidgetName());
  /*    
  // add an load button
  this->LoadPointPairPushButton = vtkKWPushButton::New ( );
  this->LoadPointPairPushButton->SetParent ( buttonFrame );
  this->LoadPointPairPushButton->Create ( );
  this->LoadPointPairPushButton->SetText ("Load Points");
  this->LoadPointPairPushButton->SetWidth (12);
  this->LoadPointPairPushButton->SetBalloonHelpString("Load point pairs from a file.");

  // add a save button
  this->SavePointPairPushButton = vtkKWPushButton::New ( );
  this->SavePointPairPushButton->SetParent ( buttonFrame );
  this->SavePointPairPushButton->Create ( );
  this->SavePointPairPushButton->SetText ("Save Points");
  this->SavePointPairPushButton->SetWidth (12);
  this->SavePointPairPushButton->SetBalloonHelpString("Save all defined point pairs to a file.");
  */

  // add a delete button 
  this->DeletePointPairPushButton = vtkKWPushButton::New ( );
  this->DeletePointPairPushButton->SetParent ( buttonFrame );
  this->DeletePointPairPushButton->Create ( );
  this->DeletePointPairPushButton->SetText ("Delete One");
  this->DeletePointPairPushButton->SetWidth (12);
  this->DeletePointPairPushButton->SetBalloonHelpString("Delete the selected landmarks.");

  // add a delete button 
  this->DeleteAllPointPairPushButton = vtkKWPushButton::New ( );
  this->DeleteAllPointPairPushButton->SetParent ( buttonFrame );
  this->DeleteAllPointPairPushButton->Create ( );
  this->DeleteAllPointPairPushButton->SetText ("Delete All");
  this->DeleteAllPointPairPushButton->SetWidth (12);
  this->DeleteAllPointPairPushButton->SetBalloonHelpString("Delete all landmarks.");

  app->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
              this->DeletePointPairPushButton->GetWidgetName(),
              this->DeleteAllPointPairPushButton->GetWidgetName());



  // do registration
  vtkKWFrame *actionFrame = vtkKWFrame::New();
  actionFrame->SetParent ( regFrame->GetFrame() );
  actionFrame->Create ( );
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                actionFrame->GetWidgetName());

  // add a compute button 
  this->RegisterPushButton = vtkKWPushButton::New ( );
  this->RegisterPushButton->SetParent ( actionFrame );
  this->RegisterPushButton->Create ( );
  this->RegisterPushButton->SetText ("Compute");
  this->RegisterPushButton->SetWidth (12);
  this->RegisterPushButton->SetBalloonHelpString("Perform landmark registration.");

/*
  // add a reset button 
  this->ResetPushButton = vtkKWPushButton::New ( );
  this->ResetPushButton->SetParent ( actionFrame );
  this->ResetPushButton->Create ( );
  this->ResetPushButton->SetText ("Remove");
  this->ResetPushButton->SetWidth (12);
  this->ResetPushButton->SetBalloonHelpString("Remove the current registration.");

  app->Script("pack %s %s -side left -anchor w -padx 2 -pady 2", 
              this->RegisterPushButton->GetWidgetName(),
              this->ResetPushButton->GetWidgetName());

  */


  app->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
              this->RegisterPushButton->GetWidgetName());


  regFrame->Delete ();
  addFrame->Delete ();
  patFrame->Delete ();
  imageFrame->Delete ();
  okFrame->Delete ();
  listFrame->Delete ();
  buttonFrame->Delete ();
  actionFrame->Delete ();

}


void vtkPatientToImageRegistrationGUI::BuildGUIForICPFrame ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "PatientToImageRegistration" );


  // ----------------------------------------------------------------
  // Navigation FRAME            
  // ----------------------------------------------------------------
  vtkSlicerModuleCollapsibleFrame *icpFrame = vtkSlicerModuleCollapsibleFrame::New ( );    
  icpFrame->SetParent ( page );
  icpFrame->Create ( );
  icpFrame->SetLabelText ("Iterative Closest Point (ICP) Registration");
  icpFrame->CollapseFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                icpFrame->GetWidgetName(), page->GetWidgetName());

  icpFrame->Delete();
}


void vtkPatientToImageRegistrationGUI::TransformChangedCallback(double)
{
//    cout << "Transform changed.\n";
}


void vtkPatientToImageRegistrationGUI::TransformChangingCallback(double)
{
//    cout << "Transform changing.\n";
}
