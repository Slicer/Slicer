#include "vtkObject.h"
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

    this->SetApplicationLogic ( NULL );
    this->SetMRMLScene ( NULL );
    this->SliceWidgetCollection = vtkSlicerSliceWidgetCollection::New ( );
    this->SliceLogicCollection = vtkSlicerSliceLogicCollection::New ( );
    this->MainSlice0 = NULL;
    this->MainSlice1 = NULL;
    this->MainSlice2 = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerSliceGUI::~vtkSlicerSliceGUI ( ) {

    vtkSlicerSliceLogic *sl, *nextsl;
    vtkSlicerSliceWidget *sw, *nextsw;

    // Delete SliceLogics and their collection
    if ( this->SliceLogicCollection ) {
        this->SliceLogicCollection->InitTraversal ( );
        sl = vtkSlicerSliceLogic::SafeDownCast ( this->SliceLogicCollection->GetNextItemAsObject ( ) );
        while ( sl != NULL ) {
            nextsl = vtkSlicerSliceLogic::SafeDownCast (this->SliceLogicCollection->GetNextItemAsObject ( ) );
            this->SliceLogicCollection->RemoveItem ( sl );
            sl->Delete ( );
            sl = nextsl;
        }
        this->SliceLogicCollection->Delete ( );
        this->SliceLogicCollection = NULL;
    }

    // Delete SliceWidgets and their collection
    if ( this->SliceWidgetCollection ) {
        this->SliceWidgetCollection->InitTraversal ( );
        sw = vtkSlicerSliceWidget::SafeDownCast (this->SliceWidgetCollection->GetNextItemAsObject ( ));
        while ( sw != NULL ) {        
            nextsw = vtkSlicerSliceWidget::SafeDownCast (this->SliceWidgetCollection->GetNextItemAsObject ( ));
            this->SliceWidgetCollection->RemoveItem ( sw );
            sw->Delete ( );
            sw = nextsw;
        }
        this->SliceWidgetCollection->Delete();
        this->SliceWidgetCollection = NULL;
    }

    this->MainSlice0 = NULL;
    this->MainSlice1 = NULL;
    this->MainSlice2 = NULL;
}




//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddSliceWidget ( vtkSlicerSliceWidget *w ){

    // Create if it doesn't exist already
    if ( this->SliceWidgetCollection == NULL ) {
        this->SliceWidgetCollection = vtkSlicerSliceWidgetCollection::New();
    }
    //Add widget
    this->SliceWidgetCollection->AddItem ( w );
}




//---------------------------------------------------------------------------
vtkSlicerSliceWidget* vtkSlicerSliceGUI::GetSliceWidget ( int SliceWidgetNum ) {
    // get slicewidget 0, 1, 2
    return ( (vtkSlicerSliceWidget::SafeDownCast(this->SliceWidgetCollection->GetItemAsObject( SliceWidgetNum ) ) ) );
}





//---------------------------------------------------------------------------
vtkSlicerSliceWidget* vtkSlicerSliceGUI::GetSliceWidget ( char *SliceWidgetColor ) {
    // get slicewidget red, yellow, green
    if ( SliceWidgetColor == "r" || SliceWidgetColor == "R" )
        {
            return ( vtkSlicerSliceWidget::SafeDownCast(this->SliceWidgetCollection->GetItemAsObject( 0 )));
        } else if ( SliceWidgetColor == "g" || SliceWidgetColor == "G")
            {
                return ( vtkSlicerSliceWidget::SafeDownCast(this->SliceWidgetCollection->GetItemAsObject( 1 )));
            } else if ( SliceWidgetColor == "y" || SliceWidgetColor == "Y" )
                {
                    return ( vtkSlicerSliceWidget::SafeDownCast(this->SliceWidgetCollection->GetItemAsObject( 2 )));
                } else {
                        return NULL;
                }
}


                
//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::SetSliceLogic ( vtkSlicerSliceLogic *logic, int swNum ) {

    int numLogics = this->SliceLogicCollection->GetNumberOfItems ( );

    // Test to see if the request is out of bounds
    if ( swNum > numLogics ) {
        vtkErrorMacro ( << "There are currently " << numLogics << "logic pointers in the SliceGUI's collection; can't replace a pointer or add a new one at" << swNum );
        return;
    }

    vtkSlicerSliceLogic *sl =  this->GetSliceLogic ( swNum );
    // Don't bother if already set
    if ( logic == sl ) {
        return;
    }
    // Remove observers from application logic
    if ( sl != NULL ) {
        this->RemoveLogicObservers ( swNum );
    }
    // Add to the collection of SliceLogicCollection OR
    // replace an existing logic pointer for a slice widget
    if ( swNum == numLogics ) {
        this->AddSliceLogic ( logic );
    } else {
        this->SliceLogicCollection->ReplaceItem( swNum, logic );
    }

    // Add logic observers on new logic
    sl = this->GetSliceLogic ( swNum );
    if ( sl != NULL ) {
        this->AddLogicObservers ( swNum );
    }

}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddSliceLogic ( vtkSlicerSliceLogic *w ){

    // Create if it doesn't exist already
    if ( this->SliceLogicCollection == NULL ) {
        this->SliceLogicCollection = vtkSlicerSliceLogicCollection::New();
    }
    //Add widget
    this->SliceLogicCollection->AddItem ( w );
}



