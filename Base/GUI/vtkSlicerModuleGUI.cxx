#include "vtkObjectFactory.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkKWUserInterfacePanel.h"

//---------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerModuleGUI, "$Revision: 1.0 $");

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




