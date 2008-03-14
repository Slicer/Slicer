#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkKWMessageDialog.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerCacheAndDataIOManagerGUI.h"
#include "vtkSlicerDataTransferWidget.h"
#include "vtkSlicerDataTransferIcons.h"

//---------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkSlicerCacheAndDataIOManagerGUI, "$Revision: 1.0 $");
vtkStandardNewMacro (vtkSlicerCacheAndDataIOManagerGUI );



//---------------------------------------------------------------------------
vtkSlicerCacheAndDataIOManagerGUI::vtkSlicerCacheAndDataIOManagerGUI ( )
{
  this->CacheSizeLabel = NULL;
  this->CacheFreeLabel = NULL;
  this->CloseButton = NULL;
  this->RefreshButton = NULL;
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
  this->DataIOManager = NULL;
  this->CacheManager = NULL;
  this->DataTransferIcons = NULL;
}

//---------------------------------------------------------------------------
vtkSlicerCacheAndDataIOManagerGUI::~vtkSlicerCacheAndDataIOManagerGUI ( )
{

  if ( this->TransferWidgetCollection )
    {
    vtkSlicerDataTransferWidget *w;
    int num = this->TransferWidgetCollection->GetNumberOfItems();
    while ( num > 0 )
      {
      w = vtkSlicerDataTransferWidget::SafeDownCast ( this->TransferWidgetCollection->GetItemAsObject( 0 ) );
      this->TransferWidgetCollection->RemoveItem ( 0 );
      w->RemoveWidgetObservers();
      w->SetParent ( NULL );
      w->Delete();
      w = NULL;
      num = this->TransferWidgetCollection->GetNumberOfItems();      
      }
    this->TransferWidgetCollection->RemoveAllItems();
    this->TransferWidgetCollection->Delete();
    this->TransferWidgetCollection = NULL;
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
  if ( this->RefreshButton )
    {
    this->RefreshButton->SetParent ( NULL );
    this->RefreshButton->Delete();
    this->RefreshButton = NULL;    
    }
  if ( this->CloseButton )
    {
    this->CloseButton->SetParent ( NULL );
    this->CloseButton->Delete();
    this->CloseButton = NULL;
    }
  if ( this->CancelAllButton )
    {
    this->CancelAllButton->SetParent ( NULL );
    this->CancelAllButton->Delete();
    this->CancelAllButton = NULL;
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
  if ( this->DataTransferIcons )
    {
    this->DataTransferIcons->Delete();
    this->DataTransferIcons = NULL;
    }
  
  this->DataIOManager = NULL;
  this->CacheManager = NULL;
  this->Built = false;
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "SlicerCacheAndDataIOManagerGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "CacheSizeLabel: " << this->GetCacheSizeLabel ( ) << "\n";
  os << indent << "CacheFreeLabel: " << this->GetCacheFreeLabel ( ) << "\n";
  os << indent << "RefreshButton: " << this->GetRefreshButton () << "\n";
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

  this->CancelAllButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->RefreshButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
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
  this->CancelAllButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->RefreshButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
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
    int ret=0;
    
    if ( b == this->CloseButton && event == vtkKWPushButton::InvokedEvent )
      {
      this->WithdrawManagerWindow();
      }
    else if ( b == this->RefreshButton && event == vtkKWPushButton::InvokedEvent )
      {
      this->UpdateEntireGUI();
      }
    else if ( b == this->ClearCacheButton && event == vtkKWPushButton::InvokedEvent && this->CacheManager != NULL)
      {
      //--- prompt to make sure user wants to do this.
      vtkKWMessageDialog *d = vtkKWMessageDialog::New();
      d->SetParent ( this->ManagerTopLevel );
      d->SetStyleToYesNo();
      std::string msg = "Are you sure you want to delete all cached files in: ";
      msg += this->CacheManager->GetRemoteCacheDirectory();
      msg += "?";
      d->SetText ( msg.c_str());
      d->Create();
      ret = d->Invoke();
      d->Delete();
      if ( ret )
        {
        this->CacheManager->ClearCache();
        //--- delete all current data transfers
        //--- which causes the transfer panel to update...
        if ( this->TransferWidgetCollection != NULL )
          {
          int numW = this->TransferWidgetCollection->GetNumberOfItems();
          for (int i =0; i < numW; i++)
            {
            vtkSlicerDataTransferWidget *w =
              vtkSlicerDataTransferWidget::SafeDownCast (this->TransferWidgetCollection->GetItemAsObject ( i ));
            if ( w != NULL )
              {
              w->DeleteTransferFromCache();
              }
            }
          //--- and get rid of anything else there...
          //--- which will trigger a CacheClearEvent
          //--- that causes this overview panel to update.
          this->UpdateEntireGUI();
          }
        }
      }
    else if ( b == this->CancelAllButton && event == vtkKWPushButton::InvokedEvent )
      {
      //--- prompt to make sure user wants to do this.
      vtkKWMessageDialog *d = vtkKWMessageDialog::New();
      d->SetParent ( this->ManagerTopLevel );
      d->SetStyleToYesNo();
      std::string msg = "Are you sure you want to cancel all running and pending data transfers?";
      d->SetText ( msg.c_str());
      d->Create();
      ret = d->Invoke();
      d->Delete();
      if ( ret )
        {
        this->CancelAllDataTransfers();
        this->UpdateTransfersPanel();
        }
      }
    vtkKWCheckButton *c = vtkKWCheckButton::SafeDownCast ( caller );
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
    if ( app != NULL && this->DataIOManager != NULL && this->CacheManager != NULL )
      {
      if ( c == this->ForceReloadCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent )
        {
        //--- application will update only if the value has changed;
        app->SetEnableForceRedownload(this->ForceReloadCheckButton->GetSelectedState());
        }
      else if ( c == this->OverwriteCacheCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent )
        {
        //--- application will update only if the value has changed;
        app->SetEnableRemoteCacheOverwriting(this->OverwriteCacheCheckButton->GetSelectedState());
        }
      else if ( c == this->AsynchronousCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent )
        {
        //--- application will update only if the value has changed;
        app->SetEnableAsynchronousIO(this->AsynchronousCheckButton->GetSelectedState());
        }
      else if ( c == this->TimeOutCheckButton && event == vtkKWCheckButton::SelectedStateChangedEvent )
        {
        //--- nothing for now.
        }
      }

}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::CancelAllDataTransfers ( )
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
    if ( w != NULL )
      {
      if ( w->GetDataTransfer()->GetCancelRequested () == 0 )
        {
        w->GetDataTransfer()->SetCancelRequested ( 1 );
        w->GetDataTransfer()->SetTransferStatus ( vtkDataTransfer::CancelPending );
        }
      }
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
  
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Processing mrml events...");
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
      this->UpdateEntireGUI();
      }
    else if ( event == vtkDataIOManager::RemoteReadEvent )
      {
      this->DisplayManagerWindow();
      }
    else if ( event == vtkDataIOManager::RemoteWriteEvent )
      {
      this->DisplayManagerWindow();
      }
    else if ( event == vtkDataIOManager::SettingsUpdateEvent )
      {
      if ( this->AsynchronousCheckButton->GetSelectedState() != this->DataIOManager->GetEnableAsynchronousIO() )
        {
              this->UpdateOverviewPanel();
        }
      }
    }
  else if ( cm == this->CacheManager && cm != NULL && this->DataIOManager != NULL)
    {
    if ( event == vtkCacheManager::CacheLimitExceededEvent )
      {
      this->UpdateOverviewPanel();
      }
    else if ( event == vtkCacheManager::InsufficientFreeBufferEvent )
      {
      this->UpdateOverviewPanel();
      }
    else if ( event == vtkCacheManager::CacheClearEvent )
      {
      this->UpdateEntireGUI();
      }
    else if ( event == vtkCacheManager::CacheDeleteEvent )
      {
      this->UpdateTransfersPanel();
      }
    else if ( event == vtkCacheManager::SettingsUpdateEvent )
      {
      this->UpdateOverviewPanel();
      }
    }
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: DONE processing mrml events...");
}    





