#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerCacheAndDataIOManagerGUI.h"


//---------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkSlicerCacheAndDataIOManagerGUI, "$Revision: 1.0 $");
vtkStandardNewMacro (vtkSlicerCacheAndDataIOManagerGUI );



//---------------------------------------------------------------------------
vtkSlicerCacheAndDataIOManagerGUI::vtkSlicerCacheAndDataIOManagerGUI ( )
{
  this->CacheFullLabel = NULL;
  this->CloseButton = NULL;
  this->ClearCacheButton = NULL;
  this->ForceReloadCheckButton = NULL;
  this->OverwriteCacheCheckButton = NULL;
  this->AsynchronousCheckButton = NULL;
  this->TimeOutCheckButton = NULL;
  this->ManagerTopLevel = NULL;
  this->ControlFrame = NULL;
  this->TransfersFrame = NULL;
  this->Built = false;
}

//---------------------------------------------------------------------------
vtkSlicerCacheAndDataIOManagerGUI::~vtkSlicerCacheAndDataIOManagerGUI ( )
{
  if ( this->CacheFullLabel )
    {
    this->CacheFullLabel->SetParent ( NULL );
    this->CacheFullLabel->Delete();
    this->CacheFullLabel = NULL;
    }
  if ( this->CloseButton )
    {
    this->CloseButton->SetParent ( NULL );
    this->CloseButton->Delete();
    this->CloseButton = NULL;
    }
  if ( this->ClearCacheButton )
    {
    this->ClearCacheButton->SetParent ( NULL );
    this->ClearCacheButton->Delete();
    this->ClearCacheButton = NULL;
    }
  if ( this->ForceReloadCheckButton )
    {
    this->ForceReloadCheckButton->SetParent ( NULL );
    this->ForceReloadCheckButton->Delete();
    this->ForceReloadCheckButton = NULL;    
    }
  if ( this->OverwriteCacheCheckButton )
    {
    this->OverwriteCacheCheckButton->SetParent ( NULL);
    this->OverwriteCacheCheckButton->Delete();
    this->OverwriteCacheCheckButton = NULL;    
    }
  if ( this->AsynchronousCheckButton )
    {
    this->AsynchronousCheckButton->SetParent ( NULL );
    this->AsynchronousCheckButton->Delete();
    this->AsynchronousCheckButton = NULL;    
    }
  if ( this->TimeOutCheckButton )
    {
    this->TimeOutCheckButton->SetParent (NULL );
    this->TimeOutCheckButton->Delete();
    this->TimeOutCheckButton = NULL;    
    }
  if ( this->TransfersFrame )
    {
    this->TransfersFrame->SetParent ( NULL );
    this->TransfersFrame->Delete();
    this->TransfersFrame = NULL;    
    }
  if ( this->ControlFrame )
    {
    this->ControlFrame->SetParent ( NULL );
    this->ControlFrame->Delete();
    this->ControlFrame = NULL;    
    }
  if ( this->ManagerTopLevel )
    {
    this->ManagerTopLevel->SetParent ( NULL );
    this->ManagerTopLevel->Delete();
    this->ManagerTopLevel = NULL;    
    }
  this->Built = false;
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "SlicerCacheAndDataIOManagerGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "CacheFullLabel: " << this->GetCacheFullLabel ( ) << "\n";
  os << indent << "CloseButton: " << this->GetCloseButton () << "\n";
  os << indent << "ClearCacheButton: " << this->GetClearCacheButton () << "\n";
  os << indent << "ForceReloadCheckButton: " << this->GetForceReloadCheckButton () << "\n";
  os << indent << "OverwriteCacheCheckButton: " << this->GetOverwriteCacheCheckButton () << "\n";
  os << indent << "AsynchronousCheckButton: " << this->GetAsynchronousCheckButton () << "\n";
  os << indent << "TimeOutCheckButton: " << this->GetTimeOutCheckButton () << "\n";
  os << indent << "ControlFrame: " << this->GetControlFrame () << "\n";
  os << indent << "TransfersFrame: " << this->GetTransfersFrame () << "\n";
  os << indent << "ManagerTopLevel: " << this->GetManagerTopLevel () << "\n";
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::RemoveGUIObservers ( )
{

  this->CloseButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ClearCacheButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ForceReloadCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->OverwriteCacheCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AsynchronousCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->TimeOutCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::AddGUIObservers ( )
{
  this->CloseButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ClearCacheButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ForceReloadCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->OverwriteCacheCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AsynchronousCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->TimeOutCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );

}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::ProcessGUIEvents ( vtkObject *caller,
                                        unsigned long event, void *callData )
  {
    vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
    vtkKWCheckButton *c = vtkKWCheckButton::SafeDownCast ( caller );

    if ( b == this->CloseButton && event == vtkKWPushButton::InvokedEvent )
      {
      this->WithdrawManagerWindow();
      }
    else if ( b == this->ClearCacheButton && event == vtkKWPushButton::InvokedEvent )
      {
      this->GetCacheManager()->ClearCache();
      }
    
    if ( c == this->ForceReloadCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      }

    else if ( c == this->OverwriteCacheCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      }
    else if ( c == this->AsynchronousCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      }
    else if ( c == this->TimeOutCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent )
      {
      }

}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::ProcessLogicEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
  // If any of the logic state changes, update GUI to show.
  // Force reload?
  // Asynchronous?
  // Version in cache?
  // Timeout changes?
  // Cache size overrunning buffer?
  
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::ProcessMRMLEvents ( vtkObject *caller,
                                         unsigned long event, void *callData )
{
vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Done processing mrml events...");
}    





//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::SetDataIOManager( vtkDataIOManager *iomanager )
{
  vtkDataIOManager *oldValue = this->DataIOManager;
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &this->DataIOManager), iomanager );
  if ( oldValue != this->DataIOManager )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}



