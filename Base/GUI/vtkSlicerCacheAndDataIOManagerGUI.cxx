#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"
#include "vtkKWWidget.h"
#include "vtkKWMessageDialog.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerWindow.h"
#include "vtkKWFrame.h"
#include "vtkSlicerCacheAndDataIOManagerGUI.h"
#include "vtkSlicerDataTransferWidget.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkURIHandler.h"

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
  this->ClearDisplayButton = NULL;
  this->ClearCacheButton = NULL;
  this->ForceReloadCheckButton = NULL;
  this->OverwriteCacheCheckButton = NULL;
  this->AsynchronousCheckButton = NULL;
  this->CacheDirectoryButton = NULL;
  this->CacheLimitSpinBox = NULL;
  this->CacheFreeBufferSizeSpinBox = NULL;
  this->TimeOutCheckButton = NULL;
  this->ManagerTopLevel = NULL;
  this->ControlFrame = NULL;
  this->ButtonFrame = NULL;
  this->TransfersFrame = NULL;
  this->Built = false;
  this->TransferWidgetCollection = NULL;
  this->DataIOManager = NULL;
  this->CacheManager = NULL;
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
  if ( this->ClearDisplayButton )
    {
    this->ClearDisplayButton->SetParent ( NULL );
    this->ClearDisplayButton->Delete();
    this->ClearDisplayButton = NULL;
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
  if ( this->CacheDirectoryButton )
    {
    this->CacheDirectoryButton->SetParent ( NULL );
    this->CacheDirectoryButton->Delete();
    this->CacheDirectoryButton=NULL;
    }
  if ( this->CacheLimitSpinBox )
    {
    this->CacheLimitSpinBox->SetParent ( NULL );
    this->CacheLimitSpinBox->Delete();
    this->CacheLimitSpinBox=NULL;    
    }
  if ( this->CacheFreeBufferSizeSpinBox )
    {
    this->CacheFreeBufferSizeSpinBox->SetParent ( NULL );
    this->CacheFreeBufferSizeSpinBox->Delete();
    this->CacheFreeBufferSizeSpinBox=NULL;    
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
  os << indent << "ClearDisplayButton: " << this->GetClearDisplayButton () << "\n";
  os << indent << "RefreshButton: " << this->GetRefreshButton () << "\n";
  os << indent << "CloseButton: " << this->GetCloseButton () << "\n";
  os << indent << "ClearCacheButton: " << this->GetClearCacheButton () << "\n";
  os << indent << "ForceReloadCheckButton: " << this->GetForceReloadCheckButton () << "\n";
  os << indent << "OverwriteCacheCheckButton: " << this->GetOverwriteCacheCheckButton () << "\n";
  os << indent << "AsynchronousCheckButton: " << this->GetAsynchronousCheckButton () << "\n";
  os << indent << "CacheDirectoryButton: " << this->GetCacheDirectoryButton() << "\n";
  os << indent << "CacheLimitSpinBox: " << this->GetCacheLimitSpinBox() << "\n";
  os << indent << "CacheFreeBufferSizeSpinBox: " << this->GetCacheFreeBufferSizeSpinBox() << "\n";
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
  this->ClearDisplayButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->RefreshButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CloseButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ClearCacheButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ForceReloadCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->OverwriteCacheCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AsynchronousCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->TimeOutCheckButton->RemoveObservers ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CacheLimitSpinBox->GetWidget()->RemoveObservers ( vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CacheFreeBufferSizeSpinBox->GetWidget()->RemoveObservers ( vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );

  
}

//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::AddGUIObservers ( )
{
  this->CancelAllButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ClearDisplayButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->RefreshButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CloseButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ClearCacheButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->ForceReloadCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->OverwriteCacheCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->AsynchronousCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->TimeOutCheckButton->AddObserver ( vtkKWCheckButton::SelectedStateChangedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CacheLimitSpinBox->GetWidget()->AddObserver ( vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CacheFreeBufferSizeSpinBox->GetWidget()->AddObserver( vtkKWSpinBox::SpinBoxValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );


}


//---------------------------------------------------------------------------
void vtkSlicerCacheAndDataIOManagerGUI::ProcessGUIEvents ( vtkObject *caller,
                                                           unsigned long event, void *callData )
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );

  vtkKWSpinBox *sb = vtkKWSpinBox::SafeDownCast ( caller );
  vtkKWTopLevel *lsb = vtkKWTopLevel::SafeDownCast ( caller );


  int ret=0;

  if ( b == this->CloseButton && event == vtkKWPushButton::InvokedEvent )
    {
    this->WithdrawManagerWindow();
    }
  else if ( b == this->ClearDisplayButton && event == vtkKWPushButton::InvokedEvent )
    {
    // nothing yet.
    //--- remove all data transfers.
    //--- refresh display.
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
      this->UpdateEntireGUI();
      }
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
            w->GetDataTransfer()->SetTransferStatus ( vtkDataTransfer::Deleted );
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
    if ( sb == this->CacheLimitSpinBox->GetWidget() && event == vtkKWSpinBox::SpinBoxValueChangedEvent )
      {
      app->SetRemoteCacheLimit (this->CacheLimitSpinBox->GetWidget()->GetValue() );
      }
    else if ( sb == this->CacheFreeBufferSizeSpinBox->GetWidget() && event == vtkKWSpinBox::SpinBoxValueChangedEvent )
      {
      app->SetRemoteCacheFreeBufferSize(this->CacheFreeBufferSizeSpinBox->GetWidget()->GetValue() );
      }
    if ( lsb == this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog() && event == vtkKWTopLevel::WithdrawEvent )
      {
      app->SetRemoteCacheDirectory ( this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->GetFileName() );
      }
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
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  
  vtkDebugMacro("vtkSlicerCacheAndDataIOManagerGUI: Processing mrml events...");
  if ( dm == this->DataIOManager && dm != NULL )
    {
    if ( event == vtkDataIOManager::NewTransferEvent )
      {
      vtkDataTransfer *dt = reinterpret_cast < vtkDataTransfer*> (callData);
      this->AddNewDataTransfer ( dt );
      this->UpdateEntireGUI();
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
  else if ( cm == this->CacheManager && cm != NULL && this->DataIOManager != NULL && app != NULL)
    {
    if ( event == vtkCacheManager::CacheLimitExceededEvent )
      {
      this->UpdateOverviewPanel();
      }
    else if ( event == vtkCacheManager::SettingsUpdateEvent )
      {
      this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetFileName( app->GetRemoteCacheDirectory() );
      this->CacheLimitSpinBox->GetWidget()->SetValue(app->GetRemoteCacheLimit() );
      this->CacheFreeBufferSizeSpinBox->GetWidget()->SetValue( app->GetRemoteCacheFreeBufferSize() );
      }
    else if ( event == vtkCacheManager::InsufficientFreeBufferEvent )
      {
      //--- WJPtest: may want to take this out...
      //--- if a user has already been notified that they're out of
      //--- cache space, then we don't notify them again.
      //--- This is so that remote downloads of big MRML scenes
      //--- don't result in many, many annoying pop-up dialogs.
      //--- The dialog should pop up once, only.
      //--- After the user clears some cache space, the flag should
      //--- get reset in the DataIOManager.
      if ( cm->GetInsufficientFreeBufferNotificationFlag() == 0 )
        {
        //--- pop up dialog. --/
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication());
        if ( app != NULL )
          {
          vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
          if ( appGUI != NULL )
            {
            vtkSlicerWindow *win = appGUI->GetMainSlicerWindow();
            if ( win != NULL )
              {
              vtkKWMessageDialog *d = vtkKWMessageDialog::New();
              d->SetParent ( win->GetViewFrame() );
              d->SetStyleToMessage();
              std::string msg = "The download of this dataset can't be performed because the cache is full. If a cached version of this dataset exists, Slicer will load that instead. Try clearing the cache (from the View->Cache And Remote Data Handling panel), or increasing the Remote Cache size (from the View->Application Settings panel). This message will not be displayed again.";
              d->SetText ( msg.c_str());
              d->Create();
              d->Invoke();
              d->Delete();
              }
            }
          }
        this->UpdateOverviewPanel();
        }
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
  
  vtkSlicerApplicationGUI *appGUI = NULL;
  vtkSlicerApplication *app = NULL;
  if ( (app = vtkSlicerApplication::SafeDownCast ( this->GetApplication()) ) != NULL )
    {
    appGUI = app->GetApplicationGUI();
    }
  
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
        sprintf ( txt, "(%dMB --FULL!)   ", static_cast<int>( floor(sz)));
        }
      else
        {
        sprintf ( txt, "(%dMB used)   ", static_cast<int>( floor(sz)));
        }
      this->CacheSizeLabel->SetText ( txt );

      //---CacheFree:
      sz = this->CacheManager->GetFreeCacheSpaceRemaining();
      sprintf ( txt, "" );
      if ( sz < 0 )
        {
        sprintf ( txt, "(%dMB --FULL!)", static_cast<int>( floor(sz)));
        }
      else
        {
        sprintf ( txt, "(%dMB) free", static_cast<int>( floor(sz)));
        }
      this->CacheFreeLabel->SetText ( txt );
      }
    else
      {
      this->CacheSizeLabel->SetText ( "-   " );
      this->CacheFreeLabel->SetText ( "-");
      }
    //--- cache options:
    this->ForceReloadCheckButton->SetSelectedState ( this->CacheManager->GetEnableForceRedownload() );
    //    this->OverwriteCacheCheckbutton->SetSelectedState ( this->CacheManager->GetEnableRemoteCacheOverwriting() );
    }

  if ( appGUI != NULL )
    {
    if ( this->CacheManager->GetCurrentCacheSize() == 0 )
      {
      this->ClearCacheButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerDeleteDisabledIcon() );
      this->ClearCacheButton->SetStateToDisabled();
      }
    else
      {
      this->ClearCacheButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerDeleteIcon() );
      this->ClearCacheButton->SetStateToNormal();
      }
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
//            return;
            }
          }
        }
      }
    }


  if ( appGUI != NULL )
    {
    if ( !anyTransfersRunning )
      {
      this->CancelAllButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerCancelDisabledIcon() );
      this->CancelAllButton->SetStateToDisabled();
      }
    else
      {
      this->CancelAllButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerCancelIcon() );
      this->CancelAllButton->SetStateToNormal();
      }
    }
  delete [] txt;

  //--- update the GUI
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
    w->SetDataIOManager ( this->DataIOManager );
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
    }
  if ( this->Built == true )
    {
    this->AddGUIObservers();
    this->CreateModuleEventBindings();
    this->UpdateEntireGUI();
    }
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

  vtkSlicerApplication *app = NULL;
  vtkSlicerApplicationGUI *appGUI = NULL;
  app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app != NULL )
    {
    appGUI = app->GetApplicationGUI();
    }
  if ( appGUI != NULL )
    {

    int label_width = 10;
    
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
    this->ClearCacheButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerDeleteIcon() );
    this->ClearCacheButton->SetBorderWidth ( 0);
    this->ClearCacheButton->SetReliefToFlat();
    this->ClearCacheButton->SetBalloonHelpString ("Delete all files in cache." );

    this->RefreshButton = vtkKWPushButton::New();
    this->RefreshButton->SetParent ( this->ControlFrame );
    this->RefreshButton->Create();
    this->RefreshButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerRefreshIcon() );
    this->RefreshButton->SetBorderWidth (0);
    this->RefreshButton->SetReliefToFlat ();
    this->RefreshButton->SetBalloonHelpString ("Refresh cache space report in bottom panel." );  
  
    this->ClearDisplayButton = vtkKWPushButton::New();
    this->ClearDisplayButton->SetParent ( this->ControlFrame);
    this->ClearDisplayButton->Create();
    this->ClearDisplayButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerCleanUpIcon() );
    this->ClearDisplayButton->SetBorderWidth ( 0);
    this->ClearDisplayButton->SetReliefToFlat();
    this->ClearDisplayButton->SetBalloonHelpString ("Clean up the transfer panel without clearing the cache.");
  
    this->CancelAllButton = vtkKWPushButton::New();
    this->CancelAllButton->SetParent ( this->ControlFrame );
    this->CancelAllButton->Create();
    this->CancelAllButton->SetImageToIcon ( appGUI->GetSlicerFoundationIcons()->GetSlicerCancelIcon() );
    this->CancelAllButton->SetBorderWidth(0);
    this->CancelAllButton->SetReliefToFlat();
    this->CancelAllButton->SetBalloonHelpString ( "Cancel all pending and running data transfers (not yet implemented)." );

    this->Script ( "pack %s %s %s -side left -anchor n -padx 4 -pady 4",
                   this->ForceReloadCheckButton->GetWidgetName(),
                   this->AsynchronousCheckButton->GetWidgetName(),
                   this->OverwriteCacheCheckButton->GetWidgetName() );
    this->Script ( "pack %s %s %s %s -side right -anchor n -padx 4 -pady 4",
                   this->CancelAllButton->GetWidgetName(),
                   this->RefreshButton->GetWidgetName(),
                   this->ClearDisplayButton->GetWidgetName(),
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
    this->CacheSizeLabel->SetText ( "()");
    this->CacheSizeLabel->SetForegroundColor ( 0.3, 0.0, 1.0 );
    this->CacheSizeLabel->SetAnchorToWest ();
    this->CacheSizeLabel->SetBalloonHelpString ("Use View->Application Settings Interface->RemoteIO Settings to adjust cache size");

    this->CacheFreeLabel = vtkKWLabel::New();
    this->CacheFreeLabel->SetParent ( this->ButtonFrame );
    this->CacheFreeLabel->Create();
    this->CacheFreeLabel->SetText ( "()");
    this->CacheFreeLabel->SetForegroundColor ( 0.3, 0.0, 1.0 );
    this->CacheFreeLabel->SetAnchorToWest ();
    this->CacheFreeLabel->SetBalloonHelpString ("Use View->Application Settings Interface->RemoteIO Settings to adjust cache free buffer size");


    //--- CacheDirectoryButton
    if (!this->CacheDirectoryButton)
      {
      this->CacheDirectoryButton = vtkKWLoadSaveButtonWithLabel::New();
      }

    this->CacheDirectoryButton->SetParent(this->ButtonFrame);
    this->CacheDirectoryButton->Create();
    this->CacheDirectoryButton->SetLabelText("Cache Directory:");
    this->CacheDirectoryButton->GetWidget()->TrimPathFromFileNameOff();
    this->CacheDirectoryButton->GetWidget()
      ->GetLoadSaveDialog()->ChooseDirectoryOn();
    this->CacheDirectoryButton->GetWidget()
      ->GetLoadSaveDialog()->SaveDialogOff();
    this->CacheDirectoryButton->GetWidget()
      ->GetLoadSaveDialog()->SetTitle("Select a directory for cached files");
    this->CacheDirectoryButton->SetBalloonHelpString(
                                                           "Remote Cache directory for downloded files.");
    this->CacheDirectoryButton->GetWidget()->GetLoadSaveDialog()->SetFileName( app->GetRemoteCacheDirectory() );

    //--- CacheLimitSpinBox
     if (!this->CacheLimitSpinBox)
    {
    this->CacheLimitSpinBox = vtkKWSpinBoxWithLabel::New();
    }
     this->CacheLimitSpinBox->SetParent(this->ButtonFrame);
     this->CacheLimitSpinBox->Create();
     this->CacheLimitSpinBox->SetLabelText("Cache Limit:");
     this->CacheLimitSpinBox->GetWidget()->SetRestrictValueToInteger();
     this->CacheLimitSpinBox->GetWidget()->SetWidth (6);
     this->CacheLimitSpinBox->GetWidget()->SetRange(0,1000);
     this->CacheLimitSpinBox->SetBalloonHelpString("Set the upper limit on the size of the cache directory (Mb).");
     this->CacheLimitSpinBox->GetWidget()->SetValue(app->GetRemoteCacheLimit() );

     //--- CacheFreeBufferSizeSpinBox
     if (!this->CacheFreeBufferSizeSpinBox)
       {
       this->CacheFreeBufferSizeSpinBox = vtkKWSpinBoxWithLabel::New();
       }
     this->CacheFreeBufferSizeSpinBox->SetParent(this->ButtonFrame);
     this->CacheFreeBufferSizeSpinBox->Create();
     this->CacheFreeBufferSizeSpinBox->SetLabelText("Cache Free Buffer:");
     this->CacheFreeBufferSizeSpinBox->GetWidget()->SetRestrictValueToInteger();
     this->CacheFreeBufferSizeSpinBox->GetWidget()->SetRange(0,900);
     this->CacheFreeBufferSizeSpinBox->GetWidget()->SetWidth ( 6 );
     this->CacheFreeBufferSizeSpinBox->SetBalloonHelpString("Set the amount of space in the cache directory that should remain free (Mb).");
     this->CacheFreeBufferSizeSpinBox->GetWidget()->SetValue( app->GetRemoteCacheFreeBufferSize() );

     this->CloseButton = vtkKWPushButton::New();
     this->CloseButton->SetParent ( this->ButtonFrame );
     this->CloseButton->Create();
     this->CloseButton->SetText ( "Close");
     this->CloseButton->SetWidth ( 10 );

     this->Script ( "pack %s -side left -anchor n -padx 4 -pady 4",
                    this->CacheDirectoryButton->GetWidgetName());
    
     this->Script ( "pack %s %s -side left -anchor n -padx 2 -pady 4",
                    this->CacheLimitSpinBox->GetWidgetName(),
                    this->CacheSizeLabel->GetWidgetName() );
    
     this->Script ( "pack %s %s -side left -anchor n -padx 2 -pady 4",
                    this->CacheFreeBufferSizeSpinBox->GetWidgetName(),
                    this->CacheFreeLabel->GetWidgetName());
    
     this->Script ( "pack %s -side right -anchor n -padx 4 -pady 4", this->CloseButton->GetWidgetName() );
     this->Built = true;
    }
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

  //--- refresh the GUI
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );
  if ( app != NULL )
    {
    app->ProcessIdleTasks();
    }

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

