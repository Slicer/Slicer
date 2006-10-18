#include "vtkObjectFactory.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkKWUserInterfacePanel.h"


//---------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerModuleGUI, "$Revision: 1.0 $");
vtkStandardNewMacro ( vtkSlicerModuleGUI );

//---------------------------------------------------------------------------
vtkSlicerModuleGUI::vtkSlicerModuleGUI ( ) {

    this->UIPanel = vtkKWUserInterfacePanel::New ( );
    this->HelpText = vtkKWTextWithScrollbars::New ( );
}




//---------------------------------------------------------------------------
vtkSlicerModuleGUI::~vtkSlicerModuleGUI ( ) {

    if ( this->UIPanel != NULL ) {
        this->UIPanel->Delete ( );
        this->UIPanel = NULL;
    }
    if ( this->HelpText != NULL )
      {
      this->HelpText->SetParent ( NULL );
      this->HelpText->Delete ( );
      this->HelpText = NULL;
      }
}


//---------------------------------------------------------------------------
void vtkSlicerModuleGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerModuleGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "UIPanel: " << this->GetUIPanel ( ) << "\n";
    os << indent << "HelpText: " << this->GetHelpText ( ) << "\n";

}



