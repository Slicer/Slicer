#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindow.h"
#include "vtkImageActor.h"

#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSliceViewer.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkMRMLSliceNode.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWFrame.h"
#include "vtkKWScale.h"
#include "vtkKWEntry.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSliceGUI);
vtkCxxRevisionMacro(vtkSlicerSliceGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceGUI::vtkSlicerSliceGUI (  ) {

    // Create objects and set null pointers
    this->SliceViewer = vtkSlicerSliceViewer::New ( );
    this->SliceController = vtkSlicerSliceControllerWidget::New ( );
    this->Logic = NULL;
    this->SliceNode = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerSliceGUI::~vtkSlicerSliceGUI ( ) {

    
    if ( this->SliceViewer )
        {
            this->SliceViewer->Delete ( );
            this->SliceViewer = NULL;
        }
    if ( this->SliceController )
        {
            this->SliceController->RemoveWidgetObservers ( );
            this->SliceController->Delete ( );
            this->SliceController = NULL;
        }


    // wjp test
    this->SetApplication ( NULL );
    this->SliceNode = NULL;
    this->Logic = NULL;
    // end wjp test
    
    this->SetModuleLogic ( NULL );
    this->SetMRMLNode ( NULL );
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerSliceGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "SliceViewer: " << this->GetSliceViewer ( ) << "\n";
    os << indent << "SliceController: " << this->GetSliceController ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    os << indent << "SliceNode: " << this->GetSliceNode ( ) << "\n";
}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddGUIObservers ( ) {

    vtkSlicerSliceControllerWidget *c = this->GetSliceController();
    if ( c != NULL )
      {
      c->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
      c->GetVisibilityToggle()->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
      }
    
    this->SliceViewer->InitializeInteractor (  );

}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveGUIObservers ( ) {

    vtkSlicerSliceControllerWidget *c = this->GetSliceController();
    if ( c != NULL )
      {
      c->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
      c->GetVisibilityToggle()->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
      }

    this->SliceViewer->ShutdownInteractor (  );
}




//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessGUIEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{

    vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast(caller);
    vtkKWMenuButtonWithSpinButtonsWithLabel *o = vtkKWMenuButtonWithSpinButtonsWithLabel::SafeDownCast (caller );

    vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();
    vtkSlicerSliceControllerWidget *c = this->GetSliceController( );

    if (mrml != NULL ) {
        // Toggle the SliceNode's visibility.
        if ( this->GetSliceController()->GetVisibilityToggle() == vtkKWPushButton::SafeDownCast ( caller ) &&
             event == vtkKWPushButton::InvokedEvent )
            {
                this->MRMLScene->SaveStateForUndo ( this->SliceNode );
                if ( this->GetLogic()->GetSliceVisible() > 0 ) {
                    this->GetLogic()->SetSliceVisible ( 0 );
                } else {
                    this->GetLogic()->SetSliceVisible ( 1 );
                }
            }

        //---
        // Scale Widget
        if ( s == c->GetOffsetScale ( ) ) {

            // SET UNDO STATE
            if ( event == vtkKWScale::ScaleValueStartChangingEvent  ) {
                mrml->SaveStateForUndo ( this->GetSliceNode() );
            }
            // UNDO-ABLE APPLY
            if ( event == vtkKWScale::ScaleValueStartChangingEvent || event == vtkCommand::ModifiedEvent ) {
                if ( this->GetSliceNode() )
                  {
                  vtkMatrix4x4 *m = this->GetSliceNode()->GetSliceToRAS ( );
                  m->Identity ( );
                  m->SetElement (2, 3, c->GetOffsetScale()->GetValue ( ) );
                  this->GetSliceNode()->Modified();
                  }
            }
        }
    

        //---
        // Orientation menu
        if ( o == c->GetOrientationMenu ( ) && event == vtkCommand::ModifiedEvent ) {
            // SET UNDO STATE
            //UNDO-ABLE APPLY
            // TO DO: set the RASToSlice matrix from the menu value
            c->GetOrientationMenu()->GetWidget()->GetWidget()->GetValue ( );
        }
    }
}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessLogicEvents ( vtkObject *caller,
                                                unsigned long event, void *callData )
{
  if ( !caller )
    {
    return;
    }

  // process Logic changes
  vtkSlicerSliceLogic *sliceLogic = vtkSlicerSliceLogic::SafeDownCast(caller);
  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::SafeDownCast ( caller );
  
  if ( appLogic == this->GetApplicationLogic ( ) )
    {
    // Nothing yet
    }
  if ( sliceLogic == this->GetLogic ( ) ) 
    {
    // sliceLogic contains the pipeline that create viewer's input, so
    // assume we need to set the image data and render
    vtkSlicerSliceViewer *sliceViewer = this->GetSliceViewer( );
    vtkKWRenderWidget *rw = sliceViewer->GetRenderWidget ();
    sliceViewer->GetImageMapper()->SetInput ( sliceLogic->GetImageData( ) );
    rw->ResetCamera ( );
    rw->Render();

    //
    // Update the VisibilityButton in the SliceController to match the logic state
    //
    if ( sliceLogic->GetSliceVisible() > 0 ) 
      {
      this->GetSliceController()->GetVisibilityToggle()->SetImageToIcon ( 
            this->GetSliceController()->GetVisibilityIcons()->GetVisibleIcon ( ) );        
      } 
      else 
      {
      this->GetSliceController()->GetVisibilityToggle()->SetImageToIcon ( 
            this->GetSliceController()->GetVisibilityIcons()->GetInvisibleIcon ( ) );        
      }

    // TODO: set up corner annotations
    //vtkCornerAnnotation *ca = rw->GetCornerAnnotation ( );
    //ca->SetImageActor (iv->GetImageActor ( ) );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessMRMLEvents ( vtkObject *caller,
                                               unsigned long event, void *callData )
{



}


//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::Exit ( )
{
    // Fill in
}




//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::BuildGUI ( vtkKWFrame *f )
{

    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication ( ) );

    vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();

    this->SliceController->SetApplication ( app );
    this->SliceController->SetAndObserveMRMLScene ( mrml );
    this->SliceController->SetParent ( f );
    this->SliceController->Create (  );
    this->SliceViewer->SetApplication ( app );
    this->SliceViewer->SetParent ( f );
    this->SliceViewer->Create (  );

    // pack 
    this->Script("pack %s -pady 0 -side top -expand false -fill x", SliceController->GetWidgetName() );
    this->Script("pack %s -anchor c -side top -expand true -fill both", SliceViewer->GetRenderWidget()->GetWidgetName());

    this->SliceViewer->InitializeInteractor (  );

}




