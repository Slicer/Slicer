#include "vtkSlicerGUI.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkXMLParser.h"
#include "vtkSlicerStyle.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerGUI);
vtkCxxRevisionMacro(vtkSlicerGUI, "$Revision: 1.0 $");



//---------------------------------------------------------------------------
vtkSlicerGUI::vtkSlicerGUI ( ) {


    // allocatate for look and feel
    this->SlicerStyle = vtkSlicerStyle::New ( );
}


//---------------------------------------------------------------------------
vtkSlicerGUI::~vtkSlicerGUI ( ) {
    
    if ( this->SlicerStyle ) {
        this->SlicerStyle->Delete ( );
    }

}




