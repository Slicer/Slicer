/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDImage/vtkFourDImageGUI.cxx $
  Date:      $Date: 2009-02-10 18:26:32 -0500 (Tue, 10 Feb 2009) $
  Version:   $Revision: 3674 $

==========================================================================*/

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkFourDImageGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkMRMLColorNode.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"
#include "vtkKWMultiColumnList.h"

#include "vtkKWScaleWithEntry.h"
#include "vtkKWScale.h"
#include "vtkKWPushButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWMenuButton.h"
#include "vtkKWSpinBox.h"
#include "vtkKWCanvas.h"
#include "vtkKWRange.h"
#include "vtkKWCheckButtonWithLabel.h"

#include "vtkKWProgressDialog.h"

#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"

#include "vtkKWProgressGauge.h"

#include "vtkCornerAnnotation.h"

#include "vtkMRML4DBundleNode.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFourDImageGUI );
vtkCxxRevisionMacro ( vtkFourDImageGUI, "$Revision: 3674 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkFourDImageGUI::vtkFourDImageGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  this->Logic = NULL;
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkFourDImageGUI::DataCallback);

  this->BundleNameCount = 0;

  this->AutoPlayFG              = 0;
  this->AutoPlayBG              = 0;
  this->AutoPlayInterval        = 10;
  this->AutoPlayIntervalCounter = 0;
  
  //----------------------------------------------------------------
  // GUI widgets
  this->ProgressDialog = NULL;
  this->SelectInputDirectoryButton    = NULL;

  // Load / Save
  this->LoadImageButton               = NULL;
  this->SelectOutputDirectoryButton   = NULL;
  this->SaveImageButton               = NULL;

  this->Active4DBundleSelectorWidget  = NULL;

  // Frame control
  this->ForegroundVolumeSelectorScale = NULL;
  this->BackgroundVolumeSelectorScale = NULL;
  this->AutoPlayFGButton              = NULL;
  this->AutoPlayBGButton              = NULL;
  this->AutoPlayIntervalEntry         = NULL;


  //----------------------------------------------------------------
  // Time
  this->TimerFlag = 0;
}

