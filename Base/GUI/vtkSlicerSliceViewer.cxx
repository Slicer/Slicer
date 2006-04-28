#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkSlicerSliceViewer.h"
#include "vtkSlicerApplication.h"

#include "vtkKWWidget.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWFrame.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceViewer );
vtkCxxRevisionMacro ( vtkSlicerSliceViewer, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceViewer::vtkSlicerSliceViewer ( ) {

    //---  
    // widgets comprising the SliceViewer for now.
    this->ImageViewer = NULL;
    this->RenderWidget = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerSliceViewer::~vtkSlicerSliceViewer ( ){

    if ( this->ImageViewer ) {
        this->ImageViewer->Delete ( );
        this->ImageViewer = NULL;
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
    this->RenderWidget = vtkKWRenderWidget::New ( );
    this->RenderWidget->SetParent ( this->GetParent( ) );
    this->RenderWidget->Create();
    int w = app->GetMainLayout()->GetSliceViewerMinDim ( );
    this->RenderWidget->SetWidth ( w );
    this->RenderWidget->SetHeight ( w );
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

}



//----------------------------------------------------------------------------
void vtkSlicerSliceViewer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // widgets?
}

