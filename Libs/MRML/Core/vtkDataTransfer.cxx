// MRML includes
#include "vtkDataTransfer.h"
#include "vtkURIHandler.h"

// VTK includes
#include <vtkObjectFactory.h>


vtkStandardNewMacro ( vtkDataTransfer );
vtkCxxSetObjectMacro( vtkDataTransfer, Handler, vtkURIHandler );

//----------------------------------------------------------------------------
vtkDataTransfer::vtkDataTransfer()
{
  this->SourceURI = nullptr;
  this->DestinationURI = nullptr;
  this->Handler = nullptr;
  this->TransferStatus = vtkDataTransfer::Ready;
  this->TransferID = -1;
  this->TransferType = vtkDataTransfer::Unspecified;
  this->TransferNodeID = nullptr;
  this->Progress = 0;
  this->CancelRequested = 0;
  this->TransferCached = 0;
  this->SizeOnDisk = 0;
}


//----------------------------------------------------------------------------
vtkDataTransfer::~vtkDataTransfer()
{

  this->SourceURI = nullptr;
  this->DestinationURI = nullptr;
  if ( this->Handler != nullptr )
    {
    this->SetHandler (nullptr);
    }
  this->TransferStatus = vtkDataTransfer::Ready;
  this->TransferID = -1;
  this->TransferType = vtkDataTransfer::Unspecified;
  this->TransferNodeID = nullptr;
  this->Progress = 0;
  this->CancelRequested = 0;
  this->TransferCached = 0;
  this->SizeOnDisk = 0;
}


//----------------------------------------------------------------------------
void vtkDataTransfer::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
  os << indent << "SourceURI: " <<
    ( this->SourceURI ? this->SourceURI : "(none)") << "\n";
  os << indent << "DestinationURI: " <<
    ( this->DestinationURI ? this->DestinationURI : "(none)") << "\n";
  os << indent << "Handler: " << this->GetHandler() << "\n";
  os << indent << "TransferCached: " << this->GetTransferCached() << "\n";
  os << indent << "TransferStatus: " << this->GetTransferStatus() << "\n";
  os << indent << "CancelRequested: " << this->GetCancelRequested() << "\n";
  os << indent << "TransferID: " << this->GetTransferID() << "\n";
  os << indent << "TransferType: " << this->GetTransferType() << "\n";
  os << indent << "TransferNodeID: " << this->GetTransferNodeID() << "\n";
  os << indent << "Progress: " << this->GetProgress() << "\n";
  os << indent << "SizeOnDisk: " << this->GetSizeOnDisk() << "\n";
}