//---------------------------------------------------------------------------
vtkFourDImageGUI::~vtkFourDImageGUI ( )
{

  //----------------------------------------------------------------
  // Remove Callbacks

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  //----------------------------------------------------------------
  // Remove Observers

  this->RemoveGUIObservers();

  //----------------------------------------------------------------
  // Remove GUI widgets

  // Progress Dialog
  if (this->ProgressDialog)
    {
    this->ProgressDialog->SetParent(NULL);
    this->ProgressDialog->Delete();
    }

  // Load / Save
  if (this->SelectInputDirectoryButton)
    {
    this->SelectInputDirectoryButton->SetParent(NULL);
    this->SelectInputDirectoryButton->Delete();
    }
  if (this->LoadImageButton)
    {
    this->LoadImageButton->SetParent(NULL);
    this->LoadImageButton->Delete();
    }
  if (this->SelectOutputDirectoryButton)
    {
    this->SelectOutputDirectoryButton->SetParent(NULL);
    this->SelectOutputDirectoryButton->Delete();
    }
  if (this->SaveImageButton)
    {
    this->SaveImageButton->SetParent(NULL);
    this->SaveImageButton->Delete();
    }

  // 4D Bundle Selector
  if (this->Active4DBundleSelectorWidget)
    {
    this->Active4DBundleSelectorWidget->SetParent(NULL);
    this->Active4DBundleSelectorWidget->Delete();
    }

  // Frame control
  if (this->ForegroundVolumeSelectorScale)
    {
    this->ForegroundVolumeSelectorScale->SetParent(NULL);
    this->ForegroundVolumeSelectorScale->Delete();
    }
  if (this->BackgroundVolumeSelectorScale)
    {
    this->BackgroundVolumeSelectorScale->SetParent(NULL);
    this->BackgroundVolumeSelectorScale->Delete();
    }
  if (this->AutoPlayFGButton)
    {
    this->AutoPlayFGButton->SetParent(NULL);
    this->AutoPlayFGButton->Delete();
    }
  if (this->AutoPlayBGButton)
    {
    this->AutoPlayBGButton->SetParent(NULL);
    this->AutoPlayBGButton->Delete();
    }
  if (this->AutoPlayIntervalEntry)
    {
    this->AutoPlayIntervalEntry->SetParent(NULL);
    this->AutoPlayIntervalEntry->Delete();
    }
  if (this->WindowLevelRange)
    {
    this->WindowLevelRange->SetParent(NULL);
    this->WindowLevelRange->Delete();
    }
  if (this->ThresholdRange)
    {
    this->ThresholdRange->SetParent(NULL);
    this->ThresholdRange->Delete();
    }

  // Icons
  if (this->Icons)
    {
    this->Icons->Delete();
    }


  //----------------------------------------------------------------
  // Unregister Logic class

}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::Init()
{
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::Enter()
{
  // Fill in
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
    this->TimerInterval = 100;  // 100 ms
    ProcessTimerEvents();
    }

  // register node type to the MRML scene
  vtkMRMLScene* scene = this->GetMRMLScene();

  // 4D bundle node (vtkMRML4DBundleNode)
  vtkMRML4DBundleNode* bundleNode = vtkMRML4DBundleNode::New();
  scene->RegisterNodeClass(bundleNode);
  bundleNode->Delete();

}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "FourDImageGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::RemoveGUIObservers ( )
{
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (this->LoadImageButton)
    {
    this->LoadImageButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->SaveImageButton)
    {
    this->SaveImageButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->Active4DBundleSelectorWidget)
    {
    this->Active4DBundleSelectorWidget
      ->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ForegroundVolumeSelectorScale)
    {
    this->ForegroundVolumeSelectorScale
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->BackgroundVolumeSelectorScale)
    {
    this->BackgroundVolumeSelectorScale
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AutoPlayFGButton)
    {
    this->AutoPlayFGButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AutoPlayBGButton)
    {
    this->AutoPlayBGButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AutoPlayIntervalEntry)
    {
    this->AutoPlayIntervalEntry
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->WindowLevelRange)
    {
    this->WindowLevelRange
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ThresholdRange)
    {
    this->ThresholdRange
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  this->RemoveLogicObservers();

}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  //----------------------------------------------------------------
  // GUI Observers

  if (this->LoadImageButton)
    {
    this->LoadImageButton
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->SaveImageButton)
    {
    this->SaveImageButton
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->Active4DBundleSelectorWidget)
    {
    this->Active4DBundleSelectorWidget
      ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ForegroundVolumeSelectorScale)
    {
    this->ForegroundVolumeSelectorScale
      ->AddObserver(vtkKWScale::ScaleValueChangingEvent /*vtkKWScale::ScaleValueChangedEvent*/, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->BackgroundVolumeSelectorScale)
    {
    this->BackgroundVolumeSelectorScale
      ->AddObserver(vtkKWScale::ScaleValueChangingEvent /*vtkKWScale::ScaleValueChangedEvent*/, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AutoPlayFGButton)
    {
    this->AutoPlayFGButton
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AutoPlayBGButton)
    {
    this->AutoPlayBGButton
      ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->AutoPlayIntervalEntry)
    {
    this->AutoPlayIntervalEntry
      ->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->WindowLevelRange)
    {
    this->WindowLevelRange
      ->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ThresholdRange)
    {
    this->ThresholdRange
      ->AddObserver(vtkKWRange::RangeValueChangingEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  this->AddLogicObservers();

}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::RemoveLogicObservers ( )
{
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    this->GetLogic()->AddObserver(vtkFourDImageLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    return;
    }

  if (this->LoadImageButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    const char* path = this->SelectInputDirectoryButton->GetWidget()->GetFileName();
    const char* bundleName = this->SelectInputDirectoryButton->GetWidget()->GetText();
    this->GetLogic()->AddObserver(vtkFourDImageLogic::ProgressDialogEvent, 
                                  this->LogicCallbackCommand);
    vtkMRML4DBundleNode* newNode = this->GetLogic()->LoadImagesFromDir(path, bundleName);

    this->GetLogic()->RemoveObservers(vtkFourDImageLogic::ProgressDialogEvent,
                                      this->LogicCallbackCommand);

    UpdateSeriesSelectorMenus();

    vtkMRML4DBundleNode *bundleNode = 
      vtkMRML4DBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());

    if (newNode && bundleNode &&
        strcmp(newNode->GetID(), bundleNode->GetID()) == 0)  // new node is selected as the active bundle
      {
      this->Window = 1.0;
      this->Level  = 0.5;
      this->ThresholdUpper = 0.0;
      this->ThresholdLower = 1.0;

      SelectActive4DBundle(bundleNode);
      }
    }

  if (this->SaveImageButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    const char* path = this->SelectOutputDirectoryButton->GetWidget()->GetFileName();
    vtkMRML4DBundleNode *bundleNode = 
      vtkMRML4DBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    if (bundleNode)
      {
      this->GetLogic()->AddObserver(vtkFourDImageLogic::ProgressDialogEvent,  this->LogicCallbackCommand);
      this->GetLogic()->SetApplication(vtkSlicerApplication::SafeDownCast(this->GetApplication()));
      this->GetLogic()->SaveImagesToDir(path, bundleNode->GetID(), "out", "nrrd");
      this->GetLogic()->RemoveObservers(vtkFourDImageLogic::ProgressDialogEvent,  this->LogicCallbackCommand);
      }
    }
  else if (this->Active4DBundleSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
           && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRML4DBundleNode *bundleNode = 
      vtkMRML4DBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    SelectActive4DBundle(bundleNode);
    }
  else if (this->ForegroundVolumeSelectorScale == vtkKWScaleWithEntry::SafeDownCast(caller)
      && event == vtkKWScale::ScaleValueChangingEvent /*vtkKWScale::ScaleValueChangedEvent*/)
    {
    vtkMRML4DBundleNode *bundleNode = 
      vtkMRML4DBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    int volume = (int)this->ForegroundVolumeSelectorScale->GetValue();
    if (bundleNode)
      {
      SetForeground(bundleNode->GetID(), volume);
      }
    }
  else if (this->BackgroundVolumeSelectorScale == vtkKWScaleWithEntry::SafeDownCast(caller)
      && event == vtkKWScale::ScaleValueChangingEvent /*vtkKWScale::ScaleValueChangedEvent*/ )
    {
    int volume = (int)this->BackgroundVolumeSelectorScale->GetValue();
    vtkMRML4DBundleNode *bundleNode = 
      vtkMRML4DBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
    if (bundleNode)
      {
      SetBackground(bundleNode->GetID(), volume);
      }
    }
  if (this->AutoPlayFGButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    if (!this->AutoPlayFG)
      {
      // NOTE: interval = TimerInterval * AutoPlayInterval; 
      double interval_s = this->AutoPlayIntervalEntry->GetValueAsDouble();
      this->AutoPlayFG              = 1;
      this->AutoPlayInterval        = (int) (interval_s * 1000.0 / (double)this->TimerInterval);  
      this->AutoPlayIntervalCounter = 0;
      this->AutoPlayFGButton->SetImageToIcon(this->Icons->GetPauseIcon());
      this->AutoPlayFGButton->Modified();
      }
    else
      {
      this->AutoPlayFG              = 0;
      this->AutoPlayFGButton->SetImageToIcon(this->Icons->GetPlayIcon());
      this->AutoPlayFGButton->Modified();
      }
    }
  if (this->AutoPlayBGButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    if (!this->AutoPlayBG)
      {
      // NOTE: interval = TimerInterval * AutoPlayInterval; 
      double interval_s = this->AutoPlayIntervalEntry->GetValueAsDouble();
      this->AutoPlayBG              = 1;
      this->AutoPlayInterval        = (int) (interval_s * 1000.0 / (double)this->TimerInterval);  
      this->AutoPlayIntervalCounter = 0;
      this->AutoPlayBGButton->SetImageToIcon(this->Icons->GetPauseIcon());
      this->AutoPlayBGButton->Modified();
      }
    else
      {
      this->AutoPlayBG              = 0;
      this->AutoPlayBGButton->SetImageToIcon(this->Icons->GetPlayIcon());
      this->AutoPlayBGButton->Modified();
      }
    }
  if (this->AutoPlayIntervalEntry == vtkKWEntry::SafeDownCast(caller)
      && event == vtkKWEntry::EntryValueChangedEvent)
    {
    // NOTE: interval = TimerInterval * AutoPlayInterval; 
    double interval_s = this->AutoPlayIntervalEntry->GetValueAsDouble();
    this->AutoPlayInterval = (int) (interval_s * 1000.0 / (double)this->TimerInterval);
    }
  else if (this->WindowLevelRange == vtkKWRange::SafeDownCast(caller)
      && event == vtkKWRange::RangeValueChangingEvent)
    {
    double wllow, wlhigh;
    this->WindowLevelRange->GetRange(wllow, wlhigh);
    this->Window = wlhigh - wllow;
    this->Level  = (wlhigh + wllow) / 2.0;
    SetWindowLevelForCurrentFrame();
    }
  else if (this->ThresholdRange == vtkKWRange::SafeDownCast(caller)
      && event == vtkKWRange::RangeValueChangingEvent)
    {
    double thlow, thhigh;
    this->ThresholdRange->GetRange(thlow, thhigh);
    this->ThresholdUpper  = thhigh; 
    this->ThresholdLower  = thlow; 
    SetWindowLevelForCurrentFrame();
    }
} 


//---------------------------------------------------------------------------
void vtkFourDImageGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkFourDImageGUI *self = reinterpret_cast<vtkFourDImageGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkFourDImageGUI DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
  if (event == vtkFourDImageLogic::StatusUpdateEvent)
    {
    //this->UpdateDeviceStatus();
    }
  else if (event ==  vtkCommand::ProgressEvent) 
    {
    double progress = *((double *)callData);
    this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(100*progress);
    }
  else if (event == vtkFourDImageLogic::ProgressDialogEvent)
    {
    vtkFourDImageLogic::StatusMessageType* smsg
      = (vtkFourDImageLogic::StatusMessageType*)callData;
    if (smsg->show)
      {
      if (!this->ProgressDialog)
        {
        this->ProgressDialog = vtkKWProgressDialog::New();
        this->ProgressDialog->SetParent(this->GetApplicationGUI()->GetMainSlicerWindow());
        this->ProgressDialog->SetMasterWindow(this->GetApplicationGUI()->GetMainSlicerWindow());
        this->ProgressDialog->Create();
        }
      this->ProgressDialog->SetMessageText(smsg->message.c_str());
      this->ProgressDialog->UpdateProgress(smsg->progress);
      this->ProgressDialog->Display();
      }
    else
      {
      if (this->ProgressDialog)
        {
        this->ProgressDialog->SetParent(NULL);
        this->ProgressDialog->Delete();
        this->ProgressDialog = NULL;
        }
      }
    }

}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  if (event == vtkMRMLScene::NodeAddedEvent)
    {
    UpdateSeriesSelectorMenus();
    }
  else if (event == vtkMRMLScene::SceneCloseEvent)
    {
    }
  /*
  else if (event == vtkMRMLVolumeNode::ImageDataModifiedEvent)
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(caller);
    }
  */
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
    if (this->AutoPlayFG || this->AutoPlayBG)
      {
      this->AutoPlayIntervalCounter ++;
      if (this->AutoPlayInterval != 0 &&
          this->AutoPlayIntervalCounter % this->AutoPlayInterval == 0)
        {
        double current;
        double range[2];
        int    volume;
        this->AutoPlayIntervalCounter = 0;
          
        // increment the frame id for foreground
        if (this->AutoPlayFG)
          {
          this->ForegroundVolumeSelectorScale->GetRange(range);
          current = this->ForegroundVolumeSelectorScale->GetValue();
          current += 1;
          if (current > range[1])
            {
            current = 0.0;
            }
          this->ForegroundVolumeSelectorScale->SetValue((double)current);
          volume = (int)current;
          vtkMRML4DBundleNode *bundleNode = 
            vtkMRML4DBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
          if (bundleNode)
            {
            SetForeground(bundleNode->GetID(), volume);
            }
          }
        // increment the frame id for foreground
        if (this->AutoPlayBG)
          {
          this->BackgroundVolumeSelectorScale->GetRange(range);
          current = this->BackgroundVolumeSelectorScale->GetValue();
          current += 1;
          if (current > range[1])
            {
            current = 0.0;
            }
          this->BackgroundVolumeSelectorScale->SetValue((double)current);
          volume = (int)current;
          vtkMRML4DBundleNode *bundleNode = 
            vtkMRML4DBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
          if (bundleNode)
            {
            SetBackground(bundleNode->GetID(), volume);
            }
          }
        }
      }
    // update timer
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "FourDImage", "FourDImage", NULL );

  this->Icons = vtkFourDImageIcons::New();

  BuildGUIForHelpFrame();
  BuildGUIForLoadFrame(1);
  BuildGUIForActiveBundleSelectorFrame();
  BuildGUIForFrameControlFrame(0);
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::BuildGUIForActiveBundleSelectorFrame ()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDImage");

  //  Volume to select
  this->Active4DBundleSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->Active4DBundleSelectorWidget->SetParent(page);
  this->Active4DBundleSelectorWidget->Create();
  this->Active4DBundleSelectorWidget->SetNodeClass("vtkMRML4DBundleNode", NULL, NULL, NULL);
  this->Active4DBundleSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->Active4DBundleSelectorWidget->SetBorderWidth(2);
  this->Active4DBundleSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->Active4DBundleSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->Active4DBundleSelectorWidget->SetLabelText( "Active 4D Bundle: ");
  this->Active4DBundleSelectorWidget->SetBalloonHelpString("Select a volume from the current scene.");
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               this->Active4DBundleSelectorWidget->GetWidgetName());

}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::BuildGUIForHelpFrame ()
{
  // ----------------------------------------------------------------
  // HELP FRAME         
  // ----------------------------------------------------------------

  // Define your help text here.
  const char *help = 
    "**The 4D Analysis Module** helps you to load, view and analyze a series of 3D images (4D image),"
    "such as perfusion MRI, DCE MRI, and fMRI. "
    "<a>http://www.slicer.org/slicerWiki/index.php/Modules:FourDAnalysis</a> for details.";
  const char *about =
    "This project is directed by Hiroto Hatabu, MD, PhD (BWH)."
    "The module is designed and implemented by Junichi Tokuda, PhD (BWH), under supports from"
    "Center for Pulmonary Functional Imaging at BWH, NCIGT and NA-MIC.";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "FourDImage" );
  this->BuildHelpAndAboutFrame (page, help, about);

}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::BuildGUIForLoadFrame (int show)
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDImage");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Load / Save / Active Series");
  if (!show)
    {
    conBrowsFrame->CollapseFrame();
    }
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Input File Frame
  vtkKWFrameWithLabel *inFrame = vtkKWFrameWithLabel::New();
  inFrame->SetParent(conBrowsFrame->GetFrame());
  inFrame->Create();
  inFrame->SetLabelText ("Input");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 inFrame->GetWidgetName() );
  
  this->SelectInputDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
  this->SelectInputDirectoryButton->SetParent(inFrame->GetFrame());
  this->SelectInputDirectoryButton->Create();
  this->SelectInputDirectoryButton->SetWidth(50);
  this->SelectInputDirectoryButton->GetWidget()->SetText ("Browse Input Directory");
  this->SelectInputDirectoryButton->GetWidget()->TrimPathFromFileNameOn();
  this->SelectInputDirectoryButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  /*
    this->SelectInputDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
    "{ {ProstateNav} {*.dcm} }");
  */
  this->SelectInputDirectoryButton->GetWidget()->GetLoadSaveDialog()
    ->RetrieveLastPathFromRegistry("OpenPath");

  this->LoadImageButton = vtkKWPushButton::New ( );
  this->LoadImageButton->SetParent ( inFrame->GetFrame() );
  this->LoadImageButton->Create ( );
  this->LoadImageButton->SetText ("Load Series");
  this->LoadImageButton->SetWidth (12);

  this->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
               this->SelectInputDirectoryButton->GetWidgetName(),
               this->LoadImageButton->GetWidgetName());

  // -----------------------------------------
  // Output File Frame

  vtkKWFrameWithLabel *outFrame = vtkKWFrameWithLabel::New();
  outFrame->SetParent(conBrowsFrame->GetFrame());
  outFrame->Create();
  outFrame->SetLabelText ("Output");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 outFrame->GetWidgetName() );

  this->SelectOutputDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
  this->SelectOutputDirectoryButton->SetParent(outFrame->GetFrame());
  this->SelectOutputDirectoryButton->Create();
  this->SelectOutputDirectoryButton->SetWidth(50);
  this->SelectOutputDirectoryButton->GetWidget()->SetText ("Browse Output Directory");
  this->SelectOutputDirectoryButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->SelectOutputDirectoryButton->GetWidget()->GetLoadSaveDialog()
    ->RetrieveLastPathFromRegistry("OpenPath");
  
  this->SaveImageButton = vtkKWPushButton::New ( );
  this->SaveImageButton->SetParent ( outFrame->GetFrame() );
  this->SaveImageButton->Create ( );
  this->SaveImageButton->SetText ("Save Series");
  this->SaveImageButton->SetWidth (12);

  this->Script("pack %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
               this->SelectOutputDirectoryButton->GetWidgetName(),
               this->SaveImageButton->GetWidgetName());


  conBrowsFrame->Delete();
  inFrame->Delete();
  outFrame->Delete();
}


