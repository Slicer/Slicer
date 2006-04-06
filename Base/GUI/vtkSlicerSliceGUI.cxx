#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSliceWidgetCollection.h"
#include "vtkSlicerSliceWidget.h"
#include "vtkImageViewer2.h"
#include "vtkCornerAnnotation.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkSlicerSliceLogic.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSliceGUI);
vtkCxxRevisionMacro(vtkSlicerSliceGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSliceGUI::vtkSlicerSliceGUI (  ) {

    this->Logic = NULL;
    this->SliceLogic = NULL;
    this->SliceWidgets = vtkSlicerSliceWidgetCollection::New();
    this->MainSlice0 = NULL;
    this->MainSlice1 = NULL;
    this->MainSlice2 = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerSliceGUI::~vtkSlicerSliceGUI ( ) {

    if ( this->SliceWidgets ) {
        this->SliceWidgets->RemoveAllItems ( );
        this->SliceWidgets->Delete();
        this->SliceWidgets = NULL;
    }
    if ( this->MainSlice0 ) {
        this->MainSlice0->Delete ( );
        this->MainSlice0 = NULL;
    }
    if ( this->MainSlice1 ) {
        this->MainSlice1->Delete ( );
        this->MainSlice1 = NULL;
    }
    if ( this->MainSlice2 ) {
        this->MainSlice2->Delete ( );
        this->MainSlice2 = NULL;
    }    
    this->SetSliceLogic(NULL);
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddSliceWidget ( vtkSlicerSliceWidget *w ){

    // Create if it doesn't exist already
    if ( this->SliceWidgets == NULL ) {
        this->SliceWidgets = vtkSlicerSliceWidgetCollection::New();
    }
    //Add widget
    this->SliceWidgets->AddItem ( w );
}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddGUIObservers ( ) {

    // add observers ont SliceWidget(0)
    vtkSlicerSliceWidget *sw = this->GetSliceWidget (0);
    vtkKWScaleWithEntry *s = sw->GetOffsetScale () ;
    vtkKWEntryWithLabel *e = sw->GetFieldOfViewEntry ();
    vtkKWMenuButtonWithLabel *m = sw->GetOrientationMenu ();

    s->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
    s->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICommand );
    e->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
    m->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );

    // add observers onto all SliceWidgets (do later)
    /*
    int n = this->SliceWidgets->GetNumberOfItems ( );
    for ( i = 0; i < n; i++) {
        vtkSlicerSliceWidget *sw = this->SliceWidgets->GetItemAsObject ( i );
        vtkImageViewer2 *iv = sw->GetImageViewer ();
        vtkKWScaleWithEntry *s = sw->GetOffsetScale () ;
        vtkKWEntryWithLabel *e = sw->GetFieldOfViewEntry ();
        vtkKWMenuButtonWithLabel *m = sw->GetOrientationMenu ();

        s->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
        s->AddObserver (vtkScale::StartEvent, (vtkCommand *)this->GUICommand );
        e->AddObserver (vtkCommand::ModifiedEvent (vtkCommand *)this->GUICommand );
        m->AddObserver (vtkCommand::ModifiedEvent (vtkCommand *)this->GUICommand );
    }
    */
}






//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddLogicObservers ( ) {
    unsigned long tag;
    
    // add observers onto mrml scene; observe the widget's SliceLogic
    // Before a widget's SliceLogic is set, no observers are created.
    if ( this->GetSliceWidget(0)->GetSliceLogic ( ) != NULL ) {
        tag = this->GetSliceWidget(0)->GetSliceLogic()->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->LogicCommand );
        this->GetSliceWidget(0)->SetSliceLogicObserverTag ( tag );
    }

}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveGUIObservers ( ) {

    vtkSlicerSliceWidget *sw = this->GetSliceWidget (0);
    if (sw)
      {
      vtkKWScaleWithEntry *s = sw->GetOffsetScale () ;
      vtkKWEntryWithLabel *e = sw->GetFieldOfViewEntry ();
      vtkKWMenuButtonWithLabel *m = sw->GetOrientationMenu ();

      s->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
      s->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICommand );
      e->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
      m->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
      }

}


