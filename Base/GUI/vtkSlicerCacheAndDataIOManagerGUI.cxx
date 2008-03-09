#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerCacheAndDataIOManagerGUI.h"
#include "vtkSlicerDataTransferWidget.h"

//---------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkSlicerCacheAndDataIOManagerGUI, "$Revision: 1.0 $");
vtkStandardNewMacro (vtkSlicerCacheAndDataIOManagerGUI );



//---------------------------------------------------------------------------
vtkSlicerCacheAndDataIOManagerGUI::vtkSlicerCacheAndDataIOManagerGUI ( )
{
  this->CacheSizeLabel = NULL;
  this->CacheFreeLabel = NULL;
  this->CloseButton = NULL;
  this->ClearCacheButton = NULL;
  this->ForceReloadCheckButton = NULL;
  this->OverwriteCacheCheckButton = NULL;
  this->AsynchronousCheckButton = NULL;
  this->TimeOutCheckButton = NULL;
  this->ManagerTopLevel = NULL;
  this->ControlFrame = NULL;
  this->ButtonFrame = NULL;
  this->TransfersFrame = NULL;
  this->Built = false;
  this->TransferWidgetCollection = NULL;
}

//---------------------------------------------------------------------------
vtkSlicerCacheAndDataIOManagerGUI::~vtkSlicerCacheAndDataIOManagerGUI ( )
{
  if ( this->TransferWidgetCollection )
    {
    this->TransferWidgetCollection->RemoveAllItems();
    this->TransferWidgetCollection->Delete();
    }
  if ( this->CacheSizeLabel )
    {
    this->CacheSizeLabel->SetParent ( NULL );
    this->CacheSizeLabel->Delete();
    this->CacheSizeLabel = NULL;
    }
  if ( this->CacheFreeLabel )
    {
    this->CacheFreeLabel->SetParent ( NULL );
    this->CacheFreeLabel->Delete();
    this->CacheFreeLabel = NULL;
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
  if ( this->ButtonFrame )
    {
    this->ButtonFrame->SetParent ( NULL );
    this->ButtonFrame->Delete();
    this->ButtonFrame = NULL;
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
  os << indent << "CacheSizeLabel: " << this->GetCacheSizeLabel ( ) << "\n";
  os << indent << "CacheFreeLabel: " << this->GetCacheFreeLabel ( ) << "\n";
  os << indent << "CloseButton: " << this->GetCloseButton () << "\n";
  os << indent << "ClearCacheButton: " << this->GetClearCacheButton () << "\n";
  os << indent << "ForceReloadCheckButton: " << this->GetForceReloadCheckButton () << "\n";
  os << indent << "OverwriteCacheCheckButton: " << this->GetOverwriteCacheCheckButton () << "\n";
  os << indent << "AsynchronousCheckButton: " << this->GetAsynchronousCheckButton () << "\n";
  os << indent << "TimeOutCheckButton: " << this->GetTimeOutCheckButton () << "\n";
  os << indent << "ControlFrame: " << this->GetControlFrame () << "\n";
  os << indent << "ButtonFrame: " << this->GetButtonFrame () << "\n";
  os << indent << "TransfersFrame: " << this->GetTransfersFrame () << "\n";
  os << indent << "ManagerTopLevel: " << this->GetManagerTopLevel () << "\n";
  os << indent << "TransferWidgetCollection: " << this->GetTransferWidgetCollection() << "\n";
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

  vtkDataIOManager *dm = vtkDataIOManager::SafeDownCast ( caller );
  vtkCacheManager *cm = vtkCacheManager::SafeDownCast ( caller );
  
  if ( dm == this->DataIOManager && dm != NULL )
    {
    if ( event == vtkDataIOManager::NewTransferEvent )
      {
      vtkDataTransfer *dt = reinterpret_cast < vtkDataTransfer*> (callData);
      this->AddNewDataTransfer ( dt );
      }
    else if ( event == vtkDataIOManager::TransferUpdateEvent )
      {
      vtkDataTransfer *dt = reinterpret_cast < vtkDataTransfer*> (callData);
      this->UpdateGUI();
      }
    }
  else if ( cm == this->CacheManager && dm != NULL )
    {
    if ( event == vtkCacheManager::CacheLimitExceededEvent )
      {
      this->UpdateGUI();
      }
    else if ( event == vtkCacheManager::InsufficientFreeBufferEvent )
      {
      this->UpdateGUI();
      }
    }
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
  vtkIntArray  *events = vtkIntArray::New();
  events->InsertNextValue( vtkDataIOManager::RemoteReadEvent);
  events->InsertNextValue( vtkDataIOManager::RemoteWriteEvent);
  events->InsertNextValue( vtkDataIOManager::LocalReadEvent);
  events->InsertNextValue( vtkDataIOManager::LocalWriteEvent);
  events->InsertNextValue( vtkDataIOManager::TransferUpdateEvent);
  vtkSetAndObserveMRMLNodeEventsMacro ( this->DataIOManager, iomanager, events );
  events->Delete();
  events = NULL;
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

  vtkIntArray  *events = vtkIntArray::New();
  events->InsertNextValue( vtkCacheManager::CacheLimitExceededEvent);
  events->InsertNextValue( vtkCacheManager::InsufficientFreeBufferEvent);
  vtkSetAndObserveMRMLNodeEventsMacro ( this->CacheManager, manager, events );
  events->Delete();
  events = NULL;
}



//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::UpdateGUI()
{

  int intsz;
  char *txt = new char[256];
  
  if ( this->CacheManager != NULL )
    {
    //---CacheSize:
    sprintf ( txt, "" );
    intsz = this->CacheManager->GetCurrentCacheSize();
    if ( this->CacheManager->GetFreeCacheSpaceRemaining() < 0 )
      {
      sprintf ( txt, "Cache size: %d (FULL!)", intsz);
      }
    else
      {
      sprintf ( txt, "Cache size: %d", intsz);
      }
    this->CacheSizeLabel->SetText ( txt );

    //---CacheFree:
    intsz = this->CacheManager->GetRemoteCacheFreeBufferSize();
    sprintf ( txt, "" );
    if ( this->CacheManager->GetFreeCacheSpaceRemaining() < 0 )
      {
      sprintf ( txt, "Cache free: %d (None!)", intsz);
      }
    else
      {
      sprintf ( txt, "Cache free: %d", intsz);
      }
    this->CacheSizeLabel->SetText ( txt );

    //--- cache options:
    this->ForceReloadCheckButton->SetSelectedState ( this->CacheManager->GetEnableForceRedownload() );
    //    this->OverwriteCacheCheckbutton->SetSelectedState ( this->CacheManager->GetEnableRemoteCacheOverwriting() );
    }

  //--- DataIO options:
  if ( this->DataIOManager != NULL )
    {
    this->AsynchronousCheckButton->SetSelectedState ( this->DataIOManager->GetEnableAsynchronousIO() );
    }
  delete [] txt;  

  //--- update widgets

  if ( this->TransferWidgetCollection != NULL )
    {
    int numW = this->TransferWidgetCollection->GetNumberOfItems();
    for (int i =0; i < numW; i++)
      {
      vtkSlicerDataTransferWidget *w =
        vtkSlicerDataTransferWidget::SafeDownCast (this->TransferWidgetCollection->GetItemAsObject ( i ));
      w->UpdateWidget();
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::AddNewDataTransfer ( vtkDataTransfer *transfer )
{
  if ( this->TransferWidgetCollection == NULL )
    {
    return;
    }
  vtkSlicerDataTransferWidget *w = vtkSlicerDataTransferWidget::New();
  if ( w != NULL )
    {
    w->SetDataTransfer ( transfer );
    w->UpdateWidget();
    w->SetParent ( this->TransfersFrame );
    this->Script ( "pack %s -side top -anchor nw -padx 1 -pady 1", w->GetWidgetName() );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::DeleteDataTransfer ( vtkDataTransfer *transfer )
{
  if ( this->TransferWidgetCollection == NULL )
    {
    return;
    }
  int numW = this->TransferWidgetCollection->GetNumberOfItems();
  for (int i =0; i < numW; i++)
    {
    vtkSlicerDataTransferWidget *w =
      vtkSlicerDataTransferWidget::SafeDownCast (this->TransferWidgetCollection->GetItemAsObject ( i ));
    if ( w->GetDataTransfer() == transfer )
      {
      this->Script ( "pack forget %s", w->GetWidgetName() );
      w->SetParent ( NULL );
      w->Delete();
      w = NULL;
      }
    }
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
  this->TransferWidgetCollection->RemoveAllItems();
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::BuildGUI ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWTopLevel *master = app->GetNthWindow(0);

  this->ManagerTopLevel = vtkKWTopLevel::New();
  this->ManagerTopLevel->SetApplication ( app );
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
    this->ButtonFrame = vtkKWFrame::New();
    this->ButtonFrame->SetParent ( this->ManagerTopLevel );
    this->ButtonFrame->Create();

    this->Script ( "pack %s -side top -anchor nw -padx 0 -pady 0", this->ControlFrame->GetWidgetName() );
    this->Script ( "pack %s -side top -anchor nw -padx 0 -pady 0", this->TransfersFrame->GetWidgetName() );  
    this->Script ( "pack %s -side top -anchor nw -padx 0 -pady 0", this->ButtonFrame->GetWidgetName() );

    // all widgets in the Button Frame
    this->CloseButton = vtkKWPushButton::New();
    this->CloseButton->SetParent ( this->ButtonFrame );
    this->CloseButton->Create();
    this->CloseButton->SetText ( "Close");

    this->ClearCacheButton = vtkKWPushButton::New();
    this->ClearCacheButton->SetParent ( this->ButtonFrame );
    this->ClearCacheButton->Create();
    this->ClearCacheButton->SetText ( "Clear cache");

    this->Script ( "pack %s %s -side top -anchor c -padx 4 -pady 4",
                   this->ClearCacheButton->GetWidgetName(),
                   this->CloseButton->GetWidgetName() );
    
    // all widgets in the Control Frame
    this->CacheSizeLabel = vtkKWLabel::New();
    this->CacheSizeLabel->SetParent ( this->ControlFrame );
    this->CacheSizeLabel->Create();

    this->CacheFreeLabel = vtkKWLabel::New();
    this->CacheFreeLabel->SetParent ( this->ControlFrame );
    this->CacheFreeLabel->Create();

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

    this->Script ( "pack %s %s %s %s -side top -anchor nw -padx 4 -pady 4",
                   this->CacheSizeLabel->GetWidgetName(),
                   this->CacheFreeLabel->GetWidgetName(),
                   this->ForceReloadCheckButton->GetWidgetName(),
                   this->AsynchronousCheckButton->GetWidgetName() );
    
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