//---------------------------------------------------------------------------
void vtkFourDImageGUI::BuildGUIForFrameControlFrame(int show)
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("FourDImage");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Frame Control");
  if (!show)
    {
    conBrowsFrame->CollapseFrame();
    }
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Frame Control

  vtkKWFrameWithLabel *fframe = vtkKWFrameWithLabel::New();
  fframe->SetParent(conBrowsFrame->GetFrame());
  fframe->Create();
  fframe->SetLabelText ("Frame");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 fframe->GetWidgetName() );
  
  //vtkKWFrame *sframe = vtkKWFrame::New();
  //sframe->SetParent(fframe->GetFrame());
  //sframe->Create();
  //this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
  //               sframe->GetWidgetName() );

  vtkKWFrame *fgframe = vtkKWFrame::New();
  fgframe->SetParent(fframe->GetFrame());
  fgframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 fgframe->GetWidgetName() );

  this->ForegroundVolumeSelectorScale = vtkKWScaleWithEntry::New();
  this->ForegroundVolumeSelectorScale->SetParent(fgframe);
  this->ForegroundVolumeSelectorScale->Create();
  this->ForegroundVolumeSelectorScale->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionRight);
  this->ForegroundVolumeSelectorScale->SetOrientationToHorizontal();
  this->ForegroundVolumeSelectorScale->SetLabelText("FG");
  this->ForegroundVolumeSelectorScale->SetRange(0.0, 100.0);
  this->ForegroundVolumeSelectorScale->SetResolution(1.0);
  this->ForegroundVolumeSelectorScale->ExpandEntryOff();
  //this->ForegroundVolumeSelectorScale->SetWidth(30);

  this->AutoPlayFGButton = vtkKWPushButton::New ( );
  this->AutoPlayFGButton->SetParent ( fgframe );
  this->AutoPlayFGButton->Create ( );
  this->AutoPlayFGButton->SetImageToIcon(this->Icons->GetPlayIcon());
  //this->AutoPlayFGButton->SetWidth (10);

  this->Script("pack %s -side right -anchor w -padx 2 -pady 2", 
               this->AutoPlayFGButton->GetWidgetName());
  this->Script("pack %s -side right -fill x -expand y -padx 2 -pady 2", 
               this->ForegroundVolumeSelectorScale->GetWidgetName());

  vtkKWFrame *bgframe = vtkKWFrame::New();
  bgframe->SetParent(fframe->GetFrame());
  bgframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 bgframe->GetWidgetName() );

  this->BackgroundVolumeSelectorScale = vtkKWScaleWithEntry::New();
  this->BackgroundVolumeSelectorScale->SetParent(bgframe);
  this->BackgroundVolumeSelectorScale->Create();
  this->BackgroundVolumeSelectorScale->SetEntryPosition(vtkKWScaleWithEntry::EntryPositionRight);
  this->BackgroundVolumeSelectorScale->SetOrientationToHorizontal();
  this->BackgroundVolumeSelectorScale->SetLabelText("BG");
  this->BackgroundVolumeSelectorScale->SetRange(0.0, 100.0);
  this->BackgroundVolumeSelectorScale->SetResolution(1.0);
  this->BackgroundVolumeSelectorScale->ExpandEntryOff();
  //this->BackgroundVolumeSelectorScale->SetWidth(30);
  
  this->AutoPlayBGButton = vtkKWPushButton::New ( );
  this->AutoPlayBGButton->SetParent ( bgframe );
  this->AutoPlayBGButton->Create ( );
  this->AutoPlayBGButton->SetImageToIcon(this->Icons->GetPlayIcon());
  //this->AutoPlayBGButton->SetWidth (10);

  this->Script("pack %s -side right -anchor w -padx 2 -pady 2", 
               this->AutoPlayBGButton->GetWidgetName());
  this->Script("pack %s -side right -fill x -expand y -padx 2 -pady 2", 
               this->BackgroundVolumeSelectorScale->GetWidgetName());

  vtkKWFrame *apframe = vtkKWFrame::New();
  apframe->SetParent(fframe->GetFrame());
  apframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 apframe->GetWidgetName() );
  
  vtkKWLabel *frlabel1 = vtkKWLabel::New();
  frlabel1->SetParent( apframe );
  frlabel1->Create();
  frlabel1->SetText("Interval: ");

  this->AutoPlayIntervalEntry = vtkKWEntry::New ( );
  this->AutoPlayIntervalEntry->SetParent( apframe );
  this->AutoPlayIntervalEntry->Create();
  this->AutoPlayIntervalEntry->SetWidth(8);
  this->AutoPlayIntervalEntry->SetRestrictValueToDouble();
  this->AutoPlayIntervalEntry->SetValueAsDouble(1.0);

  vtkKWLabel *frlabel2 = vtkKWLabel::New();
  frlabel2->SetParent( apframe );
  frlabel2->Create();
  frlabel2->SetText(" s ");

  this->Script("pack %s %s %s -side left -fill x -padx 2 -pady 2", 
               frlabel1->GetWidgetName(),
               this->AutoPlayIntervalEntry->GetWidgetName(),
               frlabel2->GetWidgetName() );

  // -----------------------------------------
  // Contrast control

  vtkKWFrameWithLabel *cframe = vtkKWFrameWithLabel::New();
  cframe->SetParent(conBrowsFrame->GetFrame());
  cframe->Create();
  cframe->SetLabelText ("Contrast");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 cframe->GetWidgetName() );

  vtkKWFrame *lwframe = vtkKWFrame::New();
  lwframe->SetParent(cframe->GetFrame());
  lwframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 lwframe->GetWidgetName() );

  vtkKWLabel *lwLabel = vtkKWLabel::New();
  lwLabel->SetParent(lwframe);
  lwLabel->Create();
  lwLabel->SetText("Window/Level: ");

  this->WindowLevelRange = vtkKWRange::New();
  this->WindowLevelRange->SetParent(lwframe);
  this->WindowLevelRange->Create();
  this->WindowLevelRange->SymmetricalInteractionOn();
  this->WindowLevelRange->EntriesVisibilityOff ();  
  this->WindowLevelRange->SetWholeRange(0.0, 1.0);
  /*
  this->WindowLevelRange->SetCommand(this, "ProcessWindowLevelCommand");
  this->WindowLevelRange->SetStartCommand(this, "ProcessWindowLevelStartCommand");
  */
  this->Script("pack %s %s -side left -anchor nw -expand yes -fill x -padx 2 -pady 2",
               lwLabel->GetWidgetName(),
               this->WindowLevelRange->GetWidgetName());

  vtkKWFrame *thframe = vtkKWFrame::New();
  thframe->SetParent(cframe->GetFrame());
  thframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 thframe->GetWidgetName() );

  vtkKWLabel *thLabel = vtkKWLabel::New();
  thLabel->SetParent(thframe);
  thLabel->Create();
  thLabel->SetText("Threashold:   ");

  this->ThresholdRange = vtkKWRange::New();
  this->ThresholdRange->SetParent(thframe);
  this->ThresholdRange->Create();
  this->ThresholdRange->SymmetricalInteractionOff();
  this->ThresholdRange->EntriesVisibilityOff ();
  this->ThresholdRange->SetWholeRange(0.0, 1.0);
  /*
  this->ThresholdRange->SetCommand(this, "ProcessThresholdCommand");
  this->ThresholdRange->SetStartCommand(this, "ProcessThresholdStartCommand");
  */
  this->Script("pack %s %s -side left -anchor w -expand y -fill x -padx 2 -pady 2", 
               thLabel->GetWidgetName(),
               this->ThresholdRange->GetWidgetName());


  // -----------------------------------------
  // Delete pointers

  conBrowsFrame->Delete();
  fframe->Delete();
  //sframe->Delete();

}


