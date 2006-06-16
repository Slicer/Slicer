#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerTransformsGUI.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerApplication.h"
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

    this->TransformManagerWidget = NULL;
    this->TransformEditorWidget = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerTransformsGUI::~vtkSlicerTransformsGUI ( )
{

  if (this->TransformManagerWidget)
    {
    this->TransformManagerWidget->RemoveWidgetObservers ( );
    this->TransformManagerWidget->Delete ( );
    }
  if (this->TransformEditorWidget)
    {
    this->TransformEditorWidget->RemoveWidgetObservers ( );
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
  
  // HELP FRAME
  vtkKWFrameWithLabel *helpFrame = vtkKWFrameWithLabel::New ( );
  helpFrame->SetParent ( this->UIPanel->GetPageWidget ( "Transforms" ) );
  helpFrame->Create ( );
  helpFrame->CollapseFrame ( );
  helpFrame->SetLabelText ("Help");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                helpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Transforms")->GetWidgetName());
  
  // ---
  this->TransformManagerWidget = vtkSlicerTransformManagerWidget::New ( );
  this->TransformManagerWidget->SetMRMLScene(this->GetMRMLScene() );
  this->TransformManagerWidget->SetParent ( this->UIPanel->GetPageWidget ( "Transforms" ) );
  this->TransformManagerWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->TransformManagerWidget->GetWidgetName(), this->UIPanel->GetPageWidget("Transforms")->GetWidgetName());
  
  // ---
  this->TransformEditorWidget = vtkSlicerTransformEditorWidget::New ( );
  this->TransformEditorWidget->SetMRMLScene(this->GetMRMLScene() );
  this->TransformEditorWidget->SetParent ( this->UIPanel->GetPageWidget ( "Transforms" ) );
  this->TransformEditorWidget->Create ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->TransformEditorWidget->GetWidgetName(), this->UIPanel->GetPageWidget("Transforms")->GetWidgetName());
  
  helpFrame->Delete();
}