//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveLogicObservers ( ) {

  if ( this->GetSliceWidget(0)->GetSliceLogic ( ) != NULL ) {
  this->GetSliceWidget(0)->GetSliceLogic()->RemoveObservers ( vtkCommand::ModifiedEvent, (vtkCommand *)this->LogicCommand );
    }


}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessGUIEvents ( vtkObject *caller,
                                                   unsigned long event,
                                           void *callData ) {

    vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast(caller);
    vtkKWMenuButtonWithLabel *o = vtkKWMenuButtonWithLabel::SafeDownCast (caller );
    vtkMRMLScene *mrml = this->Logic->GetMRMLScene();

    if (mrml != NULL ) {
        //---
        // Scale Widget
        vtkSlicerSliceWidget *sw = this->GetSliceWidget(0);
        if ( s == sw->GetOffsetScale ( ) ) {

            // SET UNDO STATE
            if ( event == vtkKWScale::ScaleValueStartChangingEvent  ) {
                mrml->SaveStateForUndo ( sw->GetSliceLogic()->GetSliceNode() );
            }
            // UNDO-ABLE APPLY
            if ( event == vtkKWScale::ScaleValueStartChangingEvent || event == vtkCommand::ModifiedEvent ) {
                vtkMatrix4x4 *m = sw->GetSliceLogic()->GetSliceNode()->GetSliceToRAS ( );
                m->Identity ( );
                m->SetElement (2, 3, sw->GetOffsetScale()->GetValue ( ) );
                sw->GetSliceLogic()->GetSliceNode()->Modified();
            }
        }
    
        //---
        // FieldOfView Entry Widget
        if ( e == sw->GetFieldOfViewEntry() && event == vtkCommand::ModifiedEvent ) {
            // SET UNDO STATE
            mrml->SaveStateForUndo ( sw->GetSliceLogic()->GetSliceNode() );
            // UNDO-ABLE APPLY
            double val = sw->GetFieldOfViewEntry()->GetWidget()->GetValueAsDouble();
            if ( val != 0 ) {
                sw->GetSliceLogic()->GetSliceNode()->SetFieldOfView ( val, val, val );
                sw->GetSliceLogic()->GetSliceNode()->Modified();
            }
        }

        //---
        // Orientation menu
        if ( o == sw->GetOrientationMenu ( ) && event == vtkCommand::ModifiedEvent ) {
            // SET UNDO STATE
            //UNDO-ABLE APPLY
            // TO DO: set the RASToSlice matrix from the menu value
            sw->GetOrientationMenu()->GetWidget()->GetValue ( );
        }
    }
}