//----------------------------------------------------------------------------
void vtkFourDImageGUI::UpdateAll()
{
}

//----------------------------------------------------------------------------
void vtkFourDImageGUI::SelectActive4DBundle(vtkMRML4DBundleNode* bundleNode)
{
  if (bundleNode == NULL)
    {
    return;
    }

  int volume = (int)this->ForegroundVolumeSelectorScale->GetValue();

  // frame control
  int n = bundleNode->GetNumberOfFrames();
  this->ForegroundVolumeSelectorScale->SetRange(0.0, (double) n-1);
  this->BackgroundVolumeSelectorScale->SetRange(0.0, (double) n-1);
  bundleNode->SwitchDisplayBuffer(0, volume);
  bundleNode->SwitchDisplayBuffer(1, volume);

  this->ForegroundVolumeSelectorScale->SetRange(0.0, (double) n-1);
  this->BackgroundVolumeSelectorScale->SetRange(0.0, (double) n-1);
  SetForeground(bundleNode->GetID(), 0);
  SetBackground(bundleNode->GetID(), 0);
  
}



//----------------------------------------------------------------------------
void vtkFourDImageGUI::SetForeground(const char* bundleID, int index)
{

  int i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  vtkMRML4DBundleNode* bundleNode 
    = vtkMRML4DBundleNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(bundleID));

  if (!bundleNode)
    {
    return;
    }

  vtkMRMLVolumeNode* volNode;
  //volNode = vtkMRMLVolumeNode::SafeDownCast(bundleNode->GetFrameNode(index));
  bundleNode->SwitchDisplayBuffer(0, index);
  volNode = vtkMRMLVolumeNode::SafeDownCast(bundleNode->GetDisplayBufferNode(0));

  if (volNode)
    {
    std::cerr << "volume node name  = " <<  volNode->GetName() << std::endl;
    nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");
    for ( i=0; i<nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
        this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
      if ( cnode != NULL)
        {
        cnode->SetForegroundVolumeID(volNode->GetID());
        }
      }
    //SetWindowLevelForCurrentFrame();
    }
}


