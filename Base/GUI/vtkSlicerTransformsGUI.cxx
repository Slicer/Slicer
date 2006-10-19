#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerTransformsGUI.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

#include "vtkKWWidget.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenu.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerTransformsGUI );
vtkCxxRevisionMacro ( vtkSlicerTransformsGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerTransformsGUI::vtkSlicerTransformsGUI ( )
{

    this->TransformEditorWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerTransformsGUI::~vtkSlicerTransformsGUI ( )
{
  //this->UIPanel->RemovePage("Transforms");

  if (this->TransformEditorWidget)
    {
    this->TransformEditorWidget->RemoveWidgetObservers ( );
    this->TransformEditorWidget->SetParent (NULL);
    this->TransformEditorWidget->Delete ( );
    }

}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerTransformsGUI: " << this->GetClassName ( ) << "\n";
    // print widgets?
}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::RemoveGUIObservers ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::AddGUIObservers ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::ProcessGUIEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
    // Fill in
} 

//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerTransformsGUI::BuildGUI ( )
{
  // Fill in *placeholder GUI*
  // Define your help text here.
  const char *help = "**Transforms Module:** Create and edit transforms. ";
  
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "Transforms", "Transforms", NULL );
  
  // HELP FRAME
  vtkSlicerModuleCollapsibleFrame *helpFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  helpFrame->SetParent ( this->UIPanel->GetPageWidget ( "Transforms" ) );
  helpFrame->Create ( );
  helpFrame->CollapseFrame ( );
  helpFrame->SetLabelText ("Help");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                helpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Transforms")->GetWidgetName());
  
    // configure the parent classes help text widget
    this->HelpText->SetParent ( helpFrame->GetFrame() );
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
    
  // DISPLAY & EDIT FRAME
  vtkSlicerModuleCollapsibleFrame *displayAndEditFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  displayAndEditFrame->SetParent ( this->UIPanel->GetPageWidget ( "Transforms" ) );
  displayAndEditFrame->Create ( );
  displayAndEditFrame->ExpandFrame ( );
  displayAndEditFrame->SetLabelText ("Display and edit");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                displayAndEditFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Transforms")->GetWidgetName());

  // ---
  this->TransformEditorWidget = vtkSlicerTransformEditorWidget::New ( );
  this->TransformEditorWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
  this->TransformEditorWidget->SetParent ( displayAndEditFrame->GetFrame() );
  this->TransformEditorWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->TransformEditorWidget->GetWidgetName(), displayAndEditFrame->GetFrame()->GetWidgetName());

  displayAndEditFrame->Delete ();
  helpFrame->Delete();
}