//---------------------------------------------------------------------------
vtkSlicerSliceLogic* vtkSlicerSliceGUI::GetSliceLogic ( int SliceLogicNum ) {

    // get slicelogic for widgets 0, 1, 2
    return ( vtkSlicerSliceLogic::SafeDownCast(this->SliceLogicCollection->GetItemAsObject( SliceLogicNum )));
    
}





//---------------------------------------------------------------------------
vtkSlicerSliceLogic* vtkSlicerSliceGUI::GetSliceLogic ( char *SliceWidgetColor ) {

    // get slicewidget red, yellow, green
    if ( SliceWidgetColor == "r" || SliceWidgetColor == "R" )
        {
            return ( vtkSlicerSliceLogic::SafeDownCast(this->SliceLogicCollection->GetItemAsObject( 0 )));
        } else if ( SliceWidgetColor == "g" || SliceWidgetColor == "G")
            {
                return ( vtkSlicerSliceLogic::SafeDownCast(this->SliceLogicCollection->GetItemAsObject( 1 )));
            } else if ( SliceWidgetColor == "y" || SliceWidgetColor == "Y" )
                {
                    return ( vtkSlicerSliceLogic::SafeDownCast(this->SliceLogicCollection->GetItemAsObject( 2 )));
                } else
                    {
                        return NULL;
                    }
}




//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddGUIObservers ( ) {

    // Add observers on gui components for all slice widgets
    int numWidgets = this->SliceWidgetCollection->GetNumberOfItems ( );

    vtkSlicerSliceWidget *sw;
    vtkKWScaleWithEntry *s;
    vtkKWEntryWithLabel *e;
    vtkKWMenuButtonWithLabel *m;
    for ( int i = 0; i < numWidgets; i++) {
        sw = this->GetSliceWidget (i);
        s = sw->GetOffsetScale () ;
        e = sw->GetFieldOfViewEntry ();
        m = sw->GetOrientationMenu ();

        s->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
        s->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICommand );
        e->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
        m->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
    }

}


//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveGUIObservers ( ) {

    // Add observers on gui components for all slice widgets
    int numWidgets = this->SliceWidgetCollection->GetNumberOfItems ( );

    vtkSlicerSliceWidget *sw;
    vtkKWScaleWithEntry *s;
    vtkKWEntryWithLabel *e;
    vtkKWMenuButtonWithLabel *m;
    for ( int i = 0; i < numWidgets; i++) {
        sw = this->GetSliceWidget (i);
        s = sw->GetOffsetScale () ;
        e = sw->GetFieldOfViewEntry ();
        m = sw->GetOrientationMenu ();

        s->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
        s->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICommand );
        e->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
        m->RemoveObservers (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
    }

}




//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddLogicObserver ( vtkSlicerSliceLogic* logic, int event ) {
    logic->AddObserver ( event, (vtkCommand *)this->LogicCommand);
}
//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveLogicObserver ( vtkSlicerSliceLogic* logic, int event ) {
    logic->RemoveObservers ( event, (vtkCommand *)this->LogicCommand);
}


//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddLogicObservers ( int logicnum ) {
    
    // Add observers on the widget's SliceLogic pointer.
    // Before a widget's SliceLogic is set, no observers are created.
    //
     int numLogics = this->SliceLogicCollection->GetNumberOfItems ( );

     if ( logicnum >= numLogics ) {
         vtkErrorMacro ( << "There are only" << numLogics << "widgets. No logic pointers are assigned.");
         return;
     }
     if ( this->GetSliceLogic ( logicnum ) != NULL ) {
         this->AddLogicObserver ( this->GetSliceLogic ( logicnum ), vtkCommand::DeleteEvent );
         this->AddLogicObserver ( this->GetSliceLogic ( logicnum ), vtkCommand::ModifiedEvent );
     }
}