//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::SetDataIOManager( vtkDataIOManager *iomanager )
{



  vtkDataIOManager *oldValue = this->DataIOManager;
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &this->DataIOManager), iomanager );
  if ( oldValue != this->DataIOManager )
    {
//    this->InvokeEvent (vtkCommand::ModifiedEvent);
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
  events->InsertNextValue ( vtkDataIOManager::NewTransferEvent );
  events->InsertNextValue( vtkDataIOManager::TransferUpdateEvent);
  events->InsertNextValue( vtkDataIOManager::SettingsUpdateEvent);
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
//    this->InvokeEvent (vtkCommand::ModifiedEvent);
    }
}



//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::SetAndObserveCacheManager( vtkCacheManager *manager )
{

  vtkIntArray  *events = vtkIntArray::New();
  events->InsertNextValue( vtkCacheManager::CacheLimitExceededEvent);
  events->InsertNextValue( vtkCacheManager::InsufficientFreeBufferEvent);
  events->InsertNextValue( vtkCacheManager::CacheClearEvent);
  events->InsertNextValue( vtkCacheManager::CacheDeleteEvent);
  events->InsertNextValue( vtkCacheManager::SettingsUpdateEvent );
  vtkSetAndObserveMRMLNodeEventsMacro ( this->CacheManager, manager, events );
  events->Delete();
  events = NULL;
}



