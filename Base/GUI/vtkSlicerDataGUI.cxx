#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerDataGUI.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerDataGUI );
vtkCxxRevisionMacro ( vtkSlicerDataGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerDataGUI::vtkSlicerDataGUI ( )
{
  MRMLTreeWidget = vtkSlicerMRMLTreeWidget::New();
    //this->Logic = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerDataGUI::~vtkSlicerDataGUI ( )
{
  if (this->MRMLTreeWidget)
    {
    this->MRMLTreeWidget->RemoveWidgetObservers ( );
    this->MRMLTreeWidget->SetParent (NULL );
    this->MRMLTreeWidget->Delete ( );
    }
    // class not yet defined!
    //this->SetModuleLogic ( NULL );
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerDataGUI: " << this->GetClassName ( ) << "\n";
    //os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    // print widgets?
}



//---------------------------------------------------------------------------
void vtkSlicerDataGUI::RemoveGUIObservers ( )
{
  this->MRMLTreeWidget->RemoveObservers (vtkSlicerMRMLTreeWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::AddGUIObservers ( )
{
  this->MRMLTreeWidget->AddObserver (vtkSlicerMRMLTreeWidget::SelectedEvent, (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
  const char *moduleName;
  vtkMRMLNode *node = (vtkMRMLNode *)callData;
  if (node->IsA("vtkMRMLVolumeNode"))
    {
    moduleName = "Volumes";
    }
  else if (node->IsA("vtkMRMLModelNode"))
    {
    moduleName = "Models";
    }
  else if (node->IsA("vtkMRMLTransformNode"))
    {
    moduleName = "Transforms";
    }
  else if (node->IsA("vtkMRMLFiducialListNode"))
    {
    moduleName = "Fiducials";
    }

  this->InvokeEvent(vtkSlicerModuleGUI::ModuleSelectedEvent, (void *)moduleName);
}


//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerDataGUI::Exit ( )
{
    // Fill in
}




//---------------------------------------------------------------------------
void vtkSlicerDataGUI::BuildGUI ( )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Data", "Data", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Data Module displays and permits operations on the MRML tree. ";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Data" );
    this->BuildHelpAndAboutFrame ( page, help, about );

    // INSPECT FRAME
    vtkSlicerModuleCollapsibleFrame *displayModifyFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    displayModifyFrame->SetParent ( this->UIPanel->GetPageWidget ( "Data" ) );
    displayModifyFrame->Create ( );
    displayModifyFrame->ExpandFrame ( );
    displayModifyFrame->SetLabelText ("Display & modify scene");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  displayModifyFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Data")->GetWidgetName());
    
    this->MRMLTreeWidget->SetAndObserveMRMLScene(this->GetMRMLScene() );
    this->MRMLTreeWidget->SetParent ( displayModifyFrame->GetFrame() );
    this->MRMLTreeWidget->Create ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  this->MRMLTreeWidget->GetWidgetName(), displayModifyFrame->GetFrame()->GetWidgetName());
    
    displayModifyFrame->Delete ( );
}