//----------------------------------------------------------------------------
void vtkFourDImageGUI::SetBackground(const char* bundleID, int index)
{
  int i, nnodes;
  vtkMRMLSliceCompositeNode *cnode;
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  vtkMRML4DBundleNode* bundleNode 
    = vtkMRML4DBundleNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(bundleID));

  if (!bundleNode)
    {
    return;
    }

  vtkMRMLVolumeNode* volNode;
  //volNode = vtkMRMLVolumeNode::SafeDownCast(bundleNode->GetFrameNode(index));
  bundleNode->SwitchDisplayBuffer(1, index);
  volNode = vtkMRMLVolumeNode::SafeDownCast(bundleNode->GetDisplayBufferNode(1));

  if (volNode)
    {
    //std::cerr << "node id = " << nodeID << std::endl;
    nnodes = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLSliceCompositeNode");          
    for ( i=0; i<nnodes; i++)
      {
      cnode = vtkMRMLSliceCompositeNode::SafeDownCast (
        this->GetMRMLScene()->GetNthNodeByClass (i, "vtkMRMLSliceCompositeNode"));
      if ( cnode != NULL)
        {
        cnode->SetBackgroundVolumeID(volNode->GetID());
        }
      }
    //SetWindowLevelForCurrentFrame();
    }
}


