#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkKWWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWMenu.h"
#include "vtkKWFrame.h"
#include "vtkKWMenuButton.h"
#include "vtkSlicerSliceWidget.h"
#include "vtkSlicerApplication.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceWidget );
vtkCxxRevisionMacro ( vtkSlicerSliceWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceWidget::vtkSlicerSliceWidget ( ) {

    //---  
    // widgets comprising the SliceWidget for now.
    this->OffsetScale = NULL;
    this->FieldOfViewEntry = NULL;
    this->OrientationMenu = NULL;
    this->ImageViewer = NULL;
    this->RenderWidget = NULL;
    this->ControlFrame = NULL;
    this->SliceLogic = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerSliceWidget::~vtkSlicerSliceWidget ( ){

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
    if ( this->ImageViewer ) {
        this->ImageViewer->Delete ( );
        this->ImageViewer = NULL;
    }
    if ( this->RenderWidget ) {
        this->RenderWidget->Delete ( );
        this->RenderWidget = NULL;
    }
    if ( this->ControlFrame ) {
        this->ControlFrame->Delete ( );
        this->ControlFrame = NULL;
    }

    this->SliceLogic = NULL;
    
}



//---------------------------------------------------------------------------
void vtkSlicerSliceWidget::Create ( ) {

    // the widget is a frame with a scale and an image viewer packed inside
    if (this->IsCreated ( ) ) {
        vtkErrorMacro ( << this->GetClassName() << "already created.");
        return;
    }
    this->Superclass::Create ( );
    
    //---
    // Create a render widget
    this->RenderWidget = vtkKWRenderWidget::New ( );
    this->RenderWidget->SetParent ( this );
    this->RenderWidget->Create();
    this->RenderWidget->SetWidth ( 10 );
    this->RenderWidget->SetHeight ( 10 );
    this->RenderWidget->CornerAnnotationVisibilityOn();
    this->RenderWidget->SetBorderWidth(2);
    this->RenderWidget->SetReliefToGroove ( );

    //---
    // Create an image viewer
    // Use the render window and renderer of the renderwidget
    this->ImageViewer = vtkImageViewer2::New();
    this->ImageViewer->SetRenderWindow(this->RenderWidget->GetRenderWindow());
    this->ImageViewer->SetRenderer(this->RenderWidget->GetRenderer());
    // use interactor or not?
    this->ImageViewer->SetupInteractor( this->RenderWidget->GetRenderWindow()->GetInteractor());

    this->ControlFrame = vtkKWFrame::New ( );
    this->ControlFrame->SetParent (this);
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
    this->OffsetScale->SetParent ( this );
    this->OffsetScale->Create();
    this->OffsetScale->RangeVisibilityOff ( );
            
    // pack slider and viewer into frame
    this->Script("pack %s -pady 0 -side top -expand false -fill x", this->ControlFrame->GetWidgetName() );
    this->Script("pack %s -pady 2 -padx 2 -side right -expand false", this->FieldOfViewEntry->GetWidgetName());
    this->Script("pack %s -pady 2 -padx 2 -side right -expand false", this->OrientationMenu->GetWidgetName());
    this->Script("pack %s -side top -expand false -fill x", this->OffsetScale->GetWidgetName());
    this->Script("pack %s -anchor c -side top -expand true -fill both", this->RenderWidget->GetWidgetName());

}



//----------------------------------------------------------------------------
void vtkSlicerSliceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SliceLogic: " << this->SliceLogic << endl;
}

