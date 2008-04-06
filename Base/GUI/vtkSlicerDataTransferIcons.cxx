
#include "vtkObjectFactory.h"
#include "vtkSlicerDataTransferIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerDataTransferIcons );
vtkCxxRevisionMacro ( vtkSlicerDataTransferIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerDataTransferIcons::vtkSlicerDataTransferIcons ( )
{
    //--- Transfer type
    this->TransferTypeUnspecifiedIcon = vtkKWIcon::New ( );
    this->TransferTypeLoadIcon = vtkKWIcon::New ( );
    this->TransferTypeSaveIcon = vtkKWIcon::New ( );
    this->TransferTypeRemoteLoadIcon = vtkKWIcon::New ( );
    this->TransferTypeRemoteSaveIcon = vtkKWIcon::New ( );

    //--- Transfer Status going
    this->TransferStatusGoingIcon = vtkKWIcon::New ( );
    this->TransferStatusGoing0Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing1Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing2Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing3Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing4Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing5Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing6Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing7Icon = vtkKWIcon::New ( );

    //--- Other transfer status
    this->TransferStatusReadyIcon = vtkKWIcon::New ( );
    this->TransferStatusCancelledIcon = vtkKWIcon::New ( );
    this->TransferStatusDoneIcon = vtkKWIcon::New ( );
    this->TransferStatusTimedOutIcon = vtkKWIcon::New ( );
    this->TransferStatusIdleIcon = vtkKWIcon::New();
    this->TransferStatusWaitIcon = vtkKWIcon::New();

    //--- Transfer conditions
    this->TransferStatusErrorIcon = vtkKWIcon::New();
    this->TransferStatusCancelRequestedIcon = vtkKWIcon::New();

    //--- Button icons
    this->ClearDisplayIcon = vtkKWIcon::New( );
    this->TransferCancelIcon = vtkKWIcon::New ( );
    this->TransferInformationIcon = vtkKWIcon::New ( );
    this->DeleteFromCacheIcon = vtkKWIcon::New();
    this->TransferCancelDisabledIcon = vtkKWIcon::New();
    this->DeleteFromCacheDisabledIcon = vtkKWIcon::New();
    this->RefreshSettingsIcon = vtkKWIcon::New();
    
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerDataTransferIcons::~vtkSlicerDataTransferIcons ( )
{

  if (this->ClearDisplayIcon )
    {
    this->ClearDisplayIcon->Delete();
    this->ClearDisplayIcon = NULL;
    }
  if ( this->TransferTypeUnspecifiedIcon )
    {
    this->TransferTypeUnspecifiedIcon->Delete();
    this->TransferTypeUnspecifiedIcon = NULL;
    }
  if ( this->TransferTypeLoadIcon )
    {
    this->TransferTypeLoadIcon->Delete();
    this->TransferTypeLoadIcon = NULL;
    }
  if ( this->TransferTypeSaveIcon )
    {
    this->TransferTypeSaveIcon->Delete();
    this->TransferTypeSaveIcon = NULL;
    }
  if ( this->TransferTypeRemoteLoadIcon )
    {
    this->TransferTypeRemoteLoadIcon->Delete();
    this->TransferTypeRemoteLoadIcon = NULL;
    }
  if ( this->TransferTypeRemoteSaveIcon )
    {
    this->TransferTypeRemoteSaveIcon->Delete();
    this->TransferTypeRemoteSaveIcon = NULL;
    }
  if ( this->TransferStatusWaitIcon )
    {
    this->TransferStatusWaitIcon->Delete();
    this->TransferStatusWaitIcon = NULL;
    }
  if ( this->TransferStatusReadyIcon )
    {
    this->TransferStatusReadyIcon->Delete();
    this->TransferStatusReadyIcon = NULL;
    }
  if ( this->TransferStatusGoingIcon )
    {
    this->TransferStatusGoingIcon->Delete();
    this->TransferStatusGoingIcon = NULL;
    }
  if ( this->TransferStatusGoing0Icon )
    {
    this->TransferStatusGoing0Icon->Delete();
    this->TransferStatusGoing0Icon = NULL;
    }
  if ( this->TransferStatusGoing1Icon )
    {
    this->TransferStatusGoing1Icon->Delete();
    this->TransferStatusGoing1Icon = NULL;
    }
  if ( this->TransferStatusGoing2Icon )
    {
    this->TransferStatusGoing2Icon->Delete();
    this->TransferStatusGoing2Icon = NULL;
    }
  if ( this->TransferStatusGoing3Icon )
    {
    this->TransferStatusGoing3Icon->Delete();
    this->TransferStatusGoing3Icon = NULL;
    }
  if ( this->TransferStatusGoing4Icon )
    {
    this->TransferStatusGoing4Icon->Delete();
    this->TransferStatusGoing4Icon = NULL;
    }
  if ( this->TransferStatusGoing5Icon )
    {
    this->TransferStatusGoing5Icon->Delete();
    this->TransferStatusGoing5Icon = NULL;
    }
  if ( this->TransferStatusGoing6Icon )
    {
    this->TransferStatusGoing6Icon->Delete();
    this->TransferStatusGoing6Icon = NULL;
    }
  if ( this->TransferStatusGoing7Icon )
    {
    this->TransferStatusGoing7Icon->Delete();
    this->TransferStatusGoing7Icon = NULL;
    }
  if ( this->TransferStatusCancelledIcon )
    {
    this->TransferStatusCancelledIcon->Delete();
    this->TransferStatusCancelledIcon = NULL;
    }
  if ( this->TransferStatusDoneIcon )
    {
    this->TransferStatusDoneIcon->Delete();
    this->TransferStatusDoneIcon = NULL;
    }
  if ( this->TransferStatusTimedOutIcon )
    {
    this->TransferStatusTimedOutIcon->Delete();
    this->TransferStatusTimedOutIcon = NULL;
    }

  if ( this->TransferCancelDisabledIcon )
    {
    this->TransferCancelDisabledIcon->Delete();
    this->TransferCancelDisabledIcon = NULL;
    }
  if ( this->DeleteFromCacheDisabledIcon )
    {
    this->DeleteFromCacheDisabledIcon->Delete();
    this->DeleteFromCacheDisabledIcon = NULL;
    }
  if ( this->RefreshSettingsIcon )
    {
    this->RefreshSettingsIcon->Delete();
    this->RefreshSettingsIcon = NULL;
    }
  if ( this->TransferCancelIcon )
    {
    this->TransferCancelIcon->Delete();
    this->TransferCancelIcon = NULL;
    }
  if ( this->TransferInformationIcon )
    {
    this->TransferInformationIcon->Delete();
    this->TransferInformationIcon = NULL;
    }
  if ( this->TransferStatusIdleIcon )
    {
    this->TransferStatusIdleIcon->Delete();
    this->TransferStatusIdleIcon = NULL;
    }
  if ( this->TransferStatusErrorIcon )
    {
    this->TransferStatusErrorIcon->Delete();
    this->TransferStatusErrorIcon = NULL;
    }
  if ( this->TransferStatusCancelRequestedIcon )
    {
    this->TransferStatusCancelRequestedIcon->Delete();
    this->TransferStatusCancelRequestedIcon = NULL;    
    }
  if ( this->DeleteFromCacheIcon )
    {
    this->DeleteFromCacheIcon->Delete();
    this->DeleteFromCacheIcon = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerDataTransferIcons::AssignImageDataToIcons ( )
{

  this->TransferTypeUnspecifiedIcon->SetImage( image_TransferTypeUnspecified,
                                image_TransferTypeUnspecified_width,
                                image_TransferTypeUnspecified_height,
                                image_TransferTypeUnspecified_pixel_size,
                                image_TransferTypeUnspecified_length, 0);
  this->TransferTypeLoadIcon->SetImage( image_TransferTypeLoad,
                                image_TransferTypeLoad_width,
                                image_TransferTypeLoad_height,
                                image_TransferTypeLoad_pixel_size,
                                image_TransferTypeLoad_length, 0);
  this->TransferTypeSaveIcon->SetImage( image_TransferTypeSave,
                                image_TransferTypeSave_width,
                                image_TransferTypeSave_height,
                                image_TransferTypeSave_pixel_size,
                                image_TransferTypeSave_length, 0);
  this->TransferTypeRemoteLoadIcon->SetImage( image_TransferTypeRemoteLoad,
                                image_TransferTypeRemoteLoad_width,
                                image_TransferTypeRemoteLoad_height,
                                image_TransferTypeRemoteLoad_pixel_size,
                                image_TransferTypeRemoteLoad_length, 0);
  this->TransferTypeRemoteSaveIcon->SetImage( image_TransferTypeRemoteSave,
                                image_TransferTypeRemoteSave_width,
                                image_TransferTypeRemoteSave_height,
                                image_TransferTypeRemoteSave_pixel_size,
                                image_TransferTypeRemoteSave_length, 0);

  this->TransferStatusReadyIcon->SetImage( image_SlicerWait,
                                image_SlicerWait_width,
                                image_SlicerWait_height,
                                image_SlicerWait_pixel_size,
                                image_SlicerWait_length, 0);
  this->TransferStatusGoingIcon->SetImage( image_SlicerWait,
                                image_SlicerWait_width,
                                image_SlicerWait_height,
                                image_SlicerWait_pixel_size,
                                image_SlicerWait_length, 0);
  this->TransferStatusGoing0Icon->SetImage( image_SlicerGoing0,
                                image_SlicerGoing0_width,
                                image_SlicerGoing0_height,
                                image_SlicerGoing0_pixel_size,
                                image_SlicerGoing0_length, 0);
  this->TransferStatusGoing1Icon->SetImage( image_SlicerGoing1,
                                image_SlicerGoing1_width,
                                image_SlicerGoing1_height,
                                image_SlicerGoing1_pixel_size,
                                image_SlicerGoing1_length, 0);
  this->TransferStatusGoing2Icon->SetImage( image_SlicerGoing2,
                                image_SlicerGoing2_width,
                                image_SlicerGoing2_height,
                                image_SlicerGoing2_pixel_size,
                                image_SlicerGoing2_length, 0);
  this->TransferStatusGoing3Icon->SetImage( image_SlicerGoing3,
                                image_SlicerGoing3_width,
                                image_SlicerGoing3_height,
                                image_SlicerGoing3_pixel_size,
                                image_SlicerGoing3_length, 0);
  this->TransferStatusGoing4Icon->SetImage( image_SlicerGoing4,
                                image_SlicerGoing4_width,
                                image_SlicerGoing4_height,
                                image_SlicerGoing4_pixel_size,
                                image_SlicerGoing4_length, 0);
  this->TransferStatusGoing5Icon->SetImage( image_SlicerGoing5,
                                image_SlicerGoing5_width,
                                image_SlicerGoing5_height,
                                image_SlicerGoing5_pixel_size,
                                image_SlicerGoing5_length, 0);
  this->TransferStatusGoing6Icon->SetImage( image_SlicerGoing6,
                                image_SlicerGoing6_width,
                                image_SlicerGoing6_height,
                                image_SlicerGoing6_pixel_size,
                                image_SlicerGoing6_length, 0);
  this->TransferStatusGoing7Icon->SetImage( image_SlicerGoing7,
                                image_SlicerGoing7_width,
                                image_SlicerGoing7_height,
                                image_SlicerGoing7_pixel_size,
                                image_SlicerGoing7_length, 0);
  this->TransferStatusCancelledIcon->SetImage( image_SlicerCancelled,
                                image_SlicerCancelled_width,
                                image_SlicerCancelled_height,
                                image_SlicerCancelled_pixel_size,
                                image_SlicerCancelled_length, 0);
  this->TransferStatusDoneIcon->SetImage( image_SlicerDone,
                                image_SlicerDone_width,
                                image_SlicerDone_height,
                                image_SlicerDone_pixel_size,
                                image_SlicerDone_length, 0);
  this->TransferStatusTimedOutIcon->SetImage( image_SlicerTimedOut,
                                image_SlicerTimedOut_width,
                                image_SlicerTimedOut_height,
                                image_SlicerTimedOut_pixel_size,
                                image_SlicerTimedOut_length, 0);
  this->TransferStatusWaitIcon->SetImage (image_SlicerWait,
                                          image_SlicerWait_width,
                                          image_SlicerWait_height,
                                          image_SlicerWait_pixel_size,
                                          image_SlicerWait_length, 0);                                          
  this->TransferCancelDisabledIcon->SetImage ( image_SlicerCancelDisabled,
                                               image_SlicerCancelDisabled_width,
                                               image_SlicerCancelDisabled_height,
                                               image_SlicerCancelDisabled_pixel_size,
                                               image_SlicerCancelDisabled_length, 0);
  this->DeleteFromCacheDisabledIcon->SetImage ( image_SlicerDeleteDisabled,
                                                image_SlicerDeleteDisabled_width,
                                                image_SlicerDeleteDisabled_height,
                                                image_SlicerDeleteDisabled_pixel_size,
                                                image_SlicerDeleteDisabled_length, 0);                                                
  this->RefreshSettingsIcon->SetImage ( image_SlicerRefresh,
                                        image_SlicerRefresh_width,
                                        image_SlicerRefresh_height,
                                        image_SlicerRefresh_pixel_size,
                                        image_SlicerRefresh_length, 0 );                                        
  this->ClearDisplayIcon->SetImage ( image_SlicerCleanUp,
                                     image_SlicerCleanUp_width,
                                     image_SlicerCleanUp_height,
                                     image_SlicerCleanUp_pixel_size,
                                     image_SlicerCleanUp_length, 0);                                     
  this->TransferCancelIcon->SetImage( image_SlicerCancel,
                                image_SlicerCancel_width,
                                image_SlicerCancel_height,
                                image_SlicerCancel_pixel_size,
                                image_SlicerCancel_length, 0);
  this->TransferInformationIcon->SetImage( image_SlicerInformation,
                                image_SlicerInformation_width,
                                image_SlicerInformation_height,
                                image_SlicerInformation_pixel_size,
                                image_SlicerInformation_length, 0);
  this->TransferStatusIdleIcon->SetImage ( image_SlicerWait,
                                           image_SlicerWait_width,
                                           image_SlicerWait_height,
                                           image_SlicerWait_pixel_size,
                                           image_SlicerWait_length, 0);
  this->TransferStatusErrorIcon->SetImage ( image_SlicerError,
                                      image_SlicerError_width,
                                      image_SlicerError_height,
                                      image_SlicerError_pixel_size,
                                      image_SlicerError_length, 0);
  this->TransferStatusCancelRequestedIcon->SetImage ( image_SlicerCancelRequested,
                                                image_SlicerCancelRequested_width,
                                                image_SlicerCancelRequested_height,
                                                image_SlicerCancelRequested_pixel_size,
                                                image_SlicerCancelRequested_length, 0);                                                
  this->DeleteFromCacheIcon->SetImage ( image_SlicerDelete,
                                        image_SlicerDelete_width,
                                        image_SlicerDelete_height,
                                        image_SlicerDelete_pixel_size,
                                        image_SlicerDelete_length, 0);
}



//---------------------------------------------------------------------------
void vtkSlicerDataTransferIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerDataTransferIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "TransferTypeUnspecifiedIcon: " << this->GetTransferTypeUnspecifiedIcon ( ) << "\n";
    os << indent << "TransferTypeLoadIcon: " << this->GetTransferTypeLoadIcon ( ) << "\n";
    os << indent << "TransferTypeSaveIcon: " << this->GetTransferTypeSaveIcon ( ) << "\n";
    os << indent << "TransferTypeRemoteLoadIcon: " << this->GetTransferTypeRemoteLoadIcon ( ) << "\n";
    os << indent << "TransferTypeRemoteSaveIcon: " << this->GetTransferTypeRemoteSaveIcon ( ) << "\n";

    os << indent << "TransferStatusGoingIcon: " << this->GetTransferStatusGoingIcon ( ) << "\n";
    os << indent << "TransferStatusGoing0Icon: " << this->GetTransferStatusGoing0Icon ( ) << "\n";
    os << indent << "TransferStatusGoing1Icon: " << this->GetTransferStatusGoing1Icon ( ) << "\n";
    os << indent << "TransferStatusGoing2Icon: " << this->GetTransferStatusGoing2Icon ( ) << "\n";
    os << indent << "TransferStatusGoing3Icon: " << this->GetTransferStatusGoing3Icon ( ) << "\n";
    os << indent << "TransferStatusGoing4Icon: " << this->GetTransferStatusGoing4Icon ( ) << "\n";
    os << indent << "TransferStatusGoing5Icon: " << this->GetTransferStatusGoing5Icon ( ) << "\n";
    os << indent << "TransferStatusGoing6Icon: " << this->GetTransferStatusGoing6Icon ( ) << "\n";
    os << indent << "TransferStatusGoing7Icon: " << this->GetTransferStatusGoing7Icon ( ) << "\n";

    os << indent << "TransferStatusReadyIcon: " << this->GetTransferStatusReadyIcon ( ) << "\n";
    os << indent << "TransferStatusCancelledIcon: " << this->GetTransferStatusCancelledIcon ( ) << "\n";
    os << indent << "TransferStatusDoneIcon: " << this->GetTransferStatusDoneIcon ( ) << "\n";
    os << indent << "TransferStatusTimedOutIcon: " << this->GetTransferStatusTimedOutIcon ( ) << "\n";
    os << indent << "TransferStatusIdleIcon: " << this->GetTransferStatusIdleIcon() << "\n";
    os << indent << "TransferStatusWaitIcon: " << this->GetTransferStatusWaitIcon() << "\n";
    os << indent << "TransferStatusErrorIcon: " << this->GetTransferStatusErrorIcon() << "\n";
    os << indent << "TransferStatusCancelRequestedIcon: " << this->GetTransferStatusCancelRequestedIcon() << "\n";

    os << indent << "RefreshSettingsIcon: " << this->GetRefreshSettingsIcon ( ) << "\n";
    os << indent << "TransferCancelIcon: " << this->GetTransferCancelIcon ( ) << "\n";
    os << indent << "ClearDisplayIcon: " << this->GetClearDisplayIcon ( ) << "\n";
    os << indent << "TransferCancelDisabledIcon: " << this->GetTransferCancelDisabledIcon() << "\n";
    os << indent << "TransferInformationIcon: " << this->GetTransferInformationIcon ( ) << "\n";
    os << indent << "DeleteFromCacheIcon: " << this->GetDeleteFromCacheIcon() << "\n";
    os << indent << "DeleteFromCacheDisabledIcon: " << this->GetDeleteFromCacheDisabledIcon() << "\n";
}
