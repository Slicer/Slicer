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
}




//---------------------------------------------------------------------------
vtkSlicerModuleGUI::~vtkSlicerModuleGUI ( ) {

    if ( this->UIPanel ) {
        this->UIPanel->Delete ( );
        this->UIPanel = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModuleGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerModuleGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "UIPanel: " << this->GetUIPanel ( ) << "\n";
}