//----------------------------------------------------------------------------
void vtkFourDImageGUI::SetWindowLevelForCurrentFrame()
{

  vtkMRMLSliceCompositeNode *cnode = 
    vtkMRMLSliceCompositeNode::SafeDownCast (this->GetMRMLScene()->GetNthNodeByClass (0, "vtkMRMLSliceCompositeNode"));
  if (cnode != NULL)
    {
    const char* fgNodeID = cnode->GetForegroundVolumeID();
    const char* bgNodeID = cnode->GetBackgroundVolumeID();
    if (fgNodeID)
      {
      vtkMRMLVolumeNode* fgNode =
        vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(fgNodeID));
      vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(fgNode->GetDisplayNode());
      if (displayNode)
        {
        double r[2];
        fgNode->GetImageData()->GetScalarRange(r);
        //double lower = this->RangeLower;
        //double upper = this->RangeUpper;
        double lower = r[0];
        double upper = r[1];
        double range = upper - lower;
        double thLower = lower + range * this->ThresholdLower;
        double thUpper = lower + range * this->ThresholdUpper;
        double window  = range * this->Window;
        double level   = lower + range * this->Level;
        displayNode->SetAutoWindowLevel(0);
        displayNode->SetAutoThreshold(0);
        double cThLower = displayNode->GetLowerThreshold();
        double cThUpper = displayNode->GetUpperThreshold();
        double cWindow  = displayNode->GetWindow();
        double cLevel   = displayNode->GetLevel();

        int m = 0;
        if (cThLower!=thLower) { displayNode->SetLowerThreshold(thLower); m = 1;}
        if (cThUpper!=thUpper) { displayNode->SetUpperThreshold(thUpper); m = 1;}
        if (cWindow!=window)   { displayNode->SetWindow(window); m = 1;}
        if (cLevel!=level)     { displayNode->SetLevel(level); m = 1;}
        if (m) { displayNode->UpdateImageDataPipeline(); }
        }
      }
    if (bgNodeID && strcmp(fgNodeID, bgNodeID) != 0)
      {
      vtkMRMLVolumeNode* bgNode =
        vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(bgNodeID));
      vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(bgNode->GetDisplayNode());
      if (displayNode)
        {
        double r[2];
        bgNode->GetImageData()->GetScalarRange(r);
        //double lower = this->RangeLower;
        //double upper = this->RangeUpper;
        double lower = r[0];
        double upper = r[1];
        double range = upper - lower;
        double thLower = lower + range * this->ThresholdLower;
        double thUpper = lower + range * this->ThresholdUpper;
        double window  = range * this->Window;
        double level   = lower + range * this->Level;
        displayNode->SetAutoWindowLevel(0);
        displayNode->SetAutoThreshold(0);
        double cThLower = displayNode->GetLowerThreshold();
        double cThUpper = displayNode->GetUpperThreshold();
        double cWindow  = displayNode->GetWindow();
        double cLevel   = displayNode->GetLevel();

        int m = 0;
        if (cThLower!=thLower) { displayNode->SetLowerThreshold(thLower); m = 1;}
        if (cThUpper!=thUpper) { displayNode->SetUpperThreshold(thUpper); m = 1;}
        if (cWindow!=window)   { displayNode->SetWindow(window); m = 1;}
        if (cLevel!=level)     { displayNode->SetLevel(level); m = 1;}
        if (m) { displayNode->UpdateImageDataPipeline(); }
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkFourDImageGUI::UpdateSeriesSelectorMenus()
{

  // generate a list of 4D Bundles
  std::vector<vtkMRMLNode*> nodes;
  std::vector<std::string>  names;

  this->GetApplicationLogic()->GetMRMLScene()->GetNodesByClass("vtkMRML4DBundleNode", nodes);

  this->BundleNodeIDList.clear();
  names.clear();
  std::vector<vtkMRMLNode*>::iterator niter;
  for (niter = nodes.begin(); niter != nodes.end(); niter ++)
    {
    this->BundleNodeIDList.push_back((*niter)->GetID());
    names.push_back((*niter)->GetName());
    }

}


//----------------------------------------------------------------------------
const char* vtkFourDImageGUI::GetActive4DBundleNodeID()
{
  vtkMRML4DBundleNode *bundleNode = 
    vtkMRML4DBundleNode::SafeDownCast(this->Active4DBundleSelectorWidget->GetSelected());
  if (bundleNode)
    {
    return bundleNode->GetID();
    }
  else
    {
    return NULL;
    }
}
