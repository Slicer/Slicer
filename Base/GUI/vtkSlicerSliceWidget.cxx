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
    this->SliceLogic = NULL;
    this->SliceLogicObserverTag = 0;
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
    this->SetSliceLogic(NULL);
    this->SliceLogicObserverTag = 0;
    
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
    this->RenderWidget->CornerAnnotationVisibilityOn();

    //---
    // Create an image viewer
    // Use the render window and renderer of the renderwidget
    this->ImageViewer = vtkImageViewer2::New();
    this->ImageViewer->SetRenderWindow(this->RenderWidget->GetRenderWindow());
    this->ImageViewer->SetRenderer(this->RenderWidget->GetRenderer());
    // use interactor or not?
    this->ImageViewer->SetupInteractor( this->RenderWidget->GetRenderWindow()->GetInteractor());

    this->FieldOfViewEntry = vtkKWEntryWithLabel::New();
    this->FieldOfViewEntry->SetParent (this);
    this->FieldOfViewEntry->SetLabelText ( "FOV:" );
    this->FieldOfViewEntry->Create ( );

    this->OrientationMenu = vtkKWMenuButtonWithLabel::New ();
    this->OrientationMenu->SetParent ( this );
    this->OrientationMenu->SetLabelText ( "Orientation: ");
    vtkKWMenuButton *mb = this->OrientationMenu->GetWidget ( );
    mb->SetWidth ( 8 );
    this->OrientationMenu->Create ( );
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
    //vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    this->Script("pack %s -anchor c -side bottom -expand true -fill both", this->RenderWidget->GetWidgetName());
    this->Script("pack %s -side bottom -expand true -fill x", this->OffsetScale->GetWidgetName());
    this->Script("pack %s -side right -expand false", this->FieldOfViewEntry->GetWidgetName());
    this->Script("pack %s -side right -expand false", this->OrientationMenu->GetWidgetName());
}



//----------------------------------------------------------------------------
void vtkSlicerSliceWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SliceLogic: " << this->SliceLogic << endl;
}

