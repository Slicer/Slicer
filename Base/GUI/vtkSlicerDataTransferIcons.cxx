
#include "vtkObjectFactory.h"
#include "vtkSlicerDataTransferIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerDataTransferIcons );
vtkCxxRevisionMacro ( vtkSlicerDataTransferIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerDataTransferIcons::vtkSlicerDataTransferIcons ( )
{
    this->TransferTypeUnspecifiedIcon = vtkKWIcon::New ( );
    this->TransferTypeLoadIcon = vtkKWIcon::New ( );
    this->TransferTypeSaveIcon = vtkKWIcon::New ( );

    this->TransferStatusUnspecifiedIcon = vtkKWIcon::New ( );
    this->TransferStatusGoingIcon = vtkKWIcon::New ( );
    this->TransferStatusGoing0Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing1Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing2Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing3Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing4Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing5Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing6Icon = vtkKWIcon::New ( );
    this->TransferStatusGoing7Icon = vtkKWIcon::New ( );
    this->TransferStatusCancelledIcon = vtkKWIcon::New ( );
    this->TransferStatusDoneIcon = vtkKWIcon::New ( );
    this->TransferStatusTimedOutIcon = vtkKWIcon::New ( );

    this->TransferCancelIcon = vtkKWIcon::New ( );
    this->TransferInformationIcon = vtkKWIcon::New ( );
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerDataTransferIcons::~vtkSlicerDataTransferIcons ( )
{

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

  if ( this->TransferStatusUnspecifiedIcon )
    {
    this->TransferStatusUnspecifiedIcon->Delete();
    this->TransferStatusUnspecifiedIcon = NULL;
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

  
  this->TransferStatusUnspecifiedIcon->SetImage( image_TransferStatusUnspecified,
                                image_TransferStatusUnspecified_width,
                                image_TransferStatusUnspecified_height,
                                image_TransferStatusUnspecified_pixel_size,
                                image_TransferStatusUnspecified_length, 0);
  this->TransferStatusGoingIcon->SetImage( image_TransferStatusGoing,
                                image_TransferStatusGoing_width,
                                image_TransferStatusGoing_height,
                                image_TransferStatusGoing_pixel_size,
                                image_TransferStatusGoing_length, 0);
  this->TransferStatusGoing0Icon->SetImage( image_TransferStatusGoing0,
                                image_TransferStatusGoing0_width,
                                image_TransferStatusGoing0_height,
                                image_TransferStatusGoing0_pixel_size,
                                image_TransferStatusGoing0_length, 0);
  this->TransferStatusGoing1Icon->SetImage( image_TransferStatusGoing1,
                                image_TransferStatusGoing1_width,
                                image_TransferStatusGoing1_height,
                                image_TransferStatusGoing1_pixel_size,
                                image_TransferStatusGoing1_length, 0);
  this->TransferStatusGoing2Icon->SetImage( image_TransferStatusGoing2,
                                image_TransferStatusGoing2_width,
                                image_TransferStatusGoing2_height,
                                image_TransferStatusGoing2_pixel_size,
                                image_TransferStatusGoing2_length, 0);
  this->TransferStatusGoing3Icon->SetImage( image_TransferStatusGoing3,
                                image_TransferStatusGoing3_width,
                                image_TransferStatusGoing3_height,
                                image_TransferStatusGoing3_pixel_size,
                                image_TransferStatusGoing3_length, 0);
  this->TransferStatusGoing4Icon->SetImage( image_TransferStatusGoing4,
                                image_TransferStatusGoing4_width,
                                image_TransferStatusGoing4_height,
                                image_TransferStatusGoing4_pixel_size,
                                image_TransferStatusGoing4_length, 0);
  this->TransferStatusGoing5Icon->SetImage( image_TransferStatusGoing5,
                                image_TransferStatusGoing5_width,
                                image_TransferStatusGoing5_height,
                                image_TransferStatusGoing5_pixel_size,
                                image_TransferStatusGoing5_length, 0);
  this->TransferStatusGoing6Icon->SetImage( image_TransferStatusGoing6,
                                image_TransferStatusGoing6_width,
                                image_TransferStatusGoing6_height,
                                image_TransferStatusGoing6_pixel_size,
                                image_TransferStatusGoing6_length, 0);
  this->TransferStatusGoing7Icon->SetImage( image_TransferStatusGoing7,
                                image_TransferStatusGoing7_width,
                                image_TransferStatusGoing7_height,
                                image_TransferStatusGoing7_pixel_size,
                                image_TransferStatusGoing7_length, 0);
  this->TransferStatusCancelledIcon->SetImage( image_TransferStatusCancelled,
                                image_TransferStatusCancelled_width,
                                image_TransferStatusCancelled_height,
                                image_TransferStatusCancelled_pixel_size,
                                image_TransferStatusCancelled_length, 0);
  this->TransferStatusDoneIcon->SetImage( image_TransferStatusDone,
                                image_TransferStatusDone_width,
                                image_TransferStatusDone_height,
                                image_TransferStatusDone_pixel_size,
                                image_TransferStatusDone_length, 0);
  this->TransferStatusTimedOutIcon->SetImage( image_TransferStatusTimedOut,
                                image_TransferStatusTimedOut_width,
                                image_TransferStatusTimedOut_height,
                                image_TransferStatusTimedOut_pixel_size,
                                image_TransferStatusTimedOut_length, 0);
  
  this->TransferCancelIcon->SetImage( image_TransferCancel,
                                image_TransferCancel_width,
                                image_TransferCancel_height,
                                image_TransferCancel_pixel_size,
                                image_TransferCancel_length, 0);
  this->TransferInformationIcon->SetImage( image_TransferInformation,
                                image_TransferInformation_width,
                                image_TransferInformation_height,
                                image_TransferInformation_pixel_size,
                                image_TransferInformation_length, 0);

}



//---------------------------------------------------------------------------
void vtkSlicerDataTransferIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerDataTransferIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "TransferTypeUnspecifiedIcon: " << this->GetTransferTypeUnspecifiedIcon ( ) << "\n";
    os << indent << "TransferTypeLoadIcon: " << this->GetTransferTypeLoadIcon ( ) << "\n";
    os << indent << "TransferTypeSaveIcon: " << this->GetTransferTypeSaveIcon ( ) << "\n";

    os << indent << "TransferStatusUnspecifiedIcon: " << this->GetTransferStatusUnspecifiedIcon ( ) << "\n";
    os << indent << "TransferStatusGoingIcon: " << this->GetTransferStatusGoingIcon ( ) << "\n";
    os << indent << "TransferStatusGoing0Icon: " << this->GetTransferStatusGoing0Icon ( ) << "\n";
    os << indent << "TransferStatusGoing1Icon: " << this->GetTransferStatusGoing1Icon ( ) << "\n";
    os << indent << "TransferStatusGoing2Icon: " << this->GetTransferStatusGoing2Icon ( ) << "\n";
    os << indent << "TransferStatusGoing3Icon: " << this->GetTransferStatusGoing3Icon ( ) << "\n";
    os << indent << "TransferStatusGoing4Icon: " << this->GetTransferStatusGoing4Icon ( ) << "\n";
    os << indent << "TransferStatusGoing5Icon: " << this->GetTransferStatusGoing5Icon ( ) << "\n";
    os << indent << "TransferStatusGoing6Icon: " << this->GetTransferStatusGoing6Icon ( ) << "\n";
    os << indent << "TransferStatusGoing7Icon: " << this->GetTransferStatusGoing7Icon ( ) << "\n";
    os << indent << "TransferStatusCancelledIcon: " << this->GetTransferStatusCancelledIcon ( ) << "\n";
    os << indent << "TransferStatusDoneIcon: " << this->GetTransferStatusDoneIcon ( ) << "\n";
    os << indent << "TransferStatusTimedOutIcon: " << this->GetTransferStatusTimedOutIcon ( ) << "\n";

    os << indent << "TransferCancelIcon: " << this->GetTransferCancelIcon ( ) << "\n";
    os << indent << "TransferInformationIcon: " << this->GetTransferInformationIcon ( ) << "\n";
}
