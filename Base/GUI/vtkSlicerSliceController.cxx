#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkImageData.h"

#include "vtkSlicerSliceController.h"
#include "vtkSlicerApplication.h"

#include "vtkKWWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWScale.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWMenu.h"
#include "vtkKWFrame.h"
#include "vtkKWMenuButton.h"



//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceController );
vtkCxxRevisionMacro ( vtkSlicerSliceController, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceController::vtkSlicerSliceController ( ) {

    //---  
    // widgets comprising the SliceController for now.
    this->OffsetScale = NULL;
    this->FieldOfViewEntry = NULL;
    this->OrientationMenu = NULL;
    this->ControlFrame = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerSliceController::~vtkSlicerSliceController ( ){

    if ( this->OffsetScale ) {
        this->OffsetScale->Delete ( );
        this->OffsetScale = NULL;
    }
    if ( this->FieldOfViewEntry ) {
        this->FieldOfViewEntry->Delete ( );
        this->FieldOfViewEntry = NULL;
    }
    if ( this->OrientationMenu ) {
        this->OrientationMenu->Delete ( );
        this->OrientationMenu = NULL;
    }
    if ( this->ControlFrame ) {
        this->ControlFrame->Delete ( );
        this->ControlFrame = NULL;
    }
    
}



//---------------------------------------------------------------------------
void vtkSlicerSliceController::Create ( ) {

    // the widget is a frame with some widgets inside
    if (this->IsCreated ( ) ) {
        vtkErrorMacro ( << this->GetClassName() << "already created.");
        return;
    }
    this->Superclass::Create ( );
    
    this->ControlFrame = vtkKWFrame::New ( );
    this->ControlFrame->SetParent ( this->GetParent( ) );
    this->ControlFrame->Create( );

    this->FieldOfViewEntry = vtkKWEntryWithLabel::New();
    this->FieldOfViewEntry->SetParent (this->ControlFrame);
    this->FieldOfViewEntry->SetLabelText ( "FOV:" );
    this->FieldOfViewEntry->Create ( );
    this->FieldOfViewEntry->SetWidth ( 8 );

    this->OrientationMenu = vtkKWMenuButtonWithLabel::New ();
    this->OrientationMenu->SetParent ( this->ControlFrame );
    this->OrientationMenu->Create ( );
    this->OrientationMenu->SetLabelText ( "Orientation: ");
    vtkKWMenuButton *mb = this->OrientationMenu->GetWidget ( );
    mb->SetWidth ( 8 );
    mb->GetMenu()->AddRadioButton ( "Axial" );
    mb->GetMenu()->AddRadioButton ( "Saggital" );
    mb->GetMenu()->AddRadioButton ( "Coronal" );
    mb->SetValue ("Axial");    
        
    //---
    // Create a scale to control the slice number displayed
    this->OffsetScale = vtkKWScaleWithEntry::New();
    this->OffsetScale->SetParent ( this->ControlFrame );
    this->OffsetScale->Create();
    this->OffsetScale->RangeVisibilityOff ( );
            
}



//----------------------------------------------------------------------------
void vtkSlicerSliceController::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // widgets?
}

