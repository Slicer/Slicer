/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.cxx,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

#include "vtkKWApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWScale.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageData.h"
#include "vtkImageViewer2.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWNotebook.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScale.h"
#include "vtkKWWindow.h"
#include "vtkKWWindowLevelPresetSelector.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkToolkits.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLVolumeNode.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerApplicationGUI);
vtkCxxRevisionMacro(vtkSlicerApplicationGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::vtkSlicerApplicationGUI (  ) {

    this->Scale = NULL;
    this->FileBrowseButton = NULL;
    this->ImageViewer = NULL;
    this->WindowLevelPresetSelector = NULL;
    this->RenderWidget = NULL;
    this->Logic = NULL;

}


//---------------------------------------------------------------------------
vtkSlicerApplicationGUI::~vtkSlicerApplicationGUI ( ) {

    if ( this->Scale ) {
        this->Scale->Delete ( );
    }
    if (this->FileBrowseButton ) {
        this->FileBrowseButton->Delete ( );
    }
    if (this->ImageViewer) {
        this->ImageViewer->Delete ( );
    }
    if (this->WindowLevelPresetSelector ) {
        this->WindowLevelPresetSelector->Delete ( );
    }
    if (this->RenderWidget ) {
        this->RenderWidget->Delete ( );
    }
    this->Logic = NULL;
}







//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddGUIObservers ( ) {
    

    this->FileBrowseButton->AddObserver ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICommand );
    this->Scale->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );

}




//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::AddLogicObservers ( ) {

    this->Logic->GetMRMLScene()->AddObserver ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->LogicCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UpdateLogicWithGUIEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) 
{

    vtkKWScale *scalewidget = vtkKWScale::SafeDownCast(caller);
    vtkKWLoadSaveButton *filebrowse = vtkKWLoadSaveButton::SafeDownCast(caller);

    if (scalewidget == this->Scale && event == vtkCommand::ModifiedEvent )
        {
            // Set the current slice in the image viewer.
            if ( this->ImageViewer->GetSlice ( ) != this->Scale->GetValue ( ) ) {
                this->ImageViewer->SetSlice((int) this->Scale->GetValue ( ) );
            }
        }
    else if (filebrowse == this->FileBrowseButton  && event == vtkCommand::ModifiedEvent )
        {
            // If a file has been selected for loading...
            if ( this->FileBrowseButton->GetFileName ( ) ) {
                this->Logic->Connect ( filebrowse->GetFileName ( ) );
            }
        }

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::UpdateGUIWithLogicEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) 
{

    // process Logic changes
    // In this example, only thing that will come from the logic is new image data.
    vtkMRMLScene *mrml = vtkMRMLScene::SafeDownCast(caller);
    
    if (mrml == (this->Logic->GetMRMLScene ( ) ) && event == vtkCommand::ModifiedEvent )
        {
            // If the MRML scene has changed, get the 0th volume node.
            // and set that as input into the ImageViewer.
            vtkMRMLVolumeNode* volumenode = vtkMRMLVolumeNode::SafeDownCast (this->Logic->GetMRMLScene()->GetNthNodeByClass( 0, "vtkMRMLVolumeNode" ) );
            if ( volumenode && volumenode->GetImageData() != NULL )
              {
              this->ImageViewer->SetInput ( volumenode->GetImageData( ) );
              double *range = volumenode->GetImageData()->GetScalarRange ( );
              this->ImageViewer->SetColorWindow ( range [1] - range [0] );
              this->ImageViewer->SetColorLevel (0.5 * (range [1] - range [0] ));
              }
            else
              {
              this->ImageViewer->SetInput (NULL);
              }
            this->ImageViewer->Render();

            // configure window, level, camera, etc.
            this->RenderWidget->ResetCamera ( );
            vtkCornerAnnotation *ca = this->RenderWidget->GetCornerAnnotation ( );
            ca->SetImageActor (this->ImageViewer->GetImageActor ( ) );
            ca->SetWindowLevel (this->ImageViewer->GetWindowLevel ( ) );

            // set the range of the slider
            this->Scale->SetRange ( this->ImageViewer->GetSliceMin ( ), this->ImageViewer->GetSliceMax ( ) );
            this->Scale->SetValue (this->ImageViewer->GetSlice ( ) );
        }

}



//---------------------------------------------------------------------------
void vtkSlicerApplicationGUI::BuildGUI ( ) {

    // for starters, the goal is to make a simple file browse,
    // image viewer and slice slider in the ViewFrame.
    
    // add a LOAD file browser (comes with a popup file browser/selector)
    this->FileBrowseButton = vtkKWLoadSaveButton::New ( );
    this->FileBrowseButton->SetParent (this->Parent );
    this->FileBrowseButton->Create ( );
    this->FileBrowseButton->SetText ("Choose a file to load");
    //this->FileBrowseButton->GetLoadSaveDialog()->SaveDialogOff ( );

    // Should be in TK format. Default is: "{{Text Document} {.txt}}"
    this->FileBrowseButton->GetLoadSaveDialog()->SetFileTypes(
        "{ {MRML Document} {.mrml .xml} }");

    // and add a render widget
    this->RenderWidget = vtkKWRenderWidget::New();
    this->RenderWidget->SetParent ( this->Parent );
    this->RenderWidget->Create();
    this->RenderWidget->CornerAnnotationVisibilityOn();
    
    // Create an image viewer
    // Use the render window and renderer of the renderwidget
    this->ImageViewer = vtkImageViewer2::New();
    this->ImageViewer->SetRenderWindow(this->RenderWidget->GetRenderWindow());
    this->ImageViewer->SetRenderer(this->RenderWidget->GetRenderer());
    this->ImageViewer->SetupInteractor( this->RenderWidget->GetRenderWindow()->GetInteractor());

    // Create a scale to control the slice number displayed
    this->Scale = vtkKWScale::New();
    this->Scale->SetParent(this->Parent );
    this->Scale->Create();

    // Pack in the window's view frame
    if ( this->GetApplication() != NULL ) {
        this->GetApplication()->Script("pack %s -expand n -side top -anchor c -padx 2 -pady 2", 
                                       this->FileBrowseButton->GetWidgetName());
        this->GetApplication()->Script("pack %s -expand y -fill both -anchor c -expand y", 
                                       this->RenderWidget->GetWidgetName());
        this->GetApplication()->Script("pack %s -side top -expand n -fill x -padx 2 -pady 2", 
                                       this->Scale->GetWidgetName());
    }

}





