#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkCornerAnnotation.h"
#include "vtkImageViewer.h"
#include "vtkRenderWindow.h"
#include "vtkImageActor.h"
#include "vtkInteractorStyleUser.h"

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
    this->SliceGUIFrame = vtkKWFrame::New ( );
    this->Logic = NULL;
    this->SliceNode = NULL;
    this->CurrentGUIEvent = NULL;
    this->GrabID = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerSliceGUI::~vtkSlicerSliceGUI ( ) {

    
    this->RemoveGUIObservers ();

    // Unpack and set parents to be NULL
    this->SliceController->SetParent ( NULL );
    this->SliceViewer->SetParent ( NULL );

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

    if ( this->SliceGUIFrame )
      {
        this->SliceGUIFrame->Delete ( );
        this->SliceGUIFrame = NULL;
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
    os << indent << "SliceGUIFrame: " << this->GetSliceGUIFrame ( ) << "\n";
    os << indent << "SliceViewer: " << this->GetSliceViewer ( ) << "\n";
    os << indent << "SliceController: " << this->GetSliceController ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";
    os << indent << "SliceNode: " << this->GetSliceNode ( ) << "\n";
}





//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::AddGUIObservers ( ) {

#if 0
  // add a higher than average observer to capture events
  this->SliceViewer->GetRenderWidget()->GetRenderWindowInteractor()->AddObserver (
    vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand, 1.0 );

#endif


  this->RemoveGUIObservers();

  // make a user interactor style to process our events
  // look at the InteractorStyle to get our events
  vtkRenderWindowInteractor *rwi = this->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkInteractorStyleUser *iStyleUser = vtkInteractorStyleUser::New();
    rwi->SetInteractorStyle (iStyleUser);
    iStyleUser->AddObserver ( vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand );
    iStyleUser->Delete();
    }
}



//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::RemoveGUIObservers ( ) {

#if 0
  this->SliceViewer->GetRenderWidget()->GetRenderWindow()->GetInteractor()->RemoveObservers (
       vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand );
#endif
   
  vtkRenderWindowInteractor *rwi = this->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkInteractorStyle *istyle = vtkInteractorStyle::SafeDownCast(rwi->GetInteractorStyle());
    if (istyle)
      {
      istyle->RemoveObservers ( vtkCommand::AnyEvent, (vtkCommand *)this->GUICallbackCommand );
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::SetGUICommandAbortFlag ( int flag )
{
  this->GetGUICallbackCommand()->SetAbortFlag(flag);
}


//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::ProcessGUIEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
  vtkKWGenericRenderWindowInteractor *rwi = vtkKWGenericRenderWindowInteractor::SafeDownCast (caller);
  vtkInteractorStyleUser *iStyleUser = vtkInteractorStyleUser::SafeDownCast (caller);

  vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();

  if (mrml == NULL ) 
    {
    return;
    }

#if 0
  // TODO - this is actually old
  if ( rwi == this->SliceViewer->GetRenderWidget()->GetRenderWindowInteractor() )
    {
    this->Script("SliceViewerHandleEvent %s %s", 
      this->GetTclName(), vtkCommand::GetStringFromEventId(event));
    }
#endif

  // handle events from the Interactor Style
  if (iStyleUser == 
      this->GetSliceViewer()->GetRenderWidget()->GetRenderWindowInteractor()->GetInteractorStyle())
    {
    this->SetCurrentGUIEvent( vtkCommand::GetStringFromEventId(event) );
    this->InvokeEvent (event, NULL);
    if ( !this->GUICallbackCommand->GetAbortFlag() )
      {
      this->Script( "SliceViewerHandleEvent %s %s", 
        this->GetTclName(), vtkCommand::GetStringFromEventId(event) );
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
    sliceViewer->RequestRender ( );
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

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication ( ) );
      vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();

      this->SliceGUIFrame->SetApplication ( app );
      this->SliceGUIFrame->SetParent ( f );
      this->SliceGUIFrame->Create ( );

      this->SliceController->SetApplication ( app );
      this->SliceController->SetAndObserveMRMLScene ( mrml );
      this->SliceController->SetParent ( this->SliceGUIFrame );
      this->SliceController->Create (  );

      this->SliceViewer->SetApplication ( app );
      this->SliceViewer->SetParent ( this->SliceGUIFrame );
      this->SliceViewer->Create (  );

      this->PackGUI();
    }
}

//-------------------------------------------------------5--------------------
void vtkSlicerSliceGUI::BuildGUI ( vtkKWFrame *f, double *c )
{

  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication ( ) );
      vtkMRMLScene *mrml = this->GetApplicationLogic()->GetMRMLScene();

      this->SliceGUIFrame->SetApplication ( app );
      this->SliceGUIFrame->SetParent ( f );
      this->SliceGUIFrame->Create ( );

      this->SliceController->SetApplication ( app );
      this->SliceController->SetAndObserveMRMLScene ( mrml );
      this->SliceController->SetParent ( this->SliceGUIFrame );
      this->SliceController->Create (  );
      this->SliceController->ApplyColorCode ( c );

      this->SliceViewer->SetApplication ( app );
      this->SliceViewer->SetParent ( this->SliceGUIFrame );
      this->SliceViewer->Create (  );

      this->PackGUI();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::PackGUI ()
{
    this->Script("pack %s -pady 0 -side left -expand y -fill both -padx 0 -pady 0", SliceGUIFrame->GetWidgetName() );
    this->Script("pack %s -pady 0 -side top -expand false -fill x", SliceController->GetWidgetName() );
    this->Script("pack %s -anchor c -side top -expand true -fill both", SliceViewer->GetRenderWidget()->GetWidgetName());
}

//---------------------------------------------------------------------------
void vtkSlicerSliceGUI::UnpackGUI ()
{
    this->Script("pack forget %s", SliceGUIFrame->GetWidgetName() );
    this->Script("pack forget %s", SliceController->GetWidgetName() );
    this->Script("pack forget %s", SliceViewer->GetRenderWidget()->GetWidgetName());
}




