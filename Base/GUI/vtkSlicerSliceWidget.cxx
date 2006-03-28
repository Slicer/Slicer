#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkKWWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWRenderWidget.h"
#include "vtkSlicerSliceWidget.h"
#include "vtkSlicerGUI.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSliceWidget );
vtkCxxRevisionMacro ( vtkSlicerSliceWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceWidget::vtkSlicerSliceWidget ( ) {

    //---  
    // widgets comprising the SliceWidget for now.
    this->Scale = NULL;
    this->SliceFrame = NULL;
    this->ImageViewer = NULL;
    this->RenderWidget = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerSliceWidget::~vtkSlicerSliceWidget ( ){
    if ( this->Scale ) {
        this->Scale->Delete ( );
    }
    if ( this->RenderWidget ) {
        this->RenderWidget->Delete ( );
    }
    if ( this->ImageViewer ) {
        this->ImageViewer->Delete ( );
    }
    if ( this->SliceFrame ) {
        this->SliceFrame->Delete ( );
    }

}


//---------------------------------------------------------------------------
void vtkSlicerSliceWidget::Create ( ) {

    vtkSlicerGUI *app = (vtkSlicerGUI *)this->GetApplication();
    
    //---
    // Create a frame
    this->SliceFrame = vtkKWFrame::New ( );
    this->SliceFrame->SetApplication ( app );
    this->SliceFrame->Create ( );
    
    //---
    // Create a render widget
    this->RenderWidget = vtkKWRenderWidget::New ( );
    this->RenderWidget->SetParent ( this->SliceFrame );
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

    //---
    // Create a scale to control the slice number displayed
    this->Scale = vtkKWScaleWithEntry::New();
    this->Scale->SetParent( this->SliceFrame );
    this->Scale->Create();
    this->Scale->SetRange(0.0, 100.0);
    this->Scale->SetResolution(1.0);
    this->Scale->RangeVisibilityOff ( );
            
    // pack 

    app->Script("pack %s -side top -fill x -padx 2 -pady 2", this->Scale->GetWidgetName());
    app->Script("pack %s -side top -anchor c", this->RenderWidget->GetWidgetName());

}