//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessLogicEvents ( vtkObject *caller,
                                                   unsigned long event,
                                                   void *callData ) 
{
    // process Logic changes
    vtkSlicerSliceLogic *n = vtkSlicerSliceLogic::SafeDownCast(caller);

    // find out whether this node belongs to any of the slice widgets
    int num = this->SliceWidgets->GetNumberOfItems ( );
    int i = 0;
    while ( i < num ) {
        if ( n == this->SliceWidgets->GetItemAsObject ( i ) ) { break; }
        i++;
    }
    if ( i == num ) {
        vtkErrorMacro(<<"ProcessLogicEvents: Unable to find SliceWidget associated with SliceLogic event: ");
        return;
    }
    
    // UPDATE THE GUI.
    vtkSlicerSliceWidget *sw = vtkSlicerSliceWidget::SafeDownCast(this->SliceWidgets->GetItemAsObject ( i )) ;    
    if ( event == vtkCommand::ModifiedEvent ) {

        // UPDATE THE FOV ENTRY
        double fov = sw->GetSliceLogic()->GetSliceNode()->GetFieldOfView()[0];
        char fovstring[80];
        sprintf (fovstring, "%g", fov);
        sw->GetFieldOfViewEntry()->GetWidget()->SetValue(fovstring);
        sw->GetFieldOfViewEntry()->Modified();

        // UPDATE THE SCALE
        double fovover2 = sw->GetSliceLogic()->GetSliceNode()->GetFieldOfView()[2] / 2.;
        sw->GetOffsetScale()->SetRange ( -fovover2, fovover2 );
        // TODO: set the scale value from the translation part
        // of the matrix with rotation. 
        // Set the scale from the Offset in the matrix.
        vtkMatrix4x4 *m = sw->GetSliceLogic()->GetSliceNode()->GetSliceToRAS();
        sw->GetOffsetScale()->SetValue ( m->GetElement(2,3) );
        sw->GetOffsetScale()->Modified();
        
        // UPDATE IMAGE VIEWER
        vtkImageViewer2 *iv = sw->GetImageViewer ();
        vtkKWRenderWidget *rw = sw->GetRenderWidget ();
        if ( sw->GetSliceLogic()->GetImageData() != NULL )
            {
                iv->SetInput ( sw->GetSliceLogic()->GetImageData( ) );
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
        sw->GetImageViewer()->Modified ();

    }

}




// get slicewidget 0, 1, 2
//---------------------------------------------------------------------------
vtkSlicerSliceWidget* vtkSlicerSliceGUI::GetSliceWidget ( int SliceWidgetNum ) {

    return ( (vtkSlicerSliceWidget *) (this->SliceWidgets->GetItemAsObject( SliceWidgetNum ) ) );
}



// get slicewidget red, yellow, green
//---------------------------------------------------------------------------
vtkSlicerSliceWidget* vtkSlicerSliceGUI::GetSliceWidget ( char *SliceWidgetColor ) {
    if ( SliceWidgetColor == "r" || SliceWidgetColor == "R" )
        {
            return ( (vtkSlicerSliceWidget *) (this->SliceWidgets->GetItemAsObject( 0 )));
        } else if ( SliceWidgetColor == "g" || SliceWidgetColor == "G")
            {
                return ( (vtkSlicerSliceWidget *) (this->SliceWidgets->GetItemAsObject( 1 )));
            } else if ( SliceWidgetColor == "y" || SliceWidgetColor == "Y" )
                {
                    return ( (vtkSlicerSliceWidget *) (this->SliceWidgets->GetItemAsObject( 2 )));
                } else
                    {
                        return NULL;
                    }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::BuildGUI ( ) {
}

    




//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::BuildGUI ( vtkKWFrame* f1, vtkKWFrame *f2, vtkKWFrame *f3  ) {

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    

    this->MainSlice0 = vtkSlicerSliceWidget::New ( );
    this->MainSlice0->SetApplication (app);
    this->MainSlice0->SetParent ( f1 );
    this->MainSlice0->Create ( );
    this->AddSliceWidget ( this->MainSlice0 );
    app->Script ( "pack %s -side top -fill both -expand y -padx 0 -pady 0", this->MainSlice0->GetWidgetName () );

    // create these others for now, but they are not observed.
    this->MainSlice1 = vtkSlicerSliceWidget::New ( );
    this->MainSlice1->SetApplication (app);
    this->MainSlice1->SetParent ( f2 );
    this->MainSlice1->Create ( );
    this->AddSliceWidget ( this->MainSlice1 );
    app->Script ( "pack %s -side top -fill both -expand y -padx 0 -pady 0", this->MainSlice1->GetWidgetName () );

    this->MainSlice2 = vtkSlicerSliceWidget::New ( );
    this->MainSlice2->SetApplication (app);
    this->MainSlice2->SetParent ( f3 );
    this->MainSlice2->Create ( );
    this->AddSliceWidget ( this->MainSlice2 );
    app->Script ( "pack %s -side top -fill both -expand y -padx 0 -pady 0", this->MainSlice2->GetWidgetName () );

}





//----------------------------------------------------------------------------
void vtkSlicerSliceGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "MainSlice0: " << this->MainSlice0 << endl;
  os << indent << "MainSlice1: " << this->MainSlice1 << endl;
  os << indent << "MainSlice2: " << this->MainSlice2 << endl;
  os << indent << "SliceLogic: " << this->SliceLogic << endl;  

}
