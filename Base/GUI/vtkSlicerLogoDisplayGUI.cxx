#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerLogoDisplayGUI.h"
#include "vtkSlicerWindow.h"

#include "vtkKWWidget.h"
#include "vtkKWLabel.h"
#include "vtkSlicerLogoIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerLogoDisplayGUI );
vtkCxxRevisionMacro ( vtkSlicerLogoDisplayGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerLogoDisplayGUI::vtkSlicerLogoDisplayGUI ( )
{

     //--- slicer icons
    this->SlicerLogoIcons = vtkSlicerLogoIcons::New ();
    //--- logo widgets to which icons are assigned.
    this->SlicerLogoLabel = vtkKWLabel::New();
    this->ModuleLogoLabel = vtkKWLabel::New();
}


//---------------------------------------------------------------------------
vtkSlicerLogoDisplayGUI::~vtkSlicerLogoDisplayGUI ( )
{

    if ( this->SlicerLogoIcons )
      {
      this->SlicerLogoIcons->Delete ( );
      this->SlicerLogoIcons = NULL;
      }
    if ( this->SlicerLogoLabel )
      {
      this->SlicerLogoLabel->SetParent ( NULL );
      this->SlicerLogoLabel->Delete ( );
      this->SlicerLogoLabel = NULL;
      }
    if ( this->ModuleLogoLabel )
      {
      this->ModuleLogoLabel->SetParent ( NULL );
      this->ModuleLogoLabel->Delete ( );
      this->ModuleLogoLabel = NULL;
      }
    this->SetApplicationGUI ( NULL );
}





//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerLogoDisplayGUI: " << this->GetClassName ( ) << "\n";

}



//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::RemoveGUIObservers ( )
{
  // FILL IN
}

//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::AddGUIObservers ( )
{
  // FILL IN
}


//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{

  if ( this->GetApplicationGUI() != NULL )
    {
      vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );
      if ( app != NULL )
        {
        // Fill in.
        }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI )
{
  this->ApplicationGUI = appGUI;
}




//---------------------------------------------------------------------------
void vtkSlicerLogoDisplayGUI::BuildGUI ( vtkKWFrame *appF )
{

  vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );
  
  // populate the application's 3DView control GUI panel
  if ( p != NULL )
    {
    if ( p->GetApplication() != NULL )
      {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );

        this->SlicerLogoLabel->SetParent ( appF );
        this->SlicerLogoLabel->Create();
        this->SlicerLogoLabel->SetImageToIcon ( this->SlicerLogoIcons->GetSlicerLogo() );

        this->ModuleLogoLabel->SetParent( appF );
        this->ModuleLogoLabel->Create();

        app->Script ( "pack %s -side left -anchor nw -padx 0 -pady 0", this->SlicerLogoLabel->GetWidgetName() );
        app->Script ( "pack %s -side right -anchor e -padx 0 -pady 0", this->ModuleLogoLabel->GetWidgetName() );
      }
    }

}