//---------------------------------------------------------------------------
 void vtkSlicerSliceGUI::RemoveLogicObservers ( int logicnum ) {
    // Add removes observers on the widget's SliceLogic pointer.
    // Before a widget's SliceLogic is set, no observers are created.
    //
     int numLogics = this->SliceLogicCollection->GetNumberOfItems ( );

     if ( logicnum >= numLogics ) {
         vtkErrorMacro ( << "There are only" << numLogics << "slice widgets. No observers on logic pointers have been created.");
         return;
     }
     if ( this->GetSliceLogic ( logicnum ) != NULL ) {
         this->RemoveLogicObserver ( this->GetSliceLogic ( logicnum ), vtkCommand::DeleteEvent );
         this->RemoveLogicObserver ( this->GetSliceLogic ( logicnum ), vtkCommand::ModifiedEvent );
     }
 }



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddLogicObservers ( ) {

    // Add observers on SliceLogic for all SliceWidgets:
     int numWidgets = this->SliceWidgetCollection->GetNumberOfItems ( );
     int numLogics = this->SliceLogicCollection->GetNumberOfItems ( );
     if ( numWidgets != numLogics ) {
         vtkErrorMacro ( << "Unequal number of SliceWidgets and SliceLogicPointers assigned. No observers on SliceLogic were added.");
         return;
     }

     for ( int i = 0; i < numWidgets; i++) {
         if ( this->GetSliceLogic ( i ) != NULL ) {
             this->AddLogicObserver ( this->GetSliceLogic ( i ), vtkCommand::DeleteEvent );
             this->AddLogicObserver ( this->GetSliceLogic ( i ), vtkCommand::ModifiedEvent );
         }
     }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveLogicObservers ( ) {

    // Delete observers on SliceLogic for all SliceWidgets:
     int numWidgets = this->SliceWidgetCollection->GetNumberOfItems ( );
     int numLogics = this->SliceLogicCollection->GetNumberOfItems ( );
     if ( numWidgets != numLogics ) {
         vtkErrorMacro ( << "Unequal number of SliceWidgets and SliceLogicPointers assigned. No observers on SliceLogic were removed.");
         return;
     }

     for ( int i = 0; i < numWidgets; i++) {
         if ( this->GetSliceLogic ( i ) != NULL ) {
             this->RemoveLogicObserver ( this->GetSliceLogic ( i ), vtkCommand::DeleteEvent );
             this->RemoveLogicObserver ( this->GetSliceLogic ( i ), vtkCommand::ModifiedEvent );
         }
     }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddMRMLObserver ( vtkMRMLNode *node, int event ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveMRMLObserver ( vtkMRMLNode *node, int event ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddMRMLObservers ( ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveMRMLObservers ( ) {
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessGUIEvents ( vtkObject *caller,
                                                   unsigned long event,
                                           void *callData ) {

    vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast(caller);
    vtkKWMenuButtonWithLabel *o = vtkKWMenuButtonWithLabel::SafeDownCast (caller );
    vtkMRMLScene *mrml = this->ApplicationLogic->GetMRMLScene();

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
    int numWidgets = this->SliceWidgetCollection->GetNumberOfItems ( );
    int numLogics = this->SliceLogicCollection->GetNumberOfItems ( );
    if ( numWidgets != numLogics ) {
        vtkErrorMacro ( << "Unequal number of SliceWidgets and SliceLogicPointers assigned. Cannot process logic event.");
        return;
    }

    int i = 0;
    while ( i < numLogics ) {
        if ( n == this->SliceLogicCollection->GetItemAsObject ( i ) ) { break; }
        i++;
    }
    if ( i == numLogics ) {
        vtkErrorMacro(<<"ProcessLogicEvents: Unable to find SliceLogic that generated event: ");
        return;
    }
    
    // UPDATE THE i^th widget whose SliceLogic generated the event.
    vtkSlicerSliceWidget *sw = this->GetSliceWidget ( i );
    // Get the MRML slicenode thru the slice logic.
    n = this->GetSliceLogic ( i );
    if ( event == vtkCommand::ModifiedEvent ) {

        // UPDATE THE FOV ENTRY
        double fov = n->GetSliceNode()->GetFieldOfView()[0];
        char fovstring[80];
        sprintf (fovstring, "%g", fov);
        sw->GetFieldOfViewEntry()->GetWidget()->SetValue(fovstring);
        sw->GetFieldOfViewEntry()->Modified();

        // UPDATE THE SCALE
        double fovover2 = n->GetSliceNode()->GetFieldOfView()[2] / 2.;
        sw->GetOffsetScale()->SetRange ( -fovover2, fovover2 );
        // TODO: set the scale value from the translation part
        // of the matrix with rotation. 
        // Set the scale from the Offset in the matrix.
        vtkMatrix4x4 *m = n->GetSliceNode()->GetSliceToRAS();
        sw->GetOffsetScale()->SetValue ( m->GetElement(2,3) );
        sw->GetOffsetScale()->Modified();
        
        // UPDATE IMAGE VIEWER
        vtkImageViewer2 *iv = sw->GetImageViewer ();
        vtkKWRenderWidget *rw = sw->GetRenderWidget ();
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
        sw->GetImageViewer()->Modified ();

    }

}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) {
    // Fill in
}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::Enter ( ) {
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::Exit ( ) {
    // Fill in
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
  os << indent << "SliceWidgetCollection: " << this->SliceWidgetCollection << endl;
  os << indent << "SliceLogicCollection: " << this->SliceLogicCollection << endl;
}
