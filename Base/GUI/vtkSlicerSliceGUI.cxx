#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageViewer2.h"
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

    // Remove observers and delete.
    if ( this->SliceViewer )
        {
            this->SliceViewer->Delete ( );
            this->SliceViewer = NULL;
        }
    if ( this->SliceController )
        {
            this->SliceController->Delete ( );
            this->SliceController = NULL;
        }
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
            vtkKWScaleWithEntry *s = c->GetOffsetScale () ;
            vtkKWEntryWithLabel *e = c->GetFieldOfViewEntry ();
            vtkKWMenuButtonWithLabel *m = c->GetOrientationMenu ();

            s->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
            s->AddObserver ( vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
            e->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
            m->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
        }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveGUIObservers ( ) {

    vtkSlicerSliceControllerWidget *c = this->GetSliceController( );
    if ( c != NULL )
        {
            vtkKWScaleWithEntry *s = c->GetOffsetScale () ;
            vtkKWEntryWithLabel *e = c->GetFieldOfViewEntry ();
            vtkKWMenuButtonWithLabel *m = c->GetOrientationMenu ();

            s->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
            s->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
            e->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
            m->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );
        }
}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessGUIEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{

    vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast(caller);
    vtkKWMenuButtonWithLabel *o = vtkKWMenuButtonWithLabel::SafeDownCast (caller );

    vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();
    vtkSlicerSliceControllerWidget *c = this->GetSliceController( );

    if (mrml != NULL ) {
        //---
        // Scale Widget
        if ( s == c->GetOffsetScale ( ) ) {

            // SET UNDO STATE
            if ( event == vtkKWScale::ScaleValueStartChangingEvent  ) {
                mrml->SaveStateForUndo ( this->GetSliceNode() );
            }
            // UNDO-ABLE APPLY
            if ( event == vtkKWScale::ScaleValueStartChangingEvent || event == vtkCommand::ModifiedEvent ) {
                vtkMatrix4x4 *m = this->GetSliceNode()->GetSliceToRAS ( );
                m->Identity ( );
                m->SetElement (2, 3, c->GetOffsetScale()->GetValue ( ) );
                this->GetSliceNode()->Modified();
            }
        }
    
        //---
        // Change Slice Node with new FieldOfView Entry Widget values
        if ( e == c->GetFieldOfViewEntry() && event == vtkCommand::ModifiedEvent ) {
            // SET UNDO STATE
            mrml->SaveStateForUndo ( this->GetSliceNode() );
            // UNDO-ABLE APPLY
            double val = c->GetFieldOfViewEntry()->GetWidget()->GetValueAsDouble();
            if ( val != 0 ) {
                this->GetSliceNode()->SetFieldOfView ( val, val, val );
                this->GetSliceNode()->Modified();
            }
        }

        //---
        // Orientation menu
        if ( o == c->GetOrientationMenu ( ) && event == vtkCommand::ModifiedEvent ) {
            // SET UNDO STATE
            //UNDO-ABLE APPLY
            // TO DO: set the RASToSlice matrix from the menu value
            c->GetOrientationMenu()->GetWidget()->GetValue ( );
        }
    }
}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessLogicEvents ( vtkObject *caller,
                                                unsigned long event, void *callData )
{
    // process Logic changes
    vtkSlicerSliceLogic *n = vtkSlicerSliceLogic::SafeDownCast(caller);
    vtkSlicerApplicationLogic *a = vtkSlicerApplicationLogic::SafeDownCast ( caller );
    
    if ( a == this->GetApplicationLogic ( ) )
        {
            // get active VolumeID
            // is this different from the ID of volume in the BG?
            // if so, make the change
            // and update the pipeline
        }
    else
        {
            if ( n == this->GetLogic ( ) ) {
                vtkSlicerSliceControllerWidget *c = this->GetSliceController( );
                // UPDATE THE FOV ENTRY
                double fov = this->GetSliceNode()->GetFieldOfView()[0];
                char fovstring[80];
                sprintf (fovstring, "%g", fov);
                c->GetFieldOfViewEntry()->GetWidget()->SetValue(fovstring);
                c->GetFieldOfViewEntry()->Modified();

                // UPDATE THE SCALE
                double fovover2 = this->GetSliceNode()->GetFieldOfView()[2] / 2.;
                c->GetOffsetScale()->SetRange ( -fovover2, fovover2 );
                // TODO: set the scale value from the translation part
                // of the matrix with rotation. 
                // Set the scale from the Offset in the matrix.
                vtkMatrix4x4 *m = n->GetSliceNode()->GetSliceToRAS();
                c->GetOffsetScale()->SetValue ( m->GetElement(2,3) );
                c->GetOffsetScale()->Modified();
        
                vtkSlicerSliceViewer *v = this->GetSliceViewer( );
                // UPDATE IMAGE VIEWER
                vtkImageViewer2 *iv = v->GetImageViewer ();
                vtkKWRenderWidget *rw = v->GetRenderWidget ();
                if ( n->GetImageData() != NULL )
                    {
                        iv->SetInput ( n->GetImageData( ) );
                    }
                else
                    {
                        iv->SetInput (NULL);
                    }
                iv->Render();
                // configure window, level, camera, etc.
                rw->ResetCamera ( );
                vtkCornerAnnotation *ca = rw->GetCornerAnnotation ( );
                ca->SetImageActor (iv->GetImageActor ( ) );
                v->Modified ();

            }
        }
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessMRMLEvents ( vtkObject *caller,
                                               unsigned long event, void *callData )
{
    // Fill in
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

    this->SliceController->SetApplication ( app );
    this->SliceController->SetParent ( f );
    this->SliceController->Create (  );
    this->SliceViewer->SetApplication ( app );
    this->SliceViewer->SetParent ( f );
    this->SliceViewer->Create (  );

    // pack 
    this->Script("pack %s -pady 0 -side top -expand false -fill x", SliceController->GetControlFrame()->GetWidgetName() );
    this->Script("pack %s -anchor c -side top -expand true -fill both", SliceViewer->GetRenderWidget()->GetWidgetName());

    this->Script("pack %s -side bottom -expand false -fill x", SliceController->GetOffsetScale()->GetWidgetName());
    this->Script("pack %s -pady 2 -padx 2 -side right -expand false", SliceController->GetFieldOfViewEntry()->GetWidgetName());
    this->Script("pack %s -pady 2 -padx 2 -side right -expand false", SliceController->GetOrientationMenu()->GetWidgetName());


}




