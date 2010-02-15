#include "vtkObject.h"
#include "vtkObjectFactory.h"
//#include "vtkCommand.h"

#include "vtkSlicerEmptyModuleGUI.h"
#include "vtkSlicerApplication.h"
//#include "vtkSlicerModuleCollapsibleFrame.h"
//#include "vtkKWFrameWithLabel.h"

//#include "vtkKWWidget.h"
//#include "vtkKWMenuButton.h"
//#include "vtkKWMenu.h"
//#include "vtkKWFrame.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerEmptyModuleGUI );
vtkCxxRevisionMacro ( vtkSlicerEmptyModuleGUI, "$Revision$");

//---------------------------------------------------------------------------
vtkSlicerEmptyModuleGUI::vtkSlicerEmptyModuleGUI ( )
{
}

//---------------------------------------------------------------------------
vtkSlicerEmptyModuleGUI::~vtkSlicerEmptyModuleGUI ( )
{
  //this->UIPanel->RemovePage("Transforms");
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerEmptyModuleGUI: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::RemoveGUIObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::AddGUIObservers ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::ProcessGUIEvents ( vtkObject * vtkNotUsed(caller),
                                                 unsigned long vtkNotUsed(event),
                                                 void *vtkNotUsed(callData) )
{
} 

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::ReleaseModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::Enter ( vtkMRMLNode * vtkNotUsed(node) )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::TearDownGUI ( )
{
  this->Exit();
  this->RemoveGUIObservers();
}

//---------------------------------------------------------------------------
void vtkSlicerEmptyModuleGUI::BuildGUI ( )
{
  // Fill in *placeholder GUI*

  // you can use this call to access the slicer application:
  //vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // create a page
  this->UIPanel->AddPage ( "EmptyModule", "EmptyModule", NULL );
    
  // Define your help text and build the help frame here.
  const char *help = "Empty Module\n";
  const char *about = "This work was supported by NA-MIC.";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "EmptyModule" );
  this->BuildHelpAndAboutFrame ( page, help, about );
}
