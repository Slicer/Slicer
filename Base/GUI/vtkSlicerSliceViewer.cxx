#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkSlicerSliceViewer.h"
#include "vtkSlicerApplication.h"

#include "vtkKWWidget.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWFrame.h"

#include "vtkImageMapper.h"
#include "vtkActor2D.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceViewer );
vtkCxxRevisionMacro ( vtkSlicerSliceViewer, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceViewer::vtkSlicerSliceViewer ( ) {

    //---  
    // widgets comprising the SliceViewer for now.
    this->RenderWidget = vtkKWRenderWidget::New ( );

    this->ImageMapper = vtkImageMapper::New();
    this->ImageMapper->SetColorWindow(255);
    this->ImageMapper->SetColorLevel(127.5);

    this->Actor2D = vtkActor2D::New();
    this->Actor2D->SetMapper( this->ImageMapper );
}


//---------------------------------------------------------------------------
vtkSlicerSliceViewer::~vtkSlicerSliceViewer ( ){

    if ( this->ImageMapper ) {
        this->ImageMapper->Delete ( );
        this->ImageMapper = NULL;
    }

    if ( this->Actor2D ) {
        this->Actor2D->Delete ( );
        this->Actor2D = NULL;
    }

    if ( this->RenderWidget ) {
        this->RenderWidget->Delete ( );
        this->RenderWidget = NULL;
    }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceViewer::CreateWidget ( ) {

    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetParent()->GetApplication() );

    // the widget is a frame with an image viewer packed inside
    if (this->IsCreated ( ) ) {
        vtkErrorMacro ( << this->GetClassName() << "already created.");
        return;
    }
    this->Superclass::CreateWidget ( );
    
    //---
    // Create a render widget
    this->RenderWidget->SetParent ( this->GetParent( ) );
    this->RenderWidget->Create();
    int w = app->GetMainLayout()->GetSliceViewerMinDim ( );
    this->RenderWidget->SetWidth ( w );
    this->RenderWidget->SetHeight ( w );
    this->RenderWidget->CornerAnnotationVisibilityOn();
    this->RenderWidget->SetBorderWidth(2);
    this->RenderWidget->SetReliefToGroove ( );

    //---
    this->RenderWidget->GetRenderer()->AddActor2D( this->Actor2D );

    // need to set up the RenderWidget events to modify the slice node

}



//----------------------------------------------------------------------------
void vtkSlicerSliceViewer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // widgets?
}

