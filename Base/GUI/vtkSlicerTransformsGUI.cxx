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
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "Transforms", "Transforms", NULL );
    
  // Define your help text and build the help frame here.
  const char *help = "The Transforms Module creates and edits transforms. ";
  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Transforms" );
  this->BuildHelpAndAboutFrame ( page, help, about );

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
}