//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::UpdateOverviewPanel()
{
  float sz;
  char *txt = new char[256];
  
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Updating Overview Panel");
  if ( this->CacheManager != NULL )
    {
    //---CacheSize:
    sprintf ( txt, "" );
    if ( this->CacheManager->GetRemoteCacheDirectory() != NULL )
      {      
      sz = this->CacheManager->ComputeCacheSize(this->CacheManager->GetRemoteCacheDirectory(), 0);
      if ( sz > this->CacheManager->GetRemoteCacheLimit() )
        {
        sprintf ( txt, "Cache size: %6.2f (FULL!)", sz);
        }
      else
        {
        sprintf ( txt, "Cache size: %6.2f", sz);
        }
      this->CacheSizeLabel->SetText ( txt );

      //---CacheFree:
      sz = this->CacheManager->GetFreeCacheSpaceRemaining();
      sprintf ( txt, "" );
      if ( sz < 0 )
        {
        sprintf ( txt, "Cache free: %6.2f (None!)", sz);
        }
      else
        {
        sprintf ( txt, "Cache free: %6.2f", sz);
        }
      this->CacheFreeLabel->SetText ( txt );
      }
    else
      {
      this->CacheSizeLabel->SetText ( "-" );
      this->CacheFreeLabel->SetText ( "-");
      }
    //--- cache options:
    this->ForceReloadCheckButton->SetSelectedState ( this->CacheManager->GetEnableForceRedownload() );
    //    this->OverwriteCacheCheckbutton->SetSelectedState ( this->CacheManager->GetEnableRemoteCacheOverwriting() );
    }


  if ( this->CacheManager->GetCurrentCacheSize() == 0 )
    {
    this->ClearCacheButton->SetImageToIcon ( this->DataTransferIcons->GetDeleteFromCacheDisabledIcon() );
    this->ClearCacheButton->SetStateToDisabled();
    }
  else
    {
    this->ClearCacheButton->SetImageToIcon ( this->DataTransferIcons->GetDeleteFromCacheIcon() );
    this->ClearCacheButton->SetStateToNormal();
    }

  //--- DataIO options:
  if ( this->DataIOManager != NULL )
    {
    this->AsynchronousCheckButton->SetSelectedState ( this->DataIOManager->GetEnableAsynchronousIO() );
    }

  int anyTransfersRunning=0;
  if ( this->TransferWidgetCollection != NULL )
    {
    int numW = this->TransferWidgetCollection->GetNumberOfItems();
    for (int i =0; i < numW; i++)
      {
      vtkSlicerDataTransferWidget *w =
        vtkSlicerDataTransferWidget::SafeDownCast (this->TransferWidgetCollection->GetItemAsObject ( i ));
      if ( w != NULL )
        {
        if ( w->GetDataTransfer() != NULL )
          {
          if (w->GetDataTransfer()->GetTransferStatus() == vtkDataTransfer::Running ||
              w->GetDataTransfer()->GetTransferStatus() == vtkDataTransfer::Pending ||
              w->GetDataTransfer()->GetTransferStatus() == vtkDataTransfer::Ready ||
              w->GetDataTransfer()->GetTransferStatus() == vtkDataTransfer::Idle )            
            {
            anyTransfersRunning = 1;
            return;
            }
          }
        }
      }
    }
  if ( !anyTransfersRunning )
    {
    this->CancelAllButton->SetImageToIcon ( this->DataTransferIcons->GetTransferCancelDisabledIcon() );
    this->CancelAllButton->SetStateToDisabled();
    }
  else
    {
    this->CancelAllButton->SetImageToIcon ( this->DataTransferIcons->GetTransferCancelIcon() );
    this->CancelAllButton->SetStateToNormal();
    }

  delete [] txt;

  //--- update the GUI
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app != NULL )
    {
    app->ProcessIdleTasks();
    }

  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: DONE Updating Overview Panel");  
}



