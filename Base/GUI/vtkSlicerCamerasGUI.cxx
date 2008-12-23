#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerCamerasGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWWidget.h"
#include "vtkKWCheckButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerCamerasGUI );
vtkCxxRevisionMacro ( vtkSlicerCamerasGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerCamerasGUI::vtkSlicerCamerasGUI ( )
{

    this->CameraSelectorWidget = NULL;
    this->ActiveCheckButton = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerCamerasGUI::~vtkSlicerCamerasGUI ( )
{
  this->RemoveGUIObservers();
    
  if (this->CameraSelectorWidget)
    {
    this->CameraSelectorWidget->SetParent(NULL );
    this->CameraSelectorWidget->Delete ( );
    }
  if (this->ActiveCheckButton)
    {
    this->ActiveCheckButton->SetParent(NULL );
    this->ActiveCheckButton->Delete ( );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerCamerasGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerCamerasGUI: " << this->GetClassName ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerCamerasGUI::RemoveGUIObservers ( )
{
  this->ActiveCheckButton->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent , 
                                            (vtkCommand *)this->GUICallbackCommand );
  this->CameraSelectorWidget->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                             (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerCamerasGUI::AddGUIObservers ( )
{
  this->ActiveCheckButton->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent , 
                                        (vtkCommand *)this->GUICallbackCommand );
  this->CameraSelectorWidget->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, 
                                             (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerCamerasGUI::ProcessGUIEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
  if (this->ActiveCheckButton == vtkKWCheckButton::SafeDownCast(caller)  && 
      event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    vtkMRMLCameraNode *cameraNode = vtkMRMLCameraNode::SafeDownCast(this->CameraSelectorWidget->GetSelected());
    if (cameraNode) 
      {
      cameraNode->SetActive(this->ActiveCheckButton->GetSelectedState());
      }
    }
  if (this->CameraSelectorWidget == vtkSlicerNodeSelectorWidget::SafeDownCast(caller) && 
        event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    vtkMRMLCameraNode *camera = 
        vtkMRMLCameraNode::SafeDownCast(this->CameraSelectorWidget->GetSelected());

    if (camera )
      {
      this->ActiveCheckButton->SetSelectedState(camera->GetActive());
      }
    }
} 

void vtkSlicerCamerasGUI::UpdateCameraSelector()

{
  this->CameraSelectorWidget->UpdateMenu();
}

//---------------------------------------------------------------------------
void vtkSlicerCamerasGUI::ProcessMRMLEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerCamerasGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerCamerasGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerCamerasGUI::BuildGUI ( )
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  // Define your help text here.
  const char *help = "**Camera Module:** Create or set active camera. ";
  
  // ---
  // MODULE GUI FRAME 
  // configure a page for a camera 
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "Cameras", "Cameras", NULL );
  
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Cameras" );
  
  // HELP FRAME
  vtkSlicerModuleCollapsibleFrame *cameraHelpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  cameraHelpFrame->SetParent ( page );
  cameraHelpFrame->Create ( );
  cameraHelpFrame->CollapseFrame ( );
  cameraHelpFrame->SetLabelText ("Help");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                cameraHelpFrame->GetWidgetName(), page->GetWidgetName());
  
  // configure the parent classes help text widget
  this->HelpText->SetParent ( cameraHelpFrame->GetFrame() );
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

  // ---
  // camera FRAME            
  vtkSlicerModuleCollapsibleFrame *cameraFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  cameraFrame->SetParent ( page );
  cameraFrame->Create ( );
  cameraFrame->SetLabelText ("Camera");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                cameraFrame->GetWidgetName(), page->GetWidgetName());

  // selector for camera
  this->CameraSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
  this->CameraSelectorWidget->SetParent ( cameraFrame->GetFrame() );
  this->CameraSelectorWidget->Create ( );
  this->CameraSelectorWidget->SetNodeClass("vtkMRMLCameraNode", NULL, NULL, NULL);
  this->CameraSelectorWidget->SetMRMLScene(this->GetMRMLScene());
  this->CameraSelectorWidget->SetNewNodeEnabled(1);
  this->CameraSelectorWidget->SetShowHidden(1);
  this->CameraSelectorWidget->SetNoneEnabled(0);
  this->CameraSelectorWidget->SetBorderWidth(2);
  this->CameraSelectorWidget->SetPadX(2);
  this->CameraSelectorWidget->SetPadY(2);
  this->CameraSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->CameraSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
  this->CameraSelectorWidget->SetLabelText( "Camera");
  this->CameraSelectorWidget->SetBalloonHelpString("select a camera from the current scene.");  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->CameraSelectorWidget->GetWidgetName());

  // is this active camera
  this->ActiveCheckButton = vtkKWCheckButton::New();
  this->ActiveCheckButton->SetParent(cameraFrame->GetFrame());
  this->ActiveCheckButton->Create();
  this->ActiveCheckButton->SelectedStateOff();
  this->ActiveCheckButton->SetText("Active Camera");
  this->Script(
               "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
               this->ActiveCheckButton->GetWidgetName());

  cameraFrame->Delete();
  cameraHelpFrame->Delete();
}





