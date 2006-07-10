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

    
    this->RemoveGUIObservers ();

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

    // Remove observers and references 
    this->SetModuleLogic ( NULL );
    this->SetMRMLNode ( NULL );

    // give the slice viewer code a chance to free any vtk objects
    // it allocated.  This can be called with no impact on other 
    // slice gui instances, since the tcl code automatically re-initializes
    // if the event handler is called again.
    this->Script("SliceViewerShutdown");
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

  this->SliceViewer->GetRenderWidget()->GetRenderWindowInteractor()->AddObserver (
    vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand );

}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveGUIObservers ( ) {

    this->SliceViewer->GetRenderWidget()->GetRenderWindow()->GetInteractor()->RemoveObservers (
        vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand );
}




//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessGUIEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  vtkRenderWindowInteractor *rwi = vtkRenderWindowInteractor::SafeDownCast (caller);

  vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();

  if (mrml == NULL ) 
    {
    return;
    }

  if ( rwi == this->SliceViewer->GetRenderWidget()->GetRenderWindowInteractor() )
    {
    this->Script("SliceViewerHandleEvent %s %s", 
      this->GetTclName(), vtkCommand::GetStringFromEventId(event));
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

    // TODO: can this be done directly in C++?
    // and - how do we know when VTK events are idle?
    //rw->Render();
    this->Script("after idle \"%s Render\"", rw->GetTclName());
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

//-------------------------------------------------------5--------------------
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

    this->PackGUI();

}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::PackGUI ()
{
    this->Script("pack %s -pady 0 -side top -expand false -fill x", SliceController->GetWidgetName() );
    this->Script("pack %s -anchor c -side top -expand true -fill both", SliceViewer->GetRenderWidget()->GetWidgetName());
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::UnpackGUI ()
{
    this->Script("pack forget %s", SliceController->GetWidgetName() );
    this->Script("pack forget %s", SliceViewer->GetRenderWidget()->GetWidgetName());
}