//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::UpdateTransfersPanel()
{
  //--- update widgets

  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Updating Transfers Panel");
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
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: DONE Updating Transfers Panel");

  //--- update the GUI
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app != NULL )
    {
    app->ProcessIdleTasks();
    }

}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::UpdateEntireGUI()
{
  this->UpdateOverviewPanel();
  this->UpdateTransfersPanel();
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::AddNewDataTransfer ( vtkDataTransfer *transfer )
{

  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Adding new data transfer");
  if ( this->TransferWidgetCollection == NULL )
    {
    this->TransferWidgetCollection = vtkCollection::New();
    }
  vtkSlicerDataTransferWidget *w = vtkSlicerDataTransferWidget::New();
  if ( w != NULL )
    {
    w->SetParent ( this->TransfersFrame->GetFrame() );
    w->SetDataTransfer ( transfer );
    w->SetTransferID ( transfer->GetTransferID() );
    w->SetCacheManager ( this->CacheManager);
    w->SetApplication ( this->GetApplication() );
    w->Create();
    w->UpdateWidget();
    w->AddWidgetObservers();
    this->TransferWidgetCollection->AddItem ( w );
    }
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: DONE adding new data transfer");

}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::DeleteDataTransfer ( vtkDataTransfer *transfer )
{

  if ( this->TransferWidgetCollection == NULL )
    {
    return;
    }
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Deleting new data transfer");
  int numW = this->TransferWidgetCollection->GetNumberOfItems();
  for (int i =0; i < numW; i++)
    {
    vtkSlicerDataTransferWidget *w =
      vtkSlicerDataTransferWidget::SafeDownCast (this->TransferWidgetCollection->GetItemAsObject ( i ));
    if ( w->GetDataTransfer() == transfer )
      {
      w->RemoveWidgetObservers();
      this->Script ( "pack forget %s", w->GetWidgetName() );
      w->SetParent ( NULL );
      w->Delete();
      w = NULL;
      }
    }
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Done deleting new data transfer");
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::DeleteDataTransfer ( int ID )
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
    if ( w != NULL )
      {
      if ( w->GetDataTransfer()->GetTransferID() == ID )
        {
        w->RemoveWidgetObservers();
        this->Script ( "pack forget %s", w->GetWidgetName() );
        w->SetParent ( NULL );
        w->Delete();
        w = NULL;
        }
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
  this->UpdateEntireGUI();
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::TearDownGUI ( )
{
  this->WithdrawManagerWindow();
//  this->ManagerTopLevel->SetMasterWindow(NULL);
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


  this->ManagerTopLevel = vtkKWTopLevel::New();
  this->ManagerTopLevel->SetApplication ( app );
  vtksys_stl::string title = "Cache & Remote Data I/O Manager Window";
  
  this->ManagerTopLevel->SetTitle(title.c_str());
//    this->ManagerTopLevel->SetMasterWindow(master);
  this->ManagerTopLevel->SetApplication ( app );
  this->ManagerTopLevel->Create();
  this->ManagerTopLevel->SetBorderWidth ( 2 );
  this->ManagerTopLevel->SetReliefToFlat();
  this->ManagerTopLevel->SetDisplayPositionToPointer();
  this->ManagerTopLevel->SetSize ( 910, 300 );
  this->ManagerTopLevel->SetMinimumSize ( 910, 100 );
  this->ManagerTopLevel->Withdraw();
  this->ManagerTopLevel->SetDeleteWindowProtocolCommand ( this, "WithdrawManagerWindow" );
    
  this->DataTransferIcons = vtkSlicerDataTransferIcons::New();
  
  this->ControlFrame = vtkKWFrame::New();
  this->ControlFrame->SetParent ( this->ManagerTopLevel );
  this->ControlFrame->Create();
  this->TransfersFrame = vtkKWFrameWithScrollbar::New();
  this->TransfersFrame->SetParent ( this->ManagerTopLevel );
  this->TransfersFrame->Create();
  this->ButtonFrame = vtkKWFrame::New();
  this->ButtonFrame->SetParent ( this->ManagerTopLevel );
  this->ButtonFrame->Create();
  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( this->ManagerTopLevel );
  f->Create();
  f->SetBackgroundColor ( 0.7, 0.7, 0.7);
  this->Script ( "grid %s -row 0 -column 0 -sticky ew -padx 0 -pady 0", this->ControlFrame->GetWidgetName() );
  this->Script ( "grid %s -row 1 -column 0 -sticky ew -padx 0 -pady 0", f->GetWidgetName() );
  this->Script ( "grid %s -row 3 -column 0 -sticky ew -padx 0 -pady 0", this->ButtonFrame->GetWidgetName() );
  this->Script ( "grid %s -row 2 -column 0 -sticky news -padx 0 -pady 0", this->TransfersFrame->GetWidgetName() );  
  this->Script ( "grid rowconfigure %s 0 -weight 0", this->ManagerTopLevel->GetWidgetName() );
  this->Script ( "grid rowconfigure %s 1 -weight 0", this->ManagerTopLevel->GetWidgetName() );
  this->Script ( "grid rowconfigure %s 2 -weight 1", this->ManagerTopLevel->GetWidgetName() );
  this->Script ( "grid rowconfigure %s 3 -weight 0", this->ManagerTopLevel->GetWidgetName() );
  this->Script ( "grid columnconfigure %s 0 -weight 1", this->ManagerTopLevel->GetWidgetName() );

  // all widgets in the Control Frame

  this->ForceReloadCheckButton = vtkKWCheckButton::New();
  this->ForceReloadCheckButton->SetParent ( this->ControlFrame );
  this->ForceReloadCheckButton->Create();
  this->ForceReloadCheckButton->SetText ( "Always re-download");
  if ( this->CacheManager != NULL )
    {
    this->ForceReloadCheckButton->SetSelectedState(this->CacheManager->GetEnableForceRedownload() );
    }
  else
    {
    this->ForceReloadCheckButton->SetSelectedState(0);
    }
    
  this->OverwriteCacheCheckButton = vtkKWCheckButton::New();
  this->OverwriteCacheCheckButton->SetParent ( this->ControlFrame );
  this->OverwriteCacheCheckButton->Create();
  this->OverwriteCacheCheckButton->SetText ("Overwrite cached files");
/*
  if ( this->CacheManager != NULL )
    {
    this->OverwriteCacheCheckButton->SetSelectedState(this->CacheManager->GetEnableRemoteCacheOverwriting() );
    }
  else
    {
    this->OverwriteCacheCheckButton->SetSelectedState(0);
    }
*/
  this->OverwriteCacheCheckButton->SetStateToDisabled();
  this->OverwriteCacheCheckButton->SetSelectedState(0);

  this->AsynchronousCheckButton = vtkKWCheckButton::New();
  this->AsynchronousCheckButton->SetParent ( this->ControlFrame );
  this->AsynchronousCheckButton->Create();
  this->AsynchronousCheckButton->SetText ("Use asynchronous I/O");
  if ( this->DataIOManager != NULL )
    {
    this->AsynchronousCheckButton->SetSelectedState(this->DataIOManager->GetEnableAsynchronousIO() );
    }
  else
    {
    this->AsynchronousCheckButton->SetSelectedState(0);
    }

  // not packed for now.
  this->TimeOutCheckButton = vtkKWCheckButton::New();
  this->TimeOutCheckButton->SetParent ( this->ControlFrame );
  this->TimeOutCheckButton->Create();
  this->TimeOutCheckButton->SetSelectedState(0);

  this->ClearCacheButton = vtkKWPushButton::New();
  this->ClearCacheButton->SetParent ( this->ControlFrame );
  this->ClearCacheButton->Create(); 
 this->ClearCacheButton->SetImageToIcon ( this->DataTransferIcons->GetDeleteFromCacheIcon() );
  this->ClearCacheButton->SetBorderWidth ( 0);
  this->ClearCacheButton->SetReliefToFlat();
  this->ClearCacheButton->SetBalloonHelpString ("Delete all files in cache." );

  this->RefreshButton = vtkKWPushButton::New();
  this->RefreshButton->SetParent ( this->ControlFrame );
  this->RefreshButton->Create();
  this->RefreshButton->SetImageToIcon ( this->DataTransferIcons->GetRefreshSettingsIcon() );
  this->RefreshButton->SetBorderWidth (0);
  this->RefreshButton->SetReliefToFlat ();
  this->RefreshButton->SetBalloonHelpString ("Refresh cache space report in panel." );  
  
  this->CancelAllButton = vtkKWPushButton::New();
  this->CancelAllButton->SetParent ( this->ControlFrame );
  this->CancelAllButton->Create();
  this->CancelAllButton->SetImageToIcon ( this->DataTransferIcons->GetTransferCancelIcon() );
  this->CancelAllButton->SetBorderWidth(0);
  this->CancelAllButton->SetReliefToFlat();
  this->CancelAllButton->SetBalloonHelpString ( "Cancel all pending and running data transfers." );

  this->Script ( "pack %s %s %s -side left -anchor n -padx 4 -pady 4",
                 this->ForceReloadCheckButton->GetWidgetName(),
                 this->AsynchronousCheckButton->GetWidgetName(),
                 this->OverwriteCacheCheckButton->GetWidgetName() );
  this->Script ( "pack %s %s %s -side right -anchor n -padx 4 -pady 4",
                 this->CancelAllButton->GetWidgetName(),
                 this->RefreshButton->GetWidgetName(),
                 this->ClearCacheButton->GetWidgetName());


  // all title widgets in the Transfer Frame
  vtkKWLabel *l;
  l = vtkKWLabel::New();
  l->SetParent (f);
  l->Create();
  l->SetWidth ( 6 );
  l->SetText ("type");
  this->Script ( "grid %s -row 0 -column 0 -sticky w -padx 1 -pady 2 ", l->GetWidgetName());
  this->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  l->Delete();
  l = vtkKWLabel::New();
  l->SetParent (f);
  l->Create();
  l->SetWidth ( 6 );
  l->SetText ("status");
  this->Script ( "grid %s -row 0 -column 1 -sticky w -padx 1 -pady 2 ", l->GetWidgetName());
  this->Script ( "grid columnconfigure %s 1 -weight 0", f->GetWidgetName() );
  l->Delete();
  l = vtkKWLabel::New();
  l->SetParent (f);
  l->Create();
  l->SetWidth ( 6 );
  l->SetText ("cancel");
  this->Script ( "grid %s -row 0 -column 2 -sticky w -padx 1 -pady 2 ", l->GetWidgetName());
  this->Script ( "grid columnconfigure %s 2 -weight 0", f->GetWidgetName() );
  l->Delete();
  l = vtkKWLabel::New();
  l->SetParent (f);
  l->Create();
  l->SetWidth ( 6 );
  l->SetText ("clear");
  this->Script ( "grid %s -row 0 -column 3 -sticky w -padx 1 -pady 2 ", l->GetWidgetName());
  this->Script ( "grid columnconfigure %s 3 -weight 0", f->GetWidgetName() );
  l->Delete();
  l = vtkKWLabel::New();
  l->SetParent (f);
  l->Create();
  l->SetWidth ( 6 );
  l->SetText ("info");
  this->Script ( "grid %s -row 0 -column 4 -sticky w -padx 1 -pady 2 ", l->GetWidgetName());
  this->Script ( "grid columnconfigure %s 4 -weight 0", f->GetWidgetName() );
  l->Delete();
  l = vtkKWLabel::New();
  l->SetParent (f);
  l->Create();
  l->SetText ("source uri");
  this->Script ( "grid %s -row 0 -column 5 -sticky news -padx 1 -pady 2 ", l->GetWidgetName());
  this->Script ( "grid columnconfigure %s 5 -weight 1", f->GetWidgetName() );
  l->Delete();
  f->Delete();


  // all widgets in the Button Frame
  this->CacheSizeLabel = vtkKWLabel::New();
  this->CacheSizeLabel->SetParent ( this->ButtonFrame );
  this->CacheSizeLabel->Create();
  this->CacheSizeLabel->SetWidth (20);
  this->CacheSizeLabel->SetText ( "Cache size: ");
  this->CacheSizeLabel->SetAnchorToWest ();
  this->CacheSizeLabel->SetBalloonHelpString ("Use View->Application Settings Interface->RemoteIO Settings to adjust cache size");

  this->CacheFreeLabel = vtkKWLabel::New();
  this->CacheFreeLabel->SetParent ( this->ButtonFrame );
  this->CacheFreeLabel->Create();
  this->CacheFreeLabel->SetWidth ( 20 );
  this->CacheFreeLabel->SetText ( "Cache free: ");
  this->CacheFreeLabel->SetAnchorToWest ();
  this->CacheFreeLabel->SetBalloonHelpString ("Use View->Application Settings Interface->RemoteIO Settings to adjust cache free buffer size");

  this->CloseButton = vtkKWPushButton::New();
  this->CloseButton->SetParent ( this->ButtonFrame );
  this->CloseButton->Create();
  this->CloseButton->SetText ( "Close");
  this->CloseButton->SetWidth ( 10 );

  this->Script ( "pack %s %s -side left -anchor n -padx 4 -pady 4",
                 this->CacheSizeLabel->GetWidgetName(),
                 this->CacheFreeLabel->GetWidgetName() );
  this->Script ( "pack %s -side right -anchor n -padx 4 -pady 4", this->CloseButton->GetWidgetName() );
}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::DisplayManagerWindow ( )
{

  if (! this->Built )
    {
    return;
    }
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Displaying Manager Window");
  this->ManagerTopLevel->DeIconify();
  this->ManagerTopLevel->Raise();
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: DONE displaying Manager Window");
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::WithdrawManagerWindow ( )
{
  if ( ! this->Built )
    {
    return;
    }
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Withdrawing Manager Window");
  this->ManagerTopLevel->Withdraw();
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Done withdrawing Manager Window.");
}

