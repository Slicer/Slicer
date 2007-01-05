#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerSlicesGUI.h"
#include "vtkSlicerSliceGUICollection.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSlicesGUI);
vtkCxxRevisionMacro(vtkSlicerSlicesGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSlicesGUI::vtkSlicerSlicesGUI (  )
{

    this->SliceGUICollection = vtkSlicerSliceGUICollection::New ( );
}


//---------------------------------------------------------------------------
vtkSlicerSlicesGUI::~vtkSlicerSlicesGUI ( )
{

    vtkSlicerSliceGUI *s, *nexts;

    
    // Remove observers, delete individual SliceGUIs and their collection
    if ( this->SliceGUICollection ) {
        this->SliceGUICollection->InitTraversal ( );
        s = vtkSlicerSliceGUI::SafeDownCast ( this->SliceGUICollection->GetNextItemAsObject ( ) );
        while ( s != NULL ) {
            nexts = vtkSlicerSliceGUI::SafeDownCast (this->SliceGUICollection->GetNextItemAsObject ( ) );
            this->SliceGUICollection->RemoveItem ( s );
            s->Delete ( );
            s = NULL;
            s = nexts;
        }
        this->SliceGUICollection->Delete ( );
        this->SliceGUICollection = NULL;
    }
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::AddSliceGUI ( vtkSlicerSliceGUI *s )
{

    // Create if it doesn't exist already
    if ( this->SliceGUICollection == NULL ) {
        this->SliceGUICollection = vtkSlicerSliceGUICollection::New();
    }
    //Add new sliceGUI
    this->SliceGUICollection->AddItem ( s );

}


//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::AddAndObserveSliceGUI ( vtkSlicerSliceGUI *s )
{

    this->AddSliceGUI ( s );
    s->AddGUIObservers ( );
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::RemoveSliceGUI ( vtkSlicerSliceGUI *s )
{

    // Remove observers, remove from collection and delete
    if ( this->SliceGUICollection && s != NULL )
        {
            this->SliceGUICollection->InitTraversal ( );
            vtkSlicerSliceGUI *g = vtkSlicerSliceGUI::SafeDownCast ( this->SliceGUICollection->GetNextItemAsObject ( ) );
            while ( g != NULL ) {
                if ( g == s )
                    {
                        g->RemoveGUIObservers ( );
                        this->SliceGUICollection->RemoveItem ( g );
                        g->Delete ( );
                        break;
                    }
                g = vtkSlicerSliceGUI::SafeDownCast (this->SliceGUICollection->GetNextItemAsObject ( ) );
            }
        }
}


//---------------------------------------------------------------------------
vtkSlicerSliceGUI* vtkSlicerSlicesGUI::GetSliceGUI ( int SliceGUINum )
    {
    // get slicewidget 0, 1, 2
    return ( (vtkSlicerSliceGUI::SafeDownCast(this->SliceGUICollection->GetItemAsObject( SliceGUINum ) ) ) );
}





//---------------------------------------------------------------------------
vtkSlicerSliceGUI* vtkSlicerSlicesGUI::GetSliceGUI ( char *SliceGUIColor )
    {
    // get slicewidget red, yellow, green
    if ( SliceGUIColor == "r" || SliceGUIColor == "R" )
        {
            return ( vtkSlicerSliceGUI::SafeDownCast(this->SliceGUICollection->GetItemAsObject( 0 )));
        } else if ( SliceGUIColor == "g" || SliceGUIColor == "G")
            {
                return ( vtkSlicerSliceGUI::SafeDownCast(this->SliceGUICollection->GetItemAsObject( 1 )));
            } else if ( SliceGUIColor == "y" || SliceGUIColor == "Y" )
                {
                    return ( vtkSlicerSliceGUI::SafeDownCast(this->SliceGUICollection->GetItemAsObject( 2 )));
                } else {
                        return NULL;
                }
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::AddGUIObservers ( )
{
  // TODO: add observers for all widgets in slicesGUIs UIpanel.
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::RemoveGUIObservers ( )
{

  // TODO: remove observers on all widgets in slicesGUIs UIpanel.
      
  // Remove any observers on any SliceGUIs in the collection
  if ( this->SliceGUICollection )
    {
      this->SliceGUICollection->InitTraversal ( );
      vtkSlicerSliceGUI *g = vtkSlicerSliceGUI::SafeDownCast ( this->SliceGUICollection->GetNextItemAsObject ( ) );
      while ( g != NULL )
        {
          g->RemoveGUIObservers ( );
          g = vtkSlicerSliceGUI::SafeDownCast (this->SliceGUICollection->GetNextItemAsObject ( ) );
        }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event, void *callData)
    {
        // Fill in
    }


//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{
    // Fill in
}
 

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}
 

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::BuildGUI (  )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    
    // ---
    // MODULE GUI FRAME 
    // configure a page for a volume loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Slices", "Slices", NULL );
    
    // Define your help text and build the help frame here.
    const char *help = "The Slices Module manages the display of the Slice Viewers.";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "Slices" );
    this->BuildHelpAndAboutFrame ( page, help, about );

    // ---
    // DISPLAY FRAME            
    vtkSlicerModuleCollapsibleFrame *sliceDisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    sliceDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Slices" ) );
    sliceDisplayFrame->Create ( );
    sliceDisplayFrame->SetLabelText ("Slice information");
    sliceDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  sliceDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Slices")->GetWidgetName());

    sliceDisplayFrame->Delete();

}


//----------------------------------------------------------------------------
void vtkSlicerSlicesGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "SlicerSlicesGUI:" << this->GetClassName ( ) << "\n";
  os << indent << "SliceGUICollection: " << this->SliceGUICollection << endl;
}