//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::SetAndObserveDataIOManager( vtkDataIOManager *iomanager )
{
  vtkDataIOManager *oldManager = this->DataIOManager;
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->DataIOManager), iomanager );
  if ( oldManager != this->DataIOManager )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}



//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::SetCacheManager( vtkCacheManager *manager )
{
  vtkCacheManager *oldManager = this->CacheManager;
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &this->CacheManager), manager );
  if ( oldManager != this->CacheManager )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}



//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::SetAndObserveCacheManager( vtkCacheManager *manager )
{
  vtkCacheManager *oldManager = this->CacheManager;
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->CacheManager), manager );
  if ( oldManager != this->CacheManager )
    {
    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}



//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::UpdateGUI()
{
// Fill in

}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::CreateModuleEventBindings ( )
{
// Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::ReleaseModuleEventBindings ( )
{
// Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::Enter ( )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();
  this->UpdateGUI();
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::TearDownGUI ( )
{
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::BuildGUI ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWTopLevel *master = app->GetNthWindow(0);

  this->ManagerTopLevel = vtkKWTopLevel::New();

  if (master)
    {
    vtksys_stl::string title;
    if (master->GetTitle())
      {
      title += master->GetTitle();
      title += " : ";
      }
    title += "Asynchronous I/O Manager Window";
    this->ManagerTopLevel->SetTitle(title.c_str());
    this->ManagerTopLevel->SetMasterWindow(master);
    this->ManagerTopLevel->SetApplication ( app );
    this->ManagerTopLevel->Create();
    this->ManagerTopLevel->SetBorderWidth ( 2 );
    this->ManagerTopLevel->SetReliefToFlat();
    this->ManagerTopLevel->SetDisplayPositionToPointer();
    this->ManagerTopLevel->SetSize ( 400, 300 );
    this->ManagerTopLevel->Withdraw();
    this->ManagerTopLevel->SetDeleteWindowProtocolCommand ( this, "WithdrawManagerWindow" );
    

    this->ControlFrame = vtkKWFrame::New();
    this->ControlFrame->SetParent ( this->ManagerTopLevel );
    this->ControlFrame->Create();
    this->TransfersFrame = vtkKWFrameWithScrollbar::New();
    this->TransfersFrame->SetParent ( this->ManagerTopLevel );
    this->TransfersFrame->Create();
    this->Script ( "pack %s -side top -anchor nw -padx 0 -pady 0", this->ControlFrame->GetWidgetName() );
    this->Script ( "pack %s -side top -anchor nw -padx 0 -pady 0", this->TransfersFrame->GetWidgetName() );  

    // all widgets in the Control Frame
    this->CacheFullLabel = vtkKWLabel::New();
    this->CacheFullLabel->SetParent ( this->ControlFrame );
    this->CacheFullLabel->Create();

    this->CloseButton = vtkKWPushButton::New();
    this->CloseButton->SetParent ( this->ControlFrame );
    this->CloseButton->Create();
    this->CloseButton->SetText ( "Close");

    this->ClearCacheButton = vtkKWPushButton::New();
    this->ClearCacheButton->SetParent ( this->ControlFrame );
    this->ClearCacheButton->Create();

    this->ForceReloadCheckButton = vtkKWCheckButton::New();
    this->ForceReloadCheckButton->SetParent ( this->ControlFrame );
    this->ForceReloadCheckButton->Create();
    this->ForceReloadCheckButton->SetSelectedState(0);
    
    this->OverwriteCacheCheckButton = vtkKWCheckButton::New();
    this->OverwriteCacheCheckButton->SetParent ( this->ControlFrame );
    this->OverwriteCacheCheckButton->Create();
    this->OverwriteCacheCheckButton->SetSelectedState(1);

    this->AsynchronousCheckButton = vtkKWCheckButton::New();
    this->AsynchronousCheckButton->SetParent ( this->ControlFrame );
    this->AsynchronousCheckButton->Create();
    this->AsynchronousCheckButton->SetSelectedState(1);

    this->TimeOutCheckButton = vtkKWCheckButton::New();
    this->TimeOutCheckButton->SetParent ( this->ControlFrame );
    this->TimeOutCheckButton->Create();
    this->TimeOutCheckButton->SetSelectedState(0);

    // data from the transfer queue are exposed using
    // DataTransferWidgets in the TransfersFrame
    }  
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::DisplayManagerWindow ( )
{

  if (! this->Built )
    {
    return;
    }
  this->ManagerTopLevel->DeIconify();
  this->ManagerTopLevel->Raise();
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::WithdrawManagerWindow ( )
{
  if ( ! this->Built )
    {
    return;
    }
  this->ManagerTopLevel->Withdraw();
}

