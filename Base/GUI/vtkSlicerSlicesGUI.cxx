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
#include "vtkKWFrameWithLabel.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSlicesGUI);
vtkCxxRevisionMacro(vtkSlicerSlicesGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerSlicesGUI::vtkSlicerSlicesGUI (  )
{

    this->SliceGUICollection = vtkSlicerSliceGUICollection::New ( );
    this->MainSliceGUI0 = vtkSlicerSliceGUI::New ( );
    this->MainSliceGUI1 = vtkSlicerSliceGUI::New ( );
    this->MainSliceGUI2 = vtkSlicerSliceGUI::New ( );
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

    // Add observers on gui components for all slice widgets
    if ( this->SliceGUICollection )
        {
            this->SliceGUICollection->InitTraversal ( );
            vtkSlicerSliceGUI *g = vtkSlicerSliceGUI::SafeDownCast ( this->SliceGUICollection->GetNextItemAsObject ( ) );
            while ( g != NULL ) {
                {
                    g->AddGUIObservers ( );
                    g = vtkSlicerSliceGUI::SafeDownCast (this->SliceGUICollection->GetNextItemAsObject ( ) );
                }
            }
        }
    }


//---------------------------------------------------------------------------
void vtkSlicerSlicesGUI::RemoveGUIObservers ( )
    {

    // Remove observers
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
void vtkSlicerSlicesGUI::BuildGUI ( vtkKWFrame* f1, vtkKWFrame *f2, vtkKWFrame *f3  )
{

    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // ---
    // MODULE GUI FRAME 
    // configure a page for a volume loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "Slices", "Slices", NULL );
    
    // HELP FRAME
    vtkKWFrameWithLabel *sliceHelpFrame = vtkKWFrameWithLabel::New ( );
    sliceHelpFrame->SetParent ( this->UIPanel->GetPageWidget ( "Slices" ) );
    sliceHelpFrame->Create ( );
    sliceHelpFrame->CollapseFrame ( );
    sliceHelpFrame->SetLabelText ("Help");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  sliceHelpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Slices")->GetWidgetName());

    // ---
    // DISPLAY FRAME            
    vtkKWFrameWithLabel *sliceDisplayFrame = vtkKWFrameWithLabel::New ( );
    sliceDisplayFrame->SetParent ( this->UIPanel->GetPageWidget ( "Slices" ) );
    sliceDisplayFrame->Create ( );
    sliceDisplayFrame->SetLabelText ("Slice information");
    sliceDisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  sliceDisplayFrame->GetWidgetName(), this->UIPanel->GetPageWidget("Slices")->GetWidgetName());

    sliceHelpFrame->Delete();
    sliceDisplayFrame->Delete();

    // CREATE 3 Default SLICE GUIs
    this->MainSliceGUI0->SetApplication ( app );
    this->MainSliceGUI1->SetApplication ( app );
    this->MainSliceGUI2->SetApplication ( app );
    this->MainSliceGUI0->SetAndObserveMRMLScene ( this->MRMLScene );
    this->MainSliceGUI1->SetAndObserveMRMLScene ( this->MRMLScene );
    this->MainSliceGUI2->SetAndObserveMRMLScene ( this->MRMLScene );
    this->MainSliceGUI0->SetApplicationLogic ( this->ApplicationLogic );
    this->MainSliceGUI1->SetApplicationLogic ( this->ApplicationLogic );
    this->MainSliceGUI2->SetApplicationLogic ( this->ApplicationLogic );

    this->MainSliceGUI0->BuildGUI ( f1 );
    this->MainSliceGUI1->BuildGUI ( f2 );
    this->MainSliceGUI2->BuildGUI ( f3 );

    this->AddSliceGUI ( MainSliceGUI0 );
    this->AddSliceGUI ( MainSliceGUI1 );
    this->AddSliceGUI ( MainSliceGUI2 );
}



//----------------------------------------------------------------------------
void vtkSlicerSlicesGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "SlicerSlicesGUI:" << this->GetClassName ( ) << "\n";
  os << indent << "MainSliceGUI0: " << this->MainSliceGUI0 << endl;
  os << indent << "MainSliceGUI1: " << this->MainSliceGUI1 << endl;
  os << indent << "MainSliceGUI2: " << this->MainSliceGUI2 << endl;
  os << indent << "SliceGUICollection: " << this->SliceGUICollection << endl;
}

